//=--Utils.h------------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Type declarations for map data structures and other general helper methods.
//===----------------------------------------------------------------------===//

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Analysis/Analyses/Dominators.h"

#ifndef LLVM_CLANG_DETECTERR_UTILS_H
#define LLVM_CLANG_DETECTERR_UTILS_H

using namespace clang;

typedef std::pair<std::string, std::string> FuncId;

/// Get function id for the given function declaration.
FuncId getFuncID(const clang::FunctionDecl *FD, ASTContext *C);

/// Is the expression a NULL pointer expression?
bool isNULLExpr(const clang::Expr *E, ASTContext &C);

/// Is the expression a negative integer expression?
bool isNegativeNumber(const clang::Expr *E, ASTContext &C);

/// Is the expression an integer with value 'i'
bool isInt(int i, const clang::Expr *E, ASTContext &C);

/// Is the expression a deref to the given Decl?
bool isDerefToDeclRef(const clang::Expr *E, const NamedDecl *D);

/// Is the Expr a wrapper around the given NamedDecl?
bool hasDeclRefExprTo(const clang::Expr *E, const NamedDecl *D);

/// Get the underlying expression for a Deref Expression (UnaryOperator)
Expr *getDerefExpr(const clang::Expr *E);

/// Is the expression a zero
bool isZero(const clang::Expr *E, ASTContext &C);

/// Is the expression is a variable
bool isDeclExpr(const clang::Expr *E);

/// Get the underlying DeclRefExpr
const DeclRefExpr *getDeclRefExpr(const clang::Expr *E);

/// Checks whether a particular variable (Decl) has been updated anywhere
/// in the Post-Dominator BasicBlocks of a particular BasicBlock
bool isUpdatedInPostDominators(const NamedDecl *ND, CFGBlock &CurrBB,
                               const CFGPostDomTree *PDTree, const CFG &Cfg);

/// Checks if the given statement is the last statement in the given CFGBlock
bool isLastStmtInBB(const Stmt &ST, const CFGBlock &BB);

/// Checks if the given CallExpr calls an EHF
bool isEHFCallExpr(const CallExpr *CE, const std::set<std::string> &EHFList,
                   ASTContext *Context);

/// Checks whether there is a post-dominated CFGBlock for the given block
bool hasPostDominators(CFGBlock &CurrBB, const CFGPostDomTree *PDTree,
                       const CFG &Cfg);

/// Checks if the given CFGBlock has any dominators
bool hasPreDominators(CFGBlock &CurrBB, const ControlDependencyCalculator *CDG,
                      const CFG &Cfg);

#endif //LLVM_CLANG_DETECTERR_UTILS_H
