# DetectERR Repo

This repository contains a version of LLVM/Clang toolchain alongwith the Clang-frontend tool called `DetectERR`. This tool is a part of our research paper accepted at AsiaCCS-2024, titled "Fuzzing API Error Handling Behaviors using Coverage Guided Fault Injection".

`DetectERR` identifies potential error injection points in a given library source code and provides this information in the form of a `ErrorBlocks.json`. Please refer our paper for further details.

## Build DetectERR

The instruction below are assuming Ubuntu-22.04 as the OS.

1. Install the prerequisites: clang-13, llvm-13, ninja, gdb, lld-13, bear.

2. Build
    mkdir build && cd build
    cmake ../llvm \
        -GNinja \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
        -DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra" \
        -DLLVM_PARALLEL_LINK_JOBS=2 \
        -DLLVM_TARGETS_TO_BUILD="host" \
        -DLLVM_OPTIMIZED_TABLEGEN="ON" \
        -DCLANG_INCLUDE_DOCS="OFF" \
        -DLLVM_INCLUDE_TESTS="OFF" \
        -DLLVM_USE_LINKER=lld \
        -DCMAKE_SHARED_LINKER_FLAGS='-B$ldpath -Wl,--gdb-index' \
        -DCMAKE_EXE_LINKER_FLAGS='-B$ldpath -Wl,--gdb-index'
    cmake --build . -- detecterr -j$(nproc)

## Run DetectERR

The repository contains a python script `runtool.py` that can run `detecterr` on the directory containing the source code for the library. The following command is to be run from the root of the DetectERR repository.

    clang/tools/detecterr/utils/runtool.py \
    -p </path/to/detecterr> \
    -m </path/to/detecterr_benchmarks> \
    -b </path/to/bear> \
    -s </path/to/extracted/library-code>

where:

1. `</path/to/detecterr>` is the path to the `detecterr` binary (usually `build/bin/detecterr`).
2. `</path/to/detecterr_benchmarks>` is the path to a directory where `detecterr` will write out some intermendiate files while processing the library code.
3. `</path/to/bear>` is the path to `bear` binary (usually `/usr/bin/bear`).
4. `</path/to/extracted/library-code>` is the path to the library source code. Please note that this directory should contain the compilation database file (`compile_commands.json`) for the library.

Below is a sample excerpt from the generated errblocks.json file.

```json
[
    ...
    {
        "FunctionInfo": {
            "Name": "process_data_context_main",
            "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c"
        },
        "ErrConditions": [
            {
                "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                "LineNo": 339,
                "ColNo": 5,
                "Heuristic": "H07",
                "Level": "Inner",
                "ErrorLoc": {
                    "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                    "LineNo": 341,
                    "ColNo": 7
                }
            },
            {
                "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                "LineNo": 359,
                "ColNo": 5,
                "Heuristic": "H07",
                "Level": "Inner",
                "ErrorLoc": {
                    "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                    "LineNo": 360,
                    "ColNo": 7
                }
            },
            {
                "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                "LineNo": 359,
                "ColNo": 5,
                "Heuristic": "H07",
                "Level": "Default",
                "ErrorLoc": {
                    "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                    "LineNo": 363,
                    "ColNo": 7
                }
            },
            {
                "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                "LineNo": 383,
                "ColNo": 5,
                "Heuristic": "H07",
                "Level": "Inner",
                "ErrorLoc": {
                    "File": "/home/shank/code/research/detecterr_input/libjpeg-turbo_2.1.2.orig/libjpeg-turbo-2.1.2/jdmainct.c",
                    "LineNo": 384,
                    "ColNo": 7
                }
            }
        ]
    },
    ...
]
```
