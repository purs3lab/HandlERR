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

bool isInt(int i, const clang::Expr *E, ASTContext &C) {
  E = removeAuxillaryCasts(E);
  if (E->isIntegerConstantExpr(C)) {
    auto NewAPI = E->getIntegerConstantExpr(C);
    if (NewAPI.hasValue()) {
      return (NewAPI->getSExtValue() == i);
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

  return false;
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

/// Is the expression a deref to the given Decl?
bool isDerefToDeclRef(const clang::Expr *E, const NamedDecl *D) {
  // is the expr a pointer type?

  // This is what a sample dump for *a == x looks like (for *a part)
  // ImplicitCastExpr 0x55555de82a40 'int' <LValueToRValue>
  // `-UnaryOperator 0x55555de829e8 'int' lvalue prefix '*' cannot overflow
  //   `-ImplicitCastExpr 0x55555de829b0 'int *' <LValueToRValue>
  //     `-DeclRefExpr 0x55555de82980 'int *' lvalue Var 0x55555de826c0 'a' 'int *'

  // so we can:
  // - remove the implicit cast(s)
  // - check that we have a UnaryOperator (*)
  // - again remove the implicit cast(s)
  // - get the underlying DeclRef

  const Expr *CurrE = removeAuxillaryCasts(E);
  if (const UnaryOperator *UO = dyn_cast<UnaryOperator>(CurrE)) {
    if (UO->getOpcode() == UnaryOperator::Opcode::UO_Deref) {
      for (const Stmt *child : UO->children()) {
        // there would be only one child
        if (const DeclRefExpr *ActualDRE =
                getDeclRefExpr(dyn_cast<Expr>(child))) {
          return ActualDRE->getFoundDecl()->getUnderlyingDecl() == D;
        }
      }
    }
  }

  // is the underlying Decl same as the one passed in the argument?

  return false;
}

/// Get the underlying expression for a Deref Expression (UnaryOperator)
Expr *getDerefExpr(const clang::Expr *E) {
  Expr *Result = nullptr;
  const Expr *CE = removeAuxillaryCasts(E);
  if (const UnaryOperator *UO = dyn_cast<UnaryOperator>(CE)) {
    if (UO->getOpcode() == UnaryOperator::Opcode::UO_Deref) {
      for (const Stmt *child : UO->children()) {
        // there would be only one child
        Result = (Expr *)dyn_cast_or_null<Expr>(child);
      }
    }
  }

  return Result;
}

/// Is the Expr a wrapper around the given NamedDecl?
bool hasDeclRefExprTo(const clang::Expr *E, const NamedDecl *D) {
  if (const DeclRefExpr *DRE = getDeclRefExpr(E)) {
    return DRE->getFoundDecl()->getUnderlyingDecl() == D;
  }
  return false;
}

/// Checks whether there is a post-dominated CFGBlock for the given block
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

/// Checks if the given CFGBlock has any dominators
bool hasPreDominators(CFGBlock &CurrBB, ControlDependencyCalculator *CDG,
                      const CFG &Cfg) {
  bool foundDominator = false;
  auto &CDNodes = CDG->getControlDependencies(&CurrBB);
  // iterate over all the BasicBlocks of the given CFG
  for (CFGBlock *OtherBB : CDNodes) {
    // if otherBB properly dominates CurrBB
    // if the dominator is EntryBB, ignore it
    if (OtherBB != &Cfg.getEntry()) {
      foundDominator = true;
      break;
    }
  }
  return foundDominator;
}
