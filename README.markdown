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

## Installation instructions

To install the synth, download
[the AudioUnit component](http://github.com/downloads/pereckerdal/HSPad/HSPad.component.zip),
extract the zip file, and put the file in either
`~/Library/Audio/Plug-Ins/Components` or
`/Library/Audio/Plug-Ins/Components`, depending on whether you want
to install it only for your user or for all users. If you don't know
which one to pick, I recommend putting it inside your user directory.


## Recommended settings

These settings result in a pad sound that is kind of what I had in
mind when creating this synth:

* **Volume**: 0.0dB
* **Harmonics amount**: 5.00
* **Harmonics curve steepness**: 0.85
* **Harmonics balance**: 0.50
* **Lushness**: 53ct
* **Touch sensitivity**: 0.20
* **Attack time**: 0ms
* **Release time**: 650ms

The sound will be much improved if the release time is set to zero
and a prominent and relatively long reverb effect is used instead.

## Parameters

* **Volume** sets the output level of the synth. Adjustable between
  -30dB and +6dB.
* **Harmonics amount** sets the amount of harmonics. A higher
  harmonics amount will result in a brighter sound, and will also
  increase the time required to generate the wavetables, which
  makes it take longer time before parameter changes take effect.
  Note that setting this value very low might result in that the
  synth is completely quiet. This can sometimes be partly
  compensated by setting a high value on the harmonics curve
  steepness, but in general it's not recommended to set this below
  2.0.
* **Harmonics curve steepness** sets the shape of the curve that
  dictates the loudness of each harmonic. A value of 0 will make all
  harmonics equally loud, usually creating a harsh noisy sound. A
  high value will make the low harmonics much louder than the high
  ones, resulting in a soft, warm sound.
* **Harmonics balance** sets the balance between even-order and
  odd-order harmonics. This is an effect that is difficult to
  describe, it changes the character of the sound. A low value will
  result in that the fundamental will be quiet or inaudible,
  resulting in an octave effect. 0.5 is a resonable default value.
* **Lushness** dictates the width of the frequency distribution of
  each harmonic. It is measured in percents of a semitone. A low
  value gives static, very regular, "organ-y" sound. A moderate value
  provides a lush character to the sound. A high value results in a
  nauseating sound. The lowest possible value gives a quite even
  sound, increasing it just a little bit will add a slow
  amplitude-variation, tremolo-like effect.
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
  *steepness* parameters. Note that this parameter does not influence
  the loudness of each note. To do that, I recommend the use of a
  MIDI compression effect before this synth.
* **Attack time** sets the attack time of the envelope.
* **Release time** sets the release time of the envelope. I have
  got beautiful results by setting the release time low and instead
  have a prominent reverb effect on the synth. This gives a
  beautiful, lush sound. (If that's what you're after)
