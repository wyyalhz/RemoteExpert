#!/usr/bin/env bash
set -euo pipefail

repo_root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
cd "$repo_root_dir"

placeholder_version="0.0.0-dev"
dist_name="RemoteExpert-${placeholder_version}-linux-x86_64"
workdir="out/linux/package/${dist_name}"

mkdir -p "$workdir/bin" dist

# Copy built binaries
if [ -d out/linux/bin ]; then
  cp -a out/linux/bin/* "$workdir/bin/" || true
fi

# TODO: dependency collection (linuxdeployqt or ldd) will be added later

tar -C "out/linux/package" -czf "dist/${dist_name}.tar.gz" "${dist_name}"
echo "[linux] Package created: dist/${dist_name}.tar.gz"


