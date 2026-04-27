#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${SCRIPT_DIR}/Source/__Brahma/brahma.sh" -lib_search_dir "${SCRIPT_DIR}/Source" -build_tool_path "${SCRIPT_DIR}/Intermediate/__Brahma/brahma" "$@"
