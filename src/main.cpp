#include "RtAudio.h"
#include "kernel.h"
#include <iostream>
#include <cstdlib>
#include "Sine.h"
//Sine *global_sine[100];
#define NUM_SINES 1000
#define _PI 3.1415926535897931
#define NUM_SAMPLES 512
// Two-channel sawtooth wave generator.
int saw( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
  unsigned int i, j;
  double *buffer = (double *) outputBuffer;
  double *lastValues = (double *) userData;
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
  // Write interleaved audio data.
  for ( i=0; i<nBufferFrames; i++ ) {
    for ( j=0; j<2; j++ ) {
      *buffer++ = lastValues[j];
      lastValues[j] += 0.005 * (j+1+(j*0.1));
      if ( lastValues[j] >= 1.0 ) lastValues[j] -= 2.0;
    }
  }
  return 0;
}

int additive(void *outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *UserData) {
	float *buffer = (float *)outputBuffer;
	Sine** sine = (Sine **) UserData;
    if ( status ) std::cout << "Stream underflow detected!" << std::endl;
	for (unsigned int i = 0; i < nBufferFrames; i++) {
		float val = 0; 
		for (unsigned int j = 0; j < NUM_SINES; j++) {
			val += sine[j]->tick();
		}
		*buffer++ = val;
	}
	return 0;
}

int additive_sine(void *outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData) {
  unsigned int i, j;
  float *buffer = (float *) outputBuffer;
  float *freqs = (float *) userData;
  if ( status )
    std::cout << "Stream underflow detected!" << std::endl;
  // Write interleaved audio data.
  for (i = 0; i < nBufferFrames; i++) {
	  double val = 0;
	  float angle = 2.0f * _PI * i / 44100;
	  for (j = 0; j < NUM_SINES; j++) {
		  val += sin(angle * freqs[j]) / NUM_SINES;
	  }
	  *buffer++ = val;
  }
  return 0;
}

int additive_gpu(void *outputBuffer, void* inputBuffer, unsigned int nBufferFrames,
	double streamTime, RtAudioStreamStatus status, void *userData) {
	static float angle = 0;

	
	Additive::compute_sinusoid_gpu_simple((float*)outputBuffer, angle);
	//angle+= 2.0f * _PI * NUM_SAMPLES / 44100;

	return 0;
}

int main()
{
  RtAudio dac;
  if ( dac.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    exit( 0 );
  }
  RtAudio::StreamParameters parameters;
  parameters.deviceId = dac.getDefaultOutputDevice();
  parameters.nChannels = 2;
  parameters.firstChannel = 0;
  unsigned int sampleRate = 44100;
  unsigned int bufferFrames = NUM_SAMPLES; // 256 sample frames
  double data[2];
  Sine* sine[NUM_SINES];
  for (int i = 1; i <= NUM_SINES; i++) {
	  Sine* newsine = new Sine();
	  newsine->setSamplingRate(sampleRate);
	  newsine->setFrequency(440 + i* 10);
	  sine[i] = newsine;
  }
  float freqs[NUM_SINES];
  for (int i = 1; i <= NUM_SINES; i++) {
	  freqs[i-1] =   i* 10 ;
  }

  Additive::initSynth(NUM_SINES, bufferFrames, freqs);
  try {
    dac.openStream( &parameters, NULL, RTAUDIO_FLOAT32,
                    sampleRate, &bufferFrames, &additive_gpu, (void*)&freqs);
    dac.startStream();
  }
  catch ( RtAudioError& e ) {
    e.printMessage();
    exit( 0 );
  }
  
  char input;
  std::cout << "\nPlaying ... press <enter> to quit.\n";
  std::cin.get( input );
  try {
    // Stop the stream
    dac.stopStream();
  }
  catch (RtAudioError& e) {
    e.printMessage();
  }
  if ( dac.isStreamOpen() ) dac.closeStream();
  Additive::endSynth();
  return 0;
}
