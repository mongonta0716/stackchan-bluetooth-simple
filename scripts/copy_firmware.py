Import("env")

from pathlib import Path
from datetime import datetime
from SCons.Script import COMMAND_LINE_TARGETS
import shutil
import subprocess


def get_repository_name(project_dir: Path) -> str:
    """Gitリポジトリのルートディレクトリ名を取得する。"""
    try:
        git_root = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"],
            cwd=project_dir,
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()

        return Path(git_root).name

    except (subprocess.CalledProcessError, FileNotFoundError):
        # Gitリポジトリでない場合はプロジェクトフォルダ名を使う
        return project_dir.name


def get_git_hash(project_dir: Path) -> str:
    try:
        return subprocess.check_output(
            ["git", "rev-parse", "--short", "HEAD"],
            cwd=project_dir,
            text=True,
            stderr=subprocess.DEVNULL,
        ).strip()

    except (subprocess.CalledProcessError, FileNotFoundError):
        return "unknown"


def format_address(address) -> str:
    """書き込み開始アドレスを0x始まりの16進表記に揃える。"""
    return f"0x{int(address, 0) if isinstance(address, str) else int(address):x}"


def parse_size(size) -> int:
    """PlatformIOのサイズ表記をバイト数へ変換する。"""
    if isinstance(size, int):
        return size

    normalized_size = str(size).strip().upper()
    if normalized_size.endswith("MB"):
        return int(normalized_size[:-2]) * 1024 * 1024
    if normalized_size.endswith("KB"):
        return int(normalized_size[:-2]) * 1024
    return int(normalized_size, 0)


def copy_binary(
    source_path: Path,
    env,
    binary_type: str,
    start_address: str,
    partition_size: int,
    timestamp: str,
):
    project_dir = Path(env.subst("$PROJECT_DIR"))

    output_dir = project_dir / ".pio_build_firmware"
    output_dir.mkdir(parents=True, exist_ok=True)

    repository_name = get_repository_name(project_dir)
    environment_name = env.subst("$PIOENV")
    git_hash = get_git_hash(project_dir)

    destination_name = (
        f"{repository_name}"
        f"-{environment_name}"
        f"-{git_hash}"
        f"-{timestamp}"
        f"-{start_address}"
        f"-{partition_size}bytes.bin"
    )

    destination_path = output_dir / destination_name

    shutil.copy2(source_path, destination_path)

    print()
    print("========================================")
    print(f" {binary_type} copied")
    print(f" Repository : {repository_name}")
    print(f" Environment: {environment_name}")
    print(f" Address    : {start_address}")
    print(f" Size       : {partition_size} bytes")
    print(f" Git hash   : {git_hash}")
    print(f" From       : {source_path}")
    print(f" To         : {destination_path}")
    print("========================================")


def create_merged_image(
    env,
    firmware_path: Path,
    timestamp: str,
):
    """全イメージを0x0000から書き込める単一binへ結合する。"""
    build_dir = Path(env.subst("$BUILD_DIR"))
    filesystem_path = Path(
        env.subst("$BUILD_DIR/${ESP32_FS_IMAGE_NAME}.bin")
    )
    merged_path = build_dir / "merged-flash.bin"

    images = [
        (
            int(env.subst("$ESP32_APP_OFFSET"), 0),
            firmware_path,
        ),
        (
            int(env["FS_START"]),
            filesystem_path,
        ),
    ]
    images.extend(
        (
            int(str(address), 0),
            Path(env.subst(str(image_path))),
        )
        for address, image_path in env.get("FLASH_EXTRA_IMAGES", [])
    )
    images.sort(key=lambda image: image[0])

    flash_size_setting = env.BoardConfig().get("upload.flash_size", "4MB")
    command = [
        env.subst("$PYTHONEXE"),
        env.subst("$UPLOADER"),
        "--chip",
        env.BoardConfig().get("build.mcu", "esp32"),
        "merge_bin",
        "--output",
        str(merged_path),
        "--fill-flash-size",
        str(flash_size_setting),
    ]
    for address, image_path in images:
        command.extend([format_address(address), str(image_path)])

    subprocess.check_call(command, cwd=env.subst("$PROJECT_DIR"))

    copy_binary(
        merged_path,
        env,
        "Merged flash image",
        "0x0",
        parse_size(flash_size_setting),
        timestamp,
    )


def copy_firmware(source, target, env):
    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    firmware_source_path = Path(str(target[0]))
    create_merged_image(env, firmware_source_path, timestamp)


firmware_path = env.subst("$BUILD_DIR/${PROGNAME}.bin")

filesystem_targets = {"buildfs", "uploadfs", "uploadfsota"}
if not filesystem_targets.intersection(COMMAND_LINE_TARGETS):
    filesystem_image = env.DataToBin(
        env.subst("$BUILD_DIR/${ESP32_FS_IMAGE_NAME}"),
        env.subst("$PROJECT_DATA_DIR"),
    )
    env.NoCache(filesystem_image)
    env.AlwaysBuild(filesystem_image)
    env.Depends(firmware_path, filesystem_image)
    env.AlwaysBuild(firmware_path)

env.AddPostAction(firmware_path, copy_firmware)
