//
// Created by machiry on 10/16/21.
//

#include "clang/DetectERR/Utils.h"
#include "clang/AST/Expr.h"
#include "clang/Analysis/CFG.h"
#include "clang/DetectERR/PersistentSourceLoc.h"

using namespace clang;

FuncId getFuncID(const clang::FunctionDecl *FD, ASTContext *C) {
  FuncId RetFID;
  auto PSL = PersistentSourceLoc::mkPSL(FD, *C);
  RetFID.first = FD->getNameAsString();
  RetFID.second = PSL.getFileName();
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

bool isNegativeNumber(const clang::Expr *E, ASTContext &C) {
  E = removeAuxillaryCasts(E);
  if (E->isIntegerConstantExpr(C)) {
    auto NewAPI = E->getIntegerConstantExpr(C);
    if (NewAPI.hasValue()) {
      return (NewAPI->getSExtValue() < 0);
    }
  }
  return false;
}

/// Is the expression a zero
bool isZero(const clang::Expr *E, ASTContext &C) {
  E = removeAuxillaryCasts(E);
  if (const auto *Res = dyn_cast<IntegerLiteral>(E)) {
    return Res->getValue().getSExtValue() == 0;
  }
  return false;
}

/// Checks if the expression is a variable
bool isDeclExpr(const clang::Expr *E) {
  E = removeAuxillaryCasts(E);
  return dyn_cast<DeclRefExpr>(E) != nullptr;
}

const DeclRefExpr *getDeclRefExpr(const clang::Expr *E) {
  auto *Exp = removeAuxillaryCasts(E);
  return dyn_cast<DeclRefExpr>(Exp);
}

bool hasPostDominators(CFGBlock &CurrBB, const CFGPostDomTree *PDTree,
                       const CFG &Cfg) {
  bool foundDominator = false;
  // iterate over all the BasicBlocks of the given CFG
  for (CFGBlock *OtherBB : Cfg) {
    // if otherBB properly dominates CurrBB
    if (PDTree->properlyDominates(OtherBB, &CurrBB)) {
      // if the dominator is ExitBB, ignore it
      if (OtherBB != &Cfg.getExit()) {
        foundDominator = true;
        break;
      }
    }
  }
  return foundDominator;
}

bool isUpdatedInPostDominators(const NamedDecl *ND, CFGBlock &CurrBB,
                               const CFGPostDomTree *PDTree, const CFG &Cfg) {
  // iterate over all the BasicBlocks of the given CFG
  for (CFGBlock *OtherBB : Cfg) {

    // if the OtherBB properly dominates the CurrBB
    if (PDTree->properlyDominates(OtherBB, &CurrBB)) {

      // iterate over all the statements in the OtherBB
      for (CFGElement &CFGElem : *OtherBB) {

        // we only care about Statements as the assignment would be done as part of a Statement
        if (CFGElem.getKind() == CFGElement::Kind::Statement) {
          const Stmt *CurrStmt = CFGElem.getAs<CFGStmt>()->getStmt();
          if (const BinaryOperator *BinOp =
                  dyn_cast<BinaryOperator>(CurrStmt)) {
            if (BinOp->getOpcode() == BinaryOperator::Opcode::BO_Assign) {
              const DeclRefExpr *DRE = dyn_cast<DeclRefExpr>(BinOp->getLHS());
              if (DRE) {
                const DeclRefExpr *UpdatedDRE = getDeclRefExpr(DRE);
                const auto *UpdatedNamedDecl =
                    UpdatedDRE->getFoundDecl()->getUnderlyingDecl();

                // check this against the NamedDecl for the return stmt
                if (ND == UpdatedNamedDecl) {
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }
  return false;
}

bool isLastStmtInBB(const Stmt &ST, const CFGBlock &BB) {
  // iterate over all the stmts in the BB and check the last one against
  // the given stmt

  if (BB.empty()) {
    return false;
  }

  auto it = BB.rbegin();
  if (it != BB.rend()) {
    const Stmt *LastStmt = (*it).getAs<CFGStmt>()->getStmt();
    return LastStmt == &ST;
  }
}

bool isEHFCallExpr(const CallExpr *CE, const std::set<std::string> &EHFList,
                   ASTContext *Context) {
  const Decl *CalledDecl = CE->getCalleeDecl();
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CalledDecl)) {
    std::string calledFnName = FD->getNameInfo().getAsString();
    // if the called function is a known exit function
    if (EHFList.find(calledFnName) != EHFList.end()) {
      return true;
    }
  }
  return false;
}
