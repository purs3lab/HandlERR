//=--ProjectInfo.h------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents all the information about all the source files
// collected by the detecterr.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/PersistentSourceLoc.h"
#include "clang/DetectERR/Utils.h"

#ifndef LLVM_CLANG_DETECTERR_PROJECTINFO_H
#define LLVM_CLANG_DETECTERR_PROJECTINFO_H

// This stores global information about the project.
class ProjectInfo {
public:
  ProjectInfo() {

  }

  ~ProjectInfo() {
    // clear up all elements.
    ErrGuardingConds.clear();
  }

  bool addErrorGuardingStmt(const FuncId &FID, const clang::Stmt *ST);
private:
  // map of function id and set of error guarding conditions.
  std::map<FuncId, std::set<PersistentSourceLoc>> ErrGuardingConds;
};

#endif //LLVM_CLANG_DETECTERR_PROJECTINFO_H
