#ifndef SESSION_H_INCLUDED
#define SESSION_H_INCLUDED

#include "structure.h"

#include <iostream>
#include <string>
#include <vector>

// #define OUT(X) printS(X, std::cout); std::cout << "\n"

namespace session
{
    class Session
    {
    public:
        std::vector<structure::AtomPtr> symbolTable;

        structure::AtomPtr
            trueAtomPtr,
            nilAtomPtr,
            errorAtomPtr,
            labelAtomPtr,
            quoteAtomPtr,
            lambdaAtomPtr,
            condAtomPtr,
            defineAtomPtr;
        structure::PrimaryFunctionPtr
            atomAtomPtr,
            eqAtomPtr,
            carAtomPtr,
            cdrAtomPtr,
            consAtomPtr,
            defAtomPtr;

        structure::SPairPtr globalEnvironmentPtr;

        Session();
        ~Session() {};

        void printS(structure::SExpressionPtr sexp, std::ostream& out);

        std::string preTokenise(std::string inString);
        std::vector<std::string> tokenise(std::string inString);
        structure::AtomPtr makeAtom(std::string inString);
        structure::SExpressionPtr parseSingleSExpression(std::vector<std::string>& tokens, int& depth);
        structure::SExpressionPtr read(std::istream& inStream);
        structure::AtomPtr symbolLookup(std::string keyName);

        structure::SExpressionPtr atomLisp(structure::SExpressionPtr argList);
        structure::SExpressionPtr eqLisp(structure::SExpressionPtr argList);
        structure::SExpressionPtr carLisp(structure::SExpressionPtr argList);
        structure::SExpressionPtr cdrLisp(structure::SExpressionPtr argList);
        structure::SExpressionPtr consLisp(structure::SExpressionPtr argList);
        structure::SExpressionPtr defLisp(structure::SExpressionPtr argList);

        structure::SExpressionPtr car0(structure::SExpressionPtr arg);
        structure::SExpressionPtr cdr0(structure::SExpressionPtr arg);
        structure::SExpressionPtr assoc0(structure::SExpressionPtr key, structure::SExpressionPtr environment);
        structure::SExpressionPtr eval0(structure::SExpressionPtr expression, structure::SExpressionPtr environment);
        structure::SExpressionPtr evalList(structure::SExpressionPtr expression, structure::SExpressionPtr environment);
        structure::SExpressionPtr apply0(structure::SExpressionPtr argList);
        structure::SExpressionPtr appq0(structure::SExpressionPtr argList);
    };
}

#endif // SESSION_H_INCLUDED
