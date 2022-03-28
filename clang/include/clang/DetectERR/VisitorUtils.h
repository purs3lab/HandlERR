#ifndef LLVM_CLANG_DETECTERR_VISITOR_UTILS_H
#define LLVM_CLANG_DETECTERR_VISITOR_UTILS_H

#include "clang/DetectERR/ErrGruard.h"

/// function to traverse the Checks and add ErrorGuards appropriately to the
/// Visitor
template <typename Visitor>
void addErrorGuards(std::vector<std::pair<Stmt *, CFGBlock *>> &Checks,
                    Stmt *ReturnST, Visitor &T);

#endif //LLVM_CLANG_DETECTERR_VISITOR_UTILS_H
