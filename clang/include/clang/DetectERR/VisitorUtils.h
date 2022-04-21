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

  Stmt *Inner = nullptr;

  for (unsigned long I = 0; I < Checks.size(); I++) {
    SourceRange CurrSR = Checks[I].first->getSourceRange();
    SourceRange ReturnSTSR = ReturnST->getSourceRange();
    GuardLevel Lvl = GuardLevel::Default;
    if (I == 0) {
      // TODO: DISCUSS: the following is not working as expected
      //      if (CurrSR.fullyContains(ReturnSTSR)) {
      //        Lvl = GuardLevel::Inner;
      //      }
      // workaround for the above
      if (CurrSR.getBegin() <= ReturnSTSR.getBegin() &&
          CurrSR.getEnd() >= ReturnSTSR.getBegin()) {
        Lvl = GuardLevel::Inner;
        Inner = Checks[I].first;
      }
      T.Info.addErrorGuardingStmt(T.FID, Checks[I].first, ReturnST, T.Context,
                                  T.Heuristic, Lvl);
    } else {
      // conditions for a guard to be "Outer"
      // - return statement is within it
      // - inner guard is within it
      // TODO: DISCUSS: the following is not working as expected due to the issue with
      // fully contains for ReturnSTSR
      // if (CurrSR.fullyContains(InnerSR) && CurrSR.fullyContains(ReturnSTSR)) {
      //   Lvl = GuardLevel::Outer;
      // }
      // workaround for the above
      if (Inner) {
        // we can now assign outer checks
        SourceRange InnerSR = Inner->getSourceRange();
        if (CurrSR.fullyContains(InnerSR) &&
            CurrSR.getBegin() <= ReturnSTSR.getBegin() &&
            CurrSR.getEnd() >= ReturnSTSR.getBegin()) {
          Lvl = GuardLevel::Outer;
        }
      } else {
        // check if this is the inner check
        if (CurrSR.getBegin() <= ReturnSTSR.getBegin() &&
            CurrSR.getEnd() >= ReturnSTSR.getBegin()) {
          Lvl = GuardLevel::Inner;
          Inner = Checks[I].first;
        }
      }
      T.Info.addErrorGuardingStmt(T.FID, Checks[I].first, ReturnST, T.Context,
                                  T.Heuristic, Lvl);
    }
  }
}

#endif //LLVM_CLANG_DETECTERR_VISITOR_UTILS_H
