#include "clang/DetectERR/GotoVisitors.h"
#include "clang/DetectERR/Utils.h"

std::set<std::string> GotoVisitor::ErrorLabels = {
    "error",   "err",   "fail",   "bail",   "err_out",   "failed",
    "out_err", "bad",   "errout", "err1",   "error_ret", "error_out",
    "err2",    "fail1", "abort",  "error0", "fail2",
};

/// H08 - goto to an error label is control dependent on a check
bool GotoVisitor::VisitGotoStmt(GotoStmt *S) {
  std::string LabelName = S->getLabel()->getName().lower();
  if (ErrorLabels.find(LabelName) != ErrorLabels.end()) {
    if (StMap.find(S) != StMap.end()) {
      CFGBlock *CurBB = StMap[S];
      std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
      collectChecks(Checks, *CurBB, &CDG);
      sortIntoInnerAndOuterChecks(Checks, &CDG);
      for (unsigned long I = 0; I < Checks.size(); I++) {
        if (I == 0) {
          Info.addErrorGuardingStmt(FID, Checks[I].first, Context, Heuristic,
                                    GuardLevel::Inner);
        } else {
          Info.addErrorGuardingStmt(FID, Checks[I].first, Context, Heuristic,
                                    GuardLevel::Outer);
        }
      }
    }
  }
  return true;
}
