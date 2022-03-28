//=--EHFCallVisitors.h---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents all the visitors dealing with EHF Calls.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_EHFCALLVISITORS_H
#define LLVM_CLANG_DETECTERR_EHFCALLVISITORS_H

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/Utils.h"
#include "clang/DetectERR/VisitorUtils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

/// H03 - call to an exit function is control dependent on one or more
/// if checks
class EHFCallVisitor : public RecursiveASTVisitor<EHFCallVisitor> {
public:
  explicit EHFCallVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
                          FuncId &FnID, const std::set<std::string> *EHFList)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), EhfList(EHFList), Heuristic(HeuristicID::H03) {
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

  bool VisitCallExpr(CallExpr *CE);

  friend void addErrorGuards<EHFCallVisitor>(
      std::vector<std::pair<Stmt *, CFGBlock *>> &Checks, Stmt *ReturnST,
      EHFCallVisitor &This);

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  const std::set<std::string> *EhfList;
  std::map<const Stmt *, CFGBlock *> StMap;

  HeuristicID Heuristic;
};

#endif //LLVM_CLANG_DETECTERR_EHFCALLVISITORS_H
