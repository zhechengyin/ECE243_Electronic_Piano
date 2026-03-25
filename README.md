# ECE243_Electronic_Piano


The project implements an electronic piano system on the DE1-SoC board using a PS/2 keyboard for input, real-time audio synthesis for sound generation, and VGA graphics for visual feedback. When a user presses a key on the PS/2 keyboard, the system decodes the scan code and maps it to a musical note. The audio synthesis module generates the corresponding waveform and continuously outputs PCM samples to the audio device through the audio FIFO registers (e.g., AUDIO_BASE). At the same time, the VGA display highlights the corresponding piano key on the screen using the pixel buffer and character buffer memory. The system integrates keyboard input, sound generation, and graphical feedback to provide a responsive and interactive digital piano experience on the embedded platform.



platform/ stores address, and other board related
io/ Hardware module including PS2/Audio/VGA
synth/ algorithm to handling the voice and other logics
app/ where you keep project rules, connects io and synth together and scheduling them

include/ contains header file, declares the function prototypes
src/ contains implementation
