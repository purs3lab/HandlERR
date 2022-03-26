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
#include "clang/DetectERR/ErrGruard.h"

using namespace clang;

bool ProjectInfo::addErrorGuardingStmt(const FuncId &FID,
                                       const clang::Stmt *GuardST,
                                       const clang::Stmt *ErrST, ASTContext *C,
                                       HeuristicID Heuristic, GuardLevel Lvl) {
  bool RetVal = false;
  PersistentSourceLoc GuardL = PersistentSourceLoc::mkPSL(GuardST, *C);
  PersistentSourceLoc ErrL = PersistentSourceLoc::mkPSL(ErrST, *C);
  if (GuardL.valid()) {
    ErrGuard EG = ErrGuard::mkErrGuard(GuardL, ErrL, Heuristic, Lvl);
    RetVal = ErrGuardingConds[FID].insert(EG).second;
  }
  return RetVal;
}

bool ProjectInfo::addErrorGuardingStmt(const FuncId &FID,
                                       const clang::Stmt *GuardST,
                                       const clang::Stmt *ErrST, ASTContext *C,
                                       HeuristicID Heuristic) {
  return addErrorGuardingStmt(FID, GuardST, ErrST, C, Heuristic,
                              GuardLevel::Default);
}

std::string ProjectInfo::errCondsToJsonString() const {
  std::string RetVal = "{\"ErrGuardingConditions\":[";
  bool AddComma = false;
  for (auto &FC : ErrGuardingConds) {
    if (AddComma) {
      RetVal += ",\n";
    }
    RetVal += "{\"FunctionInfo\":{\"Name\":\"" + FC.first.first +
              "\", \"File\":\"" + FC.first.second + "\"}";
    RetVal += ",\"ErrConditions\":[";
    bool AddComma1 = false;
    for (auto &ED : FC.second) {
      if (AddComma1) {
        RetVal += ",";
      }
      RetVal += ED.toJsonString();
      AddComma1 = true;
    }
    RetVal += "]}";
    AddComma = true;
  }
  RetVal += "\n]}";
  return RetVal;
}

void ProjectInfo::errCondsToJsonString(llvm::raw_ostream &O) const {
  O << errCondsToJsonString();
}
