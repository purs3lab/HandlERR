#ifndef LLVM_DUPLCIATE_H
#define LLVM_DUPLCIATE_H

#include "clang/AST/ASTContext.h"
#include "ProgramInfo.h"
#include "clang/AST/Decl.h"
#include "clang/AST/Stmt.h"
#include "clang/Rewrite/Core/Rewriter.h"

typedef std::function<bool (clang::VarDecl*)> selector;

//TODO this should be a bool for error reporting
void create_duplicate(clang::ASTContext &Context, clang::Rewriter &R,
                      ProgramInfo &Info,
                      clang::Decl *FD, selector P);



#endif //LLVM_DUPLCIATE_H
