#ifndef STRUCTURE_H_INCLUDED
#define STRUCTURE_H_INCLUDED

#include "platform.h"

#include <iostream>
#include <string>
#include <memory>

namespace session
{
    class Session;
}

namespace structure
{
    class SExpression;
    class SPair;
    class Atom;
    class PrimaryFunction;

    typedef std::shared_ptr<SExpression> SExpressionPtr;
    typedef std::shared_ptr<SPair> SPairPtr;
    typedef std::shared_ptr<Atom> AtomPtr;
    typedef std::shared_ptr<PrimaryFunction> PrimaryFunctionPtr;

    class SExpression
    {
    public:
        virtual ~SExpression() {};

        virtual bool isAtom() const {return false;};
        virtual bool isList() const {return true;};

        friend std::ostream& operator<<(std::ostream& output, const SExpressionPtr& e);
    };

    class SPair : public SExpression
    {
    public:
        SExpressionPtr data_;
        SExpressionPtr next_;

        SPair() {};
        SPair(SExpressionPtr data, SExpressionPtr next):
            data_(data), next_(next) {};

        ~SPair() {};

        friend std::ostream& operator<<(std::ostream& output, const SPairPtr& p);
    };

    class Atom : public SExpression
    {
    public:
        std::string name_;

        Atom() {};
        Atom(std::string name): name_(name) {};

        bool isAtom() const {return true;};
        bool isList() const {return false;};

        friend std::ostream& operator<<(std::ostream& output, const AtomPtr& a);
    };

    class Nil : public Atom
    {
    public:
        using Atom::Atom;

        bool isList() const {return true;};
    };

    class PrimaryFunction : public Atom
    {
    public:
        // an atom that corresponds to a function implemented by the platform
        SExpressionPtr (session::Session::* functionPtr)(SExpressionPtr);

        using Atom::Atom;

        // uses default destructor because functions can't be deleted
    };

    class SExpressionString
    {
    public:
        std::string content;
        friend std::istream& operator>>(std::istream& input, SExpressionString& s);
    };

}

#endif // STRUCTURE_H_INCLUDED
