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

#ifndef LLVM_CLANG_DETECTERR_PROJECTINFO_H
#define LLVM_CLANG_DETECTERR_PROJECTINFO_H

#include "clang/DetectERR/ErrGruard.h"
#include "clang/DetectERR/ErrPoint.h"
#include "clang/DetectERR/Utils.h"

// This stores global information about the project.
class ProjectInfo {
public:
  ProjectInfo() {}

  ~ProjectInfo() {
    // clear up all elements.
    ErrGuardingConds.clear();
  }

  bool addErrorGuardingStmt(const FuncId &FID, const clang::Stmt *GuardST,
                            const clang::Stmt *ErrST, ASTContext *C,
                            HeuristicID Heuristic, GuardLevel Lvl);

  bool addErrorPointStmt(const FuncId &FID, const clang::Stmt *ErrST,
                         FnReturnType RetType, ASTContext *C);

  // Convert error conditions to json string.
  std::string errCondsToJsonString() const;

  std::string fifuzzErrPointsToJsonString() const;

  // Write the detected error conditions to the provided output stream.
  void errCondsToJsonString(llvm::raw_ostream &O) const;

  void fifuzzErrPointsToJsonString(llvm::raw_ostream &O) const;

    void toJsonString(llvm::raw_ostream &O) const;

private:
  // map of function id and set of error guarding conditions.
  //  std::map<FuncId, std::set<PersistentSourceLoc>> ErrGuardingConds;
  std::map<FuncId, std::set<ErrGuard>> ErrGuardingConds;

  // Fifuzz error points
  std::map<FuncId, std::set<ErrPoint>> ErrPoints;
};

#endif //LLVM_CLANG_DETECTERR_PROJECTINFO_H
