//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		$Date: 2006/11/13 09:08:27 $
//
// Category     : VST 2.x SDK Samples
// Filename     : again.cpp
// Created by   : Steinberg Media Technologies
// Description  : Stereo plugin which applies Gain [-oo, 0dB]
//
// � 2006, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#include "pucktronix.delay.h"

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new PDelay (audioMaster);
}

//-------------------------------------------------------------------------------------------------------
PDelay::PDelay (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, 1)	// 1 program, 1 parameter only
{
	setNumInputs (1);		// stereo in
	setNumOutputs (1);		// stereo out
	setUniqueID ('ptdl');	// identify
	canProcessReplacing ();	// supports replacing output
	canDoubleReplacing ();	// supports double precision processing

	fGain = 1.f;			// default to 0 dB
	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name
	SR = getSampleRate();
	if (SR == 0) SR = 44100;
	delayBuffer = new float[2 * SR];
	index = 0;
}

//-------------------------------------------------------------------------------------------------------
PDelay::~PDelay ()
{
	// nothing to do here
}

//-------------------------------------------------------------------------------------------------------
void PDelay::setProgramName (char* name)
{
	vst_strncpy (programName, name, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void PDelay::getProgramName (char* name)
{
	vst_strncpy (name, programName, kVstMaxProgNameLen);
}

//-----------------------------------------------------------------------------------------
void PDelay::setParameter (VstInt32 index, float value)
{
	fGain = value;
}

//-----------------------------------------------------------------------------------------
float PDelay::getParameter (VstInt32 index)
{
	return fGain;
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterName (VstInt32 index, char* label)
{
	vst_strncpy (label, "Gain", kVstMaxParamStrLen);
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterDisplay (VstInt32 index, char* text)
{
	dB2string (fGain, text, kVstMaxParamStrLen);
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterLabel (VstInt32 index, char* label)
{
	vst_strncpy (label, "dB", kVstMaxParamStrLen);
}

//------------------------------------------------------------------------
bool PDelay::getEffectName (char* name)
{
	vst_strncpy (name, "Gain", kVstMaxEffectNameLen);
	return true;
}

//------------------------------------------------------------------------
bool PDelay::getProductString (char* text)
{
	vst_strncpy (text, "pucktronix.delay", kVstMaxProductStrLen);
	return true;
}

//------------------------------------------------------------------------
bool PDelay::getVendorString (char* text)
{
	vst_strncpy (text, "pucktronix", kVstMaxVendorStrLen);
	return true;
}

//-----------------------------------------------------------------------------------------
VstInt32 PDelay::getVendorVersion ()
{ 
	return 1000; 
}

//-----------------------------------------------------------------------------------------
void PDelay::processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames)
{
    float* in1  =  inputs[0];
 //   float* in2  =  inputs[1];
    float* out1 = outputs[0];
 //   float* out2 = outputs[1];
	
	int delayTime;
	float out;
	delayTime = (int)(0.5 * SR);
	
	for(int i = 0; i < sampleFrames; i++){
		out = delayBuffer[index];
		delayBuffer[index] = in1[i];
		(*out1++) = (out + in1[i]) * 0.5;
		if(index != delayTime - 1){
			index++;
		} else {
			index = 0;
		}
	}
/*    
	while (--sampleFrames >= 0)
    {
        (*out1++) = (*in1++) * fGain;
        (*out2++) = (*in2++) * fGain;
    }
 */
}

//-----------------------------------------------------------------------------------------
void PDelay::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
    double* in1  =  inputs[0];
   // double* in2  =  inputs[1];
    double* out1 = outputs[0];
   // double* out2 = outputs[1];
	int delayTime;
	double out;
	delayTime = (int)(0.5 * SR);
	
	for(int i = 0; i < sampleFrames; i++){
		out = delayBuffer[index];
		delayBuffer[index] = in1[i];
		(*out1++) = (out + in1[i]) * 0.5;
		index = index != delayTime - 1 ? index++ : 0;
	}
/*
    while (--sampleFrames >= 0)
    {
        (*out1++) = (*in1++) * dGain;
        (*out2++) = (*in2++) * dGain;
    }
 */
}
