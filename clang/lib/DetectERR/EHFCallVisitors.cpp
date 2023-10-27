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
#include "clang/AST/Stmt.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/DetectERR/Utils.h"

/// H03 - call to an exit function is control dependent on one or more
/// if checks
bool EHFCallVisitor::VisitCallExpr(CallExpr *CE) {
  CFGBlock *CurBB;
  if (isEHFCallExpr(CE, *EhfList, Context)) {
    // if the error marker is result of macro expansion, skip it
    if (isMacroExpanded(CE)) {
      llvm::errs() << ">>>> Skipping macro expansion\n";
      llvm::errs() << ">>>> "
                   << CE->getExprLoc().printToString(
                          Context->getSourceManager())
                   << "\n";
      return true;
    }

    if (StMap.find(CE) != StMap.end()) {
      CurBB = StMap[CE];
      std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
      collectChecks(Checks, *CurBB, &CDG);

      // if the immediate control dependent check is using params, skip this
      // and return
      if (!Checks.empty()) {
        auto [check, level] = getImmediateControlDependentCheck(
            Checks, CE, &CDG, Context->getSourceManager());
        if (isCheckUsingParams(check, *FnDecl)) {
          return true;
        }
      }

      removeChecksUsingParams(Checks, *FnDecl);
      if (!Checks.empty()) {
        // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
        // addErrorGuards(Checks, CE);
        auto [check, level] = getImmediateControlDependentCheck(
            Checks, CE, &CDG, Context->getSourceManager());

        if(isMacroExpanded(check)) {
          llvm::errs() << ">>>> Skipping macro expansion (check)\n";
          llvm::errs() << ">>>> "
                       << check->getBeginLoc().printToString(
                              Context->getSourceManager())
                       << "\n";
          return true;
        }

        addErrorGuard(check, CE, level);
      }
    }
  }
  return true;
}
