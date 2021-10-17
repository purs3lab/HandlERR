//
// Created by machiry on 10/16/21.
//

#include "clang/DetectERR/Utils.h"
#include "clang/AST/Expr.h"

using namespace clang;

FuncId getFuncID(const clang::FunctionDecl *FD) {
  FuncId RetFID;
  // TODO: fill this.
  return RetFID;
}

const Expr *removeAuxillaryCasts(const Expr *E) {
  bool NeedStrip = true;
  while (NeedStrip) {
    NeedStrip = false;
    E = E->IgnoreParenImpCasts();
    if (const CStyleCastExpr *C = dyn_cast<CStyleCastExpr>(E)) {
      E = C->getSubExpr();
      NeedStrip = true;
    }
  }
  return E;
}

bool isNULLExpr(const clang::Expr *E, ASTContext &C) {
  QualType Typ = E->getType();
  E = removeAuxillaryCasts(E);
  return Typ->isPointerType() && E->isIntegerConstantExpr(C) &&
         E->isNullPointerConstant(C, Expr::NPC_ValueDependentIsNotNull);
}