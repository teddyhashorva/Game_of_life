#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CUDA_SRC=${CUDA_SRC:-/usr/local/cuda-12.6/include}
CUDA_DST=${CUDA_DST:-"$PROJECT_ROOT/cuda_patched/include"}

echo "Patching CUDA headers"
echo "    Source: $CUDA_SRC"
echo "    Dest  : $CUDA_DST"

mkdir -p "$CUDA_DST"
rsync -a "$CUDA_SRC"/ "$CUDA_DST"/

HDR="$CUDA_DST/crt/math_functions.h"
if [[ ! -f "$HDR" ]]; then
  echo "ERROR: expected header not found at $HDR" >&2
  exit 1
fi

# patch sinpi/cospi to be noexcept
perl -pi -e 's/sinpi\(double x\);/sinpi(double x) noexcept;/g;
             s/cospi\(double x\);/cospi(double x) noexcept;/g;
             s/sinpif\(float x\);/sinpif(float x) noexcept;/g;
             s/cospif\(float x\);/cospif(float x) noexcept;/g' "$HDR"

echo "Done. Patched headers are in $CUDA_DST."
