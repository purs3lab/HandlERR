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
import json
import os
import shutil
import subprocess
import sys
import datetime
import multiprocessing

BEAR_PATH = None
PROG_PATH = None
BENCHMARKS_PATH = None
NAMES_LIKE = None
DEBUG = None

# number of usable cpus
NUM_CPUS = len(os.sched_getaffinity(0))


def extract_archives(input_path):
    """Extracts the archives present in the folder `input_path`

    Retuns:
        the list of tuples of (extracted dir, build inst) for the extracted
        directories
    """
    archives = []
    for f in os.listdir(input_path):
        # in case we are looking for some specific files
        if NAMES_LIKE and not any(n in f for n in NAMES_LIKE):
            print(f"[+] skipping {f} as it does not match {NAMES_LIKE}")
            continue

        base_dir = os.path.realpath(input_path)
        # expected archives are tar.gz files
        if f.endswith("tar.gz") or f.endswith("tar.bz2"):
            # TODO - parallelize this
            f = os.path.join(base_dir, f)
            archives.append(f)

    print(f"[+] found {len(archives)} archives...")

    extracted_dirs = []

    for f in archives:
        # the folder name for the extracted archive
        basename = os.path.basename(f).replace(".tar.bz2", "")
        basename = basename.replace(".tar.gz", "")
        output_dir = os.path.join(os.path.dirname(f), basename)

        # remove existing directory
        if os.path.isdir(output_dir):
            print("[+] found existing directory, removing it")
            shutil.rmtree(output_dir)
            print("[+] removed existing directory")

        # extract the archive
        print(f"[+] extracting {f} to {output_dir}")
        subprocess.check_call(f"mkdir -p {output_dir}", shell=True)
        subprocess.check_call(
            f"tar xf {f} --directory={output_dir}", shell=True)
        print(f"[+] extracting complete")

        # custom instructions provided?
        build_inst = None
        if os.path.isfile(f + ".build.inst"):
            build_inst = f + ".build.inst"

        extracted_dirs.append((output_dir, build_inst))

    return extracted_dirs


def configure_and_bear_make_single(path, build_inst=None):
    """
    Runs `configure` and `bear make` on the `path` directory.

    If build_inst is not None, then the steps mentioned in the build_inst file
    are executed before doing 'configure' and 'make'.
    """
    print(f"running configure_and_bear_make_single on {path}")

    # libboost, the wierd one...
    if "boost" in path:
        print("libboost detected, doing the special instructions...")
        # bootstrap
        subprocess.check_call(
            (
                "./bootstrap.sh --with-toolset=clang --with-libraries=filesystem "
            ),
            shell=True,
            cwd=path,
        )
        # bear b2
        subprocess.check_call(
            f"bear ./b2 link=shared", shell=True, cwd=path,
        )
        return

    # custom build_inst
    if build_inst:
        print(f"running custome build_inst first")
        with open(build_inst) as inst_f:
            # execute one instruction at a time
            inst = inst_f.readline().strip()
            print(f"[+] running: {inst}")
            subprocess.check_call(f"{inst}", shell=True, cwd=path)
            print("[+] configure done")

    # autogen.sh required
    autogen_required_libs = [
        "libxml2",
        "procps",
        "cairo",
    ]
    for name in autogen_required_libs:
        if name in path:
            subprocess.check_call("./autogen.sh", shell=True, cwd=path)
            break

    # configure file
    if "configure" in os.listdir(path):
        print("[+] running configure...")
        if "libgcrypt" in path:
            # custom for libgcrypt
            subprocess.check_call(
                "./configure --enable-maintainer-mode", shell=True, cwd=path
            )

        elif "glibc" in path:
            # custom for glibc
            subprocess.check_call(f"mkdir -p ../build", shell=True, cwd=path)
            configure_path = os.path.join(path, "configure")
            path = os.path.join(path, "../build")
            print(f"path: {path}")
            subprocess.check_call(
                f"CC=gcc CXX=g++ {configure_path} --prefix=/tmp/glibc",
                shell=True,
                cwd=path,
            )

        elif "elf" in path:
            # custom for libelf
            subprocess.check_call(
                "CC=gcc CXX=g++ ./configure --disable-debuginfod --disable-libdebuginfod",
                shell=True,
                cwd=path,
            )

        elif "ffmpeg" in path:
            # custom for ffmpeg libs (libavcodec, libavformat, libavutil)
            subprocess.check_call(
                "./configure --enable-shared --disable-doc",
                shell=True,
                cwd=path,
            )

        elif "cairo" in path:
            # custom for cairo
            subprocess.check_call(
                "sed 's/PTR/void */' -i ./util/cairo-trace/lookup-symbol.c",
                shell=True,
                cwd=path,
            )
            subprocess.check_call(
                "./configure --disable-static CC=clang CXX=clang++",
                shell=True,
                cwd=path,
            )

        else:
            # normal libraries, just do ./configure
            subprocess.check_call("./configure", shell=True, cwd=path)
        print("[+] configure done")

    # poppler -> cmake
    if "poppler" in path:
        print("[+] working with poppler")
        subprocess.check_call(f"mkdir -p build", shell=True, cwd=path)
        path = os.path.join(path, "build")
        subprocess.check_call(
            "CC=clang CXX=clang++ cmake .. "
            '-DCMAKE_C_FLAGS="-g -O0" -DCMAKE_CXX_FLAGS="-g -O0" '
            "-DCMAKE_BUILD_TYPE=debug -DENABLE_BOOST=OFF",
            shell=True,
            cwd=path,
        )

    # libjpeg -> cmake
    if "libjpeg" in path:
        print("[+] working with libjpeg")
        subprocess.check_call("mkdir -p build", shell=True, cwd=path)
        path = os.path.join(path, "build")
        subprocess.check_call(
            "CC=clang CXX=clang++ cmake .. -DCMAKE_BUILD_TYPE=debug -DWITH_JPEG8=ON -DENABLE_STATIC=False -DWITH_SIMD=OFF",
            shell=True,
            cwd=path,
        )

    # openssl -> Configure
    if "ssl" in path:
        print("[+] working with openssl")
        subprocess.check_call(
            "CC=clang CXX=clang++ ./Configure --debug",
            shell=True,
            cwd=path,
        )

    # bear make
    print("[+] running bear make...")
    subprocess.check_call(
        f"{BEAR_PATH} make -j{NUM_CPUS}", shell=True, cwd=path)
    print("[+] bear make done")


def configure_and_bear_make_all(extracted):
    """
    Runs `configure` and `bear make` on each of the extracted archives

    Args:
        extracted: list of tuples of (dir, build_inst)

    Returns:
        the directories where `bear make` was run for each input
    """
    build_dirs = []
    for (p, build_inst) in extracted:
        # its possible that the extracted library is in a sub-folder of the
        # current directory. Hence we will use the heuristic that if the
        # given directory contains only a single directory, then that directory
        # is the one that actually contains the extracted archive
        probable_dirs = []
        for d in os.listdir(p):
            if os.path.isdir(os.path.join(p, d)):
                probable_dirs.append(os.path.join(p, d))
        if len(probable_dirs) == 1:
            configure_and_bear_make_single(probable_dirs[0], build_inst)
            build_dirs.append(probable_dirs[0])
        else:
            configure_and_bear_make_single(p, build_inst)
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
        os.path.dirname(os.path.realpath(__file__)
                        ), "port_tools", "convert_project.py"
    )
    for d in build_dirs:
        # libc - special case
        if "libc" in d:
            d = os.path.join(d, "../build")

        # cmake cases (cd to build dir)
        # poppler
        # libjpeg
        if "poppler" in d or "libjpeg" in d:
            d = os.path.join(d, "build")

        print(f"[+] converting project {d}")
        if "compile_commands.json" in os.listdir(d):
            print("[+] compile_commands.json found")
            print(f"[+] running {convert_project_bin}")
            subprocess.check_call(
                f"{convert_project_bin} -dr -pr {d} -p {PROG_PATH}", shell=True
            )
        else:
            print("[+] compile_commands.json not found. skipping this one.")

        print("[+] converting project done")


def process(cmd):
    print(f"subprocessing cmd: {cmd}")
    return subprocess.check_call(cmd, shell=True)


def run_tool_on_all(dirs):
    """
    Runs the convert_individual.sh script present in each of the directories in
    the list passed as argument. Each of these directories is expected to
    contain a compile_individual.sh and a convert_all.sh script (generated
    from convert_project.py).
    """

    for d in dirs:
        cmds = []
        curr = ""
        if DEBUG:
            pool = multiprocessing.Pool(1)
        else:
            pool = multiprocessing.Pool(max(NUM_CPUS - 2, 1))

        # libc - special case
        if "libc" in d:
            d = os.path.join(d, "../build")

        # cmake cases
        # poppler
        # libjpeg
        if "poppler" in d or "libjpeg" in d:
            d = os.path.join(d, "build")

        print(f"[+] running tool on {d}")
        convert_individual_script = os.path.join(d, "convert_individual.sh")

        # >> existing - we have now parallized this stuff
        # subprocess.check_call(f"{convert_individual_script}", shell=True, cwd=d)

        # parallelize the process of running detecterr on individual files
        with open(convert_individual_script, "r") as f:
            lines = list(line.strip() for line in f.readlines())
            for (i, line) in enumerate(lines[1:]):  # skip first line (shebang)
                if line.startswith("cd"):
                    if i != 0:
                        cmds.append(curr)
                    curr = line.rstrip("\\")
                else:
                    curr += line.rstrip("\\")

        pool.imap(process, cmds)
        pool.close()
        pool.join()

        print("[+] running tool done")


def process_errblocks_for_dir(dir):
    """
    processes errblocks.jsons in the dir (recursively) and returns the
    cumulative data as a dictionary

    Args:
        dir: the absolute path to the directory to be processed

    Retuns:
        the list of maps contianing the cumulative errblocks.json data
    """
    if not os.path.isdir(dir):
        print(f"[+] {dir} is not a dir, ignoring")
        return []

    cumulative_data = []

    for f in os.listdir(dir):
        f_abspath = os.path.join(dir, f)

        # process files
        if (
            os.path.isfile(f_abspath)
            and f.endswith("errblocks.json")
            and f != "__project.errblocks.json"
        ):
            print(f"[+] processing {f_abspath}")
            with open(f_abspath) as err_file:
                data = json.load(err_file)
                cumulative_data.extend(data["ErrGuardingConditions"])

        # process dirs
        elif os.path.isdir(f_abspath):
            cumulative_data.extend(process_errblocks_for_dir(f_abspath))

    return cumulative_data


def _is_relative(file):
    """
    Checks if the given filepath is relative (starts with ../)
    """
    return file.startswith("../")


def create_cumulative_errblocks_json_for_each(dirs):
    """
    Collects the individual errblocks.json files generated for each c file and
    creates a cumulative json file for entire project.

    The name of the generated file is __project.errblocks.json.
    """
    for d in dirs:
        cumulative_file_ = os.path.join(d, "__project.errblocks.json")
        print(
            f"[+] creating cumulative errblocks.json for {d} as {cumulative_file_}")
        with open(cumulative_file_, "w") as cumulative_file:
            cumulative_data = process_errblocks_for_dir(d)
            deduplicated = []
            seen = set()  # set of (file, function_name)
            for entry in cumulative_data:
                file = entry["FunctionInfo"]["File"]
                function_name = entry["FunctionInfo"]["Name"]
                if _is_relative(file):
                    file = os.path.join(d, file.lstrip("../"))
                    entry["FunctionInfo"]["File"] = file
                if (file, function_name) in seen:
                    continue
                deduplicated.append(entry)
                seen.add((file, function_name))
            json.dump(deduplicated, cumulative_file)
        print("[+] creating cumulative errblocks.json done")


def generate_stats(dirs):
    """
    Collects the individual projects' errblocks.json files and copies them to
    the BENCHMARKS_PATH folder.
    Thereafter it generates a cumulative report for all the projects.
    """
    # if the BENCHMARKS_PATH is not empty, rename it as <existing_name>_<timestamp>
    # and create a new folder with <existing_name>
    if len(os.listdir(os.path.abspath(BENCHMARKS_PATH))):
        timestamp = datetime.datetime.now().strftime("%Y%m%d%H%M%S")
        new_name = os.path.abspath(BENCHMARKS_PATH) + "_" + timestamp
        shutil.move(os.path.abspath(BENCHMARKS_PATH), new_name)
        os.mkdir(os.path.join(BENCHMARKS_PATH))

    # copy the __project.errblocks.json from all dirs to the benchmarks_path
    project_files = []
    for d in dirs:
        cumulative_file_ = os.path.join(d, "__project.errblocks.json")
        print(
            f"[+] copying {cumulative_file_} to {os.path.abspath(BENCHMARKS_PATH)}")
        project_name = os.path.basename(os.path.dirname(d))
        project_filename = f"{project_name}__project.errblocks.json"
        bench_project_filename = os.path.join(
            os.path.abspath(BENCHMARKS_PATH), project_filename
        )
        shutil.copyfile(cumulative_file_, bench_project_filename)
        project_files.append(bench_project_filename)
        print(
            f"[+] copying {cumulative_file_} to {os.path.abspath(BENCHMARKS_PATH)} done"
        )

    # TODO: exactly what stats are required to be calculated?
    # individual file stats
    stats_filename = "runtool_stats.json"
    stats = []
    overall_functions = 0
    overall_err_conditions = 0
    for pf in project_files:
        cumulative_file_ = os.path.join(os.path.abspath(BENCHMARKS_PATH), pf)
        with open(cumulative_file_, "r") as cumulative_file:
            data = json.load(cumulative_file)
            s = {"project": pf, "functions": 0, "err_conditions": 0}
            s["functions"] += len(data)
            overall_functions += len(data)
            for record in data:
                s["err_conditions"] += len(record["ErrConditions"])
                overall_err_conditions += len(record["ErrConditions"])
            stats.append(s)
    stats.insert(
        0,
        {
            "overall_functions": overall_functions,
            "overall_err_conditions": overall_err_conditions,
        },
    )
    print(f"[+] cumulative stats: {stats}")
    with open(
        os.path.join(os.path.abspath(BENCHMARKS_PATH), stats_filename), "w"
    ) as stats_f:
        json.dump(stats, stats_f)


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
    run_tool_on_all(build_dirs)

    create_cumulative_errblocks_json_for_each(build_dirs)

    # print the statistics for the identified error guarding conditions
    # build_dirs = ['/workdisk/shank/dev/detecterr_input/zlib_v1.2.11/zlib-1.2.11',
    #               '/workdisk/shank/dev/detecterr_input/libpng_v1.6.35/libpng-1.6.35']
    print(f">>>> [+] build_dirs: {build_dirs}")
    generate_stats(build_dirs)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        __file__,
        description="Tool that converts the compilation commands into"
        " the commands for 3c tool and also "
        " runs the tool.",
    )

    parser.add_argument(
        "-p",
        "--prog_path",
        dest="prog_path",
        type=str,
        help="Program path to run. i.e., path to detecterr",
    )

    parser.add_argument(
        "-i",
        "--input_path",
        dest="input_path",
        type=str,
        help="Path to input folder containing archives",
    )

    parser.add_argument(
        "-m",
        "--benchmarks_path",
        dest="benchmarks_path",
        type=str,
        help="Path to benchmarks folder where the output is to be stored",
    )

    parser.add_argument(
        "-b",
        "--bear_path",
        dest="bear_path",
        type=str,
        default=shutil.which("bear"),
        help="Path to bear binary",
    )

    parser.add_argument(
        "--debug",
        action="store_true",
        help="enable debug mode (among other things, this will ensure that one process is run at a time)",
    )

    parser.add_argument(
        "-k",
        "--names_like",
        dest="names_like",
        nargs="*",
        type=str,
        help="substrings to match in names of the archives",
    )

    args = parser.parse_args()

    if not args.prog_path or not os.path.isfile(args.prog_path):
        print("Error: Path to the program to run is invalid.")
        print("Provided argument: {} is not a file.".format(args.prog_path))
        sys.exit(1)

    if not args.input_path or not os.path.isdir(args.input_path):
        print("Error: Path to the input archives folder is invalid.")
        print("Provided argument: {} is not a directory.".format(args.input_path))
        sys.exit(1)

    if not args.benchmarks_path or not os.path.isdir(args.benchmarks_path):
        print("Error: Path to the benchmarks folder is invalid.")
        print("Provided argument: {} is not a directory.".format(
            args.benchmarks_path))
        sys.exit(1)

    if not args.bear_path or not os.path.isfile(args.bear_path):
        print("Error: Path to the bear binary is invalid.")
        print("Provided argument: {} is not a file.".format(args.bear_path))
        sys.exit(1)

    BEAR_PATH = args.bear_path
    PROG_PATH = args.prog_path
    BENCHMARKS_PATH = args.benchmarks_path
    NAMES_LIKE = args.names_like or []
    DEBUG = args.debug

    run_main(args)
