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
  //  llvm::errs() << "processing fn: " << FnDecl->getNameInfo().getAsString()
  //               << '\n';
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
            if (dyn_cast_or_null<IfStmt>(TStmt) ||
                dyn_cast_or_null<WhileStmt>(TStmt) ||
                dyn_cast_or_null<SwitchStmt>(TStmt)) {
              Info.addErrorGuardingStmt(FID, TStmt, Context, Heuristic);
            }
          }
        }
      }
    }
  }
  return true;
}

/// H02
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
            if (dyn_cast_or_null<IfStmt>(TStmt) ||
                dyn_cast_or_null<WhileStmt>(TStmt) ||
                dyn_cast_or_null<SwitchStmt>(TStmt)) {
              Info.addErrorGuardingStmt(FID, TStmt, Context, Heuristic);
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
          if (dyn_cast_or_null<IfStmt>(TStmt) ||
              dyn_cast_or_null<WhileStmt>(TStmt) ||
              dyn_cast_or_null<SwitchStmt>(TStmt)) {
            Info.addErrorGuardingStmt(FID, TStmt, Context, Heuristic);
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
              //              llvm::errs() << "IfStmt\n";
              Cond = IfCheck->getCond();
            }

            // While Stmt
            else if (WhileStmt *WhileCheck =
                         dyn_cast_or_null<WhileStmt>(TStmt)) {
              //              llvm::errs() << "WhileStmt\n";
              Cond = WhileCheck->getCond();
            }

            // Switch Stmt
            else if (SwitchStmt *SwitchCheck =
                         dyn_cast_or_null<SwitchStmt>(TStmt)) {
              //              llvm::errs() << "WhileStmt\n";
              Cond = SwitchCheck->getCond();
            }

            if (Cond) {
              // I: cond: x != something OR x == something
              if (BinaryOperator *BinaryOp = dyn_cast<BinaryOperator>(Cond)) {
                // we only care about '!=' OR '==
                if (BinaryOp->getOpcode() == BinaryOperator::Opcode::BO_NE ||
                    BinaryOp->getOpcode() == BinaryOperator::Opcode::BO_EQ ||
                    BinaryOp->getOpcode() == BinaryOperator::Opcode::BO_Cmp) {

                  Expr *LHS = BinaryOp->getLHS();
                  Expr *RHS = BinaryOp->getRHS();

                  if (hasDeclRefExprTo(LHS, ReturnNamedDecl)) {
                    DRE = (Expr *)getDeclRefExpr(LHS);

                  } else if (hasDeclRefExprTo(RHS, ReturnNamedDecl)) {
                    DRE = (Expr *)getDeclRefExpr(RHS);

                  } else if (isDerefToDeclRef(LHS, ReturnNamedDecl)) {
                    DRE = getDerefExpr(LHS);

                  } else if (isDerefToDeclRef(RHS, ReturnNamedDecl)) {
                    DRE = getDerefExpr(RHS);
                  }
                }
              }

              // II: cond: !x
              else if (UnaryOperator *UnaryOp = dyn_cast<UnaryOperator>(Cond)) {
                // we only care about '!'
                if (UnaryOp->getOpcode() == UnaryOperator::Opcode::UO_LNot) {
                  DRE = UnaryOp->getSubExpr();
                }
              }

              // III: cond: x
            }

            if (DRE && Cond) {
              // llvm::errs() << "TStmt loc: ";
              // SourceRange srcRange = TStmt->getSourceRange();
              // srcRange.dump(Context->getSourceManager());

              const DeclRefExpr *CheckedDRE = getDeclRefExpr(DRE);

              if (CheckedDRE) {
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
                    Info.addErrorGuardingStmt(FID, TStmt, Context, Heuristic);
                  }
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
