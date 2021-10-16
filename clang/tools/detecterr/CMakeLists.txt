set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Option
  Support
  )

add_clang_tool(detecterr
        DetectERRMain.cpp
  )

target_link_libraries(detecterr
  PRIVATE
  clangdetecterr
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  )

install(TARGETS detecterr
  RUNTIME DESTINATION bin)
