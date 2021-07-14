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
public:
  DenestStructsVisitor(ASTContext &C, Rewriter &R) : C(C), R(R) {}
  bool VisitRecordDecl(RecordDecl *RD) {
    PersistentSourceLoc PSL = PersistentSourceLoc::mkPSL(RD, C);
    if (!canWrite(PSL.getFileName()))
      return true;
    if (!RD->isCompleteDefinition())
      // Don't do anything with forward declarations.
      return true;
    if (RD->getName() == "")
      // We can't handle unnamed RecordDecls yet.
      return true;
    // Note: It's important that this visitor traverses in preorder; otherwise
    // we would have to check for descendants already moving, and it would
    // generally be a mess.
    if (isAncestorAlreadyMoving(RD))
      // We don't handle this yet either.
      return true;
    // Now determine whether we need to de-nest it. Etc.
    // We probably need to handle more contexts and avoid messing up three-level
    // nesting, but this is a starting point for testing.
    DeclContext *DC = RD->getLexicalDeclContext();
    if (RecordDecl *RD2 = dyn_cast<RecordDecl>(DC)) {
      AlreadyMovingDecls.insert(RD);
      // TODO: Error checking.
      R.InsertText(RD2->getBeginLoc(), getSourceText(RD->getSourceRange(), C) + ";\n");
      rewriteSourceRange(R, RD->getSourceRange(), tyToStr(RD->getTypeForDecl()));
    }
    return true;
  }
};

void denestStructs(ASTContext &C) {
  Rewriter R(C.getSourceManager(), C.getLangOpts());
  DenestStructsVisitor DSV(C, R);
  DSV.TraverseAST(C);

  emitFiles(R, C);
}
