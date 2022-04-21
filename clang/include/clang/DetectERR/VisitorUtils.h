#ifndef LLVM_CLANG_DETECTERR_VISITOR_UTILS_H
#define LLVM_CLANG_DETECTERR_VISITOR_UTILS_H

#include "clang/DetectERR/ErrGruard.h"

/// function to traverse the Checks and add ErrorGuards appropriately to the
/// Visitor
template <typename Visitor>
void addErrorGuards(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                    Stmt *ReturnST, Visitor &T) {
  for (unsigned long I = 0; I < Checks.size(); I++) {
    if (I == 0) {
      SourceRange CurrSR = Checks[I].first->getSourceRange();
      SourceRange ReturnSTSR = ReturnST->getSourceRange();
      GuardLevel Lvl = GuardLevel::Default;
      // TODO: DISCUSS: the following is not working as expected
      //      if (CurrSR.fullyContains(ReturnSTSR)) {
      //        Lvl = GuardLevel::Inner;
      //      }
      // workaround for the above
      if (CurrSR.getBegin() <= ReturnSTSR.getBegin() &&
          CurrSR.getEnd() >= ReturnSTSR.getBegin()) {
        Lvl = GuardLevel::Inner;
      }
      T.Info.addErrorGuardingStmt(T.FID, Checks[I].first, ReturnST, T.Context,
                                  T.Heuristic, Lvl);
    } else {
      // conditions for a guard to be "Outer"
      // - return statement is within it
      // - inner guard is within it
      SourceRange CurrSR = Checks[I].first->getSourceRange();
      SourceRange InnerSR = Checks[0].first->getSourceRange();
      SourceRange ReturnSTSR = ReturnST->getSourceRange();
      GuardLevel Lvl = GuardLevel::Default;
      // TODO: DISCUSS: the following is not working as expected due to the issue with
      // fully contains for ReturnSTSR
      // if (CurrSR.fullyContains(InnerSR) && CurrSR.fullyContains(ReturnSTSR)) {
      //   Lvl = GuardLevel::Outer;
      // }
      // workaround for the above
      if (CurrSR.fullyContains(InnerSR) && CurrSR.getBegin() <= ReturnSTSR.getBegin() &&
          CurrSR.getEnd() >= ReturnSTSR.getBegin()) {
        Lvl = GuardLevel::Outer;
      }
      T.Info.addErrorGuardingStmt(T.FID, Checks[I].first, ReturnST, T.Context,
                                  T.Heuristic, Lvl);
    }
  }
}

#endif //LLVM_CLANG_DETECTERR_VISITOR_UTILS_H
