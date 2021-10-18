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
#include "clang/DetectERR/ReturnVisitors.h"
#include "clang/Analysis/CFG.h"

using namespace llvm;
using namespace clang;

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

    ReturnNegativeNumVisitor RNegV(&C, Info, const_cast<FunctionDecl*>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return negative value handler.\n";
    }
    RNegV.TraverseDecl(const_cast<FunctionDecl*>(FD));

    if (Opts.Verbose) {
      llvm::outs() << "[+] Finished handling function:" << FID.first << "\n";
    }
  }
}