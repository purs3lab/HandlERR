#!/usr/bin/env python3
import sys
import subprocess
from typing import List
from difflib import SequenceMatcher
import re

# Copy paste from test_updater.py
keywords = "int char struct double float".split(" ")
ckeywords = "_Ptr _Array_ptr _Nt_array_ptr _Checked _Unchecked".split(" ")
keywords_re = re.compile("\\b(" + "|".join(keywords) + ")\\b")
ckeywords_re = re.compile("\\b(" + "|".join(ckeywords) + ")\\b")


def gen_checks(test_lines, conv_lines):
    matcher = SequenceMatcher(None, test_lines, conv_lines)
    checks = {}
    for c in matcher.get_opcodes():
        if c[0] in ['insert', 'replace']:
            insert_at_line = c[2] - 1
            take_from_range = conv_lines[c[3]:c[4]]
            # Hastily adapted from test_updater.py
            filtered_lines = [
                l for l in take_from_range if "/* GENERATE CHECK */" in l or
                (l.find("/*") == -1 and l.find("//") == -1 and (
                    (keywords_re.search(l) and
                     (l.find("*") != -1 or l.find("[") != -1)) or
                    ckeywords_re.search(l) or ckeywords_re.search(l)))
            ]
            checks[insert_at_line] = filtered_lines

    out = []
    for i in range(0, len(test_lines)):
        out = out + [test_lines[i]]
        if i in checks:
            out = out + ["//CHECK: " + l for l in checks[i]]

    return out


def rm_checks(lines):
    return [l for l in lines if "CHECK" not in l]


test_lines: List[str] = []
with open(sys.argv[1], 'r') as f:
    test_lines = rm_checks(f.readlines())

conv_lines: List[str] = rm_checks([
    l + '\n' for l in subprocess.check_output(["3c"] + sys.argv[1:],
                                              text=True).strip().split("\n")
])

print(''.join(gen_checks(test_lines, conv_lines)))
