//=--MultiDecls.cpp-----------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "clang/3C/MultiDecls.h"
#include "clang/3C/Utils.h"

MultiDeclMemberDecl *getAsMultiDeclMember(Decl *D) {
  // A FunctionDecl can be part of a multi-decl in C, but 3C currently doesn't
  // handle this
  // (https://github.com/correctcomputation/checkedc-clang/issues/659 part (a)).
  if (VarDecl *VD = dyn_cast<VarDecl>(D))
    return VD;
  if (FieldDecl *FD = dyn_cast<FieldDecl>(D))
    return FD;
  if (TypedefDecl *TD = dyn_cast<TypedefDecl>(D))
    return TD;
  return nullptr;
}

QualType getTypeOfMultiDeclMember(MultiDeclMemberDecl *MMD) {
  if (DeclaratorDecl *DD = dyn_cast<DeclaratorDecl>(MMD))
    return DD->getType();
  if (TypedefDecl *TD = dyn_cast<TypedefDecl>(MMD))
    return TD->getUnderlyingType();
  llvm_unreachable("Unexpected declaration type");
}

TypeSourceInfo *getTypeSourceInfoOfMultiDeclMember(MultiDeclMemberDecl *MMD) {
  if (DeclaratorDecl *DD = dyn_cast<DeclaratorDecl>(MMD))
    return DD->getTypeSourceInfo();
  if (TypedefDecl *TD = dyn_cast<TypedefDecl>(MMD))
    return TD->getTypeSourceInfo();
  llvm_unreachable("Unexpected declaration type");
}

void MultiDeclsInfo::findUsedTagNames(DeclContext *DC) {
  // We do our own traversal via `decls` rather than using RecursiveASTVisitor.
  // This has the advantage of visiting TagDecls in function parameters, which
  // RecursiveASTVisitor doesn't do by default, though such TagDecls are
  // potentially problematic for 3C anyway.
  for (Decl *D : DC->decls()) {
    if (TagDecl *TD = dyn_cast<TagDecl>(D)) {
      if (!TD->getName().empty()) {
        // Multiple TagDecls may have the same name if the same physical
        // declaration is seen in multiple translation units or different
        // TagDecls with the same name are used in different scopes. That is not
        // a problem for us here: we're simply making a list of all the names we
        // don't want to collide with.
        UsedTagNames.insert(std::string(TD->getName()));
      }
    }
    if (DeclContext *NestedDC = dyn_cast<DeclContext>(D)) {
      findUsedTagNames(NestedDC);
    }
  }
}

void MultiDeclsInfo::findUsedTagNames(ASTContext &Context) {
  findUsedTagNames(Context.getTranslationUnitDecl());
}

static const Type *unelaborateType(const Type *Ty) {
  if (const ElaboratedType *ETy = dyn_cast<ElaboratedType>(Ty)) {
    QualType QT = ETy->getNamedType();
    Ty = QT.getTypePtr();
    // Can an ElaboratedType add qualifiers to its underlying type in C? I don't
    // think so, but if it does, make sure we don't silently lose them.
    assert(QualType(Ty, 0) == QT);
  }
  return Ty;
}

void MultiDeclsInfo::findMultiDecls(DeclContext *DC, ASTContext &Context) {
  // This will automatically create a new, empty map for the TU if needed.
  TUMultiDeclsInfo &TUInfo = TUInfos[&Context];
  TagDecl *LastTagDef = nullptr;

  // Variables related to the current multi-decl.
  MultiDeclInfo *CurrentMultiDecl = nullptr;
  SourceLocation CurrentBeginLoc;
  PersistentSourceLoc TagDefPSL;
  bool TagDefNeedsName;

  for (Decl *D : DC->decls()) {
    TagDecl *TagD = dyn_cast<TagDecl>(D);
    if (TagD && TagD->isCompleteDefinition() &&
        // With -fms-extensions (default on Windows), Clang injects an implicit
        // `struct _GUID` with an invalid location.
        TagD->getBeginLoc().isValid()) {
      LastTagDef = TagD;
    }
    if (MultiDeclMemberDecl *MMD = getAsMultiDeclMember(D)) {
      if (CurrentMultiDecl == nullptr || MMD->getBeginLoc() != CurrentBeginLoc) {
        // We are starting a new multi-decl.
        CurrentBeginLoc = MMD->getBeginLoc();
        CurrentMultiDecl = &TUInfo[CurrentBeginLoc];
        TagDefNeedsName = false;
        
        // Check for an inline tag definition.
        // Wanted: CurrentBeginLoc <= LastTagDef->getBeginLoc().
        // Implemented as: !(LastTagDef->getBeginLoc() < CurrentBeginLoc).
        if (LastTagDef != nullptr &&
            !Context.getSourceManager().isBeforeInTranslationUnit(
                LastTagDef->getBeginLoc(), CurrentBeginLoc)) {
          CurrentMultiDecl->TagDefToSplit = LastTagDef;

          // Do we need to automatically name the TagDefToSplit?
          if (LastTagDef->getName().empty()) {
            // REVIEW: Assert that we don't get here with isAnonymousStructOrUnion true?
            TagDefPSL = PersistentSourceLoc::mkPSL(LastTagDef, Context);
            if (AssignedTagTypeStrs.find(TagDefPSL) == AssignedTagTypeStrs.end() &&
                canWrite(TagDefPSL.getFileName()))
              TagDefNeedsName = true;
          }
        }
      }

      CurrentMultiDecl->Members.push_back(MMD);

      std::string MemberName;
      if (TagDefNeedsName &&
          // REVIEW: Can the name be empty? Should we assert that it isn't?
          !(MemberName = std::string(MMD->getName())).empty()) {
        // Special case: If the first member of the multi-decl is a typedef
        // whose type is exactly the TagDecl type (`typedef struct { ... } T`),
        // then we refer to the TagDecl via that typedef. (The typedef must be the
        // first member so that it is defined in time for other members to refer
        // to it.)
        TypedefDecl *TyD;
        QualType Underlying;
        if (CurrentMultiDecl->Members.size() == 1 &&
            (TyD = dyn_cast<TypedefDecl>(MMD)) != nullptr &&
            // FIXME: This is a terrible mess. Figure out how we should be
            // handling the difference between Type and QualType.
            !(Underlying = TyD->getUnderlyingType()).hasLocalQualifiers() &&
            QualType(unelaborateType(Underlying.getTypePtr()), 0) ==
                Context.getTagDeclType(LastTagDef)) {
          AssignedTagTypeStrs.insert(std::make_pair(TagDefPSL, MemberName));
          TagDefNeedsName = false;
          // Tell the rewriter that the tag definition should not be moved out of
          // the typedef.
          CurrentMultiDecl->TagDefToSplit = nullptr;
        } else {
          // Otherwise, just generate a new tag name based on the member name.
          // Example: `struct { ... } foo;` ->
          // `struct foo_struct_1 { ... }; struct foo_struct_1 foo;`
          // If `foo_struct_1` is already taken, use `foo_struct_2`, etc.
          std::string KindName = std::string(LastTagDef->getKindName());
          std::string NewName;
          for (int Num = 1; ; Num++) {
            NewName = MemberName + "_" + KindName + "_" + std::to_string(Num);
            if (UsedTagNames.find(NewName) == UsedTagNames.end())
              break;
          }
          AssignedTagTypeStrs.insert(std::make_pair(TagDefPSL, KindName + " " + NewName));
          TagDefNeedsName = false;
          // Consider this name taken and ensure that other automatically
          // generated names do not collide with it.
          //
          // If the multi-decl doesn't end up getting rewritten, this name
          // ultimately may not be used, creating a gap in the numbering in 3C's
          // output. But this cosmetic inconsistency is a small price to pay for
          // the architectural convenience of being able to store the assigned
          // names in the PointerVariableConstraints when they are constructed
          // rather than trying to assign and store the names after we know
          // which multi-decls will be rewritten.
          UsedTagNames.insert(NewName);
        }
      }
    }
    if (DeclContext *NestedDC = dyn_cast<DeclContext>(D)) {
      findMultiDecls(NestedDC, Context);
    }
  }
}

void MultiDeclsInfo::findMultiDecls(ASTContext &Context) {
  findMultiDecls(Context.getTranslationUnitDecl(), Context);
}

llvm::Optional<std::string> MultiDeclsInfo::getTypeStrOverride(const Type *Ty, ASTContext &C) {
  Ty = unelaborateType(Ty);
  if (const TagType *TTy = dyn_cast<TagType>(Ty)) {
    TagDecl *TD = TTy->getDecl();
    if (TD->getName().empty()) {
      PersistentSourceLoc PSL = PersistentSourceLoc::mkPSL(TD, C);
      auto Iter = AssignedTagTypeStrs.find(PSL);
      if (Iter != AssignedTagTypeStrs.end())
        return Iter->second;
      // REVIEW: Assert that we don't get here in writable code? We should have
      // named all unnamed TagDecls in writable code.
    }
  }
  return llvm::None;
}

MultiDeclInfo &MultiDeclsInfo::findContainingMultiDecl(MultiDeclMemberDecl *MMD, ASTContext &C) {
  // Look for a MultiDeclInfo for the beginning location of D, then check that
  // the MultiDeclInfo actually contains D. If we had no MultiDeclInfo at all
  // for that location, this may create an empty MultiDeclInfo (before failing
  // the assertion below if assertions are enabled), but that shouldn't matter
  // because nothing iterates over the whole map.
  MultiDeclInfo &MDI = TUInfos[&C][MMD->getBeginLoc()];
  // Hope we don't have multi-decls with so many members that this becomes a
  // performance problem.
  assert(std::find(MDI.Members.begin(), MDI.Members.end(), MMD) != MDI.Members.end());
  return MDI;
}
