#!/usr/bin/env python3
"""
This tool will run detecterr on a bunch of libraries and produce the final
summary statistics regarding the findings.

Takes 3 arguments:
    - path to detecterr binary
    - path to folder containing archives of libraries
    - path to benchmarks folder
"""

import argparse
import os
import subprocess
import sys
import shutil


BEAR_PATH = None
PROG_PATH = None


def extract_archives(input_path):
    """Extracts the archives present in the folder `input_path`

    Retuns:
        the list of extracted directories
    """
    archives = []
    for f in os.listdir(input_path):
        # expected archives are tar.gz files
        base_dir = os.path.realpath(input_path)
        if f.endswith('tar.gz'):
            # TODO - parallelize this
            f = os.path.join(base_dir, f)
            archives.append(f)

    print(f"[+] found {len(archives)} archives...")

    extracted_dirs = []

    for f in archives:
        output_dir = os.path.join(
            os.path.dirname(f), os.path.basename(f).replace(".tar.gz", "")
        )

        if os.path.isdir(output_dir):
            print('[+] found existing directory, removing it')
            shutil.rmtree(output_dir)
            print('[+] removed existing directory')

        print(f"[+] extracting {f} to {output_dir}")
        # TODO - input sanitization??
        subprocess.check_call(f"mkdir -p {output_dir}", shell=True)
        subprocess.check_call(
            f"tar xf {f} --directory={output_dir}", shell=True)
        print(f"[+] extracting complete")
        extracted_dirs.append(output_dir)

    return extracted_dirs


def configure_and_bear_make_single(path):
    """
    Runs `configure` and `bear make` on the `path` directory
    """
    print(f"running configure_and_bear_make_single on {path}")
    # configure file
    if 'configure' in os.listdir(path):
        print("[+] running configure...")
        subprocess.check_call(f"./configure", shell=True, cwd=path)
        print("[+] configure done")

    # bear make
    print("[+] running bear make...")
    subprocess.check_call(f"{BEAR_PATH} make", shell=True, cwd=path)
    print("[+] bear make done")


def configure_and_bear_make_all(input_path):
    """
    Runs `configure` and `bear make` on each of the extracted archives

    Returns:
        the directories where `bear make` was run for each input
    """
    build_dirs = []
    for p in input_path:
        # its possible that the extracted library is in a sub-folder of the
        # current directory. Hence we will use the heuristic that if the
        # given directory contains only a single directory, then that directory
        # is the one that actually contains the extracted archive
        probable_dirs = []
        for d in os.listdir(p):
            if os.path.isdir(os.path.join(p, d)):
                probable_dirs.append(os.path.join(p, d))
        if len(probable_dirs) == 1:
            configure_and_bear_make_single(probable_dirs[0])
            build_dirs.append(probable_dirs[0])
        else:
            configure_and_bear_make_single(p)
            build_dirs.append(p)

    return build_dirs


def extract_and_configure_archives(input_path):
    """
    Extracts and configures the archives placed in `input_path`

    Returns:
        the directories where `bear make` was run for each input
    """
    extracted = extract_archives(input_path)
    build_dirs = configure_and_bear_make_all(extracted)
    return build_dirs


def convert_project(build_dirs):
    """
    Runs convert_project.py using the compile_commands.json from each of the
    build_dirs.
    """
    convert_project_bin = os.path.join(
        os.path.dirname(os.path.realpath(__file__)), "port_tools", "convert_project.py")
    for d in build_dirs:
        print(f"[+] converting project {d}")
        if "compile_commands.json" in os.listdir(d):
            print("[+] compile_commands.json found")
            print("[+] running convert_project.py")
            subprocess.check_call(
                f"{convert_project_bin} -dr -pr {d} -p {PROG_PATH}", shell=True)
        else:
            print("[+] compile_commands.json not found. skipping this one.")
        print(f"[+] converting project done")


def run_main(args):
    # TODO
    # - Extract, run configure, and run bear make
    # - Run convert_project.py using the compile_commands.json and path to the
    #   detecterr binary
    # - Collect the individual err json files generated for each .c file and
    #   create a cumulative json file for the entire project.
    # - Finally, have a single csv file with summaries for each project

    build_dirs = extract_and_configure_archives(args.input_path)
    convert_project(build_dirs)


if __name__ == '__main__':

    parser = argparse.ArgumentParser(__file__, description="Tool that converts the compilation commands into"
                                     " the commands for 3c tool and also "
                                     " runs the tool.")

    parser.add_argument("-p", "--prog_path", dest='prog_path', type=str,
                        help='Program path to run. i.e., path to detecterr')

    parser.add_argument("-i", "--input_path", dest='input_path', type=str,
                        help='Path to input folder containing archives')

    parser.add_argument("-m", "--benchmarks_path", dest='benchmarks_path', type=str,
                        help='Path to benchmarks folder where the output is to be stored')

    parser.add_argument("-b", "--bear_path", dest='bear_path', type=str,
                        help='Path to bear binary')

    args = parser.parse_args()

    if not args.prog_path or not os.path.isfile(args.prog_path):
        print("Error: Path to the program to run is invalid.")
        print("Provided argument: {} is not a file.".format(args.prog_path))
        sys.exit()

    if not args.input_path or not os.path.isdir(args.input_path):
        print("Error: Path to the input archives folder is invalid.")
        print("Provided argument: {} is not a directory.".format(args.input_path))
        sys.exit()

    if not args.benchmarks_path or not os.path.isdir(args.benchmarks_path):
        print("Error: Path to the benchmarks folder is invalid.")
        print("Provided argument: {} is not a directory.".format(
            args.benchmarks_path))
        sys.exit()

    if not args.bear_path or not os.path.isfile(args.bear_path):
        print("Error: Path to the bear binary is invalid.")
        print("Provided argument: {} is not a file.".format(args.bear_path))
        sys.exit()

    BEAR_PATH = args.bear_path
    PROG_PATH = args.prog_path

    run_main(args)
