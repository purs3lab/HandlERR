//=--ReturnVisitors.cpp-------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This implementation of methods in ReturnVisitors - various visitors
// dealing with return statement heuristics.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/ReturnVisitors.h"


bool ReturnNullVisitor::VisitReturnStmt(ReturnStmt *S) {
  CFGBlock *CurBB;
  if (isNULLExpr(S->getRetValue(), *Context)) {
    if (StMap.find(S) != StMap.end()) {
      CurBB = StMap[S];
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

bool ReturnNegativeNumVisitor::VisitReturnStmt(ReturnStmt *S) {
  CFGBlock *CurBB;
  if (isNegativeNumber(S->getRetValue(), *Context)) {
    if (StMap.find(S) != StMap.end()) {
      CurBB = StMap[S];
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