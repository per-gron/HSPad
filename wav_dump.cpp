/*
 *  wav_dump.cpp
 *  HSPad
 *
 *  Created by Per Eckerdal on 2010-06-08.
 *  Copyright 2010 Per Eckerdal. All rights reserved.
 *
 */

#include <stdio.h>
#include <math.h>
#include "HSWavetable.h"

void putint(int num, FILE* f) {
    char sampleRateBuf[10];
    *((int*)sampleRateBuf) = num;
    for (int i = 0; i<4; i++) fputc(sampleRateBuf[i], f);
}

void putshort(short num, FILE* f) {
    char sampleRateBuf[10];
    *((short*)sampleRateBuf) = num;
    for (int i = 0; i<2; i++) fputc(sampleRateBuf[i], f);
}

void print_wav_header(FILE* f, int samplerate, int bytespersample, int numsamples) {
    int subchunk2size = numsamples*bytespersample;
    
    fprintf(f, "RIFF");           // ChunkID
    putint(subchunk2size+36, f);  // ChunkSize
    fprintf(f, "WAVE");           // Format
    
    fprintf(f, "fmt ");                       // Subchunk1ID
    putint(16, f);                            // Subchunk1Size
    putshort(1, f);                           // AudioFormat
    putshort(1, f);                           // NumChannels
    putint(samplerate, f);                    // SampleRate
    putint(samplerate*1*bytespersample/8, f); // ByteRate
    putshort(1*bytespersample, f);            // BlockAlign
    putshort(8*bytespersample, f);            // BitsPerSample
    
    fprintf(f, "data");       // Subchunk2ID
    putint(subchunk2size, f); // Subchunk2Size
}

int pow2(int num) {
    int ret = 1;
    for (int i=0; i<num; i++) {
        ret *= 2;
    }
    return ret;
}


int main() {
    int bytes_per_sample = 3;
    int fixed_point_max = pow2(bytes_per_sample*8-1);
    
    fprintf(stderr, "%d\n", fixed_point_max);
    
    int num_wavetables = 6;
    int sample_rate = 96000;
    int num_samples = 262144;
    float lushness = 53.0;
    float harmonics_amount = 5.0;
    float harmonics_curve_steepness = 0.85;
    float harmonics_balance = 0.5;
    
    HSWavetable wt(num_wavetables, sample_rate, num_samples, lushness, 1.0, harmonics_amount, harmonics_curve_steepness, harmonics_balance, 0.6667);
    wt.lockWavetables();
    
    char buf[1000];
    char sampleRateBuf[10];
    for (int i=0; i<num_wavetables; i++) {
        sprintf(buf, "table_%d.wav", i+1);
        FILE* f = fopen(buf, "w");
        
        print_wav_header(f, sample_rate, bytes_per_sample, num_samples);
        
        float* data = wt.getWavetableData(i);
        for (int j=0; j<num_samples; j++) {
            int value = floor(data[j]*fixed_point_max);
            *((int*)sampleRateBuf) = value;
            for (int k=0; k<bytes_per_sample; k++) {
                fputc(sampleRateBuf[k], f);
            }
        }
        
        fclose(f);
    }
    
    wt.unlockWavetables();
    
    return 0;
}
