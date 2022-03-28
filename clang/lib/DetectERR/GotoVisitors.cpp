#include "clang/DetectERR/GotoVisitors.h"
#include "clang/DetectERR/Utils.h"

std::set<std::string> GotoVisitor::ErrorLabels = {
    "error",   "err",   "fail",   "bail",   "err_out",   "failed",
    "out_err", "bad",   "errout", "err1",   "error_ret", "error_out",
    "err2",    "fail1", "abort",  "error0", "fail2",
};

/// H08 - goto to an error label is control dependent on a check
bool GotoVisitor::VisitGotoStmt(GotoStmt *GotoST) {
  std::string LabelName = GotoST->getLabel()->getName().lower();
  if (ErrorLabels.find(LabelName) != ErrorLabels.end()) {
    if (StMap.find(GotoST) != StMap.end()) {
      CFGBlock *CurBB = StMap[GotoST];
      std::vector<std::pair<Stmt *, CFGBlock *>> Checks;
      collectChecks(Checks, *CurBB, &CDG);
      sortIntoInnerAndOuterChecks(Checks, &CDG);
      addErrorGuards(Checks, GotoST, *this);
    }
  }
  return true;
}
