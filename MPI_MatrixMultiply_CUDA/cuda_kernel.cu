
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <algorithm>
#include <stdio.h>
#include <curand.h>
#include <curand_kernel.h>
#include <Windows.h>

void MatrixRandomFill(unsigned long long *matrix, unsigned int m, unsigned int n)
{
	unsigned long long *dev_matrix = 0;
	cudaSetDevice(0);
	cudaMalloc((void**)&dev_matrix, m * n * sizeof(long long));

	curandGenerator_t gen;
	curandCreateGenerator(&gen, CURAND_RNG_QUASI_SCRAMBLED_SOBOL64);
	curandSetGeneratorOffset(gen, GetTickCount64());
	curandSetQuasiRandomGeneratorDimensions(gen, m * n);
	curandGenerateLongLong(gen, dev_matrix, m * n);
	
	cudaDeviceSynchronize();
	cudaMemcpy(matrix, dev_matrix, m * n * sizeof(long long), cudaMemcpyDeviceToHost);

	cudaFree(dev_matrix);
}