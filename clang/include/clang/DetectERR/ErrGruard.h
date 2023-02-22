//=--ErrGuard.h----------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents the Error Guard for a particular error condition
// It contains the location of the error guarding check (if/while/switch) and
// other metadata about the check (such as the Heuristic ID, and information
// whether the check is an inner or outer check etc.)
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_ERRGUARD_H
#define LLVM_CLANG_DETECTERR_ERRGUARD_H

#include "PersistentSourceLoc.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

enum HeuristicID {
  /// H02 - Condition guarding return negative value is error guarding.
  H02,

  /// H03 - call to an exit function is control dependent on one or more
  /// if checks
  H03,

  /// H04 - Condition guarding return NULL is error guarding.
  H04,

  /// H05 - Condition guarding return 0 value is error guarding.
  H05,

  /// H06 - a "return <val>" statement is dominated by a check for that
  /// particular value but is not control dependent on the check
  H06,

  /// H07 - For a function having a void return type, early return based on a
  /// check
  H07,

  /// H08 - goto to an error label is control dependent on a check
  H08,

  /// H09 - throwing an exception is control dependent on a check
  H09,

  /// FiFuzz
  FIFUZZ
};

/// The level at which the given check occurs (inner/outer)
enum GuardLevel { Inner, Outer, Default };

class ErrGuard {
public:
  static ErrGuard mkErrGuard(PersistentSourceLoc GuardL,
                             PersistentSourceLoc ErrL, HeuristicID HID,
                             GuardLevel Lvl) {
    return ErrGuard(GuardL, ErrL, HID, Lvl);
  };

  static ErrGuard mkErrGuard(PersistentSourceLoc GuardL,
                             PersistentSourceLoc ErrL, HeuristicID HID) {
    return ErrGuard(GuardL, ErrL, HID, GuardLevel::Default);
  };

  bool operator<(const ErrGuard &O) const {
    return GuardLoc < O.GuardLoc ||
           (GuardLoc == O.GuardLoc && ErrLoc < O.ErrLoc) ||
           (GuardLoc == O.GuardLoc && ErrLoc == O.ErrLoc && Level < O.Level);
  }

  std::string toString() const {
    return GuardLoc.toString() + ":" + ErrLoc.toString() + ":" +
           HeuristicLabel[HID] + ":" + GuardLevelLabel[Level];
  }

  std::string toJsonString() const;

  void toJsonString(llvm::raw_ostream &O) const { O << toJsonString(); }

  void print(llvm::raw_ostream &O) const { O << toString(); }

  void dump() const { print(llvm::errs()); }

private:
  ErrGuard(PersistentSourceLoc GuardL, PersistentSourceLoc ErrL,
           HeuristicID Hid, GuardLevel Lvl)
      : GuardLoc(GuardL), ErrLoc(ErrL), HID(Hid), Level(Lvl) {}

  PersistentSourceLoc GuardLoc;
  PersistentSourceLoc ErrLoc;
  HeuristicID HID;
  GuardLevel Level;
  static std::map<HeuristicID, std::string> HeuristicLabel;
  static std::map<GuardLevel, std::string> GuardLevelLabel;
};

#endif //LLVM_CLANG_DETECTERR_ERRGUARD_H
