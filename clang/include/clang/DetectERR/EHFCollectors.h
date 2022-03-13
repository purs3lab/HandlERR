//=--EHFVisitors.h---------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class represents the visitor dealing with the ErrorHandlingFunction utils
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_EHFVISITOR_H
#define LLVM_CLANG_DETECTERR_EHFVISITOR_H

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

// ErrorHandlingFunction Visitor to try and collect all the error handling functions
class EHFCategoryOneCollector
    : public RecursiveASTVisitor<EHFCategoryOneCollector> {
public:
  explicit EHFCategoryOneCollector(ASTContext *Context, FunctionDecl *FD,
                                   std::set<std::string> &EHFList)
      : Context(Context), FnDecl(FD),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), EHFList_(&EHFList) {
    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitCallExpr(CallExpr *S);
  bool VisitFunctionDecl(FunctionDecl *FD);

private:
  ASTContext *Context;
  FunctionDecl *FnDecl;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;
  std::set<std::string> *EHFList_;
};

class EHFCategoryTwoCollector
    : public RecursiveASTVisitor<EHFCategoryTwoCollector> {
public:
  explicit EHFCategoryTwoCollector(ASTContext *Context, FunctionDecl *FD,
                                   std::set<std::string> &EHFList)
      : Context(Context), FnDecl(FD),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(), Context,
                          CFG::BuildOptions())),
        CDG(Cfg.get()), EHFList_(&EHFList) {
    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitFunctionDecl(FunctionDecl *FD);

private:
  ASTContext *Context;
  FunctionDecl *FnDecl;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;
  std::set<std::string> *EHFList_;
};

#endif //LLVM_CLANG_DETECTERR_RETURNVISITORS_H
