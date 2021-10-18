//=--DetectERRASTConsumer.cpp-------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of visitor methods for the DetectERRASTConsumer class. These
// visitors run various techniques for each function.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/Utils.h"
#include "clang/Analysis/CFG.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

// Condition guarding return NULL is error guarding.
class ReturnNullVisitor : public RecursiveASTVisitor<ReturnNullVisitor> {
public:
  explicit ReturnNullVisitor(ASTContext *Context, ProjectInfo &I,
                             FunctionDecl *FD,
                             FuncId &FnID)
      : Context(Context), Info(I), FnDecl(FD), FID(FnID),
        Cfg(CFG::buildCFG(nullptr, FD->getBody(),
                          Context, CFG::BuildOptions())),
        CDG(Cfg.get()) {
    for (auto *CBlock : *(Cfg.get())) {
      for (auto &CfgElem : *CBlock) {
        if (CfgElem.getKind() == clang::CFGElement::Statement) {
          const Stmt *TmpSt = CfgElem.castAs<CFGStmt>().getStmt();
          StMap[TmpSt] = CBlock;
        }
      }
    }
  }

  bool VisitReturnStmt(ReturnStmt *S) {
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
    return true;
  }

private:
  ASTContext *Context;
  ProjectInfo &Info;
  FunctionDecl *FnDecl;
  FuncId &FID;

  std::unique_ptr<CFG> Cfg;
  ControlDependencyCalculator CDG;
  std::map<const Stmt *, CFGBlock *> StMap;
};

void DetectERRASTConsumer::HandleTranslationUnit(ASTContext &C) {
  TranslationUnitDecl *TUD = C.getTranslationUnitDecl();

  // Iterate through all function declarations.
  for (const auto &D : TUD->decls()) {
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      // Is this a function?
      handleFuncDecl(C, FD);
    }
  }
  return;
}

void DetectERRASTConsumer::handleFuncDecl(ASTContext &C,
                                          const clang::FunctionDecl *FD) {

  FullSourceLoc FL = C.getFullLoc(FD->getBeginLoc());
  if (FL.isValid() && FD->hasBody() && FD->isThisDeclarationADefinition()) {
    FuncId FID = getFuncID(FD, &C);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Handling function:" << FID.first << "\n";
    }
    // Call Return NULL visitor.
    ReturnNullVisitor RNV(&C, Info, const_cast<FunctionDecl*>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return NULL handler.\n";
    }
    RNV.TraverseDecl(const_cast<FunctionDecl*>(FD));

    if (Opts.Verbose) {
      llvm::outs() << "[+] Finished handling function:" << FID.first << "\n";
    }
  }
}