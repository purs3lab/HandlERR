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
#include "clang/DetectERR/ErrPoint.h"

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

bool ProjectInfo::addErrorPointStmt(const FuncId &FID, const clang::Stmt *ErrST,
                                    FnReturnType RetType, ASTContext *C) {
  bool RetVal = false;
  PersistentSourceLoc ErrLoc = PersistentSourceLoc::mkPSL(ErrST, *C);
  ErrPoint *EP = new ErrPoint(ErrLoc, RetType);
  RetVal = ErrPoints[FID].insert(*EP).second;

  return RetVal;
}

std::string ProjectInfo::fifuzzErrPointsToJsonString() const {
  std::string RetVal = "{\"ErrPoints\":[";
  bool AddComma = false;
  for (auto &FC : ErrPoints) {
    if (AddComma) {
      RetVal += ",\n";
    }
    RetVal += "{\"FunctionInfo\":{\"Name\":\"" + FC.first.first +
              "\", \"File\":\"" + FC.first.second + "\"}";
    RetVal += ",\"ErrConditions\":[";
    bool AddComma1 = false;
    for (auto &EP : FC.second) {
      if (AddComma1) {
        RetVal += ",";
      }
      RetVal += EP.toJsonString();
      AddComma1 = true;
    }
    RetVal += "]}";
    AddComma = true;
  }
  RetVal += "\n]}";
  return RetVal;
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

void ProjectInfo::fifuzzErrPointsToJsonString(llvm::raw_ostream &O) const {
  O << fifuzzErrPointsToJsonString();
}

void ProjectInfo::toJsonString(llvm::raw_ostream &O) const {
  if (!ErrGuardingConds.empty()) {
    llvm::errs() << "ErrGuardingConds not empty, hence converting "
                    "errguardingconds to json\n";
    errCondsToJsonString(O);

  } else if (!ErrPoints.empty()) {
    llvm::errs()
        << "ErrPoints not empty, hence converting errorpoints to json\n";
    fifuzzErrPointsToJsonString(O);
  }
}
