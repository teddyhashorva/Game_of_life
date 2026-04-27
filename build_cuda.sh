#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$PROJECT_ROOT/build}"

# set up environment for CUDA build
export PATH=/usr/local/13.3.0/bin:/usr/local/cuda-12.6/bin:$PATH
export CUDA_PATCHED_INCLUDE="${CUDA_PATCHED_INCLUDE:-$PROJECT_ROOT/cuda_patched/include}"
export CPATH="$CUDA_PATCHED_INCLUDE${CPATH:+:$CPATH}"
export CUDA_TARGET_DIR_REL="${CUDA_TARGET_DIR_REL:-../../../../${PROJECT_ROOT#/}/cuda_patched}"
export CUDA_TOOLKIT_TARGET_LIB_DIR="${CUDA_TOOLKIT_TARGET_LIB_DIR:-/usr/local/cuda-12.6/targets/x86_64-linux/lib}"

if [[ ! -d "$CUDA_PATCHED_INCLUDE" ]]; then
  echo "ERROR: Patched headers not found. Run ./patch_cuda_headers.sh first." >&2
  exit 1
fi

# configure and build the project with CUDA support
cmake -S "$PROJECT_ROOT" -B "$BUILD_DIR" \
  -DCMAKE_CUDA_COMPILER=/usr/local/cuda-12.6/bin/nvcc \
  -DCMAKE_CUDA_HOST_COMPILER=/usr/local/13.3.0/bin/gcc \
  -DCMAKE_C_COMPILER=/usr/local/13.3.0/bin/gcc \
  -DCMAKE_CXX_COMPILER=/usr/local/13.3.0/bin/g++ \
  -DCMAKE_CUDA_FLAGS="--allow-unsupported-compiler" \
  -DCMAKE_CUDA_ARCHITECTURES=61 \
  -DCAME_BUILD_TYPE=Release

cmake --build "$BUILD_DIR"
