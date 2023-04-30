//=--FiFuzzVisitors.cpp-------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This implementation of methods in FiFuzzVisitors
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/FiFuzzVisitors.h"
#include "clang/DetectERR/ErrPoint.h"
#include "clang/DetectERR/Utils.h"

/// call to a library function, which returns either a pointer or an integer
bool FiFuzzVisitor::VisitCallExpr(CallExpr *CE) {
  // CFGBlock *CurBB;
  if (isLibraryCallExpr(CE, Context)) {
    FnReturnType RetType = getReturnType(CE, Context);
    if (RetType == POINTER || RetType == INT) {
      // add the location for this call to errblocks
      // llvm::errs() << ">>>> adding error point with return type: "
      //              << fnReturnTypeStr(RetType) << "\n";
      // CE->dumpPretty(*Context);
      // llvm::errs() << "\n";
      // CE->dumpColor();
      // llvm::errs() << "\n";
      addErrorPoint(CE, RetType);
    }
  }

  // if (isEHFCallExpr(CE, *EhfList, Context)) {
  //   if (StMap.find(CE) != StMap.end()) {
  //     CurBB = StMap[CE];
  //     std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
  //     collectChecks(Checks, *CurBB, &CDG);
  //     removeChecksUsingParams(Checks, *FnDecl);
  //     if (!Checks.empty()) {
  //       // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
  //       // addErrorGuards(Checks, CE);
  //       auto [check, level] = getImmediateControlDependentCheck(
  //           Checks, CE, &CDG, Context->getSourceManager());
  //       addErrorGuard(check, CE, level);
  //     }
  //   }
  // }
  return true;
}
