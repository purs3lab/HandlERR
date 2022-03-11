#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

#ifndef LLVM_CLANG_DETECTERR_GOTOVISITORS_H
#define LLVM_CLANG_DETECTERR_GOTOVISITORS_H

/// H08 - goto to an error label is control dependent on a check
class GotoVisitor : public RecursiveASTVisitor<GotoVisitor> {
public:
  explicit GotoVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
                       FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), Heuristic(HeuristicID::H08) {

    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }

      // add goto statements to the statement map
      if (Stmt *St = CBlock->getTerminatorStmt()) {
        if (GotoStmt *GotoSt = dyn_cast_or_null<GotoStmt>(St)) {
          StMap[St] = CBlock;
        }
      }
    }
  }

  bool VisitGotoStmt(GotoStmt *S);

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;

  HeuristicID Heuristic;

  static std::set<std::string> ErrorLabels;
};

#endif //LLVM_CLANG_DETECTERR_GOTOVISITORS_H
