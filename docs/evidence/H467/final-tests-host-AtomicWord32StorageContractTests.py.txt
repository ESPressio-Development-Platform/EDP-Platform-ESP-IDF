#!/usr/bin/env python3
"""Exercise target and storage refusal without claiming host Xtensa instruction execution."""

import argparse
from pathlib import Path
import resource
import shutil
import signal
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--header", type=Path)
    root = Path(__file__).resolve().parents[2]
    workspace = root.parent
    parser.add_argument("--platform", type=Path, default=workspace / "EDP-Platform")
    parser.add_argument("--system", type=Path, default=workspace / "EDP-System")
    args = parser.parse_args()
    header = args.header or root / "src/concurrency/AtomicWord32Provider.hpp"
    compilers = [shutil.which(name) for name in ("g++", "clang++")]
    if not all(compilers):
        raise SystemExit("Both GCC and Clang are required")
    resource.setrlimit(resource.RLIMIT_CORE, (0, 0))

    with tempfile.TemporaryDirectory(prefix="edp-atomic-contract-") as directory:
        temp = Path(directory)
        (temp / "xtensa/config").mkdir(parents=True)
        (temp / "esp_cpu.h").write_text(
            "#pragma once\n#include <cstdint>\n"
            "bool esp_cpu_compare_and_set(volatile std::uint32_t*, std::uint32_t, std::uint32_t);\n"
        )
        (temp / "esp_memory_utils.h").write_text(
            "#pragma once\n"
            "inline bool esp_ptr_in_dram(const void*) {\n"
            "  static unsigned calls = 0;\n"
            "  ++calls;\n"
            "  return TEST_STORAGE_MODE == 1 || (TEST_STORAGE_MODE == 2 && calls == 1);\n"
            "}\n"
        )
        (temp / "xtensa/config/core-isa.h").write_text(
            "#pragma once\n#define XCHAL_HAVE_S32C1I TEST_NATIVE_CAS\n"
        )
        source = temp / "contract.cpp"
        source.write_text(
            '#include "' + str(header) + '"\n'
            "#include <cstdio>\n#include <type_traits>\n"
            "using P = ESPressio::Platform::ESPIDF::Concurrency::AtomicWord32Provider;\n"
            "using W = P::Word;\n"
            "static_assert(sizeof(W) == 4 && alignof(W) == 4);\n"
            "static_assert(std::is_nothrow_default_constructible_v<W>);\n"
            "static_assert(!std::is_copy_constructible_v<W> && !std::is_move_constructible_v<W>);\n"
            "int main() {\n"
            '  std::puts("before-construction"); std::fflush(stdout);\n'
            "  W word;\n"
            "  if (word.LoadRelaxed() != 0) return 4;\n"
            '  std::puts("after-construction");\n'
            "  return 0;\n}\n"
        )

        for compiler in compilers:
            binary = temp / "contract"
            common = [
                compiler, "-std=c++20", "-O2", "-DNDEBUG", "-Wall", "-Wextra", "-Werror",
                "-pedantic", f"-I{temp}", f"-I{args.platform / 'src'}",
                f"-I{args.system / 'src'}", str(source), "-o", str(binary),
            ]
            for target in ("ESP32", "ESP32S3"):
                for mode in (1, 0, 2):
                    result = subprocess.run(
                        common + [
                            "-D__XTENSA__=1", f"-DCONFIG_IDF_TARGET_{target}=1",
                            "-DTEST_NATIVE_CAS=1", f"-DTEST_STORAGE_MODE={mode}",
                        ],
                        capture_output=True, text=True, check=False,
                    )
                    if result.returncode:
                        raise AssertionError(result.stderr)
                    run = subprocess.run([str(binary)], capture_output=True, text=True, check=False)
                    if mode == 1:
                        assert run.returncode == 0, run
                        assert "after-construction" in run.stdout, run
                    else:
                        assert run.returncode == -signal.SIGABRT, run
                        assert run.stdout.strip() == "before-construction", run
                    print(f"{Path(compiler).name}: {target} storage mode {mode} PASS", flush=True)

            for target, xtensa, native, diagnostic in (
                ("ESP32S2", True, 0, "requires ESP32 or ESP32-S3 native hardware CAS"),
                ("ESP32C3", False, 0, "requires ESP32 or ESP32-S3 native hardware CAS"),
                ("ESP32S3", True, 0, "requires the native S32C1I instruction"),
            ):
                definitions = [
                    f"-DCONFIG_IDF_TARGET_{target}=1", f"-DTEST_NATIVE_CAS={native}",
                    "-DTEST_STORAGE_MODE=1",
                ]
                if xtensa:
                    definitions.append("-D__XTENSA__=1")
                result = subprocess.run(common + definitions, capture_output=True, text=True, check=False)
                assert result.returncode != 0, (target, result)
                assert diagnostic in result.stderr, result.stderr
                print(f"{Path(compiler).name}: unsupported {target}/native={native} refused PASS", flush=True)
    print("AtomicWord32 target/storage refusal suite: PASS")


if __name__ == "__main__":
    main()
