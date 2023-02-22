#include "clang/DetectERR/ErrPoint.h"

std::string fnReturnTypeStr(const FnReturnType &RT) {
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

std::string ErrPoint::toJsonString() const {
  // {
  //     "FunctionInfo": {
  //         "Name": "bar",
  //         "File": "/home/shank/code/research/HandlERR/clang/tools/detecterr/utils/tests/retnull.c"
  //     },
  //     "ErrPoints": [
  //         {
  //             "File": "/home/shank/code/research/HandlERR/clang/tools/detecterr/utils/tests/retnull.c",
  //             "LineNo": 25,
  //             "ColNo": 3,
  //             "Heuristic": "FIFUZZ",
  //         }
  //     ]
  // }
  return "{\"File\":\"" + ErrPtLoc.getFileName() +
         "\", \"LineNo\":" + std::to_string(ErrPtLoc.getLineNo()) +
         ", \"ColNo\":" + std::to_string(ErrPtLoc.getColSNo()) +
         ", \"Heuristic\":\"FIFUZZ\"" + "}";
}
