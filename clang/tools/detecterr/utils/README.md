# Conversion Utilities

This directory contains a set of utilities to help with converting a codebase.
Python 3 required.

## convert-commands.py

This script takes two named arguments `compileCommands` (`-cc`) (the path to the
`compile_commands.json` for the configuration you plan to convert) and
`progName` (`-p`), which is the path to the `detecterr` binary. It reads the
`compile_commands.json` (which must match the fields produced by CMake's
versions of such files) and produces an output file which contains a
command-line invocation of `progName` with some flags and all `.c` files which
are compiled by this configuration (and thus should be converted by `3c`). This
file is currently saved as `convert_all.sh` and can be run directly as a shell
script. The `convert-commands.py` also creates `convert_individual.sh` file that
contains the commands to run the `detecterr` tool on individual source files.

### Example:
```
python convert-commands.py -dr --build_dir --build_dir <path_to_project_folder_containing_compile_commands.json> -p <path_to_the_detecterr_binary>
```

This will create `convert_all.sh` (which runs `detecterr` on all files at once) and `convert_individual.sh` (which runs `detecterr` on individaul files).

You can execute `convert_individual.sh` to run `detecterr` on all project files.

### Generating `compile_commands.json`
#### Using `cmake`
Use the CMAKE_EXPORT_COMPILE_COMMANDS flag. You can run
```
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ...
```
or add the following line to your CMakeLists.txt script:
```
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```
The `compile_commands.json` file will be put into the build directory.
#### Using `Bear` (Recommended)
For `make` and `cmake` based build systems, you can use `Bear`.

Install Bear from: https://github.com/rizsotto/Bear, commit hash: `75ff7f561652509ec9b34095881fdb6c4a56c9e0`

Prepend `bear` to your make command i.e., if you were running `make -j4` 
then run  `bear make -j4`. 
The `compile_commands.json` file will be put into the current directory.
