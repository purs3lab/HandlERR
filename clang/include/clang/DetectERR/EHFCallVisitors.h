//=--EHFCallVisitors.h---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents all the visitors dealing with EHF Calls.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_EHFCALLVISITORS_H
#define LLVM_CLANG_DETECTERR_EHFCALLVISITORS_H

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/DetectERRVisitor.h"
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

/// H03 - call to an exit function is control dependent on one or more
/// if checks
class EHFCallVisitor : public DetectERRVisitor {
public:
  EHFCallVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
                 FuncId &FnID, const std::set<std::string> &EHFList)
      : DetectERRVisitor(Context, I, FD, FnID, HeuristicID::H03),
        EhfList(&EHFList){};

  virtual ~EHFCallVisitor() = default;

  bool VisitCallExpr(CallExpr *CE) override;

private:
  const std::set<std::string> *EhfList;
};

#endif //LLVM_CLANG_DETECTERR_EHFCALLVISITORS_H
