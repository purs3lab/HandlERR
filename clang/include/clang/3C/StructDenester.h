#ifndef LLVM_STRUCTDENESTER_H
#define LLVM_STRUCTDENESTER_H

#include "clang/AST/ASTContext.h"

void denestStructs(clang::ASTContext &C);

#endif // LLVM_STRUCTDENESTER_H
