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
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/ProjectInfo.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/VerifyDiagnosticConsumer.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetSelect.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

template <typename T, typename V, typename W>
class GenericAction : public ASTFrontendAction {
public:
  GenericAction(V &I, W &Op) : Info(I), Opts(Op) {}

  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile) {
    return std::unique_ptr<ASTConsumer>(
        new T(Info, Opts, &Compiler.getASTContext()));
  }

private:
  V &Info;
  W &Opts;
};

template <typename T>
std::unique_ptr<FrontendActionFactory>
newFrontendActionFactoryA(ProjectInfo &I, struct DetectERROptions &OP) {
  class ArgFrontendActionFactory : public FrontendActionFactory {
  public:
    explicit ArgFrontendActionFactory(ProjectInfo &I,
                                      struct DetectERROptions &OP)
        : Info(I), Opts(OP) {}

    std::unique_ptr<FrontendAction> create() override {
      return std::unique_ptr<FrontendAction>(new T(Info, Opts));
    }

  private:
    ProjectInfo &Info;
    struct DetectERROptions &Opts;
  };

  return std::unique_ptr<FrontendActionFactory>(
      new ArgFrontendActionFactory(I, OP));
}

DetectERRInterface::DetectERRInterface(
    const struct DetectERROptions &DEopt,
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

  if (DErrOptions.Verbose) {
    llvm::errs() << "[>] Parsing ASTs\n";
    llvm::errs() << "[>] Mode:" << modeStr(DErrOptions.Mode) << "\n";
  }

  bool RetVal = false;
  auto *Tool = new ClangTool(*CurrCompDB, SourceFiles);

  std::unique_ptr<ToolAction> ConstraintTool =
      newFrontendActionFactoryA<GenericAction<DetectERRASTConsumer, ProjectInfo,
                                              struct DetectERROptions>>(
          this->PInfo, this->DErrOptions);

  if (ConstraintTool) {
    Tool->run(ConstraintTool.get());
    RetVal = true;
  } else {
    llvm_unreachable("No action");
  }

  return RetVal;
}

void DetectERRInterface::dumpInfo(llvm::raw_ostream &O) {
  PInfo.toJsonString(O);
  // this->PInfo.errCondsToJsonString(O);
}

std::string modeStr(Mode &Mode) {
  switch (Mode) {
  case Normal:
    return "Mode(Normal)";
  case Fifuzz:
    return "Mode(Fifuzz)";
  default:
    llvm::errs() << "unknown mode: " << Mode << '\n';
    exit(EXIT_FAILURE);
  }
}
