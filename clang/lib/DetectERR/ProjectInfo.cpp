//=--ProjectInfo.cpp----------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of ProjectInfo methods.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/ProjectInfo.h"

using namespace clang;

bool ProjectInfo::addErrorGuardingStmt(const FuncId &FID,
                                       const clang::Stmt *ST,
                                       ASTContext *C) {
  bool RetVal = false;
  PersistentSourceLoc PSL = PersistentSourceLoc::mkPSL(ST, *C);
  if (PSL.valid()) {
    RetVal = ErrGuardingConds[FID].insert(PSL).second;
  }
  return RetVal;
}