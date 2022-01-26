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

#ifndef LLVM_CLANG_DETECTERR_UTILS_H
#define LLVM_CLANG_DETECTERR_UTILS_H

using namespace clang;

typedef std::pair<std::string, std::string> FuncId;

// Get function id for the given function declaration.
FuncId getFuncID(const clang::FunctionDecl *FD, ASTContext *C);

// Is the expression a NULL pointer expression?
bool isNULLExpr(const clang::Expr *E, ASTContext &C);

// Is the expression a negative integer expression?
bool isNegativeNumber(const clang::Expr *E, ASTContext &C);

/// Is the expression a zero
bool isZero(const clang::Expr *E, ASTContext &C);

/// Is the expression is a variable
bool isDeclExpr(const clang::Expr *E);

/// Get the underlying DeclRefExpr
const DeclRefExpr *getDeclRefExpr(const clang::Expr *E);

#endif //LLVM_CLANG_DETECTERR_UTILS_H
