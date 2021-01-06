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
  FunctionDecl *FD = dyn_cast_or_null<FunctionDecl>(CalleeDecl);

  // Find a FVConstraint for this call. If there is more than one, then they
  // will have been unified during constraint generation, so we can use any of
  // them.
  FVConstraint *FV = nullptr;
  for (auto *CV : CR.getCalleeConstraintVars(CE)) {
    if (isa<FVConstraint>(CV))
      FV = cast<FVConstraint>(CV);
    else if (isa<PVConstraint>(CV) && cast<PVConstraint>(CV)->getFV())
      FV = cast<PVConstraint>(CV)->getFV();
    if (FV)
      break;
  }

  // Note: I'm not entirely sure that this will always hold. The previous
  // implementation just returned early if FV was null, but I don't think that
  // can ever actually happen.
  assert("Could not find function constraint variable!" && FV != nullptr);

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
      if (FD && PIdx < FD->getNumParams()) {
        const TypeVariableType *TyVar =
          getTypeVariableType(FD->getParamDecl(PIdx));
        if (TyVar && TypeVars.find(TyVar->GetIndex()) != TypeVars.end() &&
            TypeVars[TyVar->GetIndex()] != nullptr)
          ArgExpr = ArgExpr->IgnoreImpCasts();
      }

      CVarSet DestinationConstraints = CR.getExprConstraintVars(ArgExpr);
      InternalExternalPair<ConstraintVariable> Dst = {
        FV->getInternalParam(PIdx), FV->getExternalParam(PIdx)};
      for (auto *DstC : DestinationConstraints) {
        InternalExternalPair<ConstraintVariable> Src = {DstC, DstC};
        if (needCasting(Src, Dst) != NO_CAST) {
          ExprsWithCast.insert(ignoreCheckedCImplicit(A));
          surroundByCast(Src, Dst, A);
          break;
        }
      }
    }
    PIdx++;
  }

  // Cast on return. Be sure not to place casts when the result is not used,
  // otherwise an externaly unsafe function whose result is not used would end
  // up with a bounds cast around it.
  if (Info.hasPersistentConstraints(CE, Context)) {
    CVarSet DestinationConstraints = CR.getExprConstraintVars(CE);
    InternalExternalPair<ConstraintVariable> Src = {FV->getInternalReturn(),
                                                    FV->getExternalReturn()};
    for (auto *DstC : DestinationConstraints) {
      // Order of ParameterC and ArgumentC is reversed from when inserting
      // parameter casts because assignment now goes from returned to its
      // local use.
      InternalExternalPair<ConstraintVariable> Dst = {DstC, DstC};
      if (ExprsWithCast.find(CE) == ExprsWithCast.end() &&
          needCasting(Src, Dst) != NO_CAST) {
        surroundByCast(Src, Dst, CE);
        ExprsWithCast.insert(ignoreCheckedCImplicit(CE));
        break;
      }
    }
  }
  return true;
}

// Check whether an explicit casting is needed when the pointer represented
// by src variable is assigned to dst. The return value specifies exactly the
// type of cast required.
CastPlacementVisitor::CastNeeded
CastPlacementVisitor::needCasting(InternalExternalPair<ConstraintVariable> Src,
                                  InternalExternalPair<ConstraintVariable> Dst) {
  ConstraintVariable *SrcInt = Src.InternalConstraint;
  ConstraintVariable *SrcExt = Src.ExternalConstraint;
  ConstraintVariable *DstInt = Dst.InternalConstraint;
  ConstraintVariable *DstExt = Dst.ExternalConstraint;

  if (isa<PVConstraint>(DstExt) && cast<PVConstraint>(DstExt)->isVoidPtr())
    return CastNeeded::NO_CAST;

  if (isa<FVConstraint>(SrcExt) &&
      SrcExt->solutionEqualTo(Info.getConstraints(), DstExt, false))
    return CastNeeded::NO_CAST;

  const auto &E = Info.getConstraints().getVariables();
  bool SIChecked = SrcInt->isFullyChecked(E) && !SrcInt->srcHasItype();
  bool SEChecked = SrcExt->isFullyChecked(E) || SrcExt->srcHasItype();
  bool DIChecked = DstInt->isFullyChecked(E) && !DstInt->srcHasItype();
  bool DEChecked = DstExt->isFullyChecked(E) || DstExt->srcHasItype();

  // The source is checked internally and externally (i.e., it's not an itype),
  // but the destination is not checked. The checked source needs to be cast to
  // wild.
  if (SEChecked && SIChecked && !DEChecked)
    return CastNeeded::CAST_TO_WILD;

  // The reverse of the above case. A fully checked destination without a
  // checked source, so the source must be cast to checked.
  if (!SEChecked && DIChecked && DEChecked)
    return CastNeeded::CAST_TO_CHECKED;

  // In this case, the source is internally unchecked (i.e., it has an itype).
  // Typically, no casting is required, but CheckedC does not allow implicit
  // casts from WILD to NT_ARR even at itype calls sites.
  if (!SIChecked && DIChecked && DEChecked) {
    if (auto *DstPVC = dyn_cast<PVConstraint>(DstExt)) {
      assert("Checked cast not to a pointer" && !DstPVC->getCvars().empty());
      ConstAtom *CA = Info.getConstraints().getAssignment(
        DstPVC->getCvars().at(0));
      if (isa<NTArrAtom>(CA))
        return CAST_TO_CHECKED;
    }
  }

  if (!SEChecked && SrcExt->isChecked(Info.getConstraints().getVariables()))
    if (!DstExt->solutionEqualTo(Info.getConstraints(), SrcExt, false) &&
        !DstExt->solutionEqualTo(Info.getConstraints(), SrcInt, false))
      return CAST_TO_WILD;

  if (!DEChecked && DstExt->isChecked((Info.getConstraints().getVariables())))
    if (!SrcExt->solutionEqualTo(Info.getConstraints(), DstExt, false) &&
        !SrcExt->solutionEqualTo(Info.getConstraints(), DstInt, false))
      return CAST_TO_CHECKED;

  // Casting requirements are stricter when the parameter is a function pointer
  // or an itype.
  bool UncheckedItypeCall = DstExt->srcHasItype() && !SEChecked;
  bool FptrCall =
    isa<FVConstraint>(DstExt) || cast<PVConstraint>(DstExt)->getFV();
  // This is also conditioned on if the src is checked to avoid adding casts to
  // WILD on top of a wild expression.
  if (SrcInt->isChecked(E) && (UncheckedItypeCall || FptrCall) &&
      !DstExt->solutionEqualTo(Info.getConstraints(), SrcInt, false))
    return CastNeeded::CAST_TO_WILD;

  // If nothing above returned, then no casting is required.
  return CastNeeded::NO_CAST;
}

// Get the string representation of the cast required for the call. The return
// is a pair of strings: a prefix and suffix string that form the complete cast
// when placed around the expression being cast.
std::pair<std::string, std::string> CastPlacementVisitor::getCastString(
  InternalExternalPair<ConstraintVariable> Src,
  InternalExternalPair<ConstraintVariable> Dst) {
  CastNeeded CastType = needCasting(Src, Dst);
  const auto &E = Info.getConstraints().getVariables();
  switch (CastType) {
    case CAST_TO_WILD:
      return std::make_pair(
        "((" + Dst.ExternalConstraint->getRewritableOriginalTy() + ")",
        ")");
    case CAST_TO_CHECKED: {
      std::string Suffix = ")";
      if (const auto *DstPVC = dyn_cast<PVConstraint>(Dst.ExternalConstraint)) {
        assert("Checked cast not to a pointer" && !DstPVC->getCvars().empty());
        ConstAtom *CA = Info.getConstraints().getAssignment(
          DstPVC->getCvars().at(0));

        // Writing an _Assume_bounds_cast to an array type requires inserting
        // the bounds for destination array. These can come from the source
        // code or the infered bounds. If neither source is available, use empty
        // bounds.
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
        "_Assume_bounds_cast<" + Dst.ExternalConstraint->mkString(E, false) +
        ">(", Suffix);
    }
    default:
      llvm_unreachable("No casting needed");
  }
}


void CastPlacementVisitor::surroundByCast(
  InternalExternalPair<ConstraintVariable> Src,
  InternalExternalPair<ConstraintVariable> Dst, Expr *E) {
  auto CastStrs = getCastString(Src, Dst);

  // If E is already a cast expression, we will try to rewrite the cast instead
  // of adding a new expression.
  if (auto *CE = dyn_cast<CStyleCastExpr>(E->IgnoreParens())) {
    SourceRange CastTypeRange(CE->getLParenLoc(), CE->getRParenLoc());
    Writer.ReplaceText(CastTypeRange, CastStrs.first.substr(1));
  } else {
    // First try to insert the cast prefix and suffix around the extression in
    // the source code.
    bool FrontRewritable = Writer.isRewritable(E->getBeginLoc());
    bool EndRewritable = Writer.isRewritable(E->getEndLoc());
    if (FrontRewritable && EndRewritable) {
      bool BFail = Writer.InsertTextBefore(E->getBeginLoc(), CastStrs.first);
      bool EFail = Writer.InsertTextAfterToken(E->getEndLoc(), CastStrs.second);
      assert("Locations were rewritable, fail should not be possible." &&
             !BFail && !EFail);
    } else {
      // Sometimes we can't insert the cast around the expression due to macros
      // getting in the way. In these cases, we can sometimes replace the entire
      // expression source with a new string containing the orginal expression
      // and the cast.
      auto CRA = CharSourceRange::getTokenRange(E->getSourceRange());
      auto NewCRA = clang::Lexer::makeFileCharRange(
          CRA, Context->getSourceManager(), Context->getLangOpts());
      std::string SrcText = clang::tooling::getText(CRA, *Context);
      // This doesn't always work either. We can't rewrite if the cast needs to
      // be placed fully inside a macro rather than around a macro or on an
      // argument to the macro.
      if (!SrcText.empty())
        Writer.ReplaceText(NewCRA, CastStrs.first + SrcText + CastStrs.second);
    }
  }
}

bool CastLocatorVisitor::VisitCastExpr(CastExpr *C) {
  ExprsWithCast.insert(C);
  if (!isa<ImplicitCastExpr>(C)) {
    Expr *Sub = ignoreCheckedCImplicit(C->getSubExpr());
    ExprsWithCast.insert(Sub);
  }
  return true;
}
