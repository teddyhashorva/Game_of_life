#include "../include/World.cuh"

__global__ void evolveKernel(
    const bool* current,
    bool* previous2,
    int width,
    int height
)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= height || y >= width) return;

    int idx = x * width + y;

    // Counting neighbours
    int sum = 0;
    for(int dx=-1; dx <= 1; dx++)
    {
        for(int dy=-1; dy <= 1; dy++)
        {
            if(dx == 0 && dy == 0) continue;
            int nx = (x + dx + height) % height;
            int ny = (y + dy + width) % width;
            sum += current[nx*width + ny];
        }
    }

    // Applying rules
    bool alive = current[idx];
    if(alive)
        previous2[idx] = (sum == 2 || sum == 3);
    else
        previous2[idx] = (sum == 3);
}

void evolveWorldCUDA(bool* current, bool* previous, bool* previous2, int width, int height, cudaStream_t stream)
{
    dim3 threads(16, 16);
    dim3 blocks(
        (width + threads.x -1) / threads.x,
        (height + threads.y -1) / threads.y
    );

    evolveKernel<<<blocks, threads, 0, stream>>>(current, previous2, width, height);

    

    checkCuda(cudaGetLastError());
    checkCuda(cudaStreamSynchronize(stream));
}

inline cudaError_t checkCuda(cudaError_t result)
{
    if(result != cudaSuccess)
    {
        fprintf(stderr, "Error: %s\n", cudaGetErrorString(result));
        assert(result == cudaSuccess);
    }
    return result;
}