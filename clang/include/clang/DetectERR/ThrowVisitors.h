#ifndef LLVM_CLANG_DETECTERR_THROWVISITORS_H
#define LLVM_CLANG_DETECTERR_THROWVISITORS_H

#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Analysis/Analyses/Dominators.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/DetectERRASTConsumer.h"
#include "clang/DetectERR/DetectERRVisitor.h"
#include "clang/DetectERR/Utils.h"
#include <algorithm>

using namespace llvm;
using namespace clang;

/// H09 - throwing an exception is control dependent on a check
class ThrowVisitor : public DetectERRVisitor {
public:
  ThrowVisitor(ASTContext *Context, ProjectInfo &I, FunctionDecl *FD,
               FuncId &FnID)
      : DetectERRVisitor{Context, I, FD, FnID, HeuristicID::H09} {};

  virtual ~ThrowVisitor() = default;

  bool VisitCXXThrowExpr(CXXThrowExpr *S) override;
};

#endif //LLVM_CLANG_DETECTERR_THROWVISITORS_H
