/*
 *  HSWavetable.cpp
 *  HSPad
 *
 *  Created by Per Eckerdal on 2010-02-19.
 *  Copyright 2010 Per Eckerdal. All rights reserved.
 *
 */

#include "HSWavetable.h"

#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "PADsynth.h"

// AFAIK this doesn't even work; there is no output (because of lack of fflush?)
//#define DEBUG_OUTPUT 1

#ifdef DEBUG_OUTPUT
FILE* dbg_f;
#endif

wavetables_data::wavetables_data(HSWavetable* hswt_, float bw_, float bwscale_, float harmonics_amount_, float harmonics_curve_steepness_, float harmonics_compensation_) :
hswt(hswt_), bw(bw_), bwscale(bwscale_), harmonics_amount(harmonics_amount_), harmonics_curve_steepness(harmonics_curve_steepness_), harmonics_compensation(harmonics_compensation_) {
    wavetables = 0;
    wavetable_frequencies = 0;
}

wavetables_data::~wavetables_data() {
    if (wavetables) {
        for (int i=0; i<hswt->getNumWavetables(); i++) free(wavetables[i]);
        free(wavetables);
    }
    if (wavetable_frequencies) free(wavetable_frequencies);
}

void wavetables_data::generate() {
    // Some convenient aliases
    const int sample_rate = hswt->getSampleRate();
    const int num_wavetables = hswt->getNumWavetables();
    const int num_samples = hswt->getNumSamples();
    PADsynth* const padsynth = hswt->getPADsynth();
    
    // Allocate memory for the wavetables
    wavetables = (float**) malloc(sizeof(float*)*num_wavetables);
    wavetable_frequencies = (float*) malloc(sizeof(float)*num_wavetables);
    for (int i=0; i<num_wavetables; i++)
        wavetables[i] = (float*) malloc(sizeof(float)*num_samples);
    
    // allocate stuff that is only used in this function
    int* wavetable_num_harmonics = (int*) malloc(sizeof(int)*num_wavetables);
    float** wavetable_harmonics = (float**) malloc(sizeof(float*)*num_wavetables);
    
    
    static const double lowest_frequency = 55.0; // TODO Put these in a global constant?
    static const double highest_frequency = 1760.0; // TODO Put this in a global constant?
    
    // We want to find a number x so that lowest_frequency*x^(num_wavetables-1) = highest_frequency
    // This is used to determine the number of harmonics that should be in the different wavetables,
    // based on which base frequency it has. (Higher frequency => less harmonics)
    const double x = pow(highest_frequency/lowest_frequency, ((double)1.0)/(num_wavetables-1));
    
    static const double middle_frequency = 440.0;
    
    for (int i=0; i<num_wavetables; i++) {
        wavetable_frequencies[i] = lowest_frequency*pow(x,i);
        
        float compensated_num_harmonics = middle_frequency/wavetable_frequencies[i]*harmonics_amount;
        float num_harmonics = harmonics_compensation*compensated_num_harmonics + (1-harmonics_compensation)*harmonics_amount;
        wavetable_num_harmonics[i] = ((int) num_harmonics)+1; // +1 just to be sure
        
        wavetable_harmonics[i] = (float*) malloc(sizeof(float)*wavetable_num_harmonics[i]);
        
        float harmonics_curve_pow = pow(harmonics_curve_steepness*2, 5);
        for (int j=0; j<wavetable_num_harmonics[i]; j++) {
            wavetable_harmonics[i][j] = pow(1-((float)j)/(wavetable_num_harmonics[i]-1), harmonics_curve_pow);
        }
    }
    
    for (int i=0; i<num_wavetables; i++) {
        padsynth->synth(sample_rate,
                        wavetable_num_harmonics[i],
                        wavetable_harmonics[i],
                        wavetable_frequencies[i],
                        bw,
                        bwscale,
                        wavetables[i]);
    }
    
    // cleanup
    free(wavetable_num_harmonics);
    for (int i=0; i<num_wavetables; i++) free(wavetable_harmonics[i]);
    free(wavetable_harmonics);
}

int wavetables_data::closestMatchingWavetable(float desired_frequency) const {
    // TODO Implement this as a binary search
    const int num_wavetables = hswt->getNumWavetables();
    int mid;
    
    if (desired_frequency < wavetable_frequencies[0]) {
        mid = 0;
    }
    else {
        mid = num_wavetables-1;
        for (int i=0; i<num_wavetables-1; i++) {
            if (desired_frequency <= (wavetable_frequencies[i]+wavetable_frequencies[i+1])/2) {
                mid = i;
                break;
            }
        }
    }
    
    return mid;
}

HSWavetable::HSWavetable(int num_wavetables_, int sample_rate_, int num_samples_, float bw_, float bwscale_, float harmonics_amount_, float harmonics_curve_steepness_, float harmonics_compensation_) {
    sample_rate = sample_rate_;
    num_samples = num_samples_;
    num_wavetables = num_wavetables_;
    
    padsynth = new PADsynth(num_samples);
    
    pthread_mutex_init(&generator_thread_quit_flag, NULL);
    pthread_mutex_init(&to_be_generated_mutex, NULL);
    pthread_mutex_init(&current_wavetable_mutex, NULL);
    
    to_be_generated = 0;
    current_wavetable = 0;
    
    pthread_create(&generator_thread, NULL, &HSWavetable::generatorThread, this);
    
    current_wavetable = new wavetables_data(this, bw_, bwscale_, harmonics_amount_, harmonics_curve_steepness_, harmonics_compensation_);
    current_wavetable->generate();
    
#ifdef DEBUG_OUTPUT
    dbg_f = fopen("/tmp/synt.txt", "w+");
#endif
}

HSWavetable::~HSWavetable() {
    // Signal to the generator thread to quit
    pthread_mutex_lock(&generator_thread_quit_flag);
    pthread_join(generator_thread, NULL);
    
    delete padsynth;
    if (to_be_generated) delete to_be_generated;
    delete current_wavetable;
    pthread_mutex_destroy(&generator_thread_quit_flag);
    pthread_mutex_destroy(&to_be_generated_mutex);
    pthread_mutex_destroy(&current_wavetable_mutex);
    
#ifdef DEBUG_OUTPUT
    fclose(dbg_f);
#endif
}

void HSWavetable::generateWavetables(float bw_, float bwscale_, float harmonics_amount_, float harmonics_curve_steepness_, float harmonics_compensation_) {
    
#ifdef DEBUG_OUTPUT
    fprintf(dbg_f, "Hej\n");
#endif
    
    wavetables_data* wtd = new wavetables_data(this, bw_, bwscale_, harmonics_amount_, harmonics_curve_steepness_, harmonics_compensation_);
    
    pthread_mutex_lock(&to_be_generated_mutex);
    if (to_be_generated) delete to_be_generated;
    to_be_generated = wtd;
    pthread_mutex_unlock(&to_be_generated_mutex);
    
}

void* HSWavetable::generatorThread(void* data) {
    HSWavetable* wt = (HSWavetable*) data;
    pthread_mutex_t to_be_generated_mutex(wt->to_be_generated_mutex);
    pthread_mutex_t current_wavetable_mutex(wt->current_wavetable_mutex);
    
    while (1) {
        // This code is a little bit odd. If the result of the trylock is that we succeeded
        // to lock the mutex, it means that it wasn't locked, so we shouldn't quit. If trylock
        // failed with EBUSY, it means that the mutex was locked to we should quit. If trylock
        // fails with EINVAL, it means that something is wrong so we quit anyways.
        int result = pthread_mutex_trylock(&wt->generator_thread_quit_flag);
        pthread_mutex_unlock(&wt->generator_thread_quit_flag);
        if (result) {
            // Quit
            pthread_exit(NULL);
            return NULL;
        }
        
        wavetables_data* tbg;
        
        pthread_mutex_lock(&to_be_generated_mutex); {
            
            tbg = wt->to_be_generated;
            if (!tbg) {
                // sleep for 40ms. This is to avoid draining unnecessary CPU.
                // This could be done with a condition variable instead, which
                // might be better, but is also slightly more complex.
                usleep(40000);
                pthread_mutex_unlock(&to_be_generated_mutex);
                continue;
            }
            wt->to_be_generated = 0;
            
        } pthread_mutex_unlock(&to_be_generated_mutex);
        
        // This is the heavy operation. It should be made without locks.
        tbg->generate();
        
        pthread_mutex_lock(&current_wavetable_mutex); {
            
            if (wt->current_wavetable) {
                // wt->current_wavetable should never be null at this point, but why risk it
                delete wt->current_wavetable;
            }
            wt->current_wavetable = tbg;
            
        } pthread_mutex_unlock(&current_wavetable_mutex);
    }
    
    pthread_exit(NULL);
    return NULL;
}
