/*
 *  HSWavetable.h
 *  HSPad
 *
 *  Created by Per Eckerdal on 2010-02-19.
 *  Copyright 2010 Per Eckerdal. All rights reserved.
 *
 */

#ifndef __HSWavetable_h__
#define __HSWavetable_h__

#include <pthread.h>

class PADsynth;
class HSWavetable;

struct wavetables_data {
    wavetables_data(HSWavetable* hswt_, float bw_, float bwscale_, float harmonics_amount_, float harmonics_curve_steepness_, float harmonics_balance_, float harmonics_compensation_);
    ~wavetables_data();
    
    void generate();
	int closestMatchingWavetable(float desired_frequency) const;
    
    // These are parameters that are used to generate the wavetables
    HSWavetable* hswt;
    float bw;
    float bwscale;
    float harmonics_amount;
    float harmonics_curve_steepness;
    float harmonics_balance;
    float harmonics_compensation;
    
    // These are the actual wavetable data (and necessary info about which base frequency each table has)
    float** wavetables;
    float* wavetable_frequencies;
};

class HSWavetable {
public:
	HSWavetable(int num_wavetables_, int sample_rate_, int num_samples_, float bw_, float bwscale_, float harmonics_amount_, float harmonics_curve_steepness_, float harmonics_balance_, float harmonics_compensation_);
    
	~HSWavetable();
    
    void generateWavetables(float bw_, float bwscale_, float harmonics_amount_, float harmonics_curve_steepness_, float harmonics_balance_, float harmonics_compensation_);
    
	int closestMatchingWavetable(float desired_frequency) {
        pthread_mutex_lock(&current_wavetable_mutex);
        int result = current_wavetable->closestMatchingWavetable(desired_frequency);
        pthread_mutex_unlock(&current_wavetable_mutex);
        return result;
    }
    
    int getSampleRate() const { return sample_rate; }
    int getNumSamples() const { return num_samples; }
    int getNumWavetables() const { return num_wavetables; }
    PADsynth* getPADsynth() const { return padsynth; }
    
    
    void lockWavetables() { pthread_mutex_lock(&current_wavetable_mutex); }
    void unlockWavetables() { pthread_mutex_unlock(&current_wavetable_mutex); }
    
    // Warning: These methods (getBaseFrequency and getWavetableData) are not safe to call without
    // first calling lockWavetables() and then unlockWavetables()!
    float getBaseFrequency(int wt_idx) const { return current_wavetable->wavetable_frequencies[wt_idx]; }
    // See the warning at the getBaseFrequency method!
    float* getWavetableData(int wt_idx) const { return current_wavetable->wavetables[wt_idx]; }
    
private:
    int num_wavetables;
	int sample_rate;
    int num_samples;
    PADsynth* padsynth;
    
    pthread_t generator_thread;
    // HSWavetable locks this mutex when it wants to signal to the generator thread to quit
    pthread_mutex_t generator_thread_quit_flag;
    
    pthread_mutex_t to_be_generated_mutex;
    wavetables_data* to_be_generated; // This is usually NULL.
    
    pthread_mutex_t current_wavetable_mutex;
    wavetables_data* current_wavetable;
    
    static void* generatorThread(void* data);
};


#endif
