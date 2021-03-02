set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Option
  Support
  )

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/DeclRewriter_5C.cpp)
  set(FIVE_C ON)
endif()

if (FIVE_C)
  set(five_c_source
    DeclRewriter_5C.cpp
  )
endif()

if (MSVC)
  set_source_files_properties(ArrayBoundsInferenceConsumer.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(ConstraintBuilder.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(DeclRewriter.cpp PROPERTIES COMPILE_FLAGS /bigobj)
  set_source_files_properties(RewriteUtils.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

add_clang_library(clang3C
  ABounds.cpp
  AVarBoundsInfo.cpp
  AVarGraph.cpp
  ArrayBoundsInferenceConsumer.cpp
  CastPlacement.cpp
  3C.cpp
  3CInteractiveData.cpp
  CheckedRegions.cpp
  ConstraintBuilder.cpp
  ConstraintResolver.cpp
  Constraints.cpp
  ConstraintsGraph.cpp
  ConstraintVariables.cpp
  DeclRewriter.cpp
  IntermediateToolHook.cpp
  MappingVisitor.cpp
  PersistentSourceLoc.cpp
  ProgramInfo.cpp
  ProgramVar.cpp
  RewriteUtils.cpp
  StructInit.cpp
  TypeVariableAnalysis.cpp
  Utils.cpp
  ${five_c_source}
  LINK_LIBS
  clangAST
  clangAnalysis
  clangBasic
  clangDriver
  clangFrontend
  clangRewriteFrontend
  clangStaticAnalyzerFrontend
  clangTooling
  clangTransformer
)

if (FIVE_C)
  # For each library X, the Clang build system (at least as currently configured
  # for us) generates a CMake object library obj.X that neither propagates
  # "usage requirements" (such as target_compile_definitions) to X nor receives
  # them from its dependencies. So in order to take advantage of CMake's support
  # for usage requirements, we have to set them on both X and obj.X and then for
  # each library Y dependent on X, add a manual target_link_libraries(obj.Y X),
  # which has no effect other than propagating usage requirements
  # (https://cmake.org/cmake/help/v3.16/command/target_link_libraries.html#linking-object-libraries).
  target_compile_definitions(obj.clang3C PUBLIC FIVE_C)
  target_compile_definitions(clang3C PUBLIC FIVE_C)
endif()
