#!/usr/bin/env python
#
# Usage: 3c-regtest -t TMPNAME [options...] SRC_FILE
#
# TMPNAME corresponds to %t and SRC_FILE corresponds to %s. Both are required.
#
# --subst can be used for things such as %clang, for example (assuming single
# --quotes are removed by the shell):
#
# --subst %clang 'clang -some-flag'
#
# (Note: A literal % has to be represented as %% in a RUN line. If we instead
# established the convention of automatically prepending the % here, then the
# RUN line would trip the "Do not use 'clang' in tests, use '%clang'." error.)
#
# Example RUN line:
#
# // RUN: %S/3c-regtest.py -t %t --subst %%clang '%clang' %s
#
# Soon, we'll add options for different kinds of 3C regression tests.

# TODO: Add Windows compatibility code once we have an easy way to test on Windows.

import sys
import os
import platform
import argparse

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)) +
                '/../../../llvm/utils/lit')
import lit.TestRunner

print "NOTICE: cwd is %s" % os.getcwd()

def die(msg):
    sys.stderr.write('Error: %s\n' % msg)
    sys.exit(1)

parser = argparse.ArgumentParser(description='Run a 3C regression test.')
# TODO: Add help
parser.add_argument('test_file')
parser.add_argument('-t', required=True)
parser.add_argument('--subst', action='append', nargs=2, default=[])
args = parser.parse_args()

test_dir = os.path.dirname(args.test_file)
if test_dir == '':
    test_dir = '.'

tmpName = args.t
tmpNameSuffix = '.tmp'
if tmpName.endswith(tmpNameSuffix):
    tmpBase = tmpName[:-len(tmpNameSuffix)]
else:
    die('-t argument %s does not end with %s' % (tmpName, tmpNameSuffix))

substitutions = [
    # #_MARKER_# is a hack copied from getDefaultSubstitutions in
    # llvm/utils/lit/lit/TestRunner.py. To explain it a bit more fully:
    # applySubstitutions processes each before/after pair in turn and replaces
    # all occurrences. So if we simply put ('%%', '%') as either the first or
    # last pair, there is a risk of a % being interpreted as part of a %s, etc.
    # Instead, we temporarily replace %% with a string that doesn't contain %
    # and that we assume doesn't occur elsewhere in the commands. The "proper"
    # way to implement this kind of substitution processing is to make one pass
    # over the input from left to right, replacing codes as they are found, but
    # apparently that wasn't worth the extra code in `lit`.
    ('%%', '#_MARKER_#'),
    ('%s', args.test_file),
    ('%S', test_dir),
    ('%t', tmpName),
]
substitutions.extend(args.subst)
substitutions.append(('#_MARKER_#', '%'))

# Starting with processor.py because it's always the same.
commands = [
    # FIXME: 'foo.c' + 'hecked.c' is a terrible hack; find the right way to do this.
    '3c -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s',
    '3c -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s',
    '3c -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -',
    '3c -output-postfix=checked -alltypes %s',
    '3c -alltypes %shecked.c -- | count 0',
    'rm %shecked.c',
]
commands = lit.TestRunner.applySubstitutions(commands, substitutions)

class FakeTestConfig:
    def __init__(self):
        self.pipefail = True  # Is this always OK?
        self.environment = dict(os.environ)

class FakeTest:
    def __init__(self):
        self.config = FakeTestConfig()

class FakeLitConfig:
    def __init__(self):
        self.isWindows = platform.system() == 'windows'
        # Let the calling `lit` handle any timeout.
        self.maxIndividualTestTime = 0

res = lit.TestRunner.executeScriptInternal(
    FakeTest(), FakeLitConfig(), tmpBase, commands, os.getcwd())
if isinstance(res, lit.Test.Result):
    die('Error: executeScriptInternal returned unexpected Result(%s, %r)' %
        (res.code.name, res.output))

out, err, exitCode, timeoutInfo = res
sys.stdout.write(out)
sys.stderr.write(err)
sys.exit(exitCode)
