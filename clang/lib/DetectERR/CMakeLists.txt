set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD} Option Support)

add_clang_library(
  clangdetecterr
  DetectERR.cpp
  DetectERRASTConsumer.cpp
  PersistentSourceLoc.cpp
  ErrGuard.cpp
  ErrPoint.cpp
  ProjectInfo.cpp
  EHFCollectors.cpp
  Utils.cpp
  ReturnVisitors.cpp
  EHFCallVisitors.cpp
  GotoVisitors.cpp
  ThrowVisitors.cpp
  FiFuzzVisitors.cpp
  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  clangTransformer)
