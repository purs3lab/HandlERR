//=--FiFuzzVisitors.h---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents all the visitors dealing with FiFuzz.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_FIFUZZVISITORS_H
#define LLVM_CLANG_DETECTERR_FIFUZZVISITORS_H

#include "clang/DetectERR/DetectERRVisitor.h"

using namespace llvm;
using namespace clang;

class FiFuzzVisitor : public DetectERRVisitor {
public:
  FiFuzzVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD, FuncId &FnID)
      : DetectERRVisitor(Context, I, FD, FnID, HeuristicID::FIFUZZ){};

  virtual ~FiFuzzVisitor() = default;

  bool VisitCallExpr(CallExpr *CE) override;
};

#endif //LLVM_CLANG_DETECTERR_EHFCALLVISITORS_H
