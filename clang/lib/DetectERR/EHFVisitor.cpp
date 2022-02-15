//=--EHFVisitor.cpp-------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This implementation of methods in EHFVisitor - various visitors
// dealing with collecting Error Handling Functions
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/EHFVisitor.h"
#include "clang/DetectERR/Utils.h"

bool EHFCategoryOneVisitor::VisitCallExpr(CallExpr *S) {
  CFGBlock *CurrBB = nullptr;
  auto Parents = Context->getParents(*S);
  if (!Parents.empty()) {
    const Stmt *St = Parents[0].get<Stmt>();
    if (St) {
      CurrBB = StMap[S];
    }
  }

  Decl *CalledDecl = S->getCalleeDecl();
  if (FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CalledDecl)) {
    std::string calledFnName = FD->getNameInfo().getAsString();
    // if the called function is a known exit function
    if (EHFList_->find(calledFnName) != EHFList_->end()) {
      // special case: exit(0) call is not a "exit" function
      if (calledFnName == "exit"){
        if(S->getNumArgs() == 0){
          // exit() => exit(0)?
          return true;
        }

        if(isZero(S->getArg(0), *Context)){
          return true;
        }
      }

      // the call stmt should be the last statement of the function
      // in other words, it should:
      // - not have any post dominator nodes
      // AND
      // - in the currBB, it should be the last statement

      // check if CurrBB has any post-dominators
      if (!hasPostDominators(*CurrBB, &CDG.getCFGPostDomTree(), *Cfg.get())) {
        if (isLastStmtInBB(*S, *CurrBB)) {
          // now we are certain that this function is an error handling function (cat-1)
          EHFList_->insert(FnDecl->getNameInfo().getAsString());
        }
      }
    }
  }

  return true;
}