//=--DetectERR.h--------------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// The main interface for invoking DetectERR tool.
// This provides various methods that can be used to access different
// aspects of the DetectERR tool.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_DETECTERR_H
#define LLVM_CLANG_DETECTERR_DETECTERR_H

#include "clang/DetectERR/ProjectInfo.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include <cstdlib>
#include <mutex>

using namespace clang;

enum Mode { Normal, Fifuzz };
std::string modeStr(Mode &Mode);

// Future, if we want to pass some options.
struct DetectERROptions {
  bool Verbose;
  Mode Mode;
};

// The main interface exposed by the DetectERR to interact with the tool.
class DetectERRInterface {
public:
  DetectERRInterface(const struct DetectERROptions &DEopt,
                     const std::vector<std::string> &SourceFileList,
                     clang::tooling::CompilationDatabase *CompDB);

  // Parse the asts of all the source files.
  bool parseASTs();

  void dumpInfo(llvm::raw_ostream &O);

private:
  ProjectInfo PInfo;
  struct DetectERROptions DErrOptions;
  tooling::CommandLineArguments SourceFiles;
  tooling::CompilationDatabase *CurrCompDB;
};

#endif // LLVM_CLANG_DETECTERR_DETECTERR_H
