//=--DetectERR.cpp------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implementation of various method in DetectERR.h
//
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/DetectERR.h"
#include "clang/DetectERR/ProjectInfo.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "llvm/Support/TargetSelect.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/VerifyDiagnosticConsumer.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "llvm/Support/TargetSelect.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Signals.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

template <typename T, typename V>
class GenericAction : public ASTFrontendAction {
public:
  GenericAction(V &I) : Info(I) {}

  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile) {
    return std::unique_ptr<ASTConsumer>(new T(Info, &Compiler.getASTContext()));
  }

private:
  V &Info;
};

template <typename T>
std::unique_ptr<FrontendActionFactory>
newFrontendActionFactoryA(ProjectInfo &I) {
  class ArgFrontendActionFactory : public FrontendActionFactory {
  public:
    explicit ArgFrontendActionFactory(ProjectInfo &I) : Info(I) {}

    std::unique_ptr<FrontendAction> create() override {
      return std::unique_ptr<FrontendAction>(new T(Info));
    }

  private:
    ProjectInfo &Info;
  };

  return std::unique_ptr<FrontendActionFactory>(
      new ArgFrontendActionFactory(I));
}

DetectERRInterface::DetectERRInterface(const struct DetectERROptions &DEopt,
                     const std::vector<std::string> &SourceFileList,
                     clang::tooling::CompilationDatabase *CompDB) {

  DErrOptions = DEopt;
  SourceFiles = SourceFileList;
  CurrCompDB = CompDB;
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmPrinters();
  llvm::InitializeAllAsmParsers();
}

bool DetectERRInterface::parseASTs() {
  bool RetVal = false;
  auto *Tool = new ClangTool(*CurrCompDB, SourceFiles);

  std::unique_ptr<ToolAction> ConstraintTool = newFrontendActionFactoryA<
      GenericAction<DetectERRASTConsumer,
                    ProjectInfo>>(this->PInfo);

  if (ConstraintTool) {
    Tool->run(ConstraintTool.get());
    RetVal = true;
  } else {
    llvm_unreachable("No action");
  }

  return RetVal;
}
