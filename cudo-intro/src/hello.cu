// compile with: nvcc src/hello.cu -o hello

#include <cstdio>

__global__ void cudaHello() {
    printf("threadIdx.x : %d, threadIdx.y : %d, threadIdx.z : %d, Hello World from GPU!\n", 
        threadIdx.x, threadIdx.y, threadIdx.z);
}

int main() {

    // A block has 2 threads in each dimension
    dim3 block(2, 2, 2);

    // We are launching 4 blocks, 2 in x dimension, 2 in y dimension, 1 in z dimension
    dim3 grid(2, 2, 1);

    // We are launching 20 threads to run the cudaHello function
    cudaHello<<<grid, block>>>(); 
    cudaDeviceSynchronize();
    cudaDeviceReset();
    return 0;
}