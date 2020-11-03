# 3C: Semi-automated conversion of C code to Checked C

The 3C (**C**hecked-**C**-**C**onvert) software facilitates conversion of existing C code to Checked C by automatically inferring as many Checked C annotations as possible and guiding the developer through the process of assigning the remaining ones.  3C aims to provide the first feasible way for organizations with large legacy C codebases (that they don't want to drop everything to rewrite in a better language) to comprehensively verify their code's spatial memory safety.

## What 3C users should know about the development process

Development of 3C is led by [Correct Computation, Inc.](https://correctcomputation.com/) (CCI) in the https://github.com/correctcomputation/checkedc-clang repository, which is a fork of Microsoft's Checked C repository (https://github.com/microsoft/checkedc-clang).  Issues and pull requests related to 3C should be submitted to the CCI repository; see [CONTRIBUTING.md](CONTRIBUTING.md) for more information.  Changes are merged frequently from Microsoft's repository to CCI's and less frequently in the opposite direction.  While some automated tests of 3C are run in Microsoft's repository, the coverage of these tests is currently mediocre, so changes to Microsoft's repository may break some functionality of its copy of 3C.  Thus, users of 3C can choose either CCI's repository (for the latest 3C with a somewhat older Checked C) or Microsoft's (for the latest Checked C with 3C that is significantly older and possibly broken).  This workflow may change in the future.

As of November 2020, 3C is pre-alpha quality and we are just starting to establish its public presence and processes.  Feel free to go ahead and file [issues](https://github.com/correctcomputation/checkedc-clang/issues) to help inform us of what users want; if we find that we're deluged with issues we're unprepared to address at this stage of development, we'll establish a different policy.  We strive to promptly fix any issues that block basic usage of 3C in our [supported environments](INSTALL.md#supported-environments), but beyond that, we have our own plans that are rapidly evolving and you should not expect that issues will be resolved in any given timeframe.

CCI is also working on a proprietary extension of 3C called 5C ("**C**orrect **C**omputation's **C**hecked-**C**-**C**onvert").  Our current plan is that 3C will contain the core inference logic, while 5C will add features to enhance developer productivity.  If you'd like more information about 5C, please contact us at info@correctcomputation.com.

## Functionality of 3C

(TODO)

(TODO: Is anything from https://github.com/correctcomputation/checkedc-clang/wiki/Checked-C-Convert#running-checked-c-convert worth including?)

This document describes the functionality of 3C in general.  The inference logic is implemented in the [`clang/lib/3C` directory](../../lib/3C) in the form of a library that can potentially be used in multiple contexts.  As of November 2020, the main way to use 3C is via the `3c` command line tool in the [`clang/tools/3c` directory](../../tools/3c); its usage is documented in [the readme there](../../tools/3c/README.md).  There is also a custom build of `clangd` named `clangd3c` that can be used to interactively convert code from within an IDE, but `clangd3c` is currently unmaintained and in poor shape; we may revive it at some point.

# TODO: Decide what to do about the old text (asked Mike)

## Redesigning and new features

Corresponding pull request: [Porting Tool Updates](https://github.com/microsoft/checkedc-clang/pull/642)

In this pull request, we tried to faithfully implement the highlevel idea as described in the paper (Section 5.3 of [https://www.microsoft.com/en-us/research/uploads/prod/2019/05/checkedc-post2019.pdf](https://www.microsoft.com/en-us/research/uploads/prod/2019/05/checkedc-post2019.pdf))

If you see the Section 5.3 of the paper, there depending on how a parameter to a function is **used** by the callers and **used** inside the callee, we use `itype`s or casting. To do this, we need to have two sets of constraint variables for each of the function parameters i.e., one for the calleers (i.e., as seen outside the function) and one for the function body. Once we solve the constraints we can assign `itype` or casts. Similarly for return types.

However, our old implementation had inconsistencies in inferring checked types of the function parameters and returns. Specifically, when a function do not have a corresponding declaration e.g., `static` methods, 

We changed this so that irrespective of whether a function has a explicit declaration or not we always maintain two sets of constraint variables for parameters and returns.

### Function Subtyping
We introduced the support for `_Nt_array_ptrs`, this resulted in function subtyping issues as discussed in detail in: [https://gist.github.com/Machiry/962bf8c24117bc5f56a31598e6782100](https://gist.github.com/Machiry/962bf8c24117bc5f56a31598e6782100)
We implemented support for this.

### Performance Improvements

We made performance imporvements to the tool. This is mainly because of a single line change (Thanks to @rchyena) in `ConstraintVariables.cpp`:
from:
```
214: auto env = CS.getVariables();
```
to
```
214: auto &env = CS.getVariables();
```
#### Numbers for Icecast :
**New**
```
real    3m51.869s  
user    3m50.132s  
sys    0m0.804s
```
**Old**
```
real    6m9.581s  
user    6m0.688s  
sys    0m8.004s
```
In general the current version of the tool is sufficiently fast, for `libarchive` (~170K lines) it took ~10min. 

