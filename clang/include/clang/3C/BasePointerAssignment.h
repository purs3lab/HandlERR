//=--PointerArithmeticAssignment.h--------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// TODO
//===----------------------------------------------------------------------===//

#ifndef LLVM_BASEPOINTERASSIGNMENT_H
#define LLVM_BASEPOINTERASSIGNMENT_H

#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/3C/ConstraintResolver.h"

// FIXME: update comment.
// Return true if an assignment LHS=RHS would result in invalidating the bounds
// of LHS by assigning to a pointer a value derived from the pointer, for
// example, by using pointer arithmetic to compute a new pointer at some offset
// from the original.
bool isBasePointerAssignment(clang::Expr *LHS, clang::Expr *RHS);

class BasePointerAssignmentVisitor
  : public RecursiveASTVisitor<BasePointerAssignmentVisitor> {
public:
  explicit BasePointerAssignmentVisitor() {}

  bool VisitBinaryOperator(BinaryOperator *O);

  virtual void visitBasePointerAssignment(Expr *LHS, Expr *RHS) = 0;
};

class BasePointerAssignmentUpdater : public BasePointerAssignmentVisitor {
public:
  explicit BasePointerAssignmentUpdater(ASTContext *C, ProgramInfo &I,
                                        Rewriter &R) : ABInfo(
    I.getABoundsInfo()), CR(I, C), R(R) {}

  void visitBasePointerAssignment(Expr *LHS, Expr *RHS) override;

private:
  AVarBoundsInfo &ABInfo;
  ConstraintResolver CR;
  Rewriter &R;
};

class BasePointerAssignmentFinder : public BasePointerAssignmentVisitor {
public:
  explicit BasePointerAssignmentFinder(ASTContext *C, ProgramInfo &I) : ABInfo(
    I.getABoundsInfo()), CR(I, C), C(C) {}

  void visitBasePointerAssignment(Expr *LHS, Expr *RHS) override;

private:
  AVarBoundsInfo &ABInfo;
  ConstraintResolver CR;
  ASTContext *C;
};
#endif //LLVM_BASEPOINTERASSIGNMENT_H
