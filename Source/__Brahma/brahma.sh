#!/usr/bin/env bash
set -euo pipefail

# ==============================================================================================================================
# Variables

BRAHMA_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)/"
OUTPUT=""
SEARCH_DIRS=()
PACKAGE_COUNT=0
LIBRARY_COUNT=0
CXX_MODE=0
SELF_DEBUG=0
ORIGINAL_ARGS=( "$@" )

# ==============================================================================================================================
# Parse Args

while [[ $# -gt 0 ]]; do
    if [[ "$1" == "-cxx" ]]; then
        CXX_MODE=1
        shift

    elif [[ "$1" == "-debug_build_tool" ]]; then
        SELF_DEBUG=1
        shift

    elif [[ "$1" == "-intermediate_output" ]]; then
        OUTPUT="$2/brahma"
        shift 2

    elif [[ "$1" == "-lib_search_dir" ]]; then
        SEARCH_DIRS+=( "$2" )
        shift 2

    else
        shift
    fi
done

# ==============================================================================================================================
# Process Parsed Args

if [[ -z "$OUTPUT" ]]; then
    echo "ERROR: No output file specified with -out. Use as: *.sh -out <file> (no extension needed)."
    exit 1
fi

if [[ ${#SEARCH_DIRS[@]} -eq 0 ]]; then
    echo "ERROR: No search directories specified with -lib_search_dir. Use as: *.sh -lib_search_dir <dir> (can be specified multiple times)."
    exit 1
fi

if [[ $CXX_MODE -eq 1 ]]; then
    OUTPUT_EXT="cpp"
else
    OUTPUT_EXT="c"
fi

# Ensure output directory exists
mkdir -p "$(dirname "$OUTPUT")"

# Clear output file(s)
echo "#define BRAHMA_EXEC"                  > "${OUTPUT}.${OUTPUT_EXT}"
echo "BRAHMA_BEGIN_LISTING_LIBRARIES()"     > "${OUTPUT}.libs.tmp"
echo "BRAHMA_BEGIN_LISTING_PACKAGES()"      > "${OUTPUT}.pkgs.tmp"

echo "#define BRAHMA_LIBRARY_IMPL"         >> "${OUTPUT}.${OUTPUT_EXT}"
echo "#include \"${BRAHMA_ROOT}Brahma.h\"" >> "${OUTPUT}.${OUTPUT_EXT}"

for DIR in "${SEARCH_DIRS[@]}"; do
    for LIB_DIR in "${DIR}"/*/; do
        [[ -d "$LIB_DIR" ]] || continue
        for FILE in "${LIB_DIR}"*._lib.h; do
            [[ -e "$FILE" ]] || continue
            echo "#include \"${FILE}\"" >> "${OUTPUT}.${OUTPUT_EXT}"

            LIBRARY_NAME="$(basename "$FILE" ._lib.h)"
            LIB_DIR_CLEAN="${LIB_DIR%/}"
            echo "BRAHMA_ADD_LIBRARY(\"${LIB_DIR_CLEAN}\", \"${FILE}\", ${LIBRARY_NAME})" >> "${OUTPUT}.libs.tmp"

            (( LIBRARY_COUNT++ )) || true
        done
    done

    for FILE in "${DIR}"/*._pkg.h; do
        [[ -e "$FILE" ]] || continue
        echo "#include \"${FILE}\"" >> "${OUTPUT}.${OUTPUT_EXT}"

        PACKAGE_NAME="$(basename "$FILE" ._pkg.h)"
        echo "BRAHMA_ADD_PACKAGE(\"${FILE}\", ${PACKAGE_NAME})" >> "${OUTPUT}.pkgs.tmp"

        (( PACKAGE_COUNT++ )) || true
    done
done

echo "BRAHMA_END_LISTING_LIBRARIES()" >> "${OUTPUT}.libs.tmp"
echo "BRAHMA_END_LISTING_PACKAGES()"  >> "${OUTPUT}.pkgs.tmp"
echo "BRAHMA_PACKAGE_COUNT(${PACKAGE_COUNT})" >> "${OUTPUT}.pkgs.tmp"
echo "BRAHMA_LIBRARY_COUNT(${LIBRARY_COUNT})" >> "${OUTPUT}.libs.tmp"

echo "#include \"${OUTPUT}.libs.tmp\"" >> "${OUTPUT}.${OUTPUT_EXT}"
echo "#include \"${OUTPUT}.pkgs.tmp\"" >> "${OUTPUT}.${OUTPUT_EXT}"

echo "Brahma build-file written to \`${OUTPUT}.${OUTPUT_EXT}\`."

# ==============================================================================================================================
# Build

if [[ $CXX_MODE -eq 1 ]]; then
    STD_FLAG="-std=c++20"
else
    STD_FLAG="-std=c17"
fi

if [[ $SELF_DEBUG -eq 1 ]]; then
    OPT_FLAGS="-D_DEBUG -g -O0"
else
    OPT_FLAGS="-DNDEBUG -O2"
fi

if [[ "$OSTYPE" == "darwin"* ]]; then
    if [[ $CXX_MODE -eq 0 ]]; then
        CC_CMD="clang"
    else
        CC_CMD="clang++"
    fi
else
    if [[ $CXX_MODE -eq 0 ]]; then
        CC_CMD="gcc"
    else
        CC_CMD="g++"
    fi
fi

if ! $CC_CMD -Wall -Werror $STD_FLAG $OPT_FLAGS -ffast-math "${OUTPUT}.${OUTPUT_EXT}" -o "${OUTPUT}"; then
    echo "Brahma build-file failed to compile."
    exit 1
fi

echo "Brahma build-file compiled to \`${OUTPUT}\`."

# ==============================================================================================================================
# Run

if [[ $SELF_DEBUG -eq 1 ]]; then
    echo "Brahma built in self-debug mode, please run the generated executable '${OUTPUT}' in a debugger."
    exit 0
fi

if ! "${OUTPUT}" "${ORIGINAL_ARGS[@]}"; then
    echo "Brahma build-process failed."
    exit 1
fi
