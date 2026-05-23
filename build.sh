#!/usr/bin/env bash
set -euo pipefail

DP0="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${DP0}/Source/__Brahma/brahma.sh" -lib_search_dir "${DP0}/Source" -intermediate_output "${DP0}/Intermediate/__Brahma" -out "${DP0}/Binaries" -clangd "${DP0}" -warnings -warn_as_err "$@"
