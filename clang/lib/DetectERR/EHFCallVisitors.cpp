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
  llvm::errs() << "processing fn: " << FnDecl->getNameInfo().getAsString()
               << '\n';
    CFGBlock *CurBB;
    if(isEHFCallExpr(CE, *EHFList_, Context)){
      if (StMap.find(CE) != StMap.end()) {
        CurBB = StMap[CE];
        auto &CDNodes = CDG.getControlDependencies(CurBB);
        if (!CDNodes.empty()) {
          // We should use all CDs
          // Get the last statement from the list of control dependencies.
          for (auto &CDGNode : CDNodes) {
            // Collect the possible length bounds keys.
            Stmt *TStmt = CDGNode->getTerminatorStmt();
            // check if this is an if statement.
            if (dyn_cast_or_null<IfStmt>(TStmt)) {
              Info.addErrorGuardingStmt(FID, TStmt, Context);
            }
          }
        }
    }
  }
  return true;
}