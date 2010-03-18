/*
 PADsynth implementation as ready-to-use C++ class.
 By: Nasca O. Paul, Tg. Mures, Romania
 This implementation and the algorithm are released under Public Domain
 Feel free to use it into your projects or your products ;-)
 
 This implementation is tested under GCC/Linux, but it's 
 very easy to port to other compiler/OS.
 
 Please notice that you have to implement the virtual method IFFT() with your IFFT routine.
 
 */

#ifndef PADSYNTH_H
#define PADSYNTH_H

#include "kiss_fftr.h"

#ifndef REALTYPE
#define REALTYPE float
#endif

class PADsynth{
public:
	/*  PADsynth:
     N                - is the samplesize (eg: 262144)
     samplerate 	 - samplerate (eg. 44100)
     number_harmonics - the number of harmonics that are computed */
	PADsynth(int N_);
    
	~PADsynth();
    
	/*  synth() generates the wavetable
     f		- the fundamental frequency (eg. 440 Hz)
     bw		- bandwidth in cents of the fundamental frequency (eg. 25 cents)
     bwscale	- how the bandwidth increase on the higher harmonics (recomanded value: 1.0)
     *smp	- a pointer to allocated memory that can hold N samples */
	void synth(int samplerate,
               int number_harmonics, REALTYPE* harmonics,
               REALTYPE f,REALTYPE bw,
               REALTYPE bwscale, REALTYPE *smp);
protected:
	int N;			//Size of the sample
    
	/* relF():
     This method returns the N'th overtone's position relative 
     to the fundamental frequency.
     By default it returns N.
     You may override it to make metallic sounds or other 
     instruments where the overtones are not harmonic.  */
	REALTYPE relF(int N);
    
	/* RND() - a random number generator that 
     returns values between 0 and 1
     */
	REALTYPE RND();
    
private:
    kiss_fftr_cfg fftr_cfg;
	REALTYPE *freq_amp;	//Amplitude spectrum
};

#endif

