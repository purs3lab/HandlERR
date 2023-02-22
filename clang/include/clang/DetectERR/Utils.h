//=--Utils.h------------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Type declarations for map data structures and other general helper methods.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_UTILS_H
#define LLVM_CLANG_DETECTERR_UTILS_H

#include "clang/AST/ASTContext.h"
#include "clang/AST/Decl.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/DetectERR/ErrGruard.h"
#include "clang/DetectERR/ErrPoint.h"
// #include "clang/DetectERR/ProjectInfo.h"

using namespace clang;

/// (function name, file name)
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

/// get the return type of a call expression (pointer or int) for fifuzz mode
FnReturnType getReturnType(const CallExpr *CE, ASTContext *Context);

/// Checks if the given CallExpr calls a library function
bool isLibraryCallExpr(const CallExpr *CE, ASTContext *Context);

/// Checks if the given CallExpr calls an EHF
bool isEHFCallExpr(const CallExpr *CE, const std::set<std::string> &EHFList,
                   ASTContext *Context);

/// Checks whether there is a post-dominated CFGBlock for the given block
bool hasPostDominators(CFGBlock &CurrBB, const CFGPostDomTree *PDTree,
                       const CFG &Cfg);

/// Checks if the given CFGBlock has any dominators
bool hasPreDominators(CFGBlock &CurrBB, const ControlDependencyCalculator *CDG,
                      const CFG &Cfg);

/// debug util: dumps the statements in the stmap
void __dbg_print_statements(std::map<const Stmt *, CFGBlock *> &StMap);

/// Bubbles the inner dominating check at 0th position so that appropriate
/// 'level' related info can be added whiled creating the ErrGuard instance
void sortIntoInnerAndOuterChecks(
    std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
    ControlDependencyCalculator *CDG, SourceManager &SM);

/// return the most immediate Control Dependent check
std::pair<Stmt *, GuardLevel> getImmediateControlDependentCheck(
    std::vector<std::pair<Stmt *, CFGBlock *>> &Checks, Stmt *ErrorST,
    ControlDependencyCalculator *CDG, SourceManager &SM);

/// Iterates over the base blocks on which the given block is control dependent
/// and collects all the terminator checks from those blocks
void collectChecks(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                   CFGBlock &CurBB, ControlDependencyCalculator *CDG);

/// returns the condition expression based on the type of check statement
/// (if/while/switch)
Expr *getCondFromCheckStmt(Stmt *ST);

/// extracts all the Values that are used in the Conditional
std::vector<const Decl *> getCondValueDecls(const Expr *Cond);

/// remove the inner check that are using params to the function
void removeInnerCheckUsingParams(
    std::vector<std::pair<Stmt *, CFGBlock *>> &Checks, Stmt *ErrorST,
    FunctionDecl &FD, SourceManager &SM);

/// remove all checks that are using params to the function
void removeChecksUsingParams(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                             FunctionDecl &FD);

#endif //LLVM_CLANG_DETECTERR_UTILS_H
