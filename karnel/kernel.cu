#include "kernel.h"
#include <math.h>
#include <device_functions.h>
#include <cmath>
#define M_PI 3.1415926535897931
#define THREADS_PER_SAMPLE 8
#define SAMPLES_PER_THREAD 10
#define ATOMIC_SYNTH 0
float* dev_frequencies, *dev_buffer;
int numSamples, numSinusoids;

void Additive::initSynth(int numSinusoid, int numSample, float* host_frequencies) {
	numSamples = numSample;
	numSinusoids = numSinusoid;
	cudaMalloc((void**)&dev_frequencies, numSinusoids * sizeof(float));
	cudaMalloc((void**)&dev_buffer, numSamples * sizeof(float));
	cudaMemcpy(dev_frequencies, host_frequencies, numSinusoids * sizeof(float), cudaMemcpyHostToDevice);
	cudaDeviceSynchronize();
}

void Additive::endSynth() {
	cudaFree(dev_buffer);
	cudaFree(dev_frequencies);
}
__global__ void sin_kernel_simple(float * buffer, float* frequencies, float angle, int numSamples, int numSinusoids) {
	int idx = blockIdx.x * blockDim.x + threadIdx.x;

	if (idx < numSamples) {
		angle = angle + 2.0f * M_PI * idx / 44100;
		float val = 0.0f;
		for (int i = 0; i < numSinusoids; i++){
			val += 0.01 * __sinf((angle * frequencies[i]));
			}
		buffer[idx] = val;

	}
}

void Additive::compute_sinusoid_gpu_simple(float* buffer, int angle) {
	int threadsPerBlock = 256; 
	int blocksPerGrid = (numSamples + threadsPerBlock - 1) / threadsPerBlock;

	sin_kernel_simple << <blocksPerGrid, threadsPerBlock >> > (dev_buffer, dev_frequencies, angle, numSamples, numSinusoids);
	
	cudaMemcpy(buffer, dev_buffer, numSamples * sizeof(float), cudaMemcpyDeviceToHost);
}

