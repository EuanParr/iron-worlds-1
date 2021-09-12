#include "audio.h"

#include <cmath>

#include "logic.h"

namespace audio
{
    PCMBuffer::PCMBuffer(int numSamples_, double sampleRate_)
        : numSamples(numSamples_), sampleRate(sampleRate_),
        fixedPointConst(1.0)
    {
        buf = new long int[numSamples];

        auto ptr = buf;
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            *ptr++ = 0;
        }
    }

    PCMBuffer::~PCMBuffer()
    {
        delete[] buf;
    }

    PCMBuffer::addSine(double frequency, double amplitude, double initialPhase)
    {
        double phase = initialPhase;
        double dPhaseDSample = logic::tau * frequency / sampleRate;

        auto ptr = buf;
        for (unsigned int i = 0; i < numSamples; ++i)
        {
            *ptr++ = static_cast<long int>(amplitude * fixedPointConst * sin(phase));
            phase += dPhaseDSample;
        }
    }
}
