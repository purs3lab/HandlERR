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
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

/// H04 - Condition guarding return NULL is error guarding.
class ReturnNullVisitor : public RecursiveASTVisitor<ReturnNullVisitor> {
public:
  explicit ReturnNullVisitor(ASTContext *Context, ProjectInfo &I,
                             FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), Heuristic(HeuristicID::H04) {
    for (auto *CBlock : *(Cfg.get())) {
      if (CBlock->size() == 0) {
        if (Stmt *St = CBlock->getTerminatorStmt()) {
          StMap[St] = CBlock;
        }
      }
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitReturnStmt(ReturnStmt *S);

  ProjectInfo &getProjectInfo() { return Info; }

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;

  HeuristicID Heuristic;
};

/// H02 - Condition guarding return negative value is error guarding.
class ReturnNegativeNumVisitor
    : public RecursiveASTVisitor<ReturnNegativeNumVisitor> {
public:
  explicit ReturnNegativeNumVisitor(ASTContext *Context, ProjectInfo &I,
                                    FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), Heuristic(HeuristicID::H02) {
    for (auto *CBlock : *(Cfg.get())) {
      if (CBlock->size() == 0) {
        if (Stmt *St = CBlock->getTerminatorStmt()) {
          StMap[St] = CBlock;
        }
      }
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

  HeuristicID Heuristic;
};

/// H05 - Condition guarding return 0 value is error guarding.
class ReturnZeroVisitor : public RecursiveASTVisitor<ReturnZeroVisitor> {
public:
  explicit ReturnZeroVisitor(ASTContext *Context, ProjectInfo &I,
                             FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), Heuristic(HeuristicID::H05) {
    for (auto *CBlock : *(Cfg.get())) {
      if (CBlock->size() == 0) {
        if (Stmt *St = CBlock->getTerminatorStmt()) {
          StMap[St] = CBlock;
        }
      }
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

  HeuristicID Heuristic;
};

/// H06 - Condition guarding return 0 value is error guarding.
class ReturnValVisitor : public RecursiveASTVisitor<ReturnValVisitor> {
public:
  explicit ReturnValVisitor(ASTContext *Context, ProjectInfo &I,
                            FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), DomTree(Cfg.get()), Heuristic(HeuristicID::H06) {
    for (auto *CBlock : *(Cfg.get())) {
      if (CBlock->size() == 0) {
        if (Stmt *St = CBlock->getTerminatorStmt()) {
          StMap[St] = CBlock;
        }
      }
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

  HeuristicID Heuristic;
};

/// H07 - For a function having a void return type, early return based on a
/// check
class ReturnEarlyVisitor : public RecursiveASTVisitor<ReturnEarlyVisitor> {
public:
  explicit ReturnEarlyVisitor(ASTContext *Context, ProjectInfo &I,
                              FunctionDecl *FD, FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), Heuristic(HeuristicID::H07) {
    for (auto *CBlock : *(Cfg.get())) {
      if (CBlock->size() == 0) {
        if (Stmt *St = CBlock->getTerminatorStmt()) {
          StMap[St] = CBlock;
        }
      }
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

  HeuristicID Heuristic;
};
#endif //LLVM_CLANG_DETECTERR_RETURNVISITORS_H
