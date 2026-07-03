#!/usr/bin/env bash
set -euo pipefail

DP0="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"${DP0}/Source/__Brahma/brahma.sh" -package MiseryDependencies -lib_search_dir "${DP0}/Source" -intermediate_output "${DP0}/Intermediate/__Brahma" -out "${DP0}/Source/MiseryDeps/Dependencies" -nodebuginfo -optimised "$@"
