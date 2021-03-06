#include "MyVstPlugin.hxx"

#include <cmath>

// ---------------------------------------------------------------------------------------
// Constructor / destructor:
// ---------------------------------------------------------------------------------------

// Define parameter indexes:
enum
{
	GAIN_PARAM_R,
	GAIN_PARAM_L,
	// (.. define other parameters here ..)
	NUM_PARAMETERS
};

// Number of programs (presets):
const int NUM_PROGRAMS = 1; // only current program (NUM_PROGRAMS = 0 causes problems with some hosts)


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MyVstPlugIn::MyVstPlugIn(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMETERS)
{
	// Set some basic properties of plug-in:
	setNumInputs(2);			// stereo in
	setNumOutputs(2);			// stereo out
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
    gain_L = 0.0f;
    gain_R = 0.0f;
	setParameter(GAIN_PARAM_R, 1.0f); // default value of 1.0f corresponds to 0 dB
	setParameter(GAIN_PARAM_L, 1.0f); // default value of 1.0f corresponds to 0 dB
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::setParameter(VstInt32 index, float value)
{
	if (index == GAIN_PARAM_R)
	{
        lastGainR = gain_R;
		gain_R = value;
	}
	if (index == GAIN_PARAM_L)
	{
        lastGainL = gain_L;
		gain_L = value;
	}
	// (.. add more parameters here ..)
}

float MyVstPlugIn::getParameter(VstInt32 index)
{
	if (index == GAIN_PARAM_R)
	{
		return gain_R;
	}
	else if (index == GAIN_PARAM_L)
    {
        return gain_L;
    }
	// (.. add more parameters here ..)
    else
    {
        return 0.0f; // invalid index
    }

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::getParameterName(VstInt32 index, char *label)
{
	if (index == GAIN_PARAM_R)
	{
		vst_strncpy(label, "Gain Right", kVstMaxParamStrLen);
	}
	else if (index == GAIN_PARAM_L)
    {
        vst_strncpy(label, "Gain Left", kVstMaxParamStrLen);
    }
    else
    {
        vst_strncpy(label, "", kVstMaxParamStrLen); // invalid index
    }
}

void MyVstPlugIn::getParameterDisplay(VstInt32 index, char *text)
{
	if (index == GAIN_PARAM_R)
	{
		dB2string(gain_R, text, kVstMaxParamStrLen); // dB2string() is a VST SDK helper function that converts a linear value to dB scale and then to a string
	}
	else if (index == GAIN_PARAM_L)
    {
        dB2string(gain_L, text, kVstMaxParamStrLen); // dB2string() is a VST SDK helper function that converts a linear value to dB scale and then to a string
    }
    else
    {
        vst_strncpy(text, "", kVstMaxParamStrLen); // invalid index
    }
}

void MyVstPlugIn::getParameterLabel(VstInt32 index, char *label)
{

	if ((index == GAIN_PARAM_R ) || (index == GAIN_PARAM_L ))
	{
			vst_strncpy(label, "dB", kVstMaxParamStrLen);
	}
	else
	{
		vst_strncpy(label, "", kVstMaxParamStrLen); // invalid index
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
    for (int j = 0; j < numSamples; ++j)
    {
        float gl = lastGainL + (gain_L - lastGainL) * (j/(float)numSamples);
        outputs[0][j] = inputs[0][j] * gl; // scale each sample in in1 by a factor gain_ and store in out1
        float gr = lastGainR + (gain_R - lastGainR) * (j/(float)numSamples);
        outputs[1][j] = inputs[1][j] * gr; // same with right channel
    }
    lastGainL = gain_L;
    lastGainR = gain_R;
}

// ---------------------------------------------------------------------------------------
// Instance creator:
// ---------------------------------------------------------------------------------------

// Method that creates an instance of the VST plug-in:
AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
	return new MyVstPlugIn(audioMaster);
}

