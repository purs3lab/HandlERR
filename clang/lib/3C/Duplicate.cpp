#include "clang/3C/Duplicate.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/3C/ProgramInfo.h"
#include "clang/AST/Stmt.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;
using namespace llvm;


class FindVarDecl : public RecursiveASTVisitor<FindVarDecl> {
public:
  explicit FindVarDecl(ASTContext &Context, Rewriter &R,
                       ProgramInfo &I, selector P)
      : Context(Context),  R(R), Info(I), P(P) {}

  bool VisitVarDecl(VarDecl *VD) {
    if(P(VD)){
      addNewVD(VD);
      return false;
    } else {
      return true;
    }
  }

  //TODO handle mixed type multi-decl
  std::string createDuplicateString(VarDecl *VD) {
    auto CV = Info.getVariable(VD, &Context);
    auto Type =
        CV.hasValue() ?
                      CV.getValue()
                          .mkString(Info.getConstraints().getVariables(),
                                    false)
                      : VD->getType().getAsString();
    auto TargetName = VD->getNameAsString();
    auto NewName = "__" + TargetName + "_copy";
    return ";\n" + Type + " " + NewName + "= " + TargetName + "";
  }

  void addNewVD(VarDecl *VD) {
    SourceLocation End = VD->getEndLoc();
    R.InsertTextAfterToken(End, createDuplicateString(VD));
  }


private:
  ASTContext &Context;
  Rewriter &R;
  ProgramInfo &Info;
  selector P;

};

void create_duplicate(ASTContext &Context, Rewriter &R,
                      ProgramInfo &Info,
                      Decl *FD, selector P) {
  auto V = FindVarDecl(Context, R, Info, P);
  V.TraverseDecl(FD);
}
