#pragma once

#include <cuda.h>
#include <cuda_runtime.h>

#include <cstdio>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <stdexcept>


namespace Additive {

	void initSynth(int numSinusoids, int numSamples, float* host_frequencies);
    void compute_sinusoid_gpu_simple(float * buffer, int angle);
    void endSynth();
}
