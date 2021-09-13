#include "audio.h"

#include <cmath>

#include "logic.h"

namespace audio
{
    PCMBuffer::PCMBuffer(int numSamples_, double sampleRate_)
        : numSamples(numSamples_), sampleRate(sampleRate_)
    {
        buf = new float[numSamples];

        float* ptr = buf;
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            *ptr++ = 0;
        }
    }

    PCMBuffer::~PCMBuffer()
    {
        delete[] buf;
    }

    void PCMBuffer::addSin(double frequency, double amplitude, double initialPhase)
    {
        double phase = initialPhase;
        double dPhaseDSample = logic::tau * frequency / sampleRate;

        float* ptr = buf;
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            *ptr++ += static_cast<float>(amplitude * sin(phase));
            phase += dPhaseDSample;
        }
    }

    void PCMBuffer::addStandingWave(double fundFreq, double fundAmp)
    {
        double amp = fundAmp;
        double atten = 0.2;
        for (unsigned int i = 1; i <= 20; ++i)
        {
            addSin(fundFreq * i, amp, 0.0);
            amp *= atten;
        }
    }

    double PCMBuffer::a4RelativeSemiTone(int step)
    {
        return fA4 * pow(pRat, step);
    }

    void PCMBuffer::putNote(int note, double amp)
    {
        addStandingWave(a4RelativeSemiTone(note), amp);
    }

}
