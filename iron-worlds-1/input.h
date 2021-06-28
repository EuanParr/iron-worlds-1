#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#define NUM_KEYS 0xffff

namespace input
{
    typedef void (*keyBoundProcedure) (bool keyIsNowDown);

    extern keyBoundProcedure globalBindings[NUM_KEYS];
    extern bool keyStates[NUM_KEYS];

    enum class Button {q, w, e, a, s, d};

}

#endif // INPUT_H_INCLUDED
