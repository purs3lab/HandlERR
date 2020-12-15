# script_generator.py: Generates an unexpanded RUN script for a test file based
# on its name and flags that specify the kind of testing to be done. Used by
# both 3c-regtest.py and 3c-regtest-unconvert.py.

import os
import re
import argparse

def remove_suffix(string, suffix):
    return string[:-len(suffix)] if string.endswith(suffix) else None

parser = argparse.ArgumentParser(
    # This will be shown as if it is a program name runnable by the user. It
    # isn't, but give the user a clue what this argument parser represents.
    prog='<3c-regtest script generator>',
    # Without this, the parser in 3c-regtest.py raises an error about
    # conflicting --help options. That's silly; one would expect argparse to
    # handle this specially.
    add_help=False
)
# TODO: Add help
parser.add_argument('test_file')
# Future: Redesign in terms of kinds of testing rather than merely selecting one
# of the existing 5 scripts of the test updating programs.
parser.add_argument('--predefined-script', required=True,
                    choices=['processor'])

# Unlike all the other % codes that are substituted by `lit`, %B is a special
# code that generate_script (below) replaces with the basename of the test file
# without the `.c` extension. This reproduces some of the scripts in existing
# test files.
predefined_scripts = {
    'processor': '''\
3c -alltypes -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_ALL","CHECK" %s
3c -addcr %s -- | FileCheck -match-full-lines -check-prefixes="CHECK_NOALL","CHECK" %s
3c -addcr %s -- | %clang -c -fcheckedc-extension -x c -o /dev/null -
3c -output-postfix=checked -alltypes %s
3c -alltypes %S/%B.checked.c -- | count 0
rm %S/%B.checked.c
''',
}

# `argobj` should be an argument object returned by `parser` or a parser based
# on it. Returns a list of command strings.
def generate_commands(argobj):
    test_file = argobj.test_file
    # FUTURE: If this assumption is no longer valid, update this code.
    test_file_basename_without_ext = remove_suffix(os.path.basename(test_file), '.c')
    if test_file_basename_without_ext is None:
        sys.exit('Test filename does not end in `.c`.')
    script = remove_suffix(predefined_scripts[argobj.predefined_script], '\n')
    script = re.sub('%B', test_file_basename_without_ext, script)
    return script.split('\n')
