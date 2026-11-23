#!/usr/bin/env python3
"""Build complete authored demos at an exact GitHub source ref and retain SDK evidence."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import time


def main():
    root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser()
    parser.add_argument("--pio", default="pio")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--target", choices=["esp32dev", "esp32s3"], action="append")
    parser.add_argument("--ref")
    parser.add_argument("--transport-cache", type=Path)
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    ref = args.ref or subprocess.check_output(
        ["git", "-C", str(root), "rev-parse", "HEAD"], text=True
    ).strip()
    env = os.environ.copy()
    if args.transport_cache:
        env.update({
            "GIT_CONFIG_COUNT": "1",
            "GIT_CONFIG_KEY_0": "url.file://" + str(args.transport_cache.resolve()) + "/.insteadOf",
            "GIT_CONFIG_VALUE_0": "https://github.com/ESPressio-Development-Platform/",
            "GIT_ALLOW_PROTOCOL": "file:https:http",
        })
    rows = []
    for form in ["PlatformIO_ESP-IDF", "PlatformIO_Arduino"]:
        source = root / "demos/atomic-word-strong" / form
        project = output / form
        project.mkdir(exist_ok=True)
        shutil.copytree(source / "src", project / "src", dirs_exist_ok=True)
        for name in ["CMakeLists.txt", "platformio.ini"]:
            if (source / name).exists():
                shutil.copyfile(source / name, project / name)
        manifest = project / "platformio.ini"
        manifest.write_text(manifest.read_text().replace(
            "EDP-Platform-ESP-IDF.git#main",
            "EDP-Platform-ESP-IDF.git#" + ref,
        ))
        for target in args.target or ["esp32dev", "esp32s3"]:
            log = output / (form + "-" + target + ".log")
            start = time.monotonic()
            with log.open("w") as stream:
                result = subprocess.run(
                    [args.pio, "run", "-d", str(project), "-e", target, "-j", "6"],
                    env=env, stdout=stream, stderr=subprocess.STDOUT,
                    timeout=1800, check=False,
                )
            row = {
                "form": form, "target": target, "exit_code": result.returncode,
                "seconds": round(time.monotonic() - start, 3),
                "project": str(project), "log": str(log), "requested_ref": ref,
                "example_sha256": hashlib.sha256((project / "src/Example.hpp").read_bytes()).hexdigest(),
            }
            if result.returncode == 0:
                installed = project / ".pio/libdeps" / target / "EDP-Platform-ESP-IDF"
                row["installed_sha"] = subprocess.check_output(
                    ["git", "-C", str(installed), "rev-parse", "HEAD"], text=True
                ).strip()
                for header in (root / "src").rglob("*.hpp"):
                    assert header.read_bytes() == (
                        installed / "src" / header.relative_to(root / "src")
                    ).read_bytes(), header
                row["production_headers_equal"] = True
            rows.append(row)
            (output / "results.json").write_text(json.dumps(rows, indent=2) + "\n")
            print(json.dumps(row), flush=True)
    return 0 if all(row["exit_code"] == 0 for row in rows) else 1


if __name__ == "__main__":
    raise SystemExit(main())
