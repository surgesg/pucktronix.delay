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
#include <math.h>

/*
	todo:
	* variable delay time - interpolation (using two variables)
	feedback filter
	feedback saturation
	feedback amount
	wet/dry mix
*/

//-------------------------------------------------------------------------------------------------------
AudioEffect* createEffectInstance (audioMasterCallback audioMaster)
{
	return new PDelay (audioMaster);
}

//-------------------------------------------------------------------------------------------------------
PDelay::PDelay (audioMasterCallback audioMaster)
: AudioEffectX (audioMaster, 1, 3)	// 1 program, 1 parameter only
{
	setNumInputs (1);		// stereo in
	setNumOutputs (1);		// stereo out
	setUniqueID ('ptdl');	// identify
	canProcessReplacing ();	// supports replacing output
	canDoubleReplacing ();	// supports double precision processing

	vst_strncpy (programName, "Default", kVstMaxProgNameLen);	// default program name
	SR = getSampleRate();
	if (SR == 0) SR = 44100;
	delayBufferL = new float[2 * SR];
	delayBufferR = new float[2 * SR];
	index = 0;
	delayTimeSeconds = 0.5;
	endDelayTimeSeconds = 0.5;
	X1L = X1R = 0;
	X2L = X2R = 0;
	Y1L = Y1R = 0;
	cutoffParam = 0.1;
	feedbackParam = 0.5;
}

//-------------------------------------------------------------------------------------------------------
PDelay::~PDelay ()
{
	// nothing to do here
	delete [] delayBufferL;
	delete [] delayBufferR;
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
	switch (index) {
		case kDelay:
			endDelayTimeSeconds = value;
			break;
		case kFeedBack:
			feedbackParam = value;
			break;
		case kFCutoff:
			cutoffParam = value;
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------------------------
float PDelay::getParameter (VstInt32 index)
{
	switch (index) {
		case kDelay:
			return delayTimeSeconds;
			break;
		case kFeedBack:
			return feedbackParam;
			break;
		case kFCutoff:
			return cutoffParam;
			break;
		default:
			break;
	}	
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterName (VstInt32 index, char* label)
{
	switch (index) {
		case kDelay:
			vst_strncpy (label, "Delay Time", kVstMaxParamStrLen);
			break;
		case kFeedBack:
			vst_strncpy (label, "Feedback", kVstMaxParamStrLen);
			break;
		case kFCutoff:
			vst_strncpy (label, "Filter Cutoff", kVstMaxParamStrLen);
			break;
		default:
			break;
	}		
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterDisplay (VstInt32 index, char* text)
{
	switch (index) {
		case kDelay:
			float2string(delayTimeSeconds * 1000, text, kVstMaxParamStrLen);
			break;
		case kFeedBack:
			float2string(feedbackParam * 1.25 , text, kVstMaxParamStrLen);
			break;
		case kFCutoff:
			float2string(cutoffParam * 20000, text, kVstMaxParamStrLen);
			break;
		default:
			break;
	}		
}

//-----------------------------------------------------------------------------------------
void PDelay::getParameterLabel (VstInt32 index, char* label)
{
	switch (index) {
		case kDelay:
			vst_strncpy (label, "ms", kVstMaxParamStrLen);
			break;
		case kFeedBack:
			vst_strncpy (label, "%", kVstMaxParamStrLen);
			break;
		case kFCutoff:
			vst_strncpy (label, "Hz", kVstMaxParamStrLen);
			break;
		default:
			break;
	}			
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
    float* in2  =  inputs[1];
    float* out1 = outputs[0];
    float* out2 = outputs[1];
	
	int maxDelayTime, readPointerInt;
	float out, filtered, readPointerFloat, variableDelayTime, frac, next, variableEndDelayTime, delayTimeIncrement;
	float a0, a1, b1, w, Norm, cutoff;
	float feedback;
	float clipped;
	
	feedback = feedbackParam;
	
	cutoff = cutoffParam * 20000;
	w = 2.0 * (8.0 * atan(1.0)) * SR;
	cutoff *= 8.0 * atan(1.0);
	Norm = 1.0f / (cutoff + w);
	b1 = (w - cutoff) * Norm;
	a0 = a1 = cutoff * Norm;
	
	variableDelayTime = delayTimeSeconds * SR; 
	variableEndDelayTime = endDelayTimeSeconds * SR;
	if(variableDelayTime != variableEndDelayTime){
		delayTimeIncrement = (variableEndDelayTime - variableDelayTime) / sampleFrames;
	} else {
		delayTimeIncrement = 0;
	}
	maxDelayTime = (int)(2 * SR);

	if(variableDelayTime > maxDelayTime) variableDelayTime = (float)maxDelayTime; // make sure delay time not too large for buffer

	for(int i = 0; i < sampleFrames; i++){
		/** pointer offset and wrap **/
		readPointerFloat = index - variableDelayTime; // offset read pointer from write pointer
		variableDelayTime += delayTimeIncrement;
		if(variableDelayTime > maxDelayTime) variableDelayTime = (float)maxDelayTime; // make sure delay time not too large for buffer
		if(readPointerFloat >= 0){
			if(readPointerFloat >= maxDelayTime){ // wrap around if pointer is out of bounds
				readPointerFloat -= maxDelayTime;
			}
		} else { // otherwise increment
			readPointerFloat += maxDelayTime;
		}
		
		/** check for wrap and interpolate to get next read value **/
		readPointerInt = (int)readPointerFloat;
		frac = readPointerFloat - readPointerInt; // get fractional portion of sample index
		
		if(readPointerInt != maxDelayTime - 1){ // check read pointer for bounds and wrap
			next = delayBufferL[readPointerInt + 1];
		} else {
			next = delayBufferL[0];
		}
		
		/** output and feedback code **/
		out = delayBufferL[readPointerInt] + frac * (next - delayBufferL[readPointerInt]); // output interpolated value from delay line
		
		// filter 
		filtered = out * a0 + X1L * a1 + Y1L * b1;
		X1L = out;
		Y1L = filtered;
		
		clipped = in1[i] + (filtered * feedback * 1.25);
		if (clipped > 1.0) {
			clipped = 1.0;
		} 
		if (clipped < -1.0){
			clipped = -1.0;
		}
		
		clipped = tanh(clipped);
		
		delayBufferL[index] = clipped; // write new sample to delay buff, mix in feedbackk
		(*out1++) = (out + in1[i]) * 0.5; // write to output buffer
		
		/* if stereo, process right chan - this code duplicates the above, using delayBufferR instead */
		if(in2 != NULL){
			if(readPointerInt != maxDelayTime - 1){ // check read pointer for bounds and wrap
				next = delayBufferR[readPointerInt + 1];
			} else {
				next = delayBufferR[0];
			}
			
			/** output and feedback code **/
			out = delayBufferR[readPointerInt] + frac * (next - delayBufferR[readPointerInt]); // output interpolated value from delay line
			
			// filter 
			filtered = out * a0 + X1R * a1 + Y1R * b1;
			X1R = out;
			Y1R = filtered;
			
			clipped = in2[i] + (filtered * feedback * 1.25);
			if (clipped > 1.0) {
				clipped = 1.0;
			} 
			if (clipped < -1.0){
				clipped = -1.0;
			}
			
			clipped = tanh(clipped);
			
			delayBufferR[index] = clipped; // write new sample to delay buff, mix in feedbackk
			(*out2++) = (out + in2[i]) * 0.5; // write to output buffer
		}
		
		if(index != maxDelayTime - 1){ // wrap write pointer if needed
			index++;
		} else {
			index = 0;
		}
	}
	delayTimeSeconds = endDelayTimeSeconds;
}

//-----------------------------------------------------------------------------------------
void PDelay::processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames)
{
	/*
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
