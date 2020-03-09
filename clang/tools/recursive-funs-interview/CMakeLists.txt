set( LLVM_LINK_COMPONENTS
        ${LLVM_TARGETS_TO_BUILD}
        Option
        Support
        )

add_clang_executable(recursive-funs-intv
        RecursiveFunsIntv.cpp
        )

target_link_libraries(recursive-funs-intv
        PRIVATE
        clangAST
        clangBasic
        clangDriver
        clangFrontend
        clangRewriteFrontend
        clangStaticAnalyzerFrontend
        clangTooling
        )

install(TARGETS recursive-funs-intv
        RUNTIME DESTINATION bin)