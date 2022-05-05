#ifndef LLVM_CLANG_DETECTERR_VISITOR_UTILS_H
#define LLVM_CLANG_DETECTERR_VISITOR_UTILS_H

#include "clang/DetectERR/ErrGruard.h"

/// function to traverse the Checks and add ErrorGuards appropriately to the
/// Visitor
template <typename Visitor>
void addErrorGuards(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                    Stmt *ReturnST, Visitor &T) {

  // It is possible that the Check that is at index 0 might not be an "inner"
  // check and that the Check at index 1 may be the correct "inner" check
  // This is evident from the following example:
  //
  // if( C1 ) {           <<<< Inner
  //    if ( C2 ) {       <<<< Default
  //        return NULL;
  //    }
  //    return NULL;      <<<< Err Loc
  // }
  //
  // Hence maintain a flag to check if we have had an "inner" check already
  // Any other "enclosing" checks can then be labelled as "outer".

  const SourceManager &SM = T.Context->getSourceManager();

  Stmt *Inner = nullptr;

  for (unsigned long I = 0; I < Checks.size(); I++) {
    const SourceRange CurrSR = Checks[I].first->getSourceRange();
    const SourceLocation CurrSLBegin = SM.getExpansionLoc(CurrSR.getBegin());
    const SourceLocation CurrSLEnd = SM.getExpansionLoc(CurrSR.getEnd());

    const SourceRange ReturnSTSR = ReturnST->getSourceRange();
    const SourceLocation ReturnSTSLBegin =
        SM.getExpansionLoc(ReturnSTSR.getBegin());
    const SourceLocation ReturnSTSLEnd =
        SM.getExpansionLoc(ReturnSTSR.getEnd());

    GuardLevel Lvl = GuardLevel::Default;

    // conditions for a guard to be "Outer"
    // - return statement is within it
    // - inner guard is within it
    if (Inner) {
      // we can now assign outer checks
      SourceRange InnerSR = Inner->getSourceRange();
      const SourceLocation InnerSLBegin =
          SM.getExpansionLoc(InnerSR.getBegin());
      const SourceLocation InnerSLEnd = SM.getExpansionLoc(InnerSR.getEnd());
      if (CurrSLBegin <= InnerSLBegin && CurrSLEnd >= InnerSLEnd) {
        Lvl = GuardLevel::Outer;
      }
    } else {
      // check if this is the inner check
      if (CurrSLBegin <= ReturnSTSLBegin && CurrSLEnd >= ReturnSTSLEnd) {
        Lvl = GuardLevel::Inner;
        Inner = Checks[I].first;
      }
    }

    T.Info.addErrorGuardingStmt(T.FID, Checks[I].first, ReturnST, T.Context,
                                T.Heuristic, Lvl);
  }
}

#endif //LLVM_CLANG_DETECTERR_VISITOR_UTILS_H
