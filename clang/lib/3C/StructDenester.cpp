#include "clang/3C/StructDenester.h"
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
public:
  DenestStructsVisitor(ASTContext &C, Rewriter &R) : C(C), R(R) {}
  bool VisitRecordDecl(RecordDecl *RD) {
    if (!RD->isCompleteDefinition())
      // Don't do anything with forward declarations.
      return true;
    // Now determine whether we need to de-nest it. Etc.
    // We probably need to handle more contexts and avoid messing up three-level
    // nesting, but this is a starting point for testing.
    DeclContext *DC = RD->getLexicalDeclContext();
    if (RecordDecl *RD2 = dyn_cast<RecordDecl>(DC)) {
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
