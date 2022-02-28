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

    if(FnDecl->getNameInfo().getAsString() == "foo"){
      __dbg_print_statements(StMap);
    }

    auto Parents = Context->getParents(*S);
    if (!Parents.empty()){
      auto Parent = Parents[0];
      Parent.dump(llvm::errs(), *Context);
      const Stmt *St = Parent.get<Stmt>();
      if (StMap.find(S) != StMap.end()) {
        llvm::errs() << "found parent of Goto!!\n";
      }else{
        llvm::errs() << "No luck bruh :(\n";
      }

      llvm::errs() << "here";
    }
    if (StMap.find(S) != StMap.end()) {
      CFGBlock *CurBB = StMap[S];
      auto &CDNodes = CDG.getControlDependencies(CurBB);
      if (!CDNodes.empty()) {
        // We should use all CDs
        // Get the last statement from the list of control dependencies.
        for (auto &CDGNode : CDNodes) {
          // Collect the possible length bounds keys.
          Stmt *TStmt = CDGNode->getTerminatorStmt();
          // check if this is an if statement.
          if (dyn_cast_or_null<IfStmt>(TStmt) ||
              dyn_cast_or_null<WhileStmt>(TStmt) ||
              dyn_cast_or_null<SwitchStmt>(TStmt)) {
            Info.addErrorGuardingStmt(FID, TStmt, Context, Heuristic);
          }
        }
      }
    }
  }
  return true;
}
