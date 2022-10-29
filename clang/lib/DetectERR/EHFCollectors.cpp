//=--EHFVisitor.cpp-------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This implementation of methods in EHFVisitor - various visitors
// dealing with collecting Error Handling Functions
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/EHFCollectors.h"
#include "clang/DetectERR/Utils.h"

/// - a call to a known exit function is the last statement of the function
///
/// this statement should not be control dependent on anything, in other words
/// once the function containing this call starts, it would definitely end
/// up calling the exit function.
bool EHFCategoryOneCollector::VisitCallExpr(CallExpr *S) {
  CFGBlock *CurrBB = nullptr;
  auto Parents = Context->getParents(*S);
  if (!Parents.empty()) {
    auto Parent = Parents[0];
    const Stmt *St = nullptr;
    while (!Parent.get<Stmt>()) {
      Parents = Context->getParents(Parent);

      // this happens when the call is a part of constructor initializer list
      // we can safely skip this CallExpr
      if (Parents.empty()) {
        return true;
      }

      Parent = Parents[0];
    }
    St = Parent.get<Stmt>();
    if (St) {
      CurrBB = StMap[S];
    }
  }

  //  if (CurrBB) {
  Decl *CalledDecl = S->getCalleeDecl();
  if (FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CalledDecl)) {
    std::string calledFnName = FD->getNameInfo().getAsString();
    // - a call to a known exit function is the last statement of the function
    // - and also this call should not be control dependent on anything
    if (EHFList_->find(calledFnName) != EHFList_->end()) {
      // CONDITION I
      // the call stmt should be the last statement of the function
      // in other words, it should:
      // - not have any post dominator nodes
      // AND
      // - in the currBB, it should be the last statement
      bool isLastStmt = false;
      // check if CurrBB has any post-dominators
      if (!hasPostDominators(*CurrBB, &CDG.getCFGPostDomTree(), *Cfg.get())) {
        if (isLastStmtInBB(*S, *CurrBB)) {
          isLastStmt = true;
        }
      }

      // CONDITION II
      // this call should not be control dependent on anything
      bool willDefinitelyHappen = false;
      if (CDG.getControlDependencies(CurrBB).empty()) {
        willDefinitelyHappen = true;
      }

      if (isLastStmt && willDefinitelyHappen) {
        // now we are certain that this function is an error handling function (cat-1)
        llvm::errs()
            << "adding function '" << FnDecl->getNameInfo().getAsString()
            << "' to EHFList_ as Cat-II due to VisitCallExpr(). It calls '"
            << calledFnName << "'\n";
        EHFList_->insert(FnDecl->getNameInfo().getAsString());
      }
    }
  }
  //    }

  return true;
}

/// - the function has a 'noreturn' attribute
bool EHFCategoryOneCollector::VisitFunctionDecl(FunctionDecl *FD) {
  if (FD->isNoReturn()) {
    llvm::errs() << "adding function '" << FD->getNameInfo().getAsString()
                 << "' to EHFList_ as Cat-I\n";
    EHFList_->insert(FD->getNameInfo().getAsString());
  }

  return true;
}

/// EHF Cat 2 functions are identified based on the following heuristics
///
/// 1. name contains "err"
/// 2. return type is void
/// 3. writes to stderr that is not control dependent on anything
/// 4. function is short (say 10 lines)
///
/// either (1) || (2 && 3 && 4) is the check that is done to decide if the given
/// function is an EHF Cat2 Function
bool EHFCategoryTwoCollector::VisitFunctionDecl(FunctionDecl *FD) {
  /// 1. name contains "err"
  std::string FnName = FD->getNameInfo().getAsString();
  if (FnName.find("err") != std::string::npos) {
    llvm::errs() << "adding function '" << FD->getNameInfo().getAsString()
                 << "' to EHFList_ as Cat-II\n";
    EHFList_->insert(FnDecl->getNameInfo().getAsString());
    return true;
  }

  /// 2. return type is void
  bool isVoidReturn = FnDecl->getReturnType()->isVoidType();

  /// 3. writes to stderr that is not control dependent on anything
  // there are many functions that write to stderr:
  // fprintf(stderr, ...)
  // vfprintf(stderr, ...)
  // dprintf(STDERR_FILENO, ...)
  // fwrite(..., stderr)
  bool writesToStderr = false;
  const Stmt *WritingStmt = nullptr;
  // iterate over all the statements
  for (auto it = StMap.begin(); it != StMap.end(); it++) {
    const Stmt *CurrStmt = it->first;
    // check if the statement is a CallExpr
    if (const CallExpr *CE = dyn_cast_or_null<CallExpr>(CurrStmt)) {
      // check that the function called is one that would write to stderr
      const Decl *CalleeDecl = CE->getCalleeDecl();
      if (const FunctionDecl *CalledFD =
              dyn_cast_or_null<FunctionDecl>(CalleeDecl)) {
        std::string CalledFnName = CalledFD->getNameInfo().getAsString();

        if (CalledFnName == "fprintf" || CalledFnName == "vfprintf") {
          // fprintf(stderr, ...)
          // vfprintf(stderr, ...)
          if (const Expr *Arg0 = CE->getArg(0)) {
            const DeclRefExpr *DRE = getDeclRefExpr(Arg0);

            if (DRE && DRE->getNameInfo().getAsString() == "stderr") {
              writesToStderr = true;
              WritingStmt = CurrStmt;
              break;
            }
          }

        } else if (CalledFnName == "dprintf") {
          // dprintf(STDERR_FILENO, ...)
          if (const Expr *Arg0 = CE->getArg(0)) {
            if (isInt(2, Arg0, *Context)) {
              writesToStderr = true;
              WritingStmt = CurrStmt;
              break;
            }
          }

        } else if (CalledFnName == "fwrite") {
          // fwrite(..., stderr)
          int nArgs = CE->getNumArgs();
          const Expr *LastArg = CE->getArg(nArgs - 1);
          const DeclRefExpr *DRE = getDeclRefExpr(LastArg);

          if (DRE && DRE->getNameInfo().getAsString() == "stderr") {
            writesToStderr = true;
            WritingStmt = CurrStmt;
            break;
          }
        }
      }
    }
  }

  // now that we know that there is something written to stderr,
  // check that this write is not control dependent on anything
  // this would mean that the function always writes to stderr and hence
  // is an error logging function (EHF Cat2 function)
  bool isIndependentWriteToStderr = writesToStderr;
  if (writesToStderr) {
    CFGBlock *CurBB = StMap[WritingStmt];
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
          isIndependentWriteToStderr = false;
          break;
        }
      }
    }
  }

  /// 4. function is short (say 10 lines)
  bool isShortFunction = (StMap.size() <= 10);

  // 2 && 3 && 4 -> EHF Cat 2 function
  if (isVoidReturn && isIndependentWriteToStderr && isShortFunction) {
    llvm::errs() << "adding function '" << FD->getNameInfo().getAsString()
                 << "' to EHFList_ as Cat-II\n";
    EHFList_->insert(FnDecl->getNameInfo().getAsString());
  }

  return true;
}
