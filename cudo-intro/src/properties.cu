#include <iostream>

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

int main() {
    int deviceCount = 0;
    cudaGetDeviceCount(&deviceCount);
    if (deviceCount == 0) {
        std::cout << "No CUDA supported device found" << std::endl;
    }

    int devNo = 0;
    cudaDeviceProp iProp;
    cudaGetDeviceProperties(&iProp, devNo);

    std::cout << "Device " << devNo << ": " << iProp.name << std::endl;
    std::cout << "  number of multiprocessors    : " << iProp.multiProcessorCount << std::endl;
    std::cout << "  clock rate                   : " << iProp.clockRate << std::endl;
    std::cout << "  compute capability           : " << iProp.major << "." << iProp.minor << std::endl;
    std::cout << "  total global memory          : " << (iProp.totalGlobalMem / 1024.0) << std::endl;
    std::cout << "  total constant memory        : " << (iProp.totalConstMem / 1024.0) << std::endl;
    std::cout << "  total shared memory per block: " << (iProp.sharedMemPerBlock / 1024.0) << std::endl;
    std::cout << "  total shared memory per MP   : " << (iProp.sharedMemPerMultiprocessor / 1024.0) << std::endl;

    cudaDeviceReset();
    return 0;
}