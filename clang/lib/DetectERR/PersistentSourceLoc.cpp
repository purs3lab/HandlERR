//=--PersistentSourceLoc.cpp--------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation of the PersistentSourceLoc infrastructure.
//===----------------------------------------------------------------------===//

#include "clang/DetectERR/PersistentSourceLoc.h"

using namespace clang;
using namespace llvm;

PersistentSourceLoc PersistentSourceLoc::mkPSL(const Decl *D,
                                               const ASTContext &C) {
  if (D == nullptr)
    return PersistentSourceLoc();
  SourceLocation SL = C.getSourceManager().getExpansionLoc(D->getLocation());
  return mkPSL(D->getSourceRange(), SL, C);
}

// Create a PersistentSourceLoc for a Stmt.
PersistentSourceLoc PersistentSourceLoc::mkPSL(const Stmt *S,
                                               const ASTContext &Context) {
  if (S == nullptr)
    return PersistentSourceLoc();
  return mkPSL(S->getSourceRange(), S->getBeginLoc(), Context);
}

// Use the PresumedLoc infrastructure to get a file name and expansion
// line and column numbers for a SourceLocation.
PersistentSourceLoc PersistentSourceLoc::mkPSL(clang::SourceRange SR,
                                               SourceLocation SL,
                                               const ASTContext &Context) {
  const SourceManager &SM = Context.getSourceManager();
  // SR.dump(SM);
  PresumedLoc PL = SM.getPresumedLoc(SL);
  // llvm::errs() << "PL.getColumn(): " << PL.getColumn() << "\n";

  // If there is no PresumedLoc, create a nullary PersistentSourceLoc.
  if (!PL.isValid())
    return PersistentSourceLoc();

  SourceLocation ESL = SM.getExpansionLoc(SL);
  // ESL.dump(SM);
  FullSourceLoc FESL = Context.getFullLoc(ESL);
  // FESL.dump();

  assert(FESL.isValid());
  // Get End location, if exists.
  uint32_t EndCol = 0;
  if (SR.getEnd().isValid() && SM.getExpansionLoc(SR.getEnd()).isValid()) {
    FullSourceLoc EFESL = Context.getFullLoc(SM.getExpansionLoc(SR.getEnd()));
    if (EFESL.isValid()) {
      EndCol = EFESL.getExpansionColumnNumber();
    }
  }
  std::string Fn = PL.getFilename();

  // Get the absolute filename of the file.
  // FullSourceLoc TFSL(SR.getBegin(), SM);
  FullSourceLoc TFSL(SM.getExpansionLoc(SR.getBegin()), SM);
  if (TFSL.isValid()) {
    const FileEntry *Fe = SM.getFileEntryForID(TFSL.getFileID());
    std::string FeAbsS = Fn;
    if (Fe != nullptr) {
      // Unlike in `emit` in RewriteUtils.cpp, we don't re-canonicalize the file
      // path because of the potential performance cost (mkPSL is called on many
      // AST nodes in each translation unit) and because we don't have a good
      // way to handle errors. If there is a problem, `emit` will detect it
      // before we actually write a file.
      FeAbsS = Fe->tryGetRealPathName().str();
    }
    Fn = std::string(sys::path::remove_leading_dotslash(FeAbsS));
  }
  PersistentSourceLoc PSL(Fn, FESL.getExpansionLineNumber(),
                          FESL.getExpansionColumnNumber(), EndCol);

  return PSL;
}
