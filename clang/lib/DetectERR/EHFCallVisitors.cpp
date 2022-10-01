//=--EHFCallVisitors.cpp-------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This implementation of methods in EHFCallVisitors - various visitors
// dealing with EHF Calls related heuristics.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/EHFCallVisitors.h"
#include "clang/DetectERR/Utils.h"

/// H03 - call to an exit function is control dependent on one or more
/// if checks
bool EHFCallVisitor::VisitCallExpr(CallExpr *CE) {
  CFGBlock *CurBB;
  if (isEHFCallExpr(CE, *EhfList, Context)) {
    if (StMap.find(CE) != StMap.end()) {
      CurBB = StMap[CE];
      std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
      collectChecks(Checks, *CurBB, &CDG);
      removeChecksUsingParams(Checks, *FnDecl);
      if (!Checks.empty()) {
        // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
        // addErrorGuards(Checks, CE);
        auto [check, level] = getImmediateControlDependentCheck(
            Checks, CE, &CDG, Context->getSourceManager());
        addErrorGuard(check, CE, level);
      }
    }
  }
  return true;
}
