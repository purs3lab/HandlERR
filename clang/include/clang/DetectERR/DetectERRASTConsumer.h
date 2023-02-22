//=--DetectERRASTConsumer.h---------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_DETECTERRASTCONSUMER_H
#define LLVM_CLANG_DETECTERR_DETECTERRASTCONSUMER_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/Decl.h"
#include "clang/DetectERR/DetectERR.h"
#include "clang/DetectERR/ProjectInfo.h"

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
  void handleFuncDecl(ASTContext &C, const FunctionDecl *FD,
                      const std::set<std::string> &EHFList);
  // This function takes care of calling various helper functions
  // on the given function decl.
  void handleNamespaceDecl(ASTContext &C, const NamespaceDecl *ND,
                           const std::set<std::string> &EHFList);

  // does the stuff  in normal mode
  void normalMode(ASTContext &C, TranslationUnitDecl *TUD);

  // does the stuff in fifuzz mode
  void fifuzzMode(ASTContext &C, TranslationUnitDecl *TUD);
  void handleFuncDeclFifuzz(ASTContext &C, const FunctionDecl *FD);

  ProjectInfo &Info;
  struct DetectERROptions Opts;
};

#endif //LLVM_CLANG_DETECTERR_DETECTERRASTCONSUMER_H
