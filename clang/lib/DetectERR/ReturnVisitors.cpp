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
// #include "clang/DetectERR/VisitorUtils.h"

/// H04 - if a "return NULL" statement is control dependent upon one or more
/// "if" checks
bool ReturnNullVisitor::VisitReturnStmt(ReturnStmt *ReturnST) {
  if (FnDecl->getReturnType()->isPointerType()) {
    CFGBlock *ReturnBB;
    if (isNULLExpr(ReturnST->getRetValue(), *Context)) {
      if (StMap.find(ReturnST) != StMap.end()) {
        ReturnBB = StMap[ReturnST];

        // collect all checks with their CFGBlocks into an array
        // do one round of bubble sort so that the one CFGBlock that is
        // dominated by all others is found at position 0
        // the one at postion 0 is the "inner" check and all others are
        // "outer" checks
        std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
        collectChecks(Checks, *ReturnBB, &CDG);
        removeChecksUsingParams(Checks, *FnDecl);
        if (!Checks.empty()) {
          // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
          // addErrorGuards(Checks, ReturnST);
          auto [check, level] = getImmediateControlDependentCheck(
              Checks, ReturnST, &CDG, Context->getSourceManager());
          addErrorGuard(check, ReturnST, level);
        }
      }
    }
  }
  return true;
}

/// H02
bool ReturnNegativeNumVisitor::VisitReturnStmt(ReturnStmt *ReturnST) {
  if (FnDecl->getReturnType()->isIntegerType()) {
    CFGBlock *CurBB;

    // tmp: shank
    // if (FnDecl->getNameAsString() == "file_size" ||
    //     FnDecl->getNameAsString() == "retneg1" ||
    //     FnDecl->getNameAsString() == "retneg2") {
    //   errs() << "=======================================================\n";

    //   ReturnST->dumpColor();

    //   errs() << "=======================================================\n";

    //   if (ReturnST->getRetValue()) {
    //     ReturnST->getRetValue()->dumpColor();
    //   } else {
    //     errs() << "ReturnST->getRetValue() is null\n";
    //   }

    //   errs() << "=======================================================\n";
    // }

    if (ReturnST->getRetValue() &&
        isNegativeNumber(ReturnST->getRetValue(), *Context)) {
      if (StMap.find(ReturnST) != StMap.end()) {
        CurBB = StMap[ReturnST];
        std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
        collectChecks(Checks, *CurBB, &CDG);
        removeChecksUsingParams(Checks, *FnDecl);
        // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
        // addErrorGuards(Checks, ReturnST);
        if (!Checks.empty()) {
          auto [check, level] = getImmediateControlDependentCheck(
              Checks, ReturnST, &CDG, Context->getSourceManager());
          addErrorGuard(check, ReturnST, level);
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
bool ReturnZeroVisitor::VisitReturnStmt(ReturnStmt *ReturnST) {
  // is the return type a pointer type?
  if (FnDecl->getReturnType()->isPointerType()) {
    CFGBlock *CurBB;
    if (isZero(ReturnST->getRetValue(), *Context)) {
      CurBB = StMap[ReturnST];
      std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
      collectChecks(Checks, *CurBB, &CDG);
      removeChecksUsingParams(Checks, *FnDecl);
      // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
      // addErrorGuards(Checks, ReturnST);
      if (!Checks.empty()) {
        auto [check, level] = getImmediateControlDependentCheck(
            Checks, ReturnST, &CDG, Context->getSourceManager());
        addErrorGuard(check, ReturnST, level);
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
///     NOTE: for now we should limit this to only checks against NULL
bool ReturnValVisitor::VisitReturnStmt(ReturnStmt *ReturnST) {
  CFGBlock *ReturnBB;
  if (FnDecl->getReturnType()->isPointerType()) { // return type = pointer
    if (!isNULLExpr(ReturnST->getRetValue(), *Context)) { // not return NULL
      // find all the blocks that dominate the exit block (containing the return stmt)
      // for each of these dominating blocks, check if their terminator stmt
      // is a IfStmt and the condition of that IfStmt is a NULL check
      // against the value being returned

      // return stmt is a 'return var'
      if (isDeclExpr(ReturnST->getRetValue())) { // return val
        ReturnBB = StMap[ReturnST];

        // store the underlying NamedDecl for comparing against later
        const Expr *E = ReturnST->getRetValue();
        const DeclRefExpr *ReturnDRE = getDeclRefExpr(E);
        const NamedDecl *ReturnNamedDecl =
            ReturnDRE->getFoundDecl()->getUnderlyingDecl();

        // find dominator nodes:
        // iterate over all blocks to find which nodes dominate this one
        for (auto &CurrBB : *Cfg.get()) {
          if (DomTree.properlyDominates(CurrBB, ReturnBB)) {
            // skip the blocks on which the Return is Control Dependent
            auto &CDNodes = CDG.getControlDependencies(ReturnBB);
            bool IsReturnCtrlDepOnCurBb = false;
            for (auto &CDNode : CDNodes) {
              if (CDNode == CurrBB) {
                IsReturnCtrlDepOnCurBb = true;
                break;
              }
            }
            if (IsReturnCtrlDepOnCurBb) {
              continue;
            }

            Stmt *TStmt = CurrBB->getTerminatorStmt();

            Expr *DRE = nullptr;

            Expr *Cond = getCondFromCheckStmt(TStmt);

            if (Cond) {
              // I: cond: x != NULL
              if (BinaryOperator *BinaryOp = dyn_cast<BinaryOperator>(Cond)) {
                // we only care about '!='
                if (BinaryOp->getOpcode() == BinaryOperator::Opcode::BO_NE) {

                  Expr *LHS = BinaryOp->getLHS();
                  Expr *RHS = BinaryOp->getRHS();

                  if (isNULLExpr(LHS, *Context)) {
                    DRE = (Expr *)getDeclRefExpr(RHS);

                  } else if (isNULLExpr(RHS, *Context)) {
                    DRE = (Expr *)getDeclRefExpr(LHS);

                  } else {
                    // we dont do anything!
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
            }

            if (DRE && Cond) {
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
                    SourceRange CurrSR = TStmt->getSourceRange();
                    SourceRange ReturnSTSR = ReturnST->getSourceRange();
                    GuardLevel Lvl = GuardLevel::Default;
                    if (CurrSR.fullyContains(ReturnSTSR)) {
                      Lvl = GuardLevel::Inner;
                    }

                    return false;
                    Info.addErrorGuardingStmt(FID, TStmt, ReturnST, Context,
                                              Heuristic, Lvl);
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

/// H07 - early return based on a check for a function having void return type
///
/// Checks for conditions like:
///
///     void myfunc(<args>) {
///         ...
///         if (<cond>){
///             return;         // <---- early return
///         }
///         ...
///         ...
///         return; // <----
///     }
///
/// Conditions:
/// - function has void return type
/// - a check is directly followed by an early return
bool ReturnEarlyVisitor::VisitReturnStmt(ReturnStmt *ReturnST) {
  if (FnDecl->getReturnType()->isVoidType()) { // return type = void
    // - BB for the return statement does not contain any other statements
    // - the immediate dominator BB has a terminator statement that is a check
    //    (if, while, switch)

    CFGBlock *ReturnBB = StMap[ReturnST];
    if (ReturnBB && ReturnBB->size() == 1) {
      std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
      collectChecks(Checks, *ReturnBB, &CDG);
      removeChecksUsingParams(Checks, *FnDecl);
      // sortIntoInnerAndOuterChecks(Checks, &CDG, Context->getSourceManager());
      // addErrorGuards(Checks, ReturnST);
      if (!Checks.empty()) {
        auto [check, level] = getImmediateControlDependentCheck(
            Checks, ReturnST, &CDG, Context->getSourceManager());
        addErrorGuard(check, ReturnST, level);
      }
    }
  }
  return true;
}
