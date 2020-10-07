set(LLVM_LINK_COMPONENTS support)

add_clang_executable(interview-template
  InterviewTemplate.cpp
  )
target_link_libraries(interview-template
  PRIVATE
  clangTooling
  clangBasic
  clangASTMatchers
  )
