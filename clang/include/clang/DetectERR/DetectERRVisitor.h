#ifndef LLVM_CLANG_DETECTERR_DETECTERRVISITORS_H
#define LLVM_CLANG_DETECTERR_DETECTERRVISITORS_H

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/ErrGruard.h"
#include "clang/DetectERR/VisitorUtils.h"

using namespace llvm;
using namespace clang;

class DetectERRVisitor : public RecursiveASTVisitor<DetectERRVisitor> {
public:
  explicit DetectERRVisitor(ASTContext *Context, ProjectInfo &I,
                            FunctionDecl *FD, FuncId &FnID,
                            HeuristicID Heuristic)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), Heuristic(Heuristic) {
    for (auto *CBlock : *(Cfg.get())) {
      if (CBlock->size() == 0) {
        if (Stmt *St = CBlock->getTerminatorStmt()) {
          StMap[St] = CBlock;
        }
      }
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  ProjectInfo &getProjectInfo() { return Info; }

  void addErrorGuards(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                      Stmt *ReturnST) {
    for (unsigned long I = 0; I < Checks.size(); I++) {
      if (I == 0) {
        SourceRange CurrSR = Checks[I].first->getSourceRange();
        SourceRange ReturnSTSR = ReturnST->getSourceRange();
        GuardLevel Lvl = GuardLevel::Default;
        if (CurrSR.fullyContains(ReturnSTSR)) {
          Lvl = GuardLevel::Inner;
        }
        Info.addErrorGuardingStmt(FID, Checks[I].first, ReturnST, Context,
                                  Heuristic, Lvl);
      } else {
        // conditions for a guard to be "Outer"
        // - return statement is within it
        // - inner guard is within it
        SourceRange CurrSR = Checks[I].first->getSourceRange();
        SourceRange InnerSR = Checks[0].first->getSourceRange();
        SourceRange ReturnSTSR = ReturnST->getSourceRange();
        GuardLevel Lvl = GuardLevel::Default;
        if (CurrSR.fullyContains(InnerSR) && CurrSR.fullyContains(ReturnSTSR)) {
          Lvl = GuardLevel::Outer;
        }
        Info.addErrorGuardingStmt(FID, Checks[I].first, ReturnST, Context,
                                  Heuristic, Lvl);
      }
    }
  }

  // Making pure virtual functions to help derived classes.
  bool VisitCallExpr(CallExpr *CE) { return true; }
  bool VisitReturnStmt(ReturnStmt *ST) { return true; }
  bool VisitGotoStmt(GotoStmt *GotoST) { return true; }

  // bool VisitReturnStmt(ReturnStmt *ST) {
  //   errs() << "DetectERRVisitor::VisitReturnStmt\n";
  //   return true;
  // };

  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;

  HeuristicID Heuristic;
};

#endif //LLVM_CLANG_DETECTERR_DETECTERRVISITORS_H
