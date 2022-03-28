#include "clang/DetectERR/VisitorUtils.h"

template <typename Visitor>
void addErrorGuards(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                    Stmt *ReturnST, Visitor &T) {
  for (unsigned long I = 0; I < Checks.size(); I++) {
    if (I == 0) {
      SourceRange CurrSR = Checks[I].first->getSourceRange();
      SourceRange ReturnSTSR = ReturnST->getSourceRange();
      GuardLevel Lvl = GuardLevel::Default;
      if (CurrSR.fullyContains(ReturnSTSR)) {
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
      if (CurrSR.fullyContains(InnerSR) && CurrSR.fullyContains(ReturnSTSR)) {
        Lvl = GuardLevel::Outer;
      }
      T.Info.addErrorGuardingStmt(T.FID, Checks[I].first, ReturnST, T.Context,
                                  T.Heuristic, Lvl);
    }
  }
}
