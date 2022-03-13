set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD} Option Support)

add_clang_library(
  clangdetecterr
  DetectERR.cpp
  DetectERRASTConsumer.cpp
  PersistentSourceLoc.cpp
  ErrGuard.cpp
  ProjectInfo.cpp
  EHFCollectors.cpp
  ReturnVisitors.cpp
  GotoVisitors.cpp
  EHFCallVisitors.cpp
  Utils.cpp
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
