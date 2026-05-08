#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys
from datetime import UTC, datetime
from pathlib import Path


PROJECT_DIR = Path(__file__).resolve().parent
DEFAULT_ENV = "esp32-c6-devkitc-1"
DEFAULT_MONITOR_BAUD = 115200
LOCAL_PIO = PROJECT_DIR / ".tools" / "pio-venv" / "bin" / "pio"
BUILD_DIR = PROJECT_DIR / "build"
VERSION_FILE = PROJECT_DIR / "VERSION"
BUILD_NUMBER_FILE = PROJECT_DIR / "BUILD_NUMBER"
APP_NAME = "Badge3000"


class ToolError(RuntimeError):
    pass


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Build, flash and monitor the Badge3000 firmware.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument("command", choices=("build", "flash", "monitor", "list", "clean", "version"), help="Operation to run.")
    parser.add_argument("--port", help="Serial port, for example /dev/ttyUSB0. Auto-detected for flash/monitor if omitted.")
    parser.add_argument("--baud", type=int, default=DEFAULT_MONITOR_BAUD, help="Serial monitor baud rate.")
    parser.add_argument("--env", default=DEFAULT_ENV, help="PlatformIO environment name.")
    parser.add_argument("--pio", type=Path, help="Explicit path to the pio executable.")
    parser.add_argument(
        "--install-platformio",
        action="store_true",
        help="Install PlatformIO into .tools/pio-venv if no pio executable is found.",
    )
    return parser.parse_args()


def command_exists(command: list[str]) -> bool:
    try:
        subprocess.run(command, cwd=PROJECT_DIR, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
    except (OSError, subprocess.CalledProcessError):
        return False
    return True


def install_platformio() -> Path:
    venv = PROJECT_DIR / ".tools" / "pio-venv"
    python = venv / "bin" / "python"
    pio = venv / "bin" / "pio"

    if not python.exists():
        print(f"Creating PlatformIO virtualenv: {venv}")
        venv.parent.mkdir(parents=True, exist_ok=True)
        subprocess.run([sys.executable, "-m", "venv", str(venv)], cwd=PROJECT_DIR, check=True)

    print("Installing/updating PlatformIO in local virtualenv")
    subprocess.run([str(python), "-m", "pip", "install", "-U", "platformio"], cwd=PROJECT_DIR, check=True)

    if not pio.exists():
        raise ToolError("PlatformIO install finished but pio executable was not found")
    return pio


def resolve_pio(explicit: Path | None, install_if_missing: bool) -> list[str]:
    candidates: list[Path] = []
    if explicit:
        candidates.append(explicit.expanduser())
    env_pio = os.environ.get("PIO")
    if env_pio:
        candidates.append(Path(env_pio).expanduser())

    path_pio = shutil.which("pio")
    if path_pio:
        candidates.append(Path(path_pio))

    candidates.append(LOCAL_PIO)

    for candidate in candidates:
        if candidate.is_file():
            return [str(candidate)]

    if command_exists([sys.executable, "-m", "platformio", "--version"]):
        return [sys.executable, "-m", "platformio"]

    if install_if_missing:
        return [str(install_platformio())]

    raise ToolError(
        "PlatformIO not found. Install pio, pass --pio /path/to/pio, or rerun with --install-platformio."
    )


def run(command: list[str]) -> None:
    print("$ " + " ".join(command))
    subprocess.run(command, cwd=PROJECT_DIR, check=True)


def read_text_file(path: Path, fallback: str) -> str:
    if not path.exists():
        return fallback
    value = path.read_text(encoding="utf-8").strip()
    return value or fallback


def next_build_number() -> int:
    raw = read_text_file(BUILD_NUMBER_FILE, "0")
    try:
        current = int(raw)
    except ValueError:
        current = 0
    value = current + 1
    BUILD_NUMBER_FILE.write_text(f"{value}\n", encoding="utf-8")
    return value


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def version_label(base_version: str, build_number: int) -> str:
    return f"v{base_version}-build.{build_number}"


def publish_artifacts(env: str) -> None:
    platformio_build_dir = PROJECT_DIR / ".pio" / "build" / env
    firmware_bin = platformio_build_dir / "firmware.bin"
    factory_bin = platformio_build_dir / "firmware.factory.bin"

    if not firmware_bin.is_file():
        raise ToolError(f"Build artifact not found: {firmware_bin}")

    base_version = read_text_file(VERSION_FILE, "0.1.0")
    build_number = next_build_number()
    label = version_label(base_version, build_number)
    timestamp = datetime.now(UTC).strftime("%Y-%m-%dT%H:%M:%SZ")

    BUILD_DIR.mkdir(parents=True, exist_ok=True)

    artifacts: dict[str, dict[str, object]] = {}
    for source, suffix in ((firmware_bin, "firmware"), (factory_bin, "factory")):
        if not source.is_file():
            continue
        target = BUILD_DIR / f"{APP_NAME}-{label}-{suffix}.bin"
        latest = BUILD_DIR / f"{APP_NAME}-latest-{suffix}.bin"
        shutil.copy2(source, target)
        shutil.copy2(source, latest)
        artifacts[suffix] = {
            "path": str(target.relative_to(PROJECT_DIR)),
            "latest": str(latest.relative_to(PROJECT_DIR)),
            "bytes": target.stat().st_size,
            "sha256": sha256_file(target),
        }

    manifest = {
        "app": APP_NAME,
        "version": base_version,
        "build": build_number,
        "label": label,
        "environment": env,
        "created_at": timestamp,
        "artifacts": artifacts,
    }
    manifest_path = BUILD_DIR / f"{APP_NAME}-{label}-manifest.json"
    latest_manifest_path = BUILD_DIR / f"{APP_NAME}-latest-manifest.json"
    manifest_text = json.dumps(manifest, indent=2, sort_keys=True) + "\n"
    manifest_path.write_text(manifest_text, encoding="utf-8")
    latest_manifest_path.write_text(manifest_text, encoding="utf-8")

    print(f"Published firmware build: {label}")
    for artifact in artifacts.values():
        print(f"  - {artifact['path']}")
    print(f"  - {manifest_path.relative_to(PROJECT_DIR)}")


def pio_capture(pio: list[str], args: list[str]) -> str:
    result = subprocess.run(
        pio + args,
        cwd=PROJECT_DIR,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    return result.stdout


def ports_from_pyserial() -> list[str]:
    try:
        from serial.tools import list_ports
    except ModuleNotFoundError:
        return []

    devices: list[str] = []
    for port in list_ports.comports():
        if port.device.startswith(("/dev/ttyUSB", "/dev/ttyACM", "/dev/cu.usb", "/dev/tty.usb")) or port.vid is not None:
            devices.append(port.device)
    return sorted(set(devices))


def ports_from_pio(pio: list[str]) -> list[str]:
    output = pio_capture(pio, ["device", "list"])
    devices: list[str] = []
    for line in output.splitlines():
        stripped = line.strip()
        if stripped.startswith(("/dev/ttyUSB", "/dev/ttyACM", "/dev/cu.usb", "/dev/tty.usb")):
            devices.append(stripped)
    return sorted(set(devices))


def detect_port(pio: list[str]) -> str:
    ports = ports_from_pyserial() or ports_from_pio(pio)
    if len(ports) == 1:
        return ports[0]
    if not ports:
        raise ToolError("No USB serial port found. Connect the badge or pass --port /dev/ttyUSB0.")
    raise ToolError("Multiple serial ports found: " + ", ".join(ports) + ". Pass --port explicitly.")


def main() -> int:
    args = parse_args()

    if args.command == "version":
        base_version = read_text_file(VERSION_FILE, "0.1.0")
        build_number = read_text_file(BUILD_NUMBER_FILE, "0")
        print(f"{APP_NAME} v{base_version} build.{build_number}")
        return 0

    pio = resolve_pio(args.pio, args.install_platformio)

    if args.command == "build":
        run(pio + ["run", "-e", args.env])
        publish_artifacts(args.env)
    elif args.command == "flash":
        port = args.port or detect_port(pio)
        run(pio + ["run", "-e", args.env, "-t", "upload", "--upload-port", port])
        publish_artifacts(args.env)
    elif args.command == "monitor":
        port = args.port or detect_port(pio)
        run(pio + ["device", "monitor", "--port", port, "--baud", str(args.baud)])
    elif args.command == "list":
        run(pio + ["device", "list"])
    elif args.command == "clean":
        run(pio + ["run", "-e", args.env, "-t", "clean"])
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except subprocess.CalledProcessError as exc:
        print(f"Command failed with exit code {exc.returncode}", file=sys.stderr)
        raise SystemExit(exc.returncode)
    except ToolError as exc:
        print(f"Error: {exc}", file=sys.stderr)
        raise SystemExit(1)
