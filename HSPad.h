/*
 *  HSPad.h
 *  HSPad
 *
 *  Created by Mehul Trivedi on 9/12/06.
 *	Copyright:  Copyright © 2010 Per Eckerdal, All Rights Reserved.
 *
 */

#include "HSPadVersion.h"
#include "AUInstrumentBase.h"
#include <AudioToolbox/AudioUnitUtilities.h>

class HSWavetable;

// 
static const UInt32 kNumNotes = 14;
static const UInt32 kMaxActiveNotes = 10;

static const UInt32 kNumWavetables = 10;
static const UInt32 kNumSamplesPerWavetable = 262144;

static const float kHarmonicsCompensation = 0.6667;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


enum {
    kParameter_Volume = 0,
    
    kParameter_HarmonicsAmount = 1,
    kParameter_HarmonicsCurveSteepness = 2,
    kParameter_HarmonicBandwidth = 3,
    kParameter_HarmonicProfile = 4,
    
    kParameter_TouchSensitivity = 5,
    kParameter_AttackTime = 6,
    kParameter_ReleaseTime = 7,
    
	kNumberOfParameters=8
};

static int kNumParametersThatAreRelevantToWavetable = 4;
static int kParametersThatAreRelevantToWavetable[] = {
    kParameter_HarmonicsAmount,
    kParameter_HarmonicsCurveSteepness,
    kParameter_HarmonicBandwidth,
    kParameter_HarmonicProfile
};

static const CFStringRef kParamName_Volume                      = CFSTR("Volume");
static const float       kDefaultValue_Volume                   = 0.0;
static const float       kMinimumValue_Volume                   = -30.0;
static const float       kMaximumValue_Volume                   = 6.0;

static const CFStringRef kParamName_HarmonicsAmount             = CFSTR("Harmonics amount");
static const float       kDefaultValue_HarmonicsAmount          = 10.0;
static const float       kMinimumValue_HarmonicsAmount          = 0.0;
static const float       kMaximumValue_HarmonicsAmount          = 25.0;

static const CFStringRef kParamName_HarmonicsCurveSteepness     = CFSTR("Harmonics curve steepness");
static const float       kDefaultValue_HarmonicsCurveSteepness  = 0.85;
static const float       kMinimumValue_HarmonicsCurveSteepness  = 0.0;
static const float       kMaximumValue_HarmonicsCurveSteepness  = 1.0;

static const CFStringRef kParamName_HarmonicBandwidth           = CFSTR("Lushness");
static const float       kDefaultValue_HarmonicBandwidth        = 53;
static const float       kMinimumValue_HarmonicBandwidth        = 1.0;
static const float       kMaximumValue_HarmonicBandwidth        = 100.0;

static const CFStringRef kParamName_HarmonicProfile             = CFSTR("Lushness type");
static const float       kDefaultValue_HarmonicProfile          = 1.0;
static const float       kMinimumValue_HarmonicProfile          = 0.0;
static const float       kMaximumValue_HarmonicProfile          = 2.0;

static const CFStringRef kParamName_TouchSensitivity            = CFSTR("Touch sensitivity");
static const float       kDefaultValue_TouchSensitivity         = 0.2;
static const float       kMinimumValue_TouchSensitivity         = 0.0;
static const float       kMaximumValue_TouchSensitivity         = 1.0;

static const CFStringRef kParamName_AttackTime                  = CFSTR("Attack time");
static const float       kDefaultValue_AttackTime               = 0.0;
static const float       kMinimumValue_AttackTime               = 0.0;
static const float       kMaximumValue_AttackTime               = 10000.0;

static const CFStringRef kParamName_ReleaseTime                 = CFSTR("Release time");
static const float       kDefaultValue_ReleaseTime              = 650.0;
static const float       kMinimumValue_ReleaseTime              = 0.0;
static const float       kMaximumValue_ReleaseTime              = 10000.0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct HSNote : public SynthNote
{
	virtual					~HSNote() {}
	
	virtual void			Attack(const MusicDeviceNoteParams &inParams);
	virtual Float32			Amplitude() { return amp; } // used for finding quietest note for voice stealing.
	virtual OSStatus		Render(UInt32 inNumFrames, AudioBufferList& inBufferList);
	
    // Instance variables related to wavetable
    int wavetable_num_samples;
    int wavetable_sample_rate;
    int wavetable_idx;
    HSWavetable* wavetable;
    
	double phase;
    
    // Instance variables related to attack envelope
    double amp, maxamp;
	double up_slope, dn_slope, fast_dn_slope;
};

class HSPad : public AUMonotimbralInstrumentBase
{
	public:
	HSPad(ComponentInstance inComponentInstance);
				
	virtual OSStatus			Initialize();
    virtual OSStatus            GenerateWavetables();
    virtual void                Cleanup();
	virtual OSStatus			Version() { return kHSPadVersion; }
    
	virtual OSStatus			GetParameterInfo(AudioUnitScope inScope, AudioUnitParameterID inParameterID, AudioUnitParameterInfo &outParameterInfo);
    
    HSWavetable* getWavetable() { return wavetable; }
	private:
	
	HSNote mHSNotes[kNumNotes];
    AUParameterListenerRef parameterListener;
    HSWavetable* wavetable;
};
