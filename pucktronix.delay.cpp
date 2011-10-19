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

/*
	todo:
	variable delay time - interpolation (using two variables)
	feedback filter
	feedback saturation
*/

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

	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name
	SR = getSampleRate();
	if (SR == 0) SR = 44100;
	delayBuffer = new float[2 * SR];
	index = 0;
	delayTimeSeconds = 0.5;
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
	delayTimeSeconds = value * 2;
}

//-----------------------------------------------------------------------------------------
float PDelay::getParameter (VstInt32 index)
{
	return delayTimeSeconds * 0.5;
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterName (VstInt32 index, char* label)
{
	vst_strncpy (label, "Delay Time", kVstMaxParamStrLen);
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterDisplay (VstInt32 index, char* text)
{
	float2string(delayTimeSeconds * 1000, text, kVstMaxParamStrLen);
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterLabel (VstInt32 index, char* label)
{
	vst_strncpy (label, "ms", kVstMaxParamStrLen);
}

//------------------------------------------------------------------------
bool PDelay::getEffectName (char* name)
{
	vst_strncpy (name, "pucktronix.delay", kVstMaxEffectNameLen);
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
	
	int maxDelayTime, readPointerInt;
	float out, readPointerFloat, variableDelayTime, frac, next;
	
	variableDelayTime = delayTimeSeconds * SR; 
	maxDelayTime = (int)(2 * SR);

	if(variableDelayTime > maxDelayTime) variableDelayTime = (float)maxDelayTime; // make sure delay time not too large for buffer
		
	for(int i = 0; i < sampleFrames; i++){
		readPointerFloat = index - variableDelayTime; // offset read pointer from write pointer
	
		if(readPointerFloat >= 0){
			if(readPointerFloat >= maxDelayTime){ // wrap around if pointer is out of bounds
				readPointerFloat -= maxDelayTime;
			}
		} else { // otherwise increment
			readPointerFloat += maxDelayTime;
		}
		
		readPointerInt = (int)readPointerFloat;
		frac = readPointerFloat - readPointerInt; // get fractional portion of sample index
		
		if(readPointerInt != maxDelayTime - 1){ // check read pointer for bounds and wrap
			next = delayBuffer[readPointerInt + 1];
		} else {
			next = delayBuffer[0];
		}
		
		out = delayBuffer[readPointerInt] + frac * (next - delayBuffer[readPointerInt]); // output interpolated value from delay line
		delayBuffer[index] = in1[i] + out * 0.5; // write new sample to delay buff, mix in feedbackk
		(*out1++) = (out + in1[i]) * 0.5; // write to output buffer
		if(index != maxDelayTime - 1){ // wrap write pointer if needed
			index++;
		} else {
			index = 0;
		}
	}

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
