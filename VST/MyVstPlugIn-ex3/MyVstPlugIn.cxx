#include "MyVstPlugin.hxx"

#include <cmath>

// ---------------------------------------------------------------------------------------
// Constructor / destructor:
// ---------------------------------------------------------------------------------------

// Define parameter indexes:
enum
{
	GAIN_PARAM,
	FC_PARAM,
	Q_PARAM,
	NUM_PARAMETERS
};

// Number of programs (presets):
const int NUM_PROGRAMS = 1; // only current program (NUM_PROGRAMS = 0 causes problems with some hosts)


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MyVstPlugIn::MyVstPlugIn(audioMasterCallback audioMaster) : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMETERS)
{
	// Set some basic properties of plug-in:
	setNumInputs(1);			// stereo in
	setNumOutputs(1);			// stereo out
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
	setParameter(GAIN_PARAM, 0.7f); 
	setParameter(FC_PARAM, 0.6f); 
	setParameter(Q_PARAM, 0.2f); 
    for (int i = 0; i < 3; i++) {
        xN_[i] = 0.0f;
        yN_[i] = 0.0f;
    }
    updateCoeffs();

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::setParameter(VstInt32 index, float value)
{
	switch (index) {
        case GAIN_PARAM:
            last_gain = gain;
            gain = value;
            break;
        case FC_PARAM:
            freq = norm2exp(value, MIN_FREQ, MAX_FREQ);
            updateCoeffs();
            break;
        case Q_PARAM:
            q = norm2lin(value, MIN_Q, MAX_Q);
            updateCoeffs();
            break;
    }
	
}

float MyVstPlugIn::getParameter(VstInt32 index)
{
    switch (index) {
        case GAIN_PARAM:
            return gain;
            break;
        case FC_PARAM:
            return exp2norm(freq, MIN_FREQ, MAX_FREQ);
            break;
        case Q_PARAM:
            return lin2norm(q, MIN_Q, MAX_Q);
            break;
        default:
            return 0.0f; // default
            break;
    }

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MyVstPlugIn::getParameterName(VstInt32 index, char *label)
{    
    switch (index) {
        case GAIN_PARAM:
            vst_strncpy(label, "Gain", kVstMaxParamStrLen);
            break;
        case FC_PARAM:
            vst_strncpy(label, "Cutoff F", kVstMaxParamStrLen);
            break;
        case Q_PARAM:
            vst_strncpy(label, "Q", kVstMaxParamStrLen);
            break;
        default:
            vst_strncpy(label, "", kVstMaxParamStrLen); // default
            break;
    }
    
}

void MyVstPlugIn::getParameterDisplay(VstInt32 index, char *text)
{
    switch (index) {
        case GAIN_PARAM:
            dB2string(gain, text, kVstMaxParamStrLen);
            break;
        case FC_PARAM:
            float2string(freq, text, kVstMaxParamStrLen);
            break;
        case Q_PARAM:
            float2string(q, text, kVstMaxParamStrLen);
            break;
        default:
            vst_strncpy(text, "", kVstMaxParamStrLen); // invalid index
            break;
    }    

}

void MyVstPlugIn::getParameterLabel(VstInt32 index, char *label)
{
    switch (index) {
        case GAIN_PARAM:
            vst_strncpy(label, "", kVstMaxParamStrLen);
            break;
        case FC_PARAM:
            vst_strncpy(label, "", kVstMaxParamStrLen);
            break;
        case Q_PARAM:
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
// Private functions:
// ---------------------------------------------------------------------------------------

void MyVstPlugIn::updateCoeffs()
{
    float w0 = 2 * PI * freq / (float)getSampleRate();
    float alpha = sin(w0) / (2.0f * q);
    
    b_[0] = (1.0f - cos(w0)) / 2.0f;
    b_[1] = 1.0f - cos(w0);
    b_[2] = b_[0];
    a_[0] = 1.0f + alpha;
    a_[1] = -2.0f * cos(w0);
    a_[2] = 1.0f - alpha;
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
    float *in = inputs[0];

    for (int j = 0; j < numSamples; ++j) {
        
        xN_[2] = xN_[1];
        xN_[1] = xN_[0];
        xN_[0] = in[j];
        yN_[2] = yN_[1];
        yN_[1] = yN_[0];
        
        yN_[0] = (b_[0]/a_[0]) * xN_[0] + (b_[1]/a_[0]) * xN_[1] + (b_[2]/a_[0]) * xN_[2] -
                 (a_[1]/a_[0]) * yN_[1] - (a_[2]/a_[0]) * yN_[2];
        
        out[j] = (gain + (gain-last_gain) * (j/(float)numSamples)) * yN_[0];
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