#include "clang/DetectERR/ErrPoint.h"

std::string FnReturnTypeStr(FnReturnType &RT) {
  switch (RT) {
  case POINTER:
    return "Pointer";
  case INT:
    return "Int";
  default:
    llvm::errs() << "unknown fn return type: " << RT << "\n";
    exit(EXIT_FAILURE);
  }
}
