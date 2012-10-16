DirectShow Sample -- AudioCap
-----------------------------

This sample application demonstrates PCM audio capture using DirectShow and allows
the user to audition the audio input while recording.  AudioCap enables the user to
perform the following tasks: 

- Capture audio to a file
- Select the audio capture device and its corresponding input pin
- View properties on the audio capture device
- Select the sample rate and number of channels (stereo or mono)

NOTE: In order to write .WAV files to your disk, you must first build and register
the WavDest filter in the Samples\Multimedia\DirectShow\Filters\WAVDest directory.
Without this filter, you may audition audio input, but you will not be able to
write it to your disk.