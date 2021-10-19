set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Option
  Support
  )

add_clang_tool(diagcollecter
        DiagCollectorMain.cpp
  )

target_link_libraries(diagcollecter
  PRIVATE
  clangAST
  clangBasic
  clangDriver
  clangFrontend
  clangTooling
  )

install(TARGETS diagcollecter
  RUNTIME DESTINATION bin)
