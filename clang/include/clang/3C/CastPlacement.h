//=--CastPlacement.h----------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class contains functions and classes that deal with
// placing casts into the text as needing during the rewrite phase
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_3C_CASTPLACEMENT_H
#define LLVM_CLANG_3C_CASTPLACEMENT_H

#include "clang/3C/ConstraintResolver.h"
#include "clang/AST/RecursiveASTVisitor.h"

class CastPlacementVisitor : public RecursiveASTVisitor<CastPlacementVisitor> {
public:
  explicit CastPlacementVisitor(ASTContext *C, ProgramInfo &I, Rewriter &R)
      : Context(C), Info(I), Writer(R), CR(Info, Context) {}

  bool VisitCallExpr(CallExpr *C);

private:
  ASTContext *Context;
  ProgramInfo &Info;
  Rewriter &Writer;
  ConstraintResolver CR;

  enum CastNeeded { NO_CAST = 0, CAST_TO_CHECKED, CAST_TO_WILD };

  CastNeeded needCasting(const ConstraintVariable *SrcInt,
                         const ConstraintVariable *SrcExt,
                         const ConstraintVariable *DstInt,
                         const ConstraintVariable *DstExt);

  std::string getCastString(const ConstraintVariable *SrcInt,
                            const ConstraintVariable *SrcExt,
                            const ConstraintVariable *DstInt,
                            const ConstraintVariable *DstExt);

  void surroundByCast(const ConstraintVariable *SrcInt,
                      const ConstraintVariable *SrcExt,
                      const ConstraintVariable *DstInt,
                      const ConstraintVariable *DstExt, Expr *);
};
#endif // LLVM_CLANG_3C_CASTPLACEMENT_H
