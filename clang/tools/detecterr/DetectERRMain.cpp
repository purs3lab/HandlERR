//=----------DetectERRMain.cpp------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// detecterr tool
//
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/DetectERR.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/VerifyDiagnosticConsumer.h"
#include "clang/Tooling/ArgumentsAdjusters.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetSelect.h"

using namespace clang::driver;
using namespace clang::tooling;
using namespace clang;
using namespace llvm;

static cl::OptionCategory DetectERRCategory("detecterr options");

static const char *HelpOverview =
    "detecterr: Automatically detect error handling if statements.\n";
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

static cl::opt<bool> OptVerbose("verbose",
                                cl::desc("Print verbose information"),
                                cl::init(false), cl::cat(DetectERRCategory));

static cl::opt<std::string>
    OptOutputJson("output",
                  cl::desc("Path to the file where all the stats "
                           "will be dumped as json"),
                  cl::init("ErrHandlingBlocks.json"),
                  cl::cat(DetectERRCategory));

static cl::opt<Mode>
    OptMode("mode", cl::desc("Normal or Fifuzz mode"),
            cl::values(clEnumValN(Mode::Normal, "normal", "normal mode"),
                       clEnumValN(Mode::Fifuzz, "fifuzz", "fifuzz mode")),
            cl::init(Mode::Normal), cl::cat(DetectERRCategory));

int main(int argc, const char **argv) {
  struct DetectERROptions DOpt;

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
      CommonOptionsParser::create(argc, (const char **)(argv),
                                  DetectERRCategory, cl::ZeroOrMore,
                                  HelpOverview);

  if (!ExpectedOptionsParser) {
    llvm::errs() << "detecterr: Error(s) parsing command-line arguments:\n"
                 << llvm::toString(ExpectedOptionsParser.takeError());
    return 1;
  }

  CommonOptionsParser &OptionsParser = *ExpectedOptionsParser;
  // Specifying cl::ZeroOrMore rather than cl::OneOrMore and then checking this
  // here lets us give a better error message than the default "Must specify at
  // least 1 positional argument".
  if (OptionsParser.getSourcePathList().empty()) {
    llvm::errs() << "detecterr: Error: No source files specified.\n"
                 << "See: " << argv[0] << " --help\n";
    return 1;
  }

  // Verbose flag.
  DOpt.Verbose = OptVerbose;
  // Mode flag.
  DOpt.Mode = OptMode;

  DetectERRInterface DErrInf(DOpt, OptionsParser.getSourcePathList(),
                             &(OptionsParser.getCompilations()));

  if (DErrInf.parseASTs()) {
    llvm::outs() << "[+] Successfully parsed ASTs.\n";
    llvm::outs().flush();
  } else {
    llvm::outs() << "[-] Unable to parse ASTs.\n";
    llvm::outs().flush();
  }

  llvm::outs() << "[+] Trying to write error handling information to:"
               << OptOutputJson << ".\n";
  llvm::outs().flush();
  std::error_code Ec;
  llvm::raw_fd_ostream OutputJson(OptOutputJson, Ec);
  if (!OutputJson.has_error()) {
    DErrInf.dumpInfo(OutputJson);
    OutputJson.close();
    llvm::outs() << "[+] Finished writing to given output file.\n";
    llvm::outs().flush();
  } else {
    llvm::outs() << "[-] Error trying to open file:" << OptOutputJson << ".\n";
    llvm::outs().flush();
    return -1;
  }
  return 0;
}
