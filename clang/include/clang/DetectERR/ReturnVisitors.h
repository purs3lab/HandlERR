//=--ReturnVisitors.h---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents all the visitors dealing with return statements.
//===----------------------------------------------------------------------===//

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

#ifndef LLVM_CLANG_DETECTERR_RETURNVISITORS_H
#define LLVM_CLANG_DETECTERR_RETURNVISITORS_H

// Condition guarding return NULL is error guarding.
class ReturnNullVisitor : public RecursiveASTVisitor<ReturnNullVisitor> {
public:
  explicit ReturnNullVisitor(ASTContext *Context, ProjectInfo &I,
                             FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()) {
    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitReturnStmt(ReturnStmt *S);

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;
};

// Condition guarding return negative value is error guarding.
class ReturnNegativeNumVisitor
    : public RecursiveASTVisitor<ReturnNegativeNumVisitor> {
public:
  explicit ReturnNegativeNumVisitor(ASTContext *Context, ProjectInfo &I,
                                    FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()) {
    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitReturnStmt(ReturnStmt *S);

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;
};

// Condition guarding return 0 value is error guarding.
class ReturnZeroVisitor : public RecursiveASTVisitor<ReturnZeroVisitor> {
public:
  explicit ReturnZeroVisitor(ASTContext *Context, ProjectInfo &I,
                             FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()) {
    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitReturnStmt(ReturnStmt *S);

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;
};

// Condition guarding return 0 value is error guarding.
class ReturnValVisitor : public RecursiveASTVisitor<ReturnValVisitor> {
public:
  explicit ReturnValVisitor(ASTContext *Context, ProjectInfo &I,
                            FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), DomTree(Cfg.get()) {
    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitReturnStmt(ReturnStmt *S);

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  CFGDomTree DomTree;
  std::map<const Stmt *, CFGBlock *> StMap;
};

#endif //LLVM_CLANG_DETECTERR_RETURNVISITORS_H
