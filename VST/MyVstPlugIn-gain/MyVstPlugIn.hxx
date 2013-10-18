#ifndef INCLUDED_MYVSTPLUGIN_HXX
#define INCLUDED_MYVSTPLUGIN_HXX

#define _CRT_SECURE_NO_WARNINGS
#include "public.sdk/source/vst2.x/audioeffectx.h"
#undef _CRT_SECURE_NO_WARNINGS

#define PI 3.14159265359f
#define MAX_FREQ 5000.0f
#define MIN_FREQ 50.0f

#define MAX(a,b) ((a)>(b)?(a):(b))
#define MIN(a,b) ((a)<(b)?(a):(b))

// Class that implements the VST plug-in.
//
// Most functions implemented are virtual functions of the AudioEffect(X) baseclass.
// This is a very minimal example, there are many more functions that can be implemented.
class MyVstPlugIn : public AudioEffectX
{
public:
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Constructor/destructor:
	MyVstPlugIn(audioMasterCallback audioMaster);
	~MyVstPlugIn();

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Parameters functions:
	void initParameters(); // not from AudioEffect(X)

	void setParameter(VstInt32 index, float value);
	float getParameter(VstInt32 index);

	void getParameterLabel(VstInt32 index, char *label);
	void getParameterDisplay(VstInt32 index, char *text);
	void getParameterName(VstInt32 index, char *text);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Program functions:
	void setProgramName(char *name);
	void getProgramName(char *name);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Processing functions:
	void resume();
	void suspend();
	void processReplacing(float **inputs, float **outputs, VstInt32 numSamples);

private:
	// Parameters values:
	float gain;
    float last_gain;
    float frequency;
    float last_freq;
    float brightness;
    float phase; // it's a buffer, not a parameter!


	// Program data:
	char programName_[kVstMaxProgNameLen + 1];
};

// ---------------------------------------------------------------------------------------
// Some helper functions for parameters:

#include <cmath>

// Convert normalized float (0.0f..1.0f) to linear scale (min..max):
inline float norm2lin(float normalized, float min, float max)
{
	return min + (max - min)*normalized;
}

// Convert linear scale (min..max) to normalized float (0.0f..1.0f):
inline float lin2norm(float v, float min, float max)
{
	return (v - min)/(max - min);
}

// Convert normalized float (0.0f..1.0f) to exponential scale (min..max):
inline float norm2exp(float normalized, float min, float max)
{
	return min*std::exp(normalized*std::log(max/min));
}

// Convert exponential scale (min..max) to normalized float (0.0f..1.0f):
inline float exp2norm(float v, float min, float max)
{
	return std::log(v/min)/std::log(max/min);
}

// Convert normalized float (0.0f..1.0f) to gain factor.
// Gain is linear on a dB scale, but below some threshold tapers off linearly (on a linear scale) to 0.0f (-inf dB).
// with minDecibels = -36, maxDecibels = 12, norm = 0.75 -> 0 dB
float norm2gainFactor(float normalized, float minDecibels = -36.0f, float maxDecibels = +12.0f, float th = 0.2f)
{
	float gainFactor;
	if (normalized >= th)
	{
		float gainDecibels = norm2lin(normalized, minDecibels, maxDecibels); // linear on dB scale
		gainFactor = std::pow(10.0f, gainDecibels/20.0f);
	}
	else
	{
		float dbAtTh = norm2lin(th, minDecibels, maxDecibels);
		float linAtTh = pow(10.0f, dbAtTh/20.0f); // less than 0.05 (with default parameters), so already quite small
		gainFactor = linAtTh*normalized/th; // scale so gainFactor is continuous
	}
	return gainFactor;
}

// Convert normalized float (0.0f..1.0f) to index (0..n-1):
inline int norm2switch(float normalized, int numCases)
{
	return (int)std::floor(normalized*(numCases-1) + 0.5f); // round (for positive values only!)
}

// Convert index (0..n-1) to normalized float (0.0f..1.0f):
inline float switch2norm(int idx, int numCases)
{
	return (float)(idx)/(float)(numCases-1);
}


#endif
