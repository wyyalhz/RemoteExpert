#!/usr/bin/env bash
set -euo pipefail

# Parse all .pro files to collect Qt modules from lines like: QT += core gui widgets \
# Supports line continuation with \

repo_root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"/.. && pwd)"

read_qt_modules() {
  local files
  if command -v git >/dev/null 2>&1 && git -C "$repo_root_dir" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    files=$(git -C "$repo_root_dir" ls-files '*.pro' || true)
  else
    files=$(cd "$repo_root_dir" && find . -type f -name '*.pro' | sed 's#^\./##')
  fi
  if [ -z "${files:-}" ]; then
    return 0
  fi
  awk '
    function trim(s){ sub(/^[ \t\r\n]+/, "", s); sub(/[ \t\r\n]+$/, "", s); return s }
    BEGIN{cont=0}
    {
      sub(/#.*/, "", $0)             # strip comments
      line=$0
      if (cont==1) acc=acc" "line; else acc=line
      if (match(line, /\\[ \t]*$/)) { cont=1; sub(/\\[ \t]*$/, "", acc); next } else cont=0
      if (match(acc, /(^|[ \t])QT[ \t]*\+=[ \t]+/)) {
        sub(/.*QT[ \t]*\+=[ \t]+/, "", acc)
        print acc
      }
      acc=""
    }
  ' $(printf '%s\n' $files) \
  | tr '\t' ' ' | tr -s ' ' \
  | xargs -n1 \
  | sed '/^$/d' \
  | sort -u
}

map_modules_linux_apt() {
  # Base provides: core, gui, widgets, network, printsupport, concurrent, sql (drivers may need extras)
  local mods=("$@")
  local -A seen=()
  local pkgs=(qt512base)
  seen[qt512base]=1
  for m in "${mods[@]}"; do
    case "$m" in
      svg)
        if [ -z "${seen[qt512svg]:-}" ]; then pkgs+=(qt512svg); seen[qt512svg]=1; fi ;;
      multimedia|multimediawidgets)
        if [ -z "${seen[qt512multimedia]:-}" ]; then pkgs+=(qt512multimedia); seen[qt512multimedia]=1; fi ;;
      serialport)
        if [ -z "${seen[qt512serialport]:-}" ]; then pkgs+=(qt512serialport); seen[qt512serialport]=1; fi ;;
      qml|quick|quickcontrols2)
        if [ -z "${seen[qt512declarative]:-}" ]; then pkgs+=(qt512declarative); seen[qt512declarative]=1; fi ;;
      webengine)
        if [ -z "${seen[qt512webengine]:-}" ]; then pkgs+=(qt512webengine); seen[qt512webengine]=1; fi ;;
    esac
  done
  printf '%s\n' "${pkgs[@]}" | sort -u
}

map_modules_mxe_pkgs() {
  local mods=("$@")
  local -A seen=()
  local pkgs=(qtbase)
  seen[qtbase]=1
  for m in "${mods[@]}"; do
    case "$m" in
      svg)
        if [ -z "${seen[qtsvg]:-}" ]; then pkgs+=(qtsvg); seen[qtsvg]=1; fi ;;
      multimedia|multimediawidgets)
        if [ -z "${seen[qtmultimedia]:-}" ]; then pkgs+=(qtmultimedia); seen[qtmultimedia]=1; fi ;;
      serialport)
        if [ -z "${seen[qtserialport]:-}" ]; then pkgs+=(qtserialport); seen[qtserialport]=1; fi ;;
      qml|quick|quickcontrols2)
        if [ -z "${seen[qtdeclarative]:-}" ]; then pkgs+=(qtdeclarative); seen[qtdeclarative]=1; fi ;;
      webengine)
        if [ -z "${seen[qtwebengine]:-}" ]; then pkgs+=(qtwebengine); seen[qtwebengine]=1; fi ;;
    esac
  done
  printf '%s\n' "${pkgs[@]}" | sort -u
}

main() {
  local modules
  modules=$(read_qt_modules || true)
  # Fallback to common set if none detected
  if [ -z "${modules:-}" ]; then
    modules="core gui widgets network svg multimedia"
  fi

  # Export for sourcing usage
  export QT_MODULES="$modules"
  export LINUX_QT_APT_PACKAGES="$(map_modules_linux_apt $modules | xargs)"
  export MXE_QT_PACKAGES="$(map_modules_mxe_pkgs $modules | xargs)"

  # If executed directly, print a summary
  if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    echo "QT_MODULES=$QT_MODULES"
    echo "LINUX_QT_APT_PACKAGES=$LINUX_QT_APT_PACKAGES"
    echo "MXE_QT_PACKAGES=$MXE_QT_PACKAGES"
  fi
}

main "$@"


