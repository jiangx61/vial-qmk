#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$REPO_ROOT"

# Stage external sources under .build so QMK's object paths cannot escape it.
TEST_STAGE=.build/windows_gui_l_tests
mkdir -p "$TEST_STAGE"
cp "$REPO_ROOT/tests/windows_gui_l/"{config.h,test.mk,test_windows_gui_l.cpp} "$TEST_STAGE/"
cp "$REPO_ROOT/keyboards/crkbd/rev4_1/standard/keymaps/vial/shortcut_blocking.c" "$TEST_STAGE/"

make -r -R -f builddefs/build_test.mk TEST=windows_gui_l TEST_OUTPUT=windows_gui_l \
    TEST_PATH="$TEST_STAGE" FULL_TESTS=windows_gui_l SILENT=false -j4
.build/test/windows_gui_l.elf
