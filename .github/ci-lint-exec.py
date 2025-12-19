#!/usr/bin/env python3
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path


def sh(cmd: list[str], env: dict[str, str] | None = None, check: bool = True) -> subprocess.CompletedProcess:
    print("+ " + " ".join(cmd), flush=True)
    return subprocess.run(cmd, env=env, check=check)


def have_buildx() -> bool:
    try:
        subprocess.run(
            ["docker", "buildx", "version"],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=True,
        )
        return True
    except Exception:
        return False


def build_image(repo_root: Path, image: str) -> None:
    dockerfile = repo_root / "ci" / "lint_imagefile"

    if have_buildx():
        cmd = [
            "docker", "buildx", "build",
            "-t", image,
            "-f", str(dockerfile),
            "--load",
            str(repo_root),
        ]
    else:
        cmd = [
            "docker", "build",
            "-t", image,
            "-f", str(dockerfile),
            str(repo_root),
        ]

    try:
        sh(cmd)
    except subprocess.CalledProcessError:
        print("Retry building image tag after failure", file=sys.stderr)
        sh(cmd)


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    image = os.environ.get("CONTAINER_NAME", "hylium-linter")

    # build the linter image
    build_image(repo_root, image)

    # docker run
    docker_cmd: list[str] = ["docker", "run", "--rm", "--init"]

    # Mount repo (macOS: delegated helps)
    mount = f"{repo_root}:/hylium"
    if sys.platform == "darwin":
        mount += ":delegated"
    docker_cmd += ["-v", mount, "-w", "/hylium"]

    # Always set a sane HOME for tools (git/cargo) when running locally.
    docker_cmd += [
        "-e", "CI=1",
        "-e", f"GITHUB_EVENT_NAME={os.environ.get('GITHUB_EVENT_NAME', 'local')}",
        "-e", "HOME=/tmp",
        "-e", "XDG_CONFIG_HOME=/tmp/.config",
        "-e", "CARGO_HOME=/tmp/.cargo",
        "-e", "RUSTUP_HOME=/tmp/.rustup",
        "-e", "GIT_CONFIG_GLOBAL=/tmp/.gitconfig",
    ]

    pr_num = os.environ.get("PR_NUMBER", "")
    if pr_num:
        docker_cmd += ["-e", f"PR_NUMBER={pr_num}"]

    # Avoid root-owned files when running locally.
    if os.environ.get("GITHUB_ACTIONS", "").lower() != "true":
        try:
            uid = os.getuid()
            gid = os.getgid()
            docker_cmd += ["-u", f"{uid}:{gid}"]
        except Exception:
            pass

    # Run lint script inside container
    docker_cmd += [image, "bash", "-lc", "./ci/lint/06_script.sh"]

    try:
        sh(docker_cmd)
        return 0
    except subprocess.CalledProcessError as e:
        return int(e.returncode)


if __name__ == "__main__":
    raise SystemExit(main())
