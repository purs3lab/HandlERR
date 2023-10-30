//=--ThrowVisitors.cpp-------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This implementation of methods in ThrowVisitors - various visitors
// dealing with throw expression heuristics.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/ThrowVisitors.h"
#include "clang/DetectERR/Utils.h"
// #include "clang/DetectERR/VisitorUtils.h"

/// H09 - throwing an exception is control dependent on a check
bool ThrowVisitor::VisitCXXThrowExpr(CXXThrowExpr *TE) {
  CFGBlock *CurBB;
  if (StMap.find(TE) != StMap.end()) {
    CurBB = StMap[TE];
    std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
    collectChecks(Checks, *CurBB, &CDG);
    removeChecksUsingParams(Checks, *FnDecl);
    if (!Checks.empty()) {
      // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
      // addErrorGuards(Checks, CE);
      auto [check, level] = getImmediateControlDependentCheck(
          Checks, TE, &CDG, Context->getSourceManager());
      if (isMacroExpanded(check)) {
        llvm::errs() << ">>>> Skipping macro expansion (check)\n";
        llvm::errs() << ">>>> "
                     << check->getBeginLoc().printToString(
                            Context->getSourceManager())
                     << "\n";
        return true;
      }
      uint32_t ErrBBL = getFirstLineNo(CurBB, Context);
      // addErrorGuard(check, TE, level);
      addErrorGuard(check, TE, level, ErrBBL);
    }
  }
  return true;
}
/// H09 - throwing an exception is control dependent on a check
