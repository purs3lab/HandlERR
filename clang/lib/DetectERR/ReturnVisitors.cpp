//=--ReturnVisitors.cpp-------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This implementation of methods in ReturnVisitors - various visitors
// dealing with return statement heuristics.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/ReturnVisitors.h"
#include "clang/DetectERR/Utils.h"

/// H04 - if a "return NULL" statement is control dependent upon one or more
/// "if" checks
bool ReturnNullVisitor::VisitReturnStmt(ReturnStmt *S) {
  if (FnDecl->getReturnType()->isPointerType()) {
    CFGBlock *CurBB;
    if (isNULLExpr(S->getRetValue(), *Context)) {
      if (StMap.find(S) != StMap.end()) {
        CurBB = StMap[S];
        auto &CDNodes = CDG.getControlDependencies(CurBB);
        if (!CDNodes.empty()) {
          // We should use all CDs
          // Get the last statement from the list of control dependencies.
          for (auto &CDGNode : CDNodes) {
            // Collect the possible length bounds keys.
            Stmt *TStmt = CDGNode->getTerminatorStmt();
            // check if this is an if statement.
            if (dyn_cast_or_null<IfStmt>(TStmt)) {
              Info.addErrorGuardingStmt(FID, TStmt, Context);
            }
          }
        }
      }
    }
  }
  return true;
}

bool ReturnNegativeNumVisitor::VisitReturnStmt(ReturnStmt *S) {
  if (FnDecl->getReturnType()->isIntegerType()) {
    CFGBlock *CurBB;
    if (isNegativeNumber(S->getRetValue(), *Context)) {
      if (StMap.find(S) != StMap.end()) {
        CurBB = StMap[S];
        auto &CDNodes = CDG.getControlDependencies(CurBB);
        if (!CDNodes.empty()) {
          // We should use all CDs
          // Get the last statement from the list of control dependencies.
          for (auto &CDGNode : CDNodes) {
            // Collect the possible length bounds keys.
            Stmt *TStmt = CDGNode->getTerminatorStmt();
            // check if this is an if statement.
            if (dyn_cast_or_null<IfStmt>(TStmt)) {
              Info.addErrorGuardingStmt(FID, TStmt, Context);
            }
          }
        }
      }
    }
  }
  return true;
}

/// H05 - if a "return 0" statement is control dependent upon one or more
/// "if" checks AND the return type of the function is a pointer type
///
/// Conditions:
/// - function has pointer return type
/// - return stmt returns a zero
/// - the return stmt is dominated by one or more checks
bool ReturnZeroVisitor::VisitReturnStmt(ReturnStmt *S) {
  // is the return type a pointer type?
  if (FnDecl->getReturnType()->isPointerType()) {
    CFGBlock *CurBB;
    if (isZero(S->getRetValue(), *Context)) {
      CurBB = StMap[S];
      auto &CDNodes = CDG.getControlDependencies(CurBB);
      if (!CDNodes.empty()) {
        // We should use all CDs
        // Get the last statement from the list of control dependencies.
        for (auto &CDGNode : CDNodes) {
          // Collect the possible length bounds keys.
          Stmt *TStmt = CDGNode->getTerminatorStmt();
          // check if this is an if statement.
          if (dyn_cast_or_null<IfStmt>(TStmt)) {
            Info.addErrorGuardingStmt(FID, TStmt, Context);
          }
        }
      }
    }
  }
  return true;
}
/// H06 - a "return <val>" statement is dominated by a check for that particular
/// value but is not control dependent on the check
///
/// Checks for conditions like:
///
///     mystruct *myfunc(<args>) {
///         ...
///         mystruct *struct_ptr = other_func();    // <----
///         ...
///         if (struct_ptr != NULL){                // <----
///             ...
///         }
///         ...
///         return struct_ptr; // <----
///     }
///
/// Conditions:
/// - function has pointer return type
/// - returns stmt returns a variable (DeclRef)
/// - there is a check for this returned variable which dominates the return
///     stmt but the return is not control dependent on the check
bool ReturnValVisitor::VisitReturnStmt(ReturnStmt *S) {
  CFGBlock *ReturnBB;
  if (FnDecl->getReturnType()->isPointerType()) {  // return type = pointer
    if (!isNULLExpr(S->getRetValue(), *Context)) { // not return NULL
      // find all the blocks that dominate the exit block (containing the return stmt)
      // for each of these dominating blocks, check if their terminator stmt
      // is a IfStmt and the condition of that IfStmt is a NULL check
      // against the value being returned

      // return stmt is a 'return var'
      if (isDeclExpr(S->getRetValue())) { // return val
        ReturnBB = StMap[S];

        // store the underlying NamedDecl for comparing against later
        const Expr *E = S->getRetValue();
        const DeclRefExpr *ReturnDRE = getDeclRefExpr(E);
        const NamedDecl *ReturnNamedDecl =
            ReturnDRE->getFoundDecl()->getUnderlyingDecl();

        // find dominator nodes:
        // iterate over all blocks to find which nodes dominate this one
        for (auto &CurrBB : *Cfg.get()) {
          if (DomTree.properlyDominates(CurrBB, ReturnBB)) {
            Stmt *TStmt = CurrBB->getTerminatorStmt();

            Expr *DRE = nullptr;
            Expr *Cond = nullptr;

            // IF Stmt
            if (IfStmt *IfCheck = dyn_cast_or_null<IfStmt>(TStmt)) {
              Cond = IfCheck->getCond();
            }

            // While Stmt
            else if (WhileStmt *WhileCheck =
                         dyn_cast_or_null<WhileStmt>(TStmt)) {
              Cond = WhileCheck->getCond();
            }

            if (Cond) {
              // I: cond: x != NULL
              if (BinaryOperator *BinaryOp = dyn_cast<BinaryOperator>(Cond)) {
                // we only care about !=
                if (BinaryOp->getOpcode() == BinaryOperator::Opcode::BO_NE) {
                  if (isNULLExpr(BinaryOp->getLHS(), *Context)) {
                    DRE = BinaryOp->getRHS();

                  } else if (isNULLExpr(BinaryOp->getRHS(), *Context)) {
                    DRE = BinaryOp->getLHS();
                  }
                }
              }

              // II: cond: !x
              else if (UnaryOperator *UnaryOp = dyn_cast<UnaryOperator>(Cond)) {
                // we only care about !
                if (UnaryOp->getOpcode() == UnaryOperator::Opcode::UO_LNot) {
                  DRE = UnaryOp->getSubExpr();
                }
              }
            }

            if (DRE && Cond) {
              const DeclRefExpr *CheckedDRE = getDeclRefExpr(DRE);
              const auto *CheckedNamedDecl =
                  CheckedDRE->getFoundDecl()->getUnderlyingDecl();

              // check this against the NamedDecl for the return stmt
              if (ReturnNamedDecl == CheckedNamedDecl) {

                // now check that there was no assignment in between the return and
                // the error check
                bool IsUpdated = isUpdatedInPostDominators(
                    ReturnNamedDecl, *CurrBB, &CDG.getCFGPostDomTree(),
                    *Cfg.get());

                // finally, note the guarding statement
                if (!IsUpdated) {
                  llvm::errs() << "not updated, adding error guarding stmt\n";
                  Info.addErrorGuardingStmt(FID, TStmt, Context);
                }
              }
            }
          }
        }
      }
    }
  }
  return true;
}
