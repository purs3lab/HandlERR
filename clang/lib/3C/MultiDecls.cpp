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
  // REVIEW: Is this the best place for this check?
  if (D->getLocation().isInvalid())
    return nullptr;

  // A FunctionDecl can be part of a multi-decl in C, but 3C currently doesn't
  // handle this
  // (https://github.com/correctcomputation/checkedc-clang/issues/659 part (a)).

  // While K&R parameter declarations can be in multi-decls in the input
  // program, we don't use any of the regular multi-decl infrastructure for
  // them; the rewriter blows them away and generates a prototype.
  if (isa<ParmVarDecl>(D))
    return nullptr;

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

void ProgramMultiDeclsInfo::findMultiDecls(DeclContext *DC, ASTContext &Context) {
  // This will automatically create a new, empty map for the TU if needed.
  TUMultiDeclsInfo &TUInfo = TUInfos[&Context];
  TagDecl *LastTagDef = nullptr;

  // Variables related to the current multi-decl.
  MultiDeclInfo *CurrentMultiDecl = nullptr;
  SourceLocation CurrentBeginLoc;

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
        assert(CurrentMultiDecl->Members.empty() &&
               "Multi-decl members are not consecutive in traversal order");
        
        // Check for an inline tag definition.
        // Wanted: CurrentBeginLoc <= LastTagDef->getBeginLoc().
        // Implemented as: !(LastTagDef->getBeginLoc() < CurrentBeginLoc).
        if (LastTagDef != nullptr &&
            !Context.getSourceManager().isBeforeInTranslationUnit(
                LastTagDef->getBeginLoc(), CurrentBeginLoc)) {
          CurrentMultiDecl->TagDefToSplit = LastTagDef;
        }
      } else {
        // Adding another member to an existing multi-decl.
        assert(Context.getSourceManager().isBeforeInTranslationUnit(
                   CurrentMultiDecl->Members.back()->getEndLoc(),
                   MMD->getEndLoc()) &&
               "Multi-decl traversal order inconsistent "
               "with source location order");
      }

      CurrentMultiDecl->Members.push_back(MMD);
    }
    if (DeclContext *NestedDC = dyn_cast<DeclContext>(D)) {
      findMultiDecls(NestedDC, Context);
    }
  }
}

void ProgramMultiDeclsInfo::findMultiDecls(ASTContext &Context) {
  findMultiDecls(Context.getTranslationUnitDecl(), Context);
}

MultiDeclInfo *
ProgramMultiDeclsInfo::findContainingMultiDecl(MultiDeclMemberDecl *MMD) {
  TUMultiDeclsInfo &TUInfo = TUInfos[&MMD->getASTContext()];
  // Look for a MultiDeclInfo for the beginning location of MMD, then check that
  // the MultiDeclInfo actually contains MMD.
  auto It = TUInfo.find(MMD->getBeginLoc());
  if (It == TUInfo.end())
    return nullptr;
  MultiDeclInfo &MDI = It->second;
  // Hope we don't have multi-decls with so many members that this becomes a
  // performance problem.
  if (std::find(MDI.Members.begin(), MDI.Members.end(), MMD) != MDI.Members.end())
    return &MDI;
  return nullptr;
}
