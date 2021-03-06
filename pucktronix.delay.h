//-------------------------------------------------------------------------------------------------------
// VST Plug-Ins SDK
// Version 2.4		$Date: 2006/11/13 09:08:27 $
//
// Category     : VST 2.x SDK Samples
// Filename     : again.h
// Created by   : Steinberg Media Technologies
// Description  : Stereo plugin which applies Gain [-oo, 0dB]
//
// � 2006, Steinberg Media Technologies, All Rights Reserved
//-------------------------------------------------------------------------------------------------------

#ifndef __pucktronix.delay__
#define __pucktronix.delay__

#include "public.sdk/source/vst2.x/audioeffectx.h"

//-------------------------------------------------------------------------------------------------------

enum
{
	// Global
	kNumPrograms = 16,
	
	// Parameters Tags
	kDelay = 0,
	kFeedBack,
	kFCutoff,
	kNumParams
};

class PDelay : public AudioEffectX
{
public:
	PDelay (audioMasterCallback audioMaster);
	~PDelay ();

	// Processing
	virtual void processReplacing (float** inputs, float** outputs, VstInt32 sampleFrames);
	virtual void processDoubleReplacing (double** inputs, double** outputs, VstInt32 sampleFrames);
	float filter(float * sig);
	
	// Program
	virtual void setProgramName (char* name);
	virtual void getProgramName (char* name);

	// Parameters
	virtual void setParameter (VstInt32 index, float value);
	virtual float getParameter (VstInt32 index);
	virtual void getParameterLabel (VstInt32 index, char* label);
	virtual void getParameterDisplay (VstInt32 index, char* text);
	virtual void getParameterName (VstInt32 index, char* text);

	virtual bool getEffectName (char* name);
	virtual bool getVendorString (char* text);
	virtual bool getProductString (char* text);
	virtual VstInt32 getVendorVersion ();

protected:
	float delayTimeSeconds;
	float endDelayTimeSeconds;
	char programName[kVstMaxProgNameLen + 1];
	float * delayBufferL;
	float * delayBufferR;
	int index; 
	int SR;
	float X1L, X2L, Y1L;
	float X1R, X2R, Y1R;
	float cutoffParam;
	float feedbackParam;
};

#endif
