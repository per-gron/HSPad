/*
 PADsynth implementation as ready-to-use C++ class.
 By: Nasca O. Paul, Tg. Mures, Romania
 This implementation and the algorithm are released under Public Domain
 Feel free to use it into your projects or your products ;-)
 
 This implementation is tested under GCC/Linux, but it's 
 very easy to port to other compiler/OS. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "PADsynth.h"

PADsynth::PADsynth(int N_){
    N=N_;
    
    fftr_cfg = kiss_fftr_alloc(N, true, 0, 0);
    freq_amp=new REALTYPE[N/2];
};

PADsynth::~PADsynth(){
    free(fftr_cfg);
    delete[] freq_amp;
};

REALTYPE PADsynth::relF(int N){
    return N;
};

void PADsynth::synth(int samplerate, int number_harmonics, REALTYPE* harmonics, REALTYPE f,REALTYPE bw,REALTYPE bwscale,REALTYPE *smp){
    int i,nh;
    
    for (i=0;i<N/2;i++) freq_amp[i]=0.0; // Default, all the frequency amplitudes are zero
    
    for (nh=1;nh<number_harmonics;nh++){ // For each harmonic
        REALTYPE bw_Hz; // Bandwidth of the current harmonic measured in Hz
        REALTYPE bwi;
        REALTYPE fi;
        REALTYPE rF=f*relF(nh);
        
        bw_Hz=(pow(2.0,bw/1200.0)-1.0)*f*pow(relF(nh),bwscale);
        
        bwi=bw_Hz/(2.0*samplerate);
        fi=rF/samplerate;
        
        // Unoptimized version of the loop: beginning_of_range=0; end_of_range = N/2
        int beginning_of_range = (-4*bwi+fi)*N;
        if (beginning_of_range < 0) beginning_of_range = 0;
        
        int end_of_range = (4*bwi+fi)*N + 1;
        if (end_of_range > N/2) end_of_range = N/2;
        
        for (i=beginning_of_range; i<end_of_range; i++) {
            REALTYPE x=((i/(REALTYPE)N)-fi)/bwi;
            x*=x;
            
            // This avoids computing the e^(-x^2) where its results are very close to zero,
            // but the optimization is made redundant because of how the loop is designed.
            if (x>14.71280603) continue;
            
            freq_amp[i] += exp(-x)/bwi * harmonics[nh];
        }
    }
    
    kiss_fft_cpx* cx_in = (kiss_fft_cpx*) malloc(sizeof(kiss_fft_cpx)*N/2+1);
    
    //Convert the freq_amp array to complex array (real/imaginary) by making the phases random
    for (i=0;i<N/2;i++){
        REALTYPE phase=RND()*2.0*3.14159265358979;
        cx_in[i].r = freq_amp[i]*cos(phase);
        cx_in[i].i = freq_amp[i]*sin(phase);
    }
    
    kiss_fftri(fftr_cfg, cx_in, smp);
    
    free(cx_in);
    
    //normalize the output
    REALTYPE max=0.0;
    for (i=0;i<N;i++) if (fabs(smp[i])>max) max=fabs(smp[i]);
    if (max<1e-5) max=1e-5;
    for (i=0;i<N;i++) smp[i]/=max*1.4142;
    
};

REALTYPE PADsynth::RND(){
    return (rand()/(RAND_MAX+1.0));
};


