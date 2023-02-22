//=--ErrPoint.h----------------------------------------*- C++-*-===//
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

#ifndef LLVM_CLANG_DETECTERR_ERRPOINT_H
#define LLVM_CLANG_DETECTERR_ERRPOINT_H

#include "PersistentSourceLoc.h"
#include <cstdlib>

enum FnReturnType { POINTER, INT, NOT_INTERESTING };
std::string fnReturnTypeStr(const FnReturnType &RT);

class ErrPoint {
public:
  ErrPoint(PersistentSourceLoc ErrPtLoc, FnReturnType RetType)
      : ErrPtLoc(ErrPtLoc), RetType(RetType) {}

  std::string toString() const {
    return ErrPtLoc.toString() + ":" + fnReturnTypeStr(RetType);
  }

  bool operator<(const ErrPoint &O) const {
    return ErrPtLoc < O.ErrPtLoc ||
           (ErrPtLoc == O.ErrPtLoc && RetType < O.RetType);
  }

    std::string toJsonString() const;

private:
  PersistentSourceLoc ErrPtLoc;
  FnReturnType RetType;
};

#endif //LLVM_CLANG_DETECTERR_ERRPOINT_H
