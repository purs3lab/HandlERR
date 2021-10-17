//=--Utils.h------------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Type declarations for map data structures and other general helper methods.
//===----------------------------------------------------------------------===//

#include "clang/AST/Decl.h"
#include "clang/AST/ASTContext.h"

#ifndef LLVM_CLANG_DETECTERR_UTILS_H
#define LLVM_CLANG_DETECTERR_UTILS_H

using namespace clang;

typedef std::pair<std::string, std::string> FuncId;

// Get function id for the given function declaration.
FuncId getFuncID(const clang::FunctionDecl *FD, ASTContext *C);

// Is the expression a NULL pointer expression?
bool isNULLExpr(const clang::Expr *E, ASTContext &C);


#endif //LLVM_CLANG_DETECTERR_UTILS_H
