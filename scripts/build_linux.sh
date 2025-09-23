#!/usr/bin/env bash
set -euo pipefail

repo_root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"
cd "$repo_root_dir"

source ./scripts/detect_qt_modules.sh >/dev/null 2>&1 || true

echo "[linux] Qt modules: ${QT_MODULES:-}" || true
echo "[linux] Suggested apt packages: ${LINUX_QT_APT_PACKAGES:-}" || true

# If running inside the Linux build container, we may install missing packages on demand
if command -v apt-get >/dev/null 2>&1 && [ -n "${LINUX_QT_APT_PACKAGES:-}" ]; then
  echo "[linux] Ensuring Qt packages installed (may be no-op)"
  (set -x; apt-get update && apt-get install -y --no-install-recommends ${LINUX_QT_APT_PACKAGES} || true)
fi

export PATH=/opt/qt512/bin:$PATH

mkdir -p out/linux/bin dist

echo "[linux] qmake version:" && (qmake -v || true)

(set -x; qmake remote_expert.pro CONFIG+=release)
(set -x; make -j"$(nproc || echo 4)" V=1)

# Collect expected executables if present
# Note: project .pro files set DESTDIR=$$PWD/../bin, so outputs land in top-level bin/
# We check both per-project and top-level bin locations
for exe in \
  client/src/main \
  client/bin/client \
  server/src/main \
  server/bin/server \
  videoplusplusplus/videoplusplusplus \
  videoplusplusplus/bin/videoplusplusplus \
  bin/client \
  bin/server \
  bin/videoplusplusplus; do
  if [ -f "$exe" ]; then
    cp -f "$exe" out/linux/bin/
  fi
done

echo "[linux] Built binaries in out/linux/bin:" && ls -al out/linux/bin || true


