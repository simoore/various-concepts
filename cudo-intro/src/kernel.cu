#include <algorithm>
#include <cstdlib>
#include <cstdio>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

__global__ void sumArrayGpu(const int *a, const int *b, int *c, int size) {
    int gid = blockIdx.x * blockDim.x + threadIdx.x;
    if (gid < size) {
        c[gid] = a[gid] + b[gid];
    }
}

void sumArrayCpu(const int *a, const int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        c[i] = a[i] + b[i];
    }
} 

int main() {
    const int arraySize = 10000;
    const int blockSize = 128;
    const int numBytes = arraySize * sizeof(int);
    cudaError error;

    int *hA = reinterpret_cast<int *>(std::malloc(numBytes));
    int *hB = reinterpret_cast<int *>(std::malloc(numBytes));
    int *gpuResults = reinterpret_cast<int *>(std::malloc(numBytes));
    int *hC = reinterpret_cast<int *>(std::malloc(numBytes));

    auto initFunc = [&](int *arr) {
        for (int i = 0; i < arraySize; i++) {
            arr[i] = static_cast<int>(rand() & 0xFF);
        }
    };

    initFunc(hA);
    initFunc(hB);
    sumArrayCpu(hA, hB, hC, arraySize);

    int * dA, *dB, *dC;

    error = cudaMalloc(&dA, numBytes);
    if (error != cudaSuccess) {
        fprintf(stderr, " Error : %s %s %d\n", cudaGetErrorString(error), __FILE__, __LINE__);
        return 1;
    }
    cudaMalloc(&dB, numBytes);
    cudaMalloc(&dC, numBytes);

    cudaMemcpy(dA, hA, numBytes, cudaMemcpyHostToDevice);
    cudaMemcpy(dB, hB, numBytes, cudaMemcpyHostToDevice);

    dim3 block(blockSize);
    dim3 grid(arraySize / block.x + 1);

    sumArrayGpu<<<grid, block>>>(dA, dB, dC, arraySize);
    
    // Wait til operation has finished.
    cudaDeviceSynchronize();

    cudaMemcpy(gpuResults, dC, numBytes, cudaMemcpyDeviceToHost);
    if (std::equal(hC, hC + arraySize, gpuResults)) {
        printf("Arrays are the same\n");
    } else {
        printf("Arrays are different\n");
    }

    cudaFree(dA);
    cudaFree(dB);
    cudaFree(dC);
    free(hA);
    free(hB);
    free(hC);
    free(gpuResults);

    cudaDeviceReset();
    return 0;
}