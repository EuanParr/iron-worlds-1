#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

namespace audio
{
    // Pulse-code modulation buffer
    // sound encoded as regular samples
    struct PCMBuffer
    {
        long int* buf;
        int numSamples;
        double sampleRate;
        double fixedPointConst;

        PCMBuffer(int numSamples_, double sampleRate_);

        ~PCMBuffer();

        // forbid copying
        PCMBuffer(const PCMBuffer& that) = delete;
        PCMBuffer& operator=(const PCMBuffer& that) = delete;

        addSine(double frequency, double amplitude, double initialPhase = 0);
    };
}

#endif // AUDIO_H_INCLUDED
