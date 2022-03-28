//=--PersistentSourceLoc.h----------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class specifies a location in a source file that persists across
// invocations of the frontend. Given a Decl/Stmt/Expr, the FullSourceLoc
// of that value can be compared with an instance of this class for
// equality. If they are equal, then you can substitute the Decl/Stmt/Expr
// for the instance of this class.
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_DETECTERR_PERISTENTSOURCELOC_H
#define LLVM_CLANG_DETECTERR_PERISTENTSOURCELOC_H

#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdint>
#include <utility>

using namespace clang;

class PersistentSourceLoc {
protected:
  PersistentSourceLoc(std::string F, uint32_t L, uint32_t C, uint32_t E)
      : FileName(F), LineNo(L), ColNoS(C), ColNoE(E), IsValid(true) {}

public:
  PersistentSourceLoc()
      : FileName(""), LineNo(0), ColNoS(0), ColNoE(0), IsValid(false) {}
  std::string getFileName() const { return FileName; }
  uint32_t getLineNo() const { return LineNo; }
  uint32_t getColSNo() const { return ColNoS; }
  uint32_t getColENo() const { return ColNoE; }
  bool valid() const { return IsValid; }

  bool operator==(const PersistentSourceLoc &O) const {
    return FileName == O.FileName && LineNo == O.LineNo && ColNoS == O.ColNoS &&
           ColNoE == O.ColNoE;
  }

  bool operator<(const PersistentSourceLoc &O) const {
    if (FileName == O.FileName)
      if (LineNo == O.LineNo)
        if (ColNoS == O.ColNoS)
          if (ColNoE == O.ColNoE)
            return false;
          else
            return ColNoE < O.ColNoE;
        else
          return ColNoS < O.ColNoS;
      else
        return LineNo < O.LineNo;
    else
      return FileName < O.FileName;
  }

  std::string toString() const {
    return FileName + ":" + std::to_string(LineNo) + ":" +
           std::to_string(ColNoS) + ":" + std::to_string(ColNoE);
  }

  std::string toJsonString() const {
    return "{\"File\":\"" + FileName +
           "\", \"LineNo\":" + std::to_string(LineNo) +
           ", \"ColNo\":" + std::to_string(ColNoS) + "}";
  }

  void toJsonString(llvm::raw_ostream &O) const { O << toJsonString(); }

  void print(llvm::raw_ostream &O) const { O << toString(); }

  void dump() const { print(llvm::errs()); }

  static PersistentSourceLoc mkPSL(const Decl *D, const ASTContext &C);

  static PersistentSourceLoc mkPSL(const clang::Stmt *S,
                                   const clang::ASTContext &Context);

  std::pair<uint32_t, uint32_t> getRowCol() {
    return std::pair<uint32_t, uint32_t>(LineNo, ColNoS);
  }

  std::string getRowColStr() {
    return "(" + std::to_string(LineNo) + "," + std::to_string(ColNoS) + ")";
  }

private:
  // Create a PersistentSourceLoc based on absolute file path
  // from the given SourceRange and SourceLocation.
  static PersistentSourceLoc mkPSL(clang::SourceRange SR,
                                   clang::SourceLocation SL,
                                   const clang::ASTContext &Context);

  // The source file name.
  std::string FileName;
  // Starting line number.
  uint32_t LineNo;
  // Column number start.
  uint32_t ColNoS;
  // Column number end.
  uint32_t ColNoE;
  bool IsValid;
};

typedef std::pair<PersistentSourceLoc, PersistentSourceLoc>
    PersistentSourceRange;

#endif //LLVM_CLANG_DETECTERR_PERISTENTSOURCELOC_H
