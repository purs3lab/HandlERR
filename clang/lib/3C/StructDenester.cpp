#include "clang/3C/StructDenester.h"
#include "clang/3C/PersistentSourceLoc.h"
#include "clang/3C/RewriteUtils.h"
#include "clang/3C/Utils.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace llvm;

class DenestStructsVisitor : public RecursiveASTVisitor<DenestStructsVisitor> {
private:
  ASTContext &C;
  Rewriter &R;
  std::set<Decl *> AlreadyMovingDecls;
  bool isAncestorAlreadyMoving(Decl *D) {
    for (;;) {
      // It seems that a DeclContext is always a Decl? At least
      // DeclContext::getLexicalParent seems to assume so, and it's what we'd
      // call here anyway. REVIEW?
      D = cast_or_null<Decl>(D->getLexicalDeclContext());
      if (D == nullptr)
        return false;
      if (AlreadyMovingDecls.find(D) != AlreadyMovingDecls.end())
        return true;
    }
  }
  // A separate method because every return having to return `true` is
  // distracting.
  void processRecordDecl(RecordDecl *RD) {
    PersistentSourceLoc PSL = PersistentSourceLoc::mkPSL(RD, C);
    if (!canWrite(PSL.getFileName()))
      return;
    if (!RD->isCompleteDefinition())
      // Don't do anything with forward declarations.
      return;
    // Note: It's important that this visitor traverses in preorder; otherwise
    // we would have to check for descendants already moving, and it would
    // generally be a mess.
    if (isAncestorAlreadyMoving(RD))
      // We don't handle this yet either.
      return;
    // Now determine whether we need to de-nest it. Etc.
    // We probably need to handle more contexts and avoid messing up three-level
    // nesting, but this is a starting point for testing.
    if (!RD->isStruct())
      // We may want to handle unions, etc. too, but the code has to be
      // generalized to use the correct keyword.
      return;
    DeclContext *DC = RD->getLexicalDeclContext();
    RecordDecl *RD2 = dyn_cast<RecordDecl>(DC);
    if (RD2 == nullptr)
      return;
    std::string NewName = std::string(RD->getName());
    std::string NewDefinition;
    if (NewName.empty()) {
      // We have to make up a name. Can we name the record after a variable?
      NamedDecl *NextDecl = cast_or_null<NamedDecl>(RD->getNextDeclInContext());
      if (NextDecl != nullptr &&
          // TODO: Wanted: Analogue of isBeforeInTranslationUnit for `<=`
          !C.getSourceManager().isBeforeInTranslationUnit(RD->getBeginLoc(), NextDecl->getBeginLoc())) {
        if (NextDecl->isImplicit())
          // This might be an implicit instance of RD flattened into a parent
          // structure. We mustn't try to de-nest RD at all.
          // TODO: More specific check? `RD->isAnonymousStructOrUnion()`?
          return;
        if (!NextDecl->getName().empty()) {
          // TODO: Collision avoidance
          NewName = std::string(NextDecl->getName()) + "_struct";
        }
      }
      if (NewName.empty()) {
        // An unnamed record not used to declare anything? Why is it in the
        // source code at all? For now, punt.
        llvm::errs() << "Unnamed record not used to declare anything?\n";
        return;
      }
      // Try to insert the name we chose into the definition.
      std::string OldDefinition = getSourceText(RD->getSourceRange(), C);
      if (StringRef(OldDefinition).startswith("struct ")) {
        NewDefinition = "struct " + NewName + " " + OldDefinition.substr(7);
      } else {
        llvm::errs() << "Struct definition does not begin with \"struct \"; punt\n";
        return;
      }
#if 0
      // Code inspired by getNextCommaOrSemicolon.
      // This is giving us the `{` instead of the `struct`. Why? What API should we use?
      Optional<Token> Tok = Lexer::findNextToken(RD->getBeginLoc(), C.getSourceManager(), C.getLangOpts());
      if (Tok.hasValue() && Tok->is(tok::kw_struct)) {
        SourceLocation EndOfStructKw = Tok->getEndLoc();
        NewDefinition = "struct " + NewName + getSourceText(SourceRange(EndOfStructKw, RD->getEndLoc()), C);
      } else {
        if (Tok.hasValue()) {
          llvm::errs() << "Token kind is " << Tok->getName() << "; expected kw_struct\n";
        } else {
          llvm::errs() << "Lexer::findNextToken didn't find a token at all.\n";
        }
        return;
      }
#endif
    } else {
      assert("struct " + NewName == tyToStr(RD->getTypeForDecl()));
      NewDefinition = getSourceText(RD->getSourceRange(), C);
    }
    AlreadyMovingDecls.insert(RD);
    // TODO: Error checking.
    R.InsertText(RD2->getBeginLoc(), NewDefinition + ";\n");
    rewriteSourceRange(R, RD->getSourceRange(), "struct " + NewName);
  }
public:
  DenestStructsVisitor(ASTContext &C, Rewriter &R) : C(C), R(R) {}
  bool VisitRecordDecl(RecordDecl *RD) {
    processRecordDecl(RD);
    return true;
  }
};

void denestStructs(ASTContext &C) {
  Rewriter R(C.getSourceManager(), C.getLangOpts());
  DenestStructsVisitor DSV(C, R);
  DSV.TraverseAST(C);

  emitFiles(R, C);
}
