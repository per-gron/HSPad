/*
 *	File:		HSPad.cpp
 *	
 *	Version:	1.0
 * 
 *	Created:	9/11/06
 *	
 *	Copyright:  Copyright © 2010 Per Eckerdal, All Rights Reserved
 * 
 *	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in 
 *				consideration of your agreement to the following terms, and your use, installation, modification 
 *				or redistribution of this Apple software constitutes acceptance of these terms.  If you do 
 *				not agree with these terms, please do not use, install, modify or redistribute this Apple 
 *				software.
 *
 *				In consideration of your agreement to abide by the following terms, and subject to these terms, 
 *				Apple grants you a personal, non-exclusive license, under Apple's copyrights in this 
 *				original Apple software (the "Apple Software"), to use, reproduce, modify and redistribute the 
 *				Apple Software, with or without modifications, in source and/or binary forms; provided that if you 
 *				redistribute the Apple Software in its entirety and without modifications, you must retain this 
 *				notice and the following text and disclaimers in all such redistributions of the Apple Software. 
 *				Neither the name, trademarks, service marks or logos of Apple Computer, Inc. may be used to 
 *				endorse or promote products derived from the Apple Software without specific prior written 
 *				permission from Apple.  Except as expressly stated in this notice, no other rights or 
 *				licenses, express or implied, are granted by Apple herein, including but not limited to any 
 *				patent rights that may be infringed by your derivative works or by other works in which the 
 *				Apple Software may be incorporated.
 *
 *				The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, EXPRESS OR 
 *				IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY 
 *				AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE 
 *				OR IN COMBINATION WITH YOUR PRODUCTS.
 *
 *				IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 *				DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
 *				OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 *				REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER 
 *				UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN 
 *				IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/*=============================================================================
	HSPad.cpp
	
 =============================================================================*/

/*
	This is an example implementation of a sin wave synth using AUInstrumentBase classes
	
	It illustrates a basic usage of these classes
	
	It artificially limits the number of notes at one time to 12, so the note-stealing 
	algorithm is used - you should know how this works!
	
	Most of the work you need to do is defining a Note class (see HSNote). AUInstrument manages the
	creation and destruction of notes, the various stages of a note's lifetime.
	
	The project also defines CA_AUTO_MIDI_MAP (OTHER_C_FLAGS). This adds all the code that is needed
	to map MIDI messages to specific parameter changes. This can be seen in AU Lab's MIDI Editor window
	CA_AUTO_MIDI_MAP is implemented in AUMIDIBase.cpp/.h
*/


#include "HSPad.h"

#include "HSWavetable.h"

COMPONENT_ENTRY(HSPad)

#pragma mark HSPad Methods

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	HSPad::HSPad
//
// This synth has No inputs, One output
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
HSPad::HSPad(ComponentInstance inComponentInstance)
	: AUMonotimbralInstrumentBase(inComponentInstance, 0, 1)
{
	CreateElements();	
	Globals()->UseIndexedParameters(kNumberOfParameters);
    
    
	Globals()->SetParameter(kParameter_Volume, kDefaultValue_Volume);
    
	Globals()->SetParameter(kParameter_HarmonicsAmount,         kDefaultValue_HarmonicsAmount);
	Globals()->SetParameter(kParameter_HarmonicsCurveSteepness, kDefaultValue_HarmonicsCurveSteepness);
	Globals()->SetParameter(kParameter_HarmonicsBalance,        kDefaultValue_HarmonicsBalance);
	Globals()->SetParameter(kParameter_HarmonicBandwidth,       kDefaultValue_HarmonicBandwidth);
	Globals()->SetParameter(kParameter_HarmonicProfile,         kDefaultValue_HarmonicProfile);
    
	Globals()->SetParameter(kParameter_TouchSensitivity, kDefaultValue_TouchSensitivity);
	Globals()->SetParameter(kParameter_AttackTime,       kDefaultValue_AttackTime);
	Globals()->SetParameter(kParameter_ReleaseTime,      kDefaultValue_ReleaseTime);
    
    parameterListener = 0;
    
    wavetable = 0;
}

void MyEventListenerProc(void *                      inUserData,
                         void *                      inObject,
                         const AudioUnitParameter *  inParameter,
                         AudioUnitParameterValue     inValue)
{
    ((HSPad*)inUserData)->GenerateWavetables();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	HSPad::Initialize
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus HSPad::Initialize()
{
	OSStatus ret = noErr;
    
    ret = AUMonotimbralInstrumentBase::Initialize();
    if (ret != noErr) return ret;
    
	SetNotes(kNumNotes, kMaxActiveNotes, mHSNotes, sizeof(HSNote));
    
    wavetable = new HSWavetable(kNumWavetables,
                                GetOutput(0)->GetStreamFormat().mSampleRate,
                                kNumSamplesPerWavetable,
                                Globals()->GetParameter(kParameter_HarmonicBandwidth),
                                Globals()->GetParameter(kParameter_HarmonicProfile), // Harmonic bandwidth scale
                                Globals()->GetParameter(kParameter_HarmonicsAmount),
                                Globals()->GetParameter(kParameter_HarmonicsCurveSteepness),
                                Globals()->GetParameter(kParameter_HarmonicsBalance),
                                kHarmonicsCompensation);
    
    if (0 == parameterListener) {
        ret = AUListenerCreate(MyEventListenerProc,
                               this,
                               CFRunLoopGetCurrent(),
                               kCFRunLoopDefaultMode,
                               0.5,
                               &parameterListener);
        
        if (ret != noErr) {
            return ret;
        }
    }
    
    for (int i=0; i<kNumParametersThatAreRelevantToWavetable; i++) {
        AudioUnitParameter  parameter; 
        parameter.mAudioUnit = GetComponentInstance();
        parameter.mParameterID = kParametersThatAreRelevantToWavetable[i];
        parameter.mScope = kAudioUnitScope_Global;
        parameter.mElement = 0;
        
        ret = AUListenerAddParameter(parameterListener, this, &parameter);
        if (ret != noErr) {
            return ret;
        }
    }
    
    return ret;
}

OSStatus HSPad::GenerateWavetables()
{
    wavetable->generateWavetables(Globals()->GetParameter(kParameter_HarmonicBandwidth),
                                  Globals()->GetParameter(kParameter_HarmonicProfile), // Harmonic bandwidth scale
                                  Globals()->GetParameter(kParameter_HarmonicsAmount),
                                  Globals()->GetParameter(kParameter_HarmonicsCurveSteepness),
                                  Globals()->GetParameter(kParameter_HarmonicsBalance),
                                  kHarmonicsCompensation);
    
    return noErr;
}

void HSPad::Cleanup()
{
    for (int i=0; i<kNumParametersThatAreRelevantToWavetable; i++) {
        AudioUnitParameter  parameter; 
        parameter.mAudioUnit = GetComponentInstance();
        parameter.mParameterID = kParametersThatAreRelevantToWavetable[i];
        parameter.mScope = kAudioUnitScope_Global;
        parameter.mElement = 0;
        
        AUListenerRemoveParameter(parameterListener, 0, &parameter);
    }
    
    delete wavetable;
    wavetable = 0;
    
    AUMonotimbralInstrumentBase::Cleanup();
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	HSPad::GetParameterInfo
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus HSPad::GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo &outParameterInfo)
{
    ComponentResult result = noErr;
    
    outParameterInfo.flags = kAudioUnitParameterFlag_IsWritable
                             | kAudioUnitParameterFlag_IsReadable;
    
    if (inScope != kAudioUnitScope_Global) return kAudioUnitErr_InvalidScope;
    
    
    switch (inParameterID) {
        case kParameter_Volume:
            AUBase::FillInParameterName(outParameterInfo, kParamName_Volume, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Decibels;
            outParameterInfo.minValue =     kMinimumValue_Volume;
            outParameterInfo.maxValue =     kMaximumValue_Volume;
            outParameterInfo.defaultValue = kDefaultValue_Volume;
            break;
            
        case kParameter_HarmonicsAmount:
            AUBase::FillInParameterName(outParameterInfo, kParamName_HarmonicsAmount, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Generic;
            outParameterInfo.minValue =     kMinimumValue_HarmonicsAmount;
            outParameterInfo.maxValue =     kMaximumValue_HarmonicsAmount;
            outParameterInfo.defaultValue = kDefaultValue_HarmonicsAmount;
            break;
            
        case kParameter_HarmonicsCurveSteepness:
            AUBase::FillInParameterName(outParameterInfo, kParamName_HarmonicsCurveSteepness, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Generic;
            outParameterInfo.minValue =     kMinimumValue_HarmonicsCurveSteepness;
            outParameterInfo.maxValue =     kMaximumValue_HarmonicsCurveSteepness;
            outParameterInfo.defaultValue = kDefaultValue_HarmonicsCurveSteepness;
            break;
            
        case kParameter_HarmonicsBalance:
            AUBase::FillInParameterName(outParameterInfo, kParamName_HarmonicsBalance, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Cents;
            outParameterInfo.minValue =     kMinimumValue_HarmonicsBalance;
            outParameterInfo.maxValue =     kMaximumValue_HarmonicsBalance;
            outParameterInfo.defaultValue = kDefaultValue_HarmonicsBalance;
            break;
            
        case kParameter_HarmonicBandwidth:
            AUBase::FillInParameterName(outParameterInfo, kParamName_HarmonicBandwidth, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Cents;
            outParameterInfo.minValue =     kMinimumValue_HarmonicBandwidth;
            outParameterInfo.maxValue =     kMaximumValue_HarmonicBandwidth;
            outParameterInfo.defaultValue = kDefaultValue_HarmonicBandwidth;
            break;
            
        case kParameter_HarmonicProfile:
            AUBase::FillInParameterName(outParameterInfo, kParamName_HarmonicProfile, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Generic;
            outParameterInfo.minValue =     kMinimumValue_HarmonicProfile;
            outParameterInfo.maxValue =     kMaximumValue_HarmonicProfile;
            outParameterInfo.defaultValue = kDefaultValue_HarmonicProfile;
            break;
            
        case kParameter_TouchSensitivity:
            AUBase::FillInParameterName(outParameterInfo, kParamName_TouchSensitivity, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Generic;
            outParameterInfo.minValue =     kMinimumValue_TouchSensitivity;
            outParameterInfo.maxValue =     kMaximumValue_TouchSensitivity;
            outParameterInfo.defaultValue = kDefaultValue_TouchSensitivity;
            break;
            
        case kParameter_AttackTime:
            AUBase::FillInParameterName(outParameterInfo, kParamName_AttackTime, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Milliseconds;
            outParameterInfo.minValue =     kMinimumValue_AttackTime;
            outParameterInfo.maxValue =     kMaximumValue_AttackTime;
            outParameterInfo.defaultValue = kDefaultValue_AttackTime;
            break;
            
        case kParameter_ReleaseTime:
            AUBase::FillInParameterName(outParameterInfo, kParamName_ReleaseTime, false);
            outParameterInfo.unit =         kAudioUnitParameterUnit_Milliseconds;
            outParameterInfo.minValue =     kMinimumValue_ReleaseTime;
            outParameterInfo.maxValue =     kMaximumValue_ReleaseTime;
            outParameterInfo.defaultValue = kDefaultValue_ReleaseTime;
            break;
            
        default:
            result = kAudioUnitErr_InvalidParameter;
            break;
    }
    
	return result;
}



#pragma mark HSNote Methods



void HSNote::Attack(const MusicDeviceNoteParams &inParams)
{
    HSPad* hsp = (HSPad*) GetAudioUnit();
    wavetable = hsp->getWavetable();
    float freq = Frequency()*(1-GetGlobalParameter(kParameter_TouchSensitivity)*pow(inParams.mVelocity/127., 2.));
    wavetable_idx = wavetable->closestMatchingWavetable(freq);
    wavetable_num_samples = wavetable->getNumSamples();
    wavetable_sample_rate = wavetable->getSampleRate();
    
    double sampleRate = SampleRate();
    phase = (rand()/(RAND_MAX+1.0))*wavetable_num_samples;
    amp = 0.;
    maxamp = 0.4 * pow(inParams.mVelocity/127., 2.); 
    
    float at = GetGlobalParameter(kParameter_AttackTime);
    // If at is 0, we set up_slope to maxamp/50. That is big enough to start the note
    // seemingly immediately, yet avoiding an ugly chipping sound that comes if you set
    // it to maxamp.
    up_slope = maxamp / (at==0.0?50.0:(at/1000.0 * sampleRate));
    
    float rt = GetGlobalParameter(kParameter_ReleaseTime);
    if (rt == 0) {
        // This is not 0, because that makes an ugly chipping sound when the note
        // is released. 0.99 is small enough to stop the note seemingly immediately
        dn_slope = 0.99;
    }
    else {
        double num_frames = rt/1000.0 * sampleRate;
        double off_threshold = 0.01; // 20dB
        dn_slope = pow(off_threshold, (double)1.0/num_frames);
    }
    
    fast_dn_slope = -maxamp / (0.005 * sampleRate);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	HSPad::Render
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus		HSNote::Render(UInt32 inNumFrames, AudioBufferList& inBufferList)
{
	int numChans = inBufferList.mNumberBuffers;
	if (numChans > 2) return -1;
    
    float volumeFactor = pow(10, GetGlobalParameter(kParameter_Volume)/10);
    
	float *left, *right;
    
    wavetable->lockWavetables(); {
        float *wt = wavetable->getWavetableData(wavetable_idx);
        float base_frequency = wavetable->getBaseFrequency(wavetable_idx);
        
        left = (float*)inBufferList.mBuffers[0].mData;
        right = numChans == 2 ? (float*)inBufferList.mBuffers[1].mData : 0;
        
        double freq = Frequency()/base_frequency*((double)wavetable_sample_rate)/SampleRate();
        
        switch (GetState())
        {
            case kNoteState_Attacked :
            case kNoteState_Sostenutoed :
            case kNoteState_ReleasedButSostenutoed :
            case kNoteState_ReleasedButSustained :
			{
				for (UInt32 frame=0; frame<inNumFrames; ++frame)
				{
					if (amp < maxamp) amp += up_slope;
                    
                    int pint = (int) phase;
                    float out1 = wt[pint%wavetable_num_samples]; // TODO This occasionally crashes while changing parameters; dunno why
                    float out2 = wt[(pint+1)%wavetable_num_samples];
                    float out =  ((1-(phase-pint))*out1+(phase-pint)*out2) * amp * volumeFactor;
                    
					phase += freq;
					if (phase >= wavetable_num_samples) phase -= wavetable_num_samples;
					left[frame] += out;
					if (right) right[frame] += out;
				}
			}
                break;
                
            case kNoteState_Released :
			{
				UInt32 endFrame = 0xFFFFFFFF;
				for (UInt32 frame=0; frame<inNumFrames; ++frame)
				{
					if (amp > 0.0) amp *= dn_slope;
					else if (endFrame == 0xFFFFFFFF) endFrame = frame;
                    
                    int pint = (int) phase;
                    float out1 = wt[pint%wavetable_num_samples];
                    float out2 = wt[(pint+1)%wavetable_num_samples];
                    float out =  ((1-(phase-pint))*out1+(phase-pint)*out2) * amp * volumeFactor;
                    
					phase += freq;
					if (phase >= wavetable_num_samples) phase -= wavetable_num_samples;
					left[frame] += out;
					if (right) right[frame] += out;
				}
				if (endFrame != 0xFFFFFFFF)
					NoteEnded(endFrame);
			}
                break;
                
            case kNoteState_FastReleased :
			{
				UInt32 endFrame = 0xFFFFFFFF;
				for (UInt32 frame=0; frame<inNumFrames; ++frame)
				{
					if (amp > 0.0) amp += fast_dn_slope;
					else if (endFrame == 0xFFFFFFFF) endFrame = frame;
                    
                    int pint = (int) phase;
                    float out1 = wt[pint%wavetable_num_samples];
                    float out2 = wt[(pint+1)%wavetable_num_samples];
                    float out =  ((1-(phase-pint))*out1+(phase-pint)*out2) * amp * volumeFactor;
                    
					phase += freq;
					if (phase >= wavetable_num_samples) phase -= wavetable_num_samples;
					left[frame] += out;
					if (right) right[frame] += out;
				}
				if (endFrame != 0xFFFFFFFF)
					NoteEnded(endFrame);
			}
                break;
            default :
                break;
        }
        
    } wavetable->unlockWavetables();
    return noErr;
}

