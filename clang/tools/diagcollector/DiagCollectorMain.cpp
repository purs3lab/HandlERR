//=----------diagcollecter.cpp------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// diagcollecter tool
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/TargetSelect.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Frontend/VerifyDiagnosticConsumer.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "llvm/Support/TargetSelect.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/Signals.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

static cl::OptionCategory DiagCollector("diagcollecter options");

static const char *HelpOverview =
    "diagcollecter: Automatically detect error handling if statements.\n";
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::opt<std::string>
    OptOutputTxt("diag",
               cl::desc("Path to the file where all the warnings and errors "
                            "should be dumped."),
                       cl::init("CompilerDiags.txt"),
                       cl::cat(DiagCollector));


class OutDiagConsumer : public IgnoringDiagConsumer {
public:
  OutDiagConsumer(llvm::raw_ostream &O) : OutputStream(O) { }

  virtual ~OutDiagConsumer() {
  }

  void HandleDiagnostic(DiagnosticsEngine::Level DiagLevel,
                        const Diagnostic &Info) override {
    SmallString<100> Buf;
    SourceManager &SM = Info.getSourceManager();

    Info.FormatDiagnostic(Buf);
    if (DiagLevel == DiagnosticsEngine::Level::Warning) {
      OutputStream << "WARNING;Location:";
      if (Info.getLocation().isValid()) {
        Info.getLocation().print(OutputStream, SM);
        OutputStream << ";";
      }
      OutputStream << Buf.str() << "\n";
    }
    if (DiagLevel == DiagnosticsEngine::Level::Error) {
      OutputStream << "ERROR;Location:";
      if (Info.getLocation().isValid()) {
        Info.getLocation().print(OutputStream, SM);
        OutputStream << ";";
      }
      OutputStream << Buf.str() << "\n";
    }
  }

private:
  llvm::raw_ostream &OutputStream;
};

class GenericAction : public ASTFrontendAction {
public:

  virtual std::unique_ptr<ASTConsumer>
  CreateASTConsumer(CompilerInstance &Compiler, StringRef InFile) {
    return nullptr;
  }
};

class ArgFrontendActionFactory : public FrontendActionFactory {

  std::unique_ptr<FrontendAction> create() override {
    return std::unique_ptr<FrontendAction>(new GenericAction());
  }
};

class DummyAction : public ToolAction {
public:
  DummyAction() {

  }
  virtual ~DummyAction() {

  }
};
int main(int argc, const char **argv) {

  sys::PrintStackTraceOnErrorSignal(argv[0]);

  // Initialize targets for clang module support.
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  // The following code is based on clangTidyMain in
  // clang-tools-extra/clang-tidy/tool/ClangTidyMain.cpp. Apparently every
  // LibTooling-based tool is supposed to duplicate it??
  llvm::Expected<CommonOptionsParser> ExpectedOptionsParser =
      CommonOptionsParser::create(argc, (const char **)(argv), DiagCollector,
                                  cl::ZeroOrMore, HelpOverview);

  if (!ExpectedOptionsParser) {
    llvm::errs() << "diagcollecter: Error(s) parsing command-line arguments:\n"
                 << llvm::toString(ExpectedOptionsParser.takeError());
    return 1;
  }

  CommonOptionsParser &OptionsParser = *ExpectedOptionsParser;
  // Specifying cl::ZeroOrMore rather than cl::OneOrMore and then checking this
  // here lets us give a better error message than the default "Must specify at
  // least 1 positional argument".
  if (OptionsParser.getSourcePathList().empty()) {
    llvm::errs() << "diagcollecter: Error: No source files specified.\n"
                 << "See: " << argv[0] << " --help\n";
    return 1;
  }

  auto *Tool = new ClangTool(OptionsParser.getCompilations(),
                             OptionsParser.getSourcePathList());

  std::error_code Ec;
  llvm::raw_fd_ostream OutputTxt(OptOutputTxt, Ec);
  if (!OutputTxt.has_error()) {
    auto *OD = new OutDiagConsumer(OutputTxt);
    auto *FWD = new ForwardingDiagnosticConsumer(*OD);
    Tool->setDiagnosticConsumer(FWD);
    Tool->run(newFrontendActionFactory<SyntaxOnlyAction>().get());
  } else {
    llvm::outs() << "[-] Error trying to open file:" << OptOutputTxt << ".\n";
    return 1;
  }
  return 0;
}
