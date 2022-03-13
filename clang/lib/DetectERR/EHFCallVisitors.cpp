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
  //  llvm::errs() << "processing fn: " << FnDecl->getNameInfo().getAsString()
  //               << '\n';

  CFGBlock *CurBB;
  if (isEHFCallExpr(CE, *EHFList_, Context)) {
    if (StMap.find(CE) != StMap.end()) {
      CurBB = StMap[CE];
      std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
      collectChecks(Checks, *CurBB, &CDG);
      sortIntoInnerAndOuterChecks(Checks, &CDG);
      for (unsigned long I = 0; I < Checks.size(); I++) {
        if (I == 0) {
          Info.addErrorGuardingStmt(FID, Checks[I].first, Context, Heuristic,
                                    GuardLevel::Inner);
        } else {
          Info.addErrorGuardingStmt(FID, Checks[I].first, Context, Heuristic,
                                    GuardLevel::Outer);
        }
      }
    }
  }
  return true;
}
