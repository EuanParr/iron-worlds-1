#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#define NUM_KEYS 0xffff

#include "platform.h"

namespace input
{
    typedef void (*keyBoundProcedure) (bool keyIsNowDown);

    extern keyBoundProcedure globalBindings[NUM_KEYS];
    extern bool keyStates[NUM_KEYS];

    class ButtonAction
    {
    public:
        virtual void trigger(bool newButtonState) = 0;

        virtual ~ButtonAction() {};
    };

    class ButtonContainer
    {
        bool isDown;
        ButtonAction* action;

    public:
        ButtonContainer(ButtonAction* newAction) : isDown(false), action(newAction) {};
        ButtonContainer() : isDown(false), action(nullptr) {};
        ~ButtonContainer() {if (action) delete action;}

        void updateState(bool newState) {isDown = newState; if (action) action->trigger(newState);}
        bool queryState() {return isDown;}
    };

    class BindingSet
    {
        std::unordered_map<platform::InputCode, ButtonContainer, platform::InputCodeHash> bindings;
    public:
        void updateButton(platform::InputCode code, bool newState)
        {
            bindings[code].updateState(newState);
            LOG(bindings[code].queryState());
        }
        bool queryState(platform::InputCode code) {return bindings[code].queryState();}
    };
}

#endif // INPUT_H_INCLUDED
