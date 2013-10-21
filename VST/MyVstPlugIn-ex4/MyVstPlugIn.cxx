#include "MyVstPlugin.hxx"

#include <cmath>

// ---------------------------------------------------------------------------------------
// Constructor / destructor:
// ---------------------------------------------------------------------------------------

// Define parameter indexes:
enum
{
	GAIN,
    FREQUENCY,
    BRIGHTNESS,
	// (.. define other parameters here ..)
	NUM_PARAMETERS
};

// Number of programs (presets):
const int NUM_PROGRAMS = 1; // only current program (NUM_PROGRAMS = 0 causes problems with some hosts)


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MyVstPlugIn::MyVstPlugIn(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMETERS)
{
	// Set some basic properties of plug-in:
	setNumInputs(1);			// mono in
	setNumOutputs(1);			// mono out
	setUniqueID('MVPI');		// unique 4 char identifier for this plug-in (here "My Vst Plug-In")
	isSynth(false);				// this plug-in is an audio effect, not a synthesizer
	canProcessReplacing(true);	// supports 'replacing output processing mode' (legacy stuff, always set to true)
	canDoubleReplacing(false);	// does not support 64 bit floating point I/O
	programsAreChunks(false);	// programs work through setParameter()/getParameter() rather than binary 'chunks'

	vst_strncpy(programName_, "default", kVstMaxProgNameLen);

	initParameters();

	// (.. do not do much else here, so scanning for plug-ins is fast ..)
	// (.. do memory allocation/deallocation in resume()/suspend() instead, but normally pointers should be initialized to NULL here ..)
}

MyVstPlugIn::~MyVstPlugIn()
{
	// (.. nothing to do here ..)
}

// ---------------------------------------------------------------------------------------
// Parameter functions:
// ---------------------------------------------------------------------------------------

void MyVstPlugIn::initParameters()
{
    gain = 0.0f;
	setParameter(GAIN, 0.7f); 
	setParameter(FREQUENCY, 0.1f); 
    setParameter(BRIGHTNESS, 0.0f);
    phase = 0.0f;
	// (.. add more parameters here ..)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::setParameter(VstInt32 index, float value)
{
	switch (index) {
        case GAIN:
            last_gain = gain;
            gain = value;
            break;
        case FREQUENCY:
            frequency = norm2exp(value,MIN_FREQ,MAX_FREQ);
            break;
        case BRIGHTNESS:
            brightness = value;
            break;
    }
}

float MyVstPlugIn::getParameter(VstInt32 index)
{
    switch (index) {
        case GAIN:
            return gain;
            break;
        case FREQUENCY:
            return exp2norm(frequency, MIN_FREQ, MAX_FREQ);
            break;
        case BRIGHTNESS:
            return brightness;
            break;
        default:
            return 0.0f; // invalid index
            break;
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::getParameterName(VstInt32 index, char *label)
{
    switch (index) {
        case GAIN:
            vst_strncpy(label, "Gain", kVstMaxParamStrLen);
            break;
        case FREQUENCY:
            vst_strncpy(label, "Freq", kVstMaxParamStrLen);
            break;
        case BRIGHTNESS:
            vst_strncpy(label, "Timbre", kVstMaxParamStrLen);
            break;
        default:
            vst_strncpy(label, "", kVstMaxParamStrLen); // invalid index
            break;
    }
}

void MyVstPlugIn::getParameterDisplay(VstInt32 index, char *text)
{
    switch (index) {
        case GAIN:
            float2string(gain, text, kVstMaxParamStrLen);
            break;
        case FREQUENCY:
            float2string(frequency, text, kVstMaxParamStrLen);
            break;
        case BRIGHTNESS:
            float2string(brightness, text, kVstMaxParamStrLen);
            break;
        default:
            vst_strncpy(text, "", kVstMaxParamStrLen); // invalid index
            break;
    }
}

void MyVstPlugIn::getParameterLabel(VstInt32 index, char *label)
{
    switch (index) {
        case GAIN:
            vst_strncpy(label, "", kVstMaxParamStrLen);
            break;
        case FREQUENCY:
            vst_strncpy(label, "", kVstMaxParamStrLen);
            break;
        case BRIGHTNESS:
            vst_strncpy(label, "", kVstMaxParamStrLen);
            break;
        default:
            vst_strncpy(label, "", kVstMaxParamStrLen); // invalid index
            break;
    }
}

// ---------------------------------------------------------------------------------------
// Program functions:
// ---------------------------------------------------------------------------------------

void MyVstPlugIn::setProgramName(char *name)
{
	vst_strncpy(programName_, name, kVstMaxProgNameLen);
}

void MyVstPlugIn::getProgramName(char *name)
{
	vst_strncpy(name, programName_, kVstMaxProgNameLen);
}

// ---------------------------------------------------------------------------------------
// Processing functions:
// ---------------------------------------------------------------------------------------

void MyVstPlugIn::resume()
{
//	const double sampleRate = getSampleRate();
//	const int maxBlockSize = getBlockSize();

	// (.. initialization stuff goes here ..)
	// (.. e.g. allocating memory ..)
}

void MyVstPlugIn::suspend()
{
	// (.. de-initialization stuff goes here ..)
	// (.. e.g. deallocating memory ..)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::processReplacing(float **inputs, float **outputs, VstInt32 numSamples)
{
    float *out = outputs[0];
    for (int j = 0; j < numSamples; ++j)
    {
        float g = last_gain + (gain - last_gain) * (j/(float)numSamples);
        phase += (2.0f * PI * frequency / (float)getSampleRate());
        if (phase > 2.0f*PI) phase -= 2.0f*PI;
        out[j] = g * MIN(MAX(sin(phase) * pow((1-brightness),-2),-1),1);
    }
    last_gain = gain;
}

// ---------------------------------------------------------------------------------------
// Instance creator:
// ---------------------------------------------------------------------------------------

// Method that creates an instance of the VST plug-in:
AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	return new MyVstPlugIn(audioMaster);
}

// getsamplerate i mirar freq / periode ... mostres cicle i calcular el pas per cada mostra