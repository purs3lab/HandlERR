//=--CastPlacement.cpp--------------------------------------------*- C++-*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// This class contains implementation of the functions and
// classes of CastPlacement.h
//===----------------------------------------------------------------------===//

#include "clang/3C/CastPlacement.h"
#include "clang/3C/3CGlobalOptions.h"
#include "clang/3C/ConstraintResolver.h"
#include "clang/3C/Utils.h"
#include <clang/Tooling/Refactoring/SourceCode.h>

using namespace clang;

bool CastPlacementVisitor::VisitCallExpr(CallExpr *CE) {
  // Get the constraint variable for the function.
  Decl *CalleeDecl = CE->getCalleeDecl();
  FVConstraint *FV = nullptr;
  FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CalleeDecl);
  if (FD) {
    FV = Info.getFuncConstraint(FD, Context);
    assert("Function has no definition" && FV != nullptr);
  } else if (isa_and_nonnull<VarDecl>(CalleeDecl)) {
    CVarOption Opt = Info.getVariable(cast<VarDecl>(CalleeDecl), Context);
    if (Opt.hasValue()) {
      if (isa<PVConstraint>(Opt.getValue()))
        FV = cast<PVConstraint>(Opt.getValue()).getFV();
      else
        FV = dyn_cast<FVConstraint>(&Opt.getValue());
    }
  }

  if (FV && Rewriter::isRewritable(CE->getExprLoc())) {
    // Now we need to check the type of the arguments and corresponding
    // parameters to see if any explicit casting is needed.
    ProgramInfo::CallTypeParamBindingsT TypeVars;
    if (Info.hasTypeParamBindings(CE, Context))
      TypeVars = Info.getTypeParamBindings(CE, Context);

    // Cast on arguments
    unsigned PIdx = 0;
    for (const auto &A : CE->arguments()) {
      if (PIdx < FV->numParams()) {
        // Avoid adding incorrect casts to generic function arguments by
        // removing implicit casts when on arguments with a consistently
        // used generic type.
        Expr *ArgExpr = A;
        if (FD) {
          const TypeVariableType *TyVar =
            getTypeVariableType(FD->getParamDecl(PIdx));
          if (TyVar && TypeVars.find(TyVar->GetIndex()) != TypeVars.end() &&
              TypeVars[TyVar->GetIndex()] != nullptr)
            ArgExpr = ArgExpr->IgnoreImpCasts();
        }

        CVarSet ArgumentConstraints = CR.getExprConstraintVars(ArgExpr);
        ConstraintVariable *ParamInt = FV->getInternalParamVar(PIdx);
        ConstraintVariable *ParamExt = FV->getParamVar(PIdx);
        for (auto *ArgC : ArgumentConstraints) {
          if (needCasting(ArgC, ArgC, ParamInt, ParamExt) != NO_CAST) {
            surroundByCast(ArgC, ArgC, ParamInt, ParamExt, A);
            break;
          }
        }
      }
      PIdx++;
    }

    // Cast on return
    if (Info.hasPersistentConstraints(CE, Context)) {
      CVarSet ArgumentConstraints = CR.getExprConstraintVars(CE);
      ConstraintVariable *RetInt = FV->getInternalReturnVar();
      ConstraintVariable *RetExt = FV->getReturnVar();
      for (auto *ArgC : ArgumentConstraints) {
        // Order of ParameterC and ArgumentC is reversed from when inserting
        // parameter casts because assignment now goes from returned to its
        // local use.
        if (!Locator.exprHasCast(CE) &&
            needCasting(RetInt, RetExt, ArgC, ArgC) != NO_CAST) {
          surroundByCast(RetInt, RetExt, ArgC, ArgC, CE);
          break;
        }
      }
    }
  }
  return true;
}

// Check whether an explicit casting is needed when the pointer represented
// by src variable is assigned to dst.
CastPlacementVisitor::CastNeeded
CastPlacementVisitor::needCasting(const ConstraintVariable *SrcInt,
                                  const ConstraintVariable *SrcExt,
                                  const ConstraintVariable *DstInt,
                                  const ConstraintVariable *DstExt) {

  if (isa<PVConstraint>(DstExt) && cast<PVConstraint>(DstExt)->isVoidPtr())
    return CastNeeded::NO_CAST;

  if (isa<FVConstraint>(SrcExt) &&
      SrcExt->solutionEqualTo(Info.getConstraints(), DstExt))
    return CastNeeded::NO_CAST;

  const auto &E = Info.getConstraints().getVariables();
  bool SIChecked = SrcInt->isFullyChecked(E) && !SrcInt->srcHasItype();
  bool SEChecked = SrcExt->isFullyChecked(E) || SrcExt->srcHasItype();
  bool DIChecked = DstInt->isFullyChecked(E) && !DstInt->srcHasItype();
  bool DEChecked = DstExt->isFullyChecked(E) || DstExt->srcHasItype();

  // Cast prefix is only the part of the cast to the left of the expression
  // being cast. It does not contain the required closing parenthesis.
  if (SEChecked && SIChecked && !DEChecked) {
    // C-style cast to wild
    return CastNeeded::CAST_TO_WILD;
  } else if ( !SEChecked && DIChecked && DEChecked) {
    // _Assume_bounds_cast to checked
    return CastNeeded::CAST_TO_CHECKED;
  }

  if (!SIChecked && DEChecked && DIChecked) {
    if (auto *DstPVC = dyn_cast<PVConstraint>(DstExt)) {
      assert("Checked cast not to a pointer" && !DstPVC->getCvars().empty());
      ConstAtom *CA = Info.getConstraints().getAssignment(
        DstPVC->getCvars().at(0));
      if (isa<NTArrAtom>(CA))
        return CAST_TO_CHECKED;
    }
  }

  // Casting requirements are stricter when the parameter is a function pointer.
  if (isa<FVConstraint>(DstExt) || cast<PVConstraint>(DstExt)->getFV()) {
    if (!DstExt->solutionEqualTo(Info.getConstraints(), SrcInt))
      return CastNeeded::CAST_TO_WILD;
  }
  return CastNeeded::NO_CAST;
}

std::pair<std::string, std::string>
CastPlacementVisitor::getCastString(const ConstraintVariable *SrcInt,
                                    const ConstraintVariable *SrcExt,
                                    const ConstraintVariable *DstInt,
                                    const ConstraintVariable *DstExt) {
  CastNeeded CastType = needCasting(SrcInt, SrcExt, DstInt, DstExt);
  const auto &E = Info.getConstraints().getVariables();
  switch (CastType) {
    case CAST_TO_WILD:
      return std::make_pair("((" + DstExt->getRewritableOriginalTy() + ")",
                            ")");
    case CAST_TO_CHECKED: {
      std::string Suffix = ")";
      if (const auto *DstPVC = dyn_cast<PVConstraint>(DstExt)) {
        assert("Checked cast not to a pointer" && !DstPVC->getCvars().empty());
        ConstAtom *CA = Info.getConstraints().getAssignment(
          DstPVC->getCvars().at(0));
        if (isa<ArrAtom>(CA) || isa<NTArrAtom>(CA)) {
          std::string Bounds = "";
          if (DstPVC->srcHasBounds())
            Bounds = DstPVC->getBoundsStr();
          else if (DstPVC->hasBoundsKey())
            Bounds = ABRewriter.getBoundsString(DstPVC, nullptr, true);
          if (Bounds.empty())
            Bounds = "byte_count(0)";

          Suffix = ", " + Bounds + ")";
        }
      }
      return std::make_pair(
        "_Assume_bounds_cast<" + DstExt->mkString(E, false) + ">(", Suffix);
    }
    default:
      llvm_unreachable("No casting needed");
  }
}


void CastPlacementVisitor::surroundByCast(const ConstraintVariable *SrcInt,
                                          const ConstraintVariable *SrcExt,
                                          const ConstraintVariable *DstInt,
                                          const ConstraintVariable *DstExt,
                                          Expr *E) {
  auto CastStrs = getCastString(SrcInt, SrcExt, DstInt, DstExt);

  // TODO: Is this spacial handling for casts-on-casts still required?
  // If E is already a cast expression, we will try to rewrite the cast instead
  // of adding a new expression.
  //if (auto *CE = dyn_cast<CStyleCastExpr>(E->IgnoreParens())) {
  //  SourceRange CastTypeRange(CE->getLParenLoc(), CE->getRParenLoc());
  //  Writer.ReplaceText(CastTypeRange, CastString);
  //} else {
    bool FrontRewritable = Writer.isRewritable(E->getBeginLoc());
    bool EndRewritable = Writer.isRewritable(E->getEndLoc());
    if (FrontRewritable && EndRewritable) {
      bool BFail = Writer.InsertTextBefore(E->getBeginLoc(), CastStrs.first);
      bool EFail = Writer.InsertTextAfterToken(E->getEndLoc(), CastStrs.second);
      assert("Locations were rewritable, fail should not be possible." &&
             !BFail && !EFail);
    } else {
      // This means we failed to insert the text at the end of the RHS.
      // This can happen because of Macro expansion.
      // We will see if this is a single expression statement?
      // If yes, then we will use parent statement to add ")"
      auto CRA = CharSourceRange::getTokenRange(E->getSourceRange());
      auto NewCRA = clang::Lexer::makeFileCharRange(
          CRA, Context->getSourceManager(), Context->getLangOpts());
      std::string SrcText = clang::tooling::getText(CRA, *Context);
      assert("Cannot insert cast! Perhaps something funny with macros?" &&
             !SrcText.empty());
      Writer.ReplaceText(NewCRA, CastStrs.first + SrcText + CastStrs.second);
    }
  //}
}

bool CastLocatorVisitor::exprHasCast(Expr *E) const {
  return isa<CastExpr>(E) || ExprsWithCast.find(E) != ExprsWithCast.end();
}

bool CastLocatorVisitor::VisitCastExpr(CastExpr *C) {
  if (!isa<ImplicitCastExpr>(C)) {
    Expr *Sub = ignoreCheckedCImplicit(C->getSubExpr());
    ExprsWithCast.insert(Sub);
  }
  return true;
}

Expr *CastLocatorVisitor::ignoreCheckedCImplicit(Expr *E) {
  Expr *Old = nullptr;
  Expr *New = E;
  while (Old != New) {
    Old = New;
    New = Old->IgnoreExprTmp()->IgnoreImplicit();
  }
  return New;
}