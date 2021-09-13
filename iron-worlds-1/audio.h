#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

namespace audio
{
    // Pulse-code modulation buffer
    // sound encoded as regular samples
    struct PCMBuffer
    {
        float* buf;
        int numSamples;
        double sampleRate;

        double fA4 = 440.0;
        double pRat = 1.05946;

        PCMBuffer(int numSamples_, double sampleRate_);

        ~PCMBuffer();

        // forbid copying
        PCMBuffer(const PCMBuffer& that) = delete;
        PCMBuffer& operator=(const PCMBuffer& that) = delete;

        void addSin(double frequency, double amplitude, double initialPhase = 0);
        void addStandingWave(double fundFreq, double fundAmp);
        double a4RelativeSemiTone(int step);
        int getBufSize() {return sizeof(float) * numSamples;};
        void putNote(int note, double amp);
    };
}

#endif // AUDIO_H_INCLUDED
