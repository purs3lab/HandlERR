//=--ReturnVisitors.h---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents all the visitors dealing with return statements.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_RETURNVISITORS_H
#define LLVM_CLANG_DETECTERR_RETURNVISITORS_H

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/DetectERRVisitor.h"
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

/// H04 - Condition guarding return NULL is error guarding.
class ReturnNullVisitor : public DetectERRVisitor {
public:
  explicit ReturnNullVisitor(ASTContext *Context, ProjectInfo &I,
                             FunctionDecl *FD, FuncId &FnID)
      : DetectERRVisitor{Context, I, FD, FnID, HeuristicID::H04} {};

  virtual ~ReturnNullVisitor() = default;

  bool VisitReturnStmt(ReturnStmt *S) override;
};

/// H02 - Condition guarding return negative value is error guarding.
class ReturnNegativeNumVisitor : public DetectERRVisitor {
public:
  explicit ReturnNegativeNumVisitor(ASTContext *Context, ProjectInfo &I,
                                    FunctionDecl *FD, FuncId &FnID)
      : DetectERRVisitor{Context, I, FD, FnID, HeuristicID::H02} {};

  virtual ~ReturnNegativeNumVisitor() = default;

  bool VisitReturnStmt(ReturnStmt *S) override;
};

/// H05 - Condition guarding return 0 value is error guarding.
class ReturnZeroVisitor : public DetectERRVisitor {
public:
  ReturnZeroVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
                    FuncId &FnID)
      : DetectERRVisitor{Context, I, FD, FnID, HeuristicID::H05} {};

  virtual ~ReturnZeroVisitor() = default;

  bool VisitReturnStmt(ReturnStmt *S) override;
};

/// H06 - a "return <val>" statement is dominated by a check for that
/// particular value but is not control dependent on the check
class ReturnValVisitor : public DetectERRVisitor {
public:
  ReturnValVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
                   FuncId &FnID)
      : DetectERRVisitor{Context, I, FD, FnID, HeuristicID::H06},
        DomTree(Cfg.get()){};

  virtual ~ReturnValVisitor() = default;

  bool VisitReturnStmt(ReturnStmt *S) override;

private:
  CFGDomTree DomTree;
};

/// H07 - For a function having a void return type, early return based on a
/// check
class ReturnEarlyVisitor : public DetectERRVisitor {
public:
  ReturnEarlyVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
                     FuncId &FnID)
      : DetectERRVisitor{Context, I, FD, FnID, HeuristicID::H07} {};

  virtual ~ReturnEarlyVisitor() = default;

  bool VisitReturnStmt(ReturnStmt *S) override;
};

#endif //LLVM_CLANG_DETECTERR_RETURNVISITORS_H
