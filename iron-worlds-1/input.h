#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#include "platform.h"

#include "functional"

namespace input
{
    class ButtonAction
    {
    public:
        virtual void trigger(bool newButtonState) = 0;

        virtual ~ButtonAction() {};
    };

    class LambdaButtonAction : public ButtonAction
    {
        std::function<void (bool)> functor;

    public:
        LambdaButtonAction(std::function<void (bool)> func)
        : functor(func) {};

        void trigger(bool newButtonState) {functor(newButtonState);};
    };

    class ButtonContainer
    {
        bool isDown;
        ButtonAction* action;

    public:
        ButtonContainer(ButtonAction* newAction) : isDown(false), action(newAction) {};
        ButtonContainer() : isDown(false), action(nullptr) {};
        ~ButtonContainer() {if (action) delete action;};

        void setAction(ButtonAction* newAction) {if (action) delete action; action = newAction;};
        void updateState(bool newState) {isDown = newState; if (action) action->trigger(newState);}
        bool queryState() {return isDown;}
    };

    class BindingSet
    {
    public:
        std::unordered_map<platform::InputCode, ButtonContainer, platform::InputCodeHash> bindings;

        void updateButton(platform::InputCode code, bool newState)
        {
            bindings[code].updateState(newState);
        }
        bool queryState(platform::InputCode code) {return bindings[code].queryState();}
    };
}

#endif // INPUT_H_INCLUDED
