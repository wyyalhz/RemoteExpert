#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
qmake
make -j
./client-factory "$@"
