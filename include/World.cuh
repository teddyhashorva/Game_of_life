#pragma once
#include <cstddef>
#include <cuda_runtime.h>
#include <stdio.h>
#include <cstdio>
#include <cassert>

void evolveWorldCUDA(bool* current, bool* previous,
    bool* previous2, int width, int height, cudaStream_t stream);

inline cudaError_t checkCuda(cudaError_t result);
