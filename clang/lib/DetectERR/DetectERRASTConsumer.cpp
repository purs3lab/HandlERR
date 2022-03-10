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
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/EHFCallVisitors.h"
#include "clang/DetectERR/EHFCollectors.h"
#include "clang/DetectERR/GotoVisitors.h"
#include "clang/DetectERR/ReturnVisitors.h"
#include "clang/DetectERR/Utils.h"

using namespace llvm;
using namespace clang;

void DetectERRASTConsumer::HandleTranslationUnit(ASTContext &C) {
  TranslationUnitDecl *TUD = C.getTranslationUnitDecl();

  // Fixed point computation for exit functions
  // populate EHFList with known error functions.
  std::set<std::string> EHFList;
  EHFList.insert("exit");
  EHFList.insert("abort");

  bool is_changed = true;
  while (is_changed) {
    unsigned num_exit_func = EHFList.size();
    for (const auto &D : TUD->decls()) {
      if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
        FullSourceLoc FL = C.getFullLoc(FD->getBeginLoc());
        if (FL.isValid() && FD->hasBody() &&
            FD->isThisDeclarationADefinition()) {
          std::string FnName = FD->getNameInfo().getAsString();

          // do we already know that this is a exit function?
          if (EHFList.find(FnName) != EHFList.end()) {
            continue;
          }

          // cat 1 exit fn?
          EHFCategoryOneCollector ECVOne(&C, const_cast<FunctionDecl *>(FD),
                                         EHFList);
          ECVOne.TraverseDecl(const_cast<FunctionDecl *>(FD));

          // cat 2 exit fn?
          EHFCategoryTwoCollector ECVTwo(&C, const_cast<FunctionDecl *>(FD),
                                         EHFList);
          ECVTwo.TraverseDecl(const_cast<FunctionDecl *>(FD));
        }
      }
    }
    is_changed = EHFList.size() != num_exit_func;
  }

  llvm::errs() << "EHFList >> \n";
  for (auto Item : EHFList) {
    llvm::errs() << Item << '\n';
  }

  // Iterate through all function declarations.
  for (const auto &D : TUD->decls()) {
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      // Is this a function?
      //      llvm::errs() << "handleFuncDecl for: " << FD->getNameInfo().getAsString()
      //                   << '\n';
      handleFuncDecl(C, FD, EHFList);
    }
  }
  return;
}

void DetectERRASTConsumer::handleFuncDecl(
    ASTContext &C, const clang::FunctionDecl *FD,
    const std::set<std::string> &EHFList) {

  FullSourceLoc FL = C.getFullLoc(FD->getBeginLoc());
  if (FL.isValid() && FD->hasBody() && FD->isThisDeclarationADefinition()) {
    FuncId FID = getFuncID(FD, &C);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Handling function:" << FID.first << "\n";
    }

    // Return NULL visitor.
    ReturnNullVisitor RNV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return NULL handler.\n";
    }
    RNV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return Negative Number Visitor
    ReturnNegativeNumVisitor RNegV(&C, Info, const_cast<FunctionDecl *>(FD),
                                   FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return negative value handler.\n";
    }
    RNegV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return 0 visitor
    ReturnZeroVisitor RZV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return zero handler.\n";
    }
    RZV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return val visitor
    ReturnValVisitor RVV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return val handler.\n";
    }
    RVV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // EHF Call Visitor
    EHFCallVisitor EHFCV(&C, Info, const_cast<FunctionDecl *>(FD), FID,
                         &EHFList);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running EHF call handler.\n";
    }
    EHFCV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return Early Visitor
    ReturnEarlyVisitor REV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running ReturnEarlyVisitor call handler.\n";
    }
    REV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Goto Visitor
    GotoVisitor GV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running GotoVisitor call handler.\n";
    }
    GV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    if (Opts.Verbose) {
      llvm::outs() << "[+] Finished handling function:" << FID.first << "\n";
    }
  }
}