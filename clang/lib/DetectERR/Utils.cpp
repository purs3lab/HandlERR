//
// Created by machiry on 10/16/21.
//

#include "clang/DetectERR/Utils.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Mangle.h"
#include "clang/Analysis/CFG.h"
#include "clang/Basic/SourceLocation.h"
#include "clang/DetectERR/ErrGruard.h"
#include "clang/DetectERR/ErrPoint.h"
#include "clang/DetectERR/PersistentSourceLoc.h"
#include <cstdlib>

using namespace clang;

FuncId getFuncID(const clang::FunctionDecl *FD, ASTContext *C) {
  FuncId RetFID;
  auto PSL = PersistentSourceLoc::mkPSL(FD, *C);

  // fix for using with C++ files as well
  ASTNameGenerator *NameGenerator = new ASTNameGenerator(*C);
  // RetFID.first = FD->getNameAsString();
  RetFID.first = NameGenerator->getName(FD);
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

  return Typ->isPointerType() &&
         E->isNullPointerConstant(C, Expr::NPC_ValueDependentIsNotNull);
}

bool isNegativeNumber(const clang::Expr *E, ASTContext &C) {
  E = removeAuxillaryCasts(E);
  if (!E->isValueDependent() && E->isIntegerConstantExpr(C)) {
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

  const auto *It = BB.rbegin();
  if (It != BB.rend()) {
    const Stmt *LastStmt = (*It).getAs<CFGStmt>()->getStmt();
    return LastStmt == &ST;
  }

  return false;
}

FnReturnType getReturnType(const CallExpr *CE, ASTContext *Context) {
  auto ReturnType = CE->getCallReturnType(*Context);
  const auto *UnqualifiedReturnType = ReturnType->getUnqualifiedDesugaredType();
  if (UnqualifiedReturnType->isAnyPointerType()) {
    return FnReturnType::POINTER;
  }
  if (UnqualifiedReturnType->isIntegerType()) {
    return FnReturnType::INT;
  }
  return FnReturnType::NOT_INTERESTING;
}

bool isLibraryCallExpr(const CallExpr *CE, ASTContext *Context) {
  // a library call is when the function being called is declared in a library file

  // check if the FUZZERR_SRC_LOCATION is set
  const char *FuzzerrFifuzzSrcLocation =
      std::getenv("FUZZERR_FIFUZZ_SRC_LOCATION");
  if (!FuzzerrFifuzzSrcLocation) {
    llvm::errs() << "FUZZERR_FIFUZZ_SRC_LOCATION env var not set\n";
    exit(EXIT_FAILURE);
  }
  char *real_path = realpath(FuzzerrFifuzzSrcLocation, NULL);
  std::string FifuzzSrcLocation(real_path);
  free(real_path);
  llvm::errs() << "FUZZERR_FIFUZZ_SRC_LOCATION => " << FifuzzSrcLocation
               << "\n";

  // check if the the file containing the declaration of the called function lies somewhere
  // inside FUZZERR_FIFUZZ_SRC_LOCATION
  const clang::Decl *CalleeDecl = CE->getCalleeDecl();
  // calleeDecl->dumpColor();
  auto CalleeDeclLoc = CalleeDecl->getLocation();
  // CalleeDeclLoc.dump(Context->getSourceManager());
  std::string DeclSrcLoc =
      CalleeDeclLoc.printToString(Context->getSourceManager());

  // we consider all relative paths as not belonging to library
  llvm::errs() << "DeclSrcLocl: " << DeclSrcLoc << "\n";
  if (DeclSrcLoc.find("..") != std::string::npos) {
    return false;
  }

  if (DeclSrcLoc.find(FifuzzSrcLocation) != std::string::npos) {
    // llvm::errs() << ">>>> not a library function\n";
    return false;
  }

  // llvm::errs() << ">>>> library function\n";
  return true;
}

bool isEHFCallExpr(const CallExpr *CE, const std::set<std::string> &EHFList,
                   ASTContext *Context) {
  const Decl *CalledDecl = CE->getCalleeDecl();
  if (const FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CalledDecl)) {
    std::string CalledFnName = FD->getNameInfo().getAsString();
    // if the called function is a known exit function
    if (EHFList.find(CalledFnName) != EHFList.end()) {
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
      for (const Stmt *Child : UO->children()) {
        // there would be only one child
        if (const DeclRefExpr *ActualDRE =
                getDeclRefExpr(dyn_cast<Expr>(Child))) {
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
      for (const Stmt *Child : UO->children()) {
        // there would be only one child
        Result = (Expr *)dyn_cast_or_null<Expr>(Child);
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

/// debug util: dumps the statements in the stmap
void __dbg_print_statements(std::map<const Stmt *, CFGBlock *> &StMap) {
  llvm::errs() << "__dbg_print_statements >>>\n";
  auto It = StMap.begin();
  while (It != StMap.end()) {
    llvm::errs() << "- Stmt:\n";
    It->first->dump();
    It++;
  }
  llvm::errs() << "__dbg_print_statements <<<\n";
}

/// return the most immediate Control Dependent check and the guard level
std::pair<Stmt *, GuardLevel> getImmediateControlDependentCheck(
    std::vector<std::pair<Stmt *, CFGBlock *>> &Checks, Stmt *ErrorST,
    ControlDependencyCalculator *CDG, SourceManager &SM) {

  // one trip of bubbling the inner most check
  if (Checks.size() > 1) {
    for (unsigned long J = 1; J < Checks.size(); J++) {
      CFGBlock *BB0 = Checks[0].second;
      CFGBlock *BBJ = Checks[J].second;
      if (!CDG->isControlDependent(BB0, BBJ)) {
        std::swap(Checks[0], Checks[J]);
      }
    }
  }

  GuardLevel Lvl = GuardLevel::Default;
    CharSourceRange CurrER = SM.getExpansionRange(Checks[0].first->getSourceRange());
    CharSourceRange ErrorSTER = SM.getExpansionRange(ErrorST->getSourceRange());
    SourceRange CurrSR = CurrER.getAsRange();
    SourceRange ErrorSTSR = ErrorSTER.getAsRange();
  if (CurrSR.fullyContains(ErrorSTSR)) {
    Lvl = GuardLevel::Inner;
  }

  return {Checks[0].first, Lvl};
}

/// Bubbles the inner dominating check at 0th position so that appropriate
/// 'level' related info can be added whiled creating the ErrGuard instance
void sortIntoInnerAndOuterChecks(
    std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
    ControlDependencyCalculator *CDG, SourceManager &SM) {
  if (Checks.size() <= 1) {
    return;
  }

  // one trip of bubbling the inner most check
  for (unsigned long J = 1; J < Checks.size(); J++) {
    CFGBlock *BB0 = Checks[0].second;
    CFGBlock *BBJ = Checks[J].second;
    if (!CDG->isControlDependent(BB0, BBJ)) {
      std::swap(Checks[0], Checks[J]);
    }
  }
}

/// Iterates over the base blocks on which the given block is control dependent
/// and collects all the terminator checks from those blocks
void collectChecks(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                   CFGBlock &CurBB, ControlDependencyCalculator *CDG) {
  auto &CDNodes = CDG->getControlDependencies(&CurBB);
  for (auto &CDGNode : CDNodes) {
    // Collect the possible length bounds keys.
    Stmt *TStmt = CDGNode->getTerminatorStmt();
    // check if this is a guard statement (if/while/switch)
    if (dyn_cast_or_null<IfStmt>(TStmt) || dyn_cast_or_null<WhileStmt>(TStmt) ||
        dyn_cast_or_null<SwitchStmt>(TStmt)) {
      Checks.push_back(std::pair<Stmt *, CFGBlock *>(TStmt, CDGNode));
    }
  }
}

/// returns the condition expression based on the type of check statement
/// (if/while/switch)
Expr *getCondFromCheckStmt(Stmt *ST) {
  Expr *Cond = nullptr;

  // IF Stmt
  if (IfStmt *IfCheck = dyn_cast_or_null<IfStmt>(ST)) {
    Cond = IfCheck->getCond();
  }

  // While Stmt
  else if (WhileStmt *WhileCheck = dyn_cast_or_null<WhileStmt>(ST)) {
    Cond = WhileCheck->getCond();
  }

  // Switch Stmt
  else if (SwitchStmt *SwitchCheck = dyn_cast_or_null<SwitchStmt>(ST)) {
    Cond = SwitchCheck->getCond();
  }

  return Cond;
}

/// extracts all the Values that are used in the Conditional
std::vector<const Decl *> getCondValueDecls(const Expr *Cond) {
  std::vector<const Decl *> Result;

  const Expr *Stripped = removeAuxillaryCasts(Cond);

  if (const BinaryOperator *BinaryOp = dyn_cast<BinaryOperator>(Stripped)) {
    // collect vals from LHS
    Expr *LHS = BinaryOp->getLHS();
    auto CondVals = getCondValueDecls(LHS);
    if (CondVals.size() > 0) {
      for (auto *Val : CondVals) {
        Result.push_back(Val);
      }
    }

    // collect vals from RHS
    Expr *RHS = BinaryOp->getRHS();
    CondVals = getCondValueDecls(RHS);
    if (CondVals.size() > 0) {
      for (auto *Val : CondVals) {
        Result.push_back(Val);
      }
    }

  } else if (const UnaryOperator *UnaryOp = dyn_cast<UnaryOperator>(Stripped)) {
    // collect vals
    Expr *SubExpr = UnaryOp->getSubExpr();
    auto CondVals = getCondValueDecls(SubExpr);
    if (CondVals.size() > 0) {
      for (auto *Val : CondVals) {
        Result.push_back(Val);
      }
    }

  } else {
    // likely just an actual value
    const DeclRefExpr *DRE = getDeclRefExpr(Stripped);
    if (DRE) {
      Result.push_back(DRE->getDecl());
    }
  }

  return Result;
}

/// does SR1 fully contain SR2?
bool doesFullyContain(const SourceRange SR1, const SourceRange SR2,
                      const SourceManager &SM) {
  if (SR1.fullyContains(SR2)) {
    return true;
  }

  // fallback, compare the actual line and column numbers
  FullSourceLoc SR1Begin = FullSourceLoc(SR1.getBegin(), SM);
  FullSourceLoc SR1End = FullSourceLoc(SR1.getEnd(), SM);
  FullSourceLoc SR2Begin = FullSourceLoc(SR2.getBegin(), SM);
  FullSourceLoc SR2End = FullSourceLoc(SR2.getEnd(), SM);

  int SR1BeginLineNum = SR1Begin.getLineNumber();
  int SR2BeginLineNum = SR2Begin.getLineNumber();
  int SR1EndLineNum = SR1End.getLineNumber();
  int SR2EndLineNum = SR2End.getLineNumber();
  if (SR1BeginLineNum <= SR2BeginLineNum && SR1EndLineNum > SR2EndLineNum) {
    return true;
  }
  if (SR1BeginLineNum < SR2BeginLineNum && SR1EndLineNum >= SR2EndLineNum) {
    return true;
  }
  if (SR1BeginLineNum == SR2BeginLineNum && SR1EndLineNum == SR2EndLineNum) {
    int SR1BeginColNum = SR1Begin.getColumnNumber();
    int SR1EndColNum = SR1End.getColumnNumber();
    int SR2BeginColNum = SR2Begin.getColumnNumber();
    int SR2EndColNum = SR2End.getColumnNumber();
    if (SR1BeginColNum < SR2BeginColNum || SR1EndColNum > SR2EndColNum) {
      return true;
    }
  }

  return false;
}

/// remove the inner check that are using params to the function
/// NOTE: that this functions expects that the `Checks` have already been sorted
/// using `sortIntoInnerAndOuterChecks()`
void removeInnerCheckUsingParams(
    std::vector<std::pair<Stmt *, CFGBlock *>> &Checks, ReturnStmt *ReturnST,
    FunctionDecl &FD, SourceManager &SM) {

  auto ChecksIter = Checks.begin();
  if (ChecksIter != Checks.end()) {
    // only doing this for the first element, since there can only be one
    // "inner" check

    SourceRange CurrSR = ChecksIter->first->getSourceRange();
    SourceRange ReturnSTSR = ReturnST->getSourceRange();

    bool CheckEnclosesReturnSt = doesFullyContain(CurrSR, ReturnSTSR, SM);

    if (CheckEnclosesReturnSt) {
      // this check is the "inner" check

      // 1. get the values used by the condition of this check
      Expr *Cond = getCondFromCheckStmt(ChecksIter->first);
      std::vector<const Decl *> CondValueDecls = getCondValueDecls(Cond);

      // 2. get fn params
      for (auto *ParamIter = FD.param_begin(); ParamIter != FD.param_end();
           ParamIter++) {

        // 3. check if the inner check depends on any of the params
        for (auto CondValIter = CondValueDecls.begin();
             CondValIter != CondValueDecls.end(); CondValIter++) {

          if (*ParamIter == *CondValIter) {
            // 4. if so, remove it from the vector

            Checks.erase(ChecksIter);
            return;
          }
        }
      }
    }
  }
}

///
/// remove all checks that are using params to the function
void removeChecksUsingParams(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                             FunctionDecl &FD) {
  auto ChecksIter = Checks.begin();
  while (ChecksIter != Checks.end()) {
    bool checkDeleted = false;

    // 1. get the values used by the condition of this check
    Expr *Cond = getCondFromCheckStmt(ChecksIter->first);
    std::vector<const Decl *> CondValueDecls = getCondValueDecls(Cond);

    // 2. get fn params
    for (auto *ParamIter = FD.param_begin(); ParamIter != FD.param_end();
         ParamIter++) {

      // 3. check if the check depends on any of the params
      for (auto CondValIter = CondValueDecls.begin();
           CondValIter != CondValueDecls.end(); CondValIter++) {

        if (*ParamIter == *CondValIter) {
          // 4. if so, remove it from the vector
          ChecksIter = Checks.erase(ChecksIter);
          checkDeleted = true;
          goto cont;
        }
      }
    }

  // label to continue after deleting a check
  cont:
    if (!checkDeleted)
      ChecksIter++;
  }
}
