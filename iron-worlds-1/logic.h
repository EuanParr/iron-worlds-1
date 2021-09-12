#ifndef LOGIC_H_INCLUDED
#define LOGIC_H_INCLUDED

#include <functional>
#include <iostream>
#include <random>

namespace logic
{
    constexpr auto pi = 3.14159265358979323846;
    constexpr auto tau = 2 * pi;

    double unitRand();

    template <typename T>
    class LiveValue
    {
        bool value;
        std::function<void(T)> callback;

        //void nop(T) {}

    public:
        explicit LiveValue(std::function<void(T)> newCallback) : value(), callback(newCallback) {}
        //LiveValue() : value(), callback([this](){}) {}

        void updateCallback(std::function<void(T)> newCallback) {callback = newCallback;}
        void updateValue(T newValue) {value = newValue; callback(value);}
        T queryValue() {return value;}
    };
}

#endif // LOGIC_H_INCLUDED
