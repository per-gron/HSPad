# HSPad

A simple AudioUnit instrument based on the PADSynth algorithm. It is
intended to sound like the characteristic pad that is all too common
in music from Hillsong United and related bands. It is capable of a
range of sounds, but the sweet spot is definitely there, and it won't
do anything very different from it.

See <http://zynaddsubfx.sourceforge.net/doc/PADsynth/PADsynth.htm>.

This is effectively a sample-based synth, but the samples are
generated on-the-fly. This makes it take relatively small amounts of
CPU, relatively large amounts of memory, but the main effect of this
is that most parameter changes do not show up immediately. In
practise, this means that it's not practical to change the parameters
of the synth while playing on it.

## Recommended settings

I get the best result by setting a relatively low attack and release
time, combining it with a relatively long and prominent reverb.

## Parameters

* **Volume** sets the output level of the synth. Adjustable between
  -30dB and +6dB.
* **Harmonics amount** sets the amount of harmonics. A higher
  harmonics amount will result in a brighter sound, and will also
  increase the time required to generate the wavetables, which
  makes it take longer time before parameter changes take effect.
* **Harmonics curve steepness** sets the shape of the curve that
  dictates the loudness of each harmonic. A value of 0 will make all
  harmonics equally loud, usually creating a harsh noisy sound. A
  high value will make the low harmonics much louder than the high
  ones, resulting in a soft, warm sound.
* **Harmonics balance** sets the balance between even-order and
  odd-order harmonics. This is an effect that is difficult to
  describe, it changes the character of the sound. A low value will
  result in that the fundamental tone will be quiet or inaudible,
  resulting in an octave effect.
* **Lushness** dictates the width of the frequency distribution of
  each harmonic. It is measured in percents of a semitone. A low
  value gives static, very regular, "organ-y" sound. A moderate value
  provides a lush character to the sound. A high value results in a
  nauseating sound.
* **Lushness type** determines the relative frequency distribution of
  the harmonics. A low value makes the low harmonics have a wider
  frequency distribution than the high harmonics. A high value makes
  the high harmonics have a wider frequency distribution than the low
  ones. This is a quite subtle effect, and I'm not sure that it's
  very usable. A value of 1.0 will give a decent sound. Higher values
  will result in an effect similar to increasing the *Lushness*
  parameter.
* **Touch sensitivity** sets how much the synth should react to
  differences in note velocity. A high value will make notes with
  high velocity sound brighter. A value of zero will make all notes
  be equally bright. Because of how this parameter is implemented,
  a higher value will result in a brighter sound overall. This can be
  compensated with the *harmonics amount* and *harmonics curve*
  *steepness* parameters.
* **Attack time** sets the attack time of the envelope.
* **Release time** sets the release time of the envelope. I have
  gotten beautiful results by setting the release time low and
  instead have a prominent reverb effect on the synth. This gives a
  beautiful, lush sound. (If that's what you're after)
