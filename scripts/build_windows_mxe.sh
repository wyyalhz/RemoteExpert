#!/usr/bin/env bash
set -euo pipefail

repo_root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
cd "$repo_root_dir"

source ./scripts/detect_qt_modules.sh >/dev/null 2>&1 || true

export MXE_TARGET="${MXE_TARGET:-x86_64-w64-mingw32.shared}"
export MXE_PREFIX="${MXE_PREFIX:-/opt/mxe/usr}"
export PATH="$MXE_PREFIX/bin:$PATH"

qmake_bin="${MXE_TARGET}-qmake-qt5"

echo "[mxe] Qt modules: ${QT_MODULES:-}" || true
echo "[mxe] MXE packages hint: ${MXE_QT_PACKAGES:-}" || true
echo "[mxe] qmake: $qmake_bin"

mkdir -p out/windows/bin dist

(set -x; "$qmake_bin" remote_expert.pro CONFIG+=release)
(set -x; make -j"$(nproc || echo 4)")

# Collect expected executables if present
for exe in \
  client/release/client.exe \
  server/release/server.exe \
  videoplusplusplus/release/videoplusplusplus.exe \
  client/bin/client.exe \
  server/bin/server.exe \
  videoplusplusplus/bin/videoplusplusplus.exe; do
  if [ -f "$exe" ]; then
    cp -f "$exe" out/windows/bin/
  fi
done

echo "[mxe] Built binaries in out/windows/bin:" && ls -al out/windows/bin || true


