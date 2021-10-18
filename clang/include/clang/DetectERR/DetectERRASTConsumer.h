//=--DetectERRASTConsumer.h---------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/ProjectInfo.h"
#include "clang/DetectERR/DetectERR.h"
#include "clang/AST/ASTConsumer.h"

#ifndef LLVM_CLANG_DETECTERR_DETECTERRASTCONSUMER_H
#define LLVM_CLANG_DETECTERR_DETECTERRASTCONSUMER_H

using namespace clang;

// The main consumer that runs various techniques on each function.
class DetectERRASTConsumer : public clang::ASTConsumer {
public:
  explicit DetectERRASTConsumer(ProjectInfo &I, struct DetectERROptions DOpts,
                                ASTContext *C)
      : Info(I), Opts(DOpts) {}

  void HandleTranslationUnit(ASTContext &) override;

private:
  // This function takes care of calling various helper functions
  // on the given function decl.
  void handleFuncDecl(ASTContext &C, const FunctionDecl *FD);
  ProjectInfo &Info;
  struct DetectERROptions Opts;
};

#endif //LLVM_CLANG_DETECTERR_DETECTERRASTCONSUMER_H
