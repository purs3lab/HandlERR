#ifndef LLVM_CLANG_DETECTERR_VISITORUTILS_H
#define LLVM_CLANG_DETECTERR_VISITORUTILS_H

#include "clang/DetectERR/ProjectInfo.h"

/// adds the error guarding statements to the project info
void addErrorGuardsToProjectInfo(
    ProjectInfo &Info, std::vector<std::pair<Stmt *, CFGBlock *>> &Checks);

#endif //LLVM_CLANG_DETECTERR_VISITORUTILS_H
