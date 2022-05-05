#ifndef LLVM_CLANG_DETECTERR_GOTOVISITORS_H
#define LLVM_CLANG_DETECTERR_GOTOVISITORS_H

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/DetectERRVisitor.h"
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

/// H08 - goto to an error label is control dependent on a check
class GotoVisitor : public DetectERRVisitor {
public:
  GotoVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
              FuncId &FnID)
      : DetectERRVisitor{Context, I, FD, FnID, HeuristicID::H08} {};

  virtual ~GotoVisitor() = default;

  bool VisitGotoStmt(GotoStmt *S) override;

private:
  static std::set<std::string> ErrorLabels;
};

#endif //LLVM_CLANG_DETECTERR_GOTOVISITORS_H
