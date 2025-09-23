#!/usr/bin/env bash
set -euo pipefail

repo_root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
cd "$repo_root_dir"

placeholder_version="0.0.0-dev"
dist_name="RemoteExpert-${placeholder_version}-windows-x86_64"
workdir="out/windows/package/${dist_name}"

mkdir -p "$workdir"/bin dist

# Copy built binaries
if [ -d out/windows/bin ]; then
  cp -a out/windows/bin/* "$workdir/bin/" || true
fi

# TODO: copy Qt DLLs and plugins (qwindows.dll, Qt5*.dll) via ntldd/objdump sweep

(cd "out/windows/package" && zip -r "${dist_name}.zip" "${dist_name}")
cp -f "out/windows/package/${dist_name}.zip" "dist/${dist_name}.zip"
echo "[mxe] Package created: dist/${dist_name}.zip"


