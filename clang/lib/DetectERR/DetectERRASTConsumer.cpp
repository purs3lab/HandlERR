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
#include "clang/DetectERR/FiFuzzVisitors.h"
#include "clang/DetectERR/GotoVisitors.h"
#include "clang/DetectERR/ReturnVisitors.h"
#include "clang/DetectERR/ThrowVisitors.h"
#include "clang/DetectERR/Utils.h"

using namespace llvm;
using namespace clang;

void DetectERRASTConsumer::HandleTranslationUnit(ASTContext &C) {
  llvm::errs() << ">>>> inside HandleTranslationUnit\n";

  TranslationUnitDecl *TUD = C.getTranslationUnitDecl();

  if (Opts.Mode == Mode::Fifuzz) {
    fifuzzMode(C, TUD);

  } else {
    normalMode(C, TUD);
  }
}

void DetectERRASTConsumer::handleNamespaceDecl(
    ASTContext &C, const clang::NamespaceDecl *ND,
    const std::set<std::string> &EHFList) {
  for (const auto &D : ND->decls()) {
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      handleFuncDecl(C, FD, EHFList);

    } else if (const NamespaceDecl *ND = dyn_cast_or_null<NamespaceDecl>(D)) {
      // nested namespace
      handleNamespaceDecl(C, ND, EHFList);
    }
  }
}

void DetectERRASTConsumer::handleFuncDeclFifuzz(ASTContext &C,
                                                const clang::FunctionDecl *FD) {
  // TODO
  FullSourceLoc FL = C.getFullLoc(FD->getBeginLoc());
  if (FL.isValid() && FD->hasBody() && FD->isThisDeclarationADefinition()) {
    FuncId FID = getFuncID(FD, &C);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Handling function:" << FID.first << "\n";
      llvm::outs().flush();
      FL.dump();
    }

    std::unique_ptr<CFG> Cfg =
        CFG::buildCFG(nullptr, FD->getBody(), &C, CFG::BuildOptions());
    if (!Cfg.get()) {
      errs() << "[!] Failed to build CFG for function (will be skipped): "
             << FID.first << "\n";
      return;
    }

    FiFuzzVisitor EHFCV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running FiFuzz Visitor.\n";
      llvm::outs().flush();
    }
    EHFCV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    if (Opts.Verbose) {
      llvm::outs() << "[+] Finished handling function:" << FID.first << "\n";
      llvm::outs().flush();
    }
  }
}

void DetectERRASTConsumer::handleFuncDecl(
    ASTContext &C, const clang::FunctionDecl *FD,
    const std::set<std::string> &EHFList) {

  FullSourceLoc FL = C.getFullLoc(FD->getBeginLoc());
  if (FL.isValid() && FD->hasBody() && FD->isThisDeclarationADefinition()) {
    FuncId FID = getFuncID(FD, &C);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Handling function:" << FID.first << "\n";
      llvm::outs().flush();
      FL.dump();
    }

    // tmp: @shank
    std::unique_ptr<CFG> Cfg =
        CFG::buildCFG(nullptr, FD->getBody(), &C, CFG::BuildOptions());
    if (!Cfg.get()) {
      errs() << "[!] Failed to build CFG for function (will be skipped): "
             << FID.first << "\n";
      return;
    }

    // Return NULL visitor.
    ReturnNullVisitor RNV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return NULL handler.\n";
      llvm::outs().flush();
    }
    RNV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return Negative Number Visitor
    ReturnNegativeNumVisitor RNegV(&C, Info, const_cast<FunctionDecl *>(FD),
                                   FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return negative value handler.\n";
      llvm::outs().flush();
    }
    RNegV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return 0 visitor
    ReturnZeroVisitor RZV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return zero handler.\n";
      llvm::outs().flush();
    }
    RZV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return val visitor
    ReturnValVisitor RVV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running return val handler.\n";
      llvm::outs().flush();
    }
    RVV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // EHF Call Visitor
    EHFCallVisitor EHFCV(&C, Info, const_cast<FunctionDecl *>(FD), FID,
                         EHFList);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running EHF call handler.\n";
      llvm::outs().flush();
    }
    EHFCV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Return Early Visitor
    ReturnEarlyVisitor REV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running ReturnEarlyVisitor call handler.\n";
      llvm::outs().flush();
    }
    REV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Goto Visitor
    GotoVisitor GV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running GotoVisitor call handler.\n";
      llvm::outs().flush();
    }
    GV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    // Throw Visitor
    ThrowVisitor TV(&C, Info, const_cast<FunctionDecl *>(FD), FID);
    if (Opts.Verbose) {
      llvm::outs() << "[+] Running ThrowVisitor call handler.\n";
      llvm::outs().flush();
    }
    TV.TraverseDecl(const_cast<FunctionDecl *>(FD));

    if (Opts.Verbose) {
      llvm::outs() << "[+] Finished handling function:" << FID.first << "\n";
      llvm::outs().flush();
    }
  }
}

void DetectERRASTConsumer::normalMode(ASTContext &C, TranslationUnitDecl *TUD) {
  // Normal Mode
  llvm::errs() << "[>] EHF computation start\n";

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

          // tmp: @shank
          // errs() << "Analyzing function: " << FnName << "\n";
          // FL.dump();

          // check that a source level CFG can actually be built by clang for this function,
          // else skip it
          std::unique_ptr<CFG> Cfg =
              CFG::buildCFG(nullptr, FD->getBody(), &C, CFG::BuildOptions());
          if (!Cfg.get()) {
            errs() << "[!] Failed to build CFG for function (will be skipped): "
                   << FnName << "\n";
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

  llvm::errs() << "[>] EHF computation end\n";
  llvm::errs() << "[>] EFList: \n";
  for (auto it = EHFList.begin(); it != EHFList.end(); it++) {
    llvm::errs() << "--- " << *it << "\n";
  }

  // Iterate through all function declarations.
  for (const auto &D : TUD->decls()) {
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      handleFuncDecl(C, FD, EHFList);

    } else if (const NamespaceDecl *ND = dyn_cast_or_null<NamespaceDecl>(D)) {
      // useful for c++
      handleNamespaceDecl(C, ND, EHFList);
    }
  }
}

void DetectERRASTConsumer::fifuzzMode(ASTContext &C, TranslationUnitDecl *TUD) {
  for (const auto &D : TUD->decls()) {
    if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(D)) {
      handleFuncDeclFifuzz(C, FD);
    }
    // We do nothing for C++, as Fifuzz doesnt handle it either
  }
}
