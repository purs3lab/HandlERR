# `detecterr`: Automatically detect error handling conditional statements.

The `detecterr` tool will detect error guarding statements.

## Running the tool

```
detecterr --output=errblocks.json <repo_path>/clang/tools/detecterr/utils/tests/retnull.c 
```
The above command will produce `errblocks.json` which has the following contents:

```
{"ErrGuardingConditions":[{"FunctionInfo":{"Name":"foo", "File":"/home/machiry/projects/HandlERR/clang/tools/detecterr/utils/tests/retnull.c"},"ErrConditions":[{"File":"/home/machiry/projects/HandlERR/clang/tools/detecterr/utils/tests/retnull.c", "LineNo":3, "ColNo":3},{"File":"/home/machiry/projects/HandlERR/clang/tools/detecterr/utils/tests/retnull.c", "LineNo":5, "ColNo":5}]}
]}
```

## Source code organization
The main logic is present in the folder: `clang/lib/DetectERR`.

The main function is: `DetectERRASTConsumer::handleFuncDecl`, which calls various visitors in sequence.

Each of these visitors implement a heuristic and identified error guarding conditions.
