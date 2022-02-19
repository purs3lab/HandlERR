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
                                       ASTContext *C,
                                       std::string Heuristic) {
  bool RetVal = false;
  PersistentSourceLoc PSL = PersistentSourceLoc::mkPSL(ST, *C, Heuristic);
  if (PSL.valid()) {
    RetVal = ErrGuardingConds[FID].insert(PSL).second;
  }
  return RetVal;
}

bool ProjectInfo::addErrorGuardingStmt(const FuncId &FID,
                                       const clang::Stmt *ST,
                                       ASTContext *C) {
  return addErrorGuardingStmt(FID, ST, C, "");
}

std::string ProjectInfo::errCondsToJsonString() const {
  std::string RetVal = "{\"ErrGuardingConditions\":[";
  bool AddComma = false;
  for (auto &FC : ErrGuardingConds) {
    if (AddComma) {
      RetVal += ",\n";
    }
    RetVal += "{\"FunctionInfo\":{\"Name\":\"" + FC.first.first + "\", \"File\":\"" + FC.first.second + "\"}";
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