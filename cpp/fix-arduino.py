#!/usr/bin/env python3
"""Repair dangling arduino-cli tool symlinks.

arduino-cli's tool installer deduplicates identical binaries (e.g. avr-g++/
avr-c++/avr-cpp are all the same file) by symlinking them together. On this
setup that finalize step reproducibly writes those symlinks pointing at the
tool's original extraction path under <data-dir>/tmp/package-<N>/... instead
of the real file that ends up installed elsewhere in the same tool directory
(sometimes right next to the symlink, sometimes one directory over, e.g.
avr/bin/ld pointing at a file that actually landed in ../../bin/). Once that
tmp dir is cleaned up after install, every name symlinked through it
(avr-g++, avr-gcc, ...) dangles, even though the real binary is sitting
untouched elsewhere, usually named <wanted-name>.<NNNN>.

Confirmed as a reproducible arduino-cli bug, not specific to this machine:
https://github.com/arduino/arduino-cli/issues/2534
https://github.com/arduino/arduino-cli/issues/2415

Note: this runs under a proot-based Linux distro (Termux proot-distro) where
lstat()-family syscalls fail with EPERM even though stat() and readlink()
work fine. So this script deliberately avoids Path.is_symlink()/os.lstat()
(and anything that calls them, like pathlib's is_file()) and instead uses
os.readlink() to detect symlinks and os.path.exists() (which follows links)
to detect dangling ones.

Usage:
    fix-arduino.py [--root ~/.arduino15/packages] [--dry-run]

Run this after any `arduino-cli core install`/`upgrade` that leaves tool
binaries failing with "no such file or directory" when executed.
"""
import argparse
import os
import sys
from pathlib import Path


def is_symlink(path: Path) -> bool:
    try:
        os.readlink(path)
        return True
    except OSError:
        return False


def scan(root: Path):
    """Return (real_files_by_name, broken_symlinks) under root."""
    real_files = {}  # name -> list of full paths
    broken = []  # (path, target) for dangling symlinks
    for dirpath, _dirnames, filenames in os.walk(root):
        d = Path(dirpath)
        for name in filenames:
            path = d / name
            if is_symlink(path):
                if not os.path.exists(path):
                    broken.append((path, os.readlink(path)))
            else:
                real_files.setdefault(name, []).append(path)
    return real_files, broken


def best_candidate(link: Path, wanted_name: str, real_files: dict):
    candidates = list(real_files.get(wanted_name, []))
    for name, paths in real_files.items():
        if name != wanted_name and name.startswith(wanted_name + "."):
            candidates.extend(paths)
    if not candidates:
        return None

    def shared_prefix_len(p: Path) -> int:
        a, b = p.parts, link.parent.parts
        n = 0
        while n < len(a) and n < len(b) and a[n] == b[n]:
            n += 1
        return n

    candidates.sort(key=lambda p: (shared_prefix_len(p), p.name))
    return candidates[-1]


def repair(root: Path, dry_run: bool) -> int:
    real_files, broken = scan(root)
    resolved = 0
    for link, target in broken:
        wanted_name = os.path.basename(target)
        candidate = best_candidate(link, wanted_name, real_files)
        if candidate is None:
            print(f"UNRESOLVED  {link} -> {target}", file=sys.stderr)
            continue
        resolved += 1
        new_target = os.path.relpath(candidate, start=link.parent)
        print(f"{'WOULD FIX' if dry_run else 'FIXING'}  {link} -> {new_target}")
        if not dry_run:
            os.unlink(link)
            os.symlink(new_target, link)

    verb = "resolvable" if dry_run else "fixed"
    print(f"\n{resolved} {verb}, {len(broken) - resolved} unresolved out of {len(broken)} dangling symlinks found.")
    return 0 if resolved == len(broken) else 1


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument(
        "--root",
        default=os.path.expanduser("~/.arduino15/packages"),
        help="arduino-cli packages dir to scan (default: ~/.arduino15/packages)",
    )
    parser.add_argument(
        "--dry-run", action="store_true", help="report fixes without applying them"
    )
    args = parser.parse_args()
    root = Path(args.root)
    if not root.is_dir():
        parser.error(f"{root} is not a directory")
    sys.exit(repair(root, args.dry_run))


if __name__ == "__main__":
    main()
