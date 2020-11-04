# `3c`: Command-line tool for conversion of C code to Checked C

The `3c` tool is a command-line interface to the 3C software for conversion of C code to Checked C.  See the [3C readme](../../docs/checkedc/3C/README.md) for general information about 3C and how to install it.  This document describes how to use `3c`.  It assumes that you have added the `build/bin` directory containing the `3c` executable to your `$PATH`.

`3c` supports an iterative workflow for converting a C program to Checked C as follows:

1. Run `3c`.  It reads a set of `.c` files and the header files they transitively include (these files may be in Checked C or, as a special case, plain C), performs a whole-program static analysis to infer as many additions or corrections to Checked C annotations as possible, and writes out the updated files.  `3c` accepts various flags to control the assumptions it makes and the types of changes it makes on a given pass; you can adjust these flags to suit your needs.

2. Review the changes made by `3c` as well as what it didn't change and any diagnostics it produced.  Manually add annotations to help Checked C verify the safety of your existing code, edit your code to make its safety easier to verify, and/or mark parts of the code that you don't want to try to verify with Checked C (because you know they are beyond what Checked C can handle or verifying them just isn't worthwhile to you at the moment).

3. Repeat until you are satisfied.

The task of `3c` is complicated by the fact that the build system for a typical C codebase will call the C compiler once per `.c` file, possibly with different flags for each file (include directories, preprocessor definitions, etc.).  A Clang-based whole-program analysis like 3C needs to process all `.c` files at once _with the correct flags for each_.  To achieve this, you get your build system to produce a Clang "compilation database" (a file named `compile_commands.json`) containing the list of `.c` files and the flags for each ([how to do this depends on the build system](../../docs/JSONCompilationDatabase.rst)), and then 3C reads this database.

However, in a simpler setting, you can manually run `3c` on one or more source files, for example:

```
3c -alltypes -output-postfix=checked foo.c bar.c
```

This will write the new version of each file `f.c` to `f.checked.c` in the same directory (or `f.h` to `f.checked.h`).  If `f.checked.c` would be identical to `f.c`, it is not created.  As an additional safeguard, `f.checked.c` is not written if it is outside the _base directory_, which defaults to the working directory but can be overridden with the `-base-dir` flag.

You can ignore the errors about a compilation database not being found.  You can specify a single set of flags to use for all files by prefixing them with `-extra-arg-before=`, for example:

```
3c -alltypes -output-postfix=checked -extra-arg-before=-Isome/include/path foo.c bar.c
```

(If you were using a compilation database, such "extra" flags would be added to any flags in the database.)

The `-alltypes` option causes `3c` to try to infer array types.  We want to make this the default but haven't done so yet because it breaks some things.

(TODO: Did I miss anything?)

(TODO: Is anything from https://github.com/correctcomputation/checkedc-clang/wiki/Checked-C-Convert#running-checked-c-convert worth including?)
