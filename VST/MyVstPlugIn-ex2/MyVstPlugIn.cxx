#include "MyVstPlugin.hxx"

#include <cmath>

// ---------------------------------------------------------------------------------------
// Constructor / destructor:
// ---------------------------------------------------------------------------------------

// Define parameter indexes:
enum
{
	BALANCE,
    GAIN,
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
    gain = 0.0f;
    balance = 0.5f;
	setParameter(BALANCE, 0.5f); 
    setParameter(GAIN, 1.0f);
	// (.. add more parameters here ..)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::setParameter(VstInt32 index, float value)
{
	if (index == BALANCE)
	{
        last_balance = balance;
		balance = value;
	} 
    else if (index == GAIN)
    {
        last_gain = gain;
        gain = value;
    }
    
}

float MyVstPlugIn::getParameter(VstInt32 index)
{
	if (index == BALANCE)
	{
		return balance;
	}
    else if (index == GAIN)
    {
        return gain;
    }
    else
    {
        return 0.0f; // invalid index
    }

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::getParameterName(VstInt32 index, char *label)
{
	if (index == BALANCE)
	{
		vst_strncpy(label, "Balance", kVstMaxParamStrLen);
	}
    else if (index == GAIN)
    {
        vst_strncpy(label, "Gain", kVstMaxParamStrLen);
    }
    else
    {
        vst_strncpy(label, "", kVstMaxParamStrLen); // invalid index
    }
}

void MyVstPlugIn::getParameterDisplay(VstInt32 index, char *text)
{
	if (index == BALANCE)
	{
		float2string(balance, text, kVstMaxParamStrLen); // dB2string() is a VST SDK helper function that converts a linear value to dB scale and then to a string
	}
    else if (index == GAIN)
    {
        float2string(gain, text, kVstMaxParamStrLen);
    }
    else
    {
        vst_strncpy(text, "", kVstMaxParamStrLen); // invalid index
    }
}

void MyVstPlugIn::getParameterLabel(VstInt32 index, char *label)
{

	if (index == BALANCE )
	{
		vst_strncpy(label, "", kVstMaxParamStrLen);
	}
    else if (index == GAIN)
    {
        vst_strncpy(label, "", kVstMaxParamStrLen);
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
    // -3 dB 
	// float gain_L = cos(balance*PI/2.0f);
	// float gain_R = sin(balance*PI/2.0f);
    
    // -4.5 dB
    for (int j = 0; j < numSamples; ++j)
    {
        float g = last_gain + (gain - last_gain) * (j/(float)numSamples);
        float b = last_balance + (balance - last_balance) * (j/(float)numSamples);
        outputs[0][j] = inputs[0][j] * g * sqrt(1-b); // scale each sample in in1 by a factor gain_ and store in out1
        outputs[1][j] = inputs[1][j] * g * sqrt(b); // same with right channel
    }
    last_gain = gain;
    last_balance = balance;
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