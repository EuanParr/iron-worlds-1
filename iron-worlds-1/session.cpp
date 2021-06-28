#include "session.h"

#include <functional>

namespace session
{
    Session::Session(): symbolTable{},
        trueAtomPtr(makeAtom("t")),
        errorAtomPtr(makeAtom("error")),
        labelAtomPtr(makeAtom("label")),
        quoteAtomPtr(makeAtom("quote")),
        lambdaAtomPtr(makeAtom("lambda")),
        condAtomPtr(makeAtom("cond")),
        defineAtomPtr(makeAtom("define"))
    {
        nilAtomPtr = std::make_shared<structure::Nil>("nil");
        symbolTable.push_back(nilAtomPtr);

        struct FunPair
        {
            std::string name;
            structure::PrimaryFunctionPtr& symbolPtrReference;
            structure::SExpressionPtr (Session::* functionPtr)(structure::SExpressionPtr);
        };
        FunPair functions[] =
        {
            {"atom", atomAtomPtr, &Session::atomLisp},
            {"eq", eqAtomPtr, &Session::eqLisp},
            {"car", carAtomPtr, &Session::carLisp},
            {"cdr", cdrAtomPtr, &Session::cdrLisp},
            {"cons", consAtomPtr, &Session::consLisp},
            {"def", defAtomPtr, &Session::defLisp}
        };
        for (auto item : functions)
        {
            structure::PrimaryFunctionPtr atom = std::make_shared<structure::PrimaryFunction>(item.name);
            atom->functionPtr = item.functionPtr;
            item.symbolPtrReference = atom;
            symbolTable.push_back(atom);
        }

        globalEnvironmentPtr = std::make_shared<structure::SPair>(nilAtomPtr, nilAtomPtr);
    }

    void Session::printS(structure::SExpressionPtr sexp, std::ostream& out)
    {
        // prints an S-expression in the usual format
        // an atom prints as itself
        if (sexp->isAtom())
        {
            out << sexp;
            return;
        }
        else
        {
            // to print a list, first open a bracket
            out << "(";
            // then print the first item
            printS(car0(sexp), out);
            // then check the next item
            auto i = cdr0(sexp);
            // the linked list will continue until we reach an atom
            while (!i->isAtom())
            {
                // print each item in the list with a space
                out << " ";
                printS(car0(i), out);
                i = cdr0(i);
            }
            // a list might not end in nil, especially if it has been consed
            if (i != nilAtomPtr || false)
            {
                out << " . " << i;
            }
            out << ")";
        }
    }

    std::string Session::preTokenise(std::string inString)
    {
        // inserts appropriate whitespace near brackets, converts all whitespace to spaces
        std::pair<std::string, std::string> replacementRules[] = {{"(", " ( "}, {")", " ) "}, {"'", " ' "}, {"\n", " "}, {"\t", " "}};

        for (std::pair<std::string, std::string>& rule : replacementRules)
        {
            std::string::iterator loc = inString.begin();
            while (loc != inString.end())
            {
                bool match = true;
                std::string::iterator patternChar = rule.first.begin();
                std::string::iterator matchChar = loc;
                while (patternChar != rule.first.end() && matchChar != inString.end())
                {
                    if (*patternChar != *matchChar)
                    {
                        match = false;
                    }

                    ++matchChar;
                    ++patternChar;
                }
                if (match)
                {
                    int index = loc - inString.begin();
                    inString.replace(loc - inString.begin(), rule.first.size(), rule.second);
                    loc = inString.begin() + index;
                    int displacement = rule.second.size() - rule.first.size();
                    while (displacement < 0 && loc != inString.end() && loc != inString.begin() - 1)
                    {
                        ++displacement;
                        --loc;
                    }
                    while (displacement > 0 && loc != inString.end())
                    {
                        --displacement;
                        ++loc;
                    }
                }
                if (loc != inString.end())
                {
                    ++loc;
                }
            }
        }
        return inString;
    }

    std::vector<std::string> Session::tokenise(std::string inString)
    {
        inString = preTokenise(inString);
        // splits by spaces, discards empty strings created
        std::vector<std::string> tokens;
        std::string::iterator startLoc = inString.begin();
        std::string::iterator endLoc = inString.begin();

        while (endLoc != inString.end())
        {
            while ((endLoc != inString.end()) && (*endLoc != ' '))
            {
                ++endLoc;
            }
            if (endLoc == inString.end())
            {
                if (startLoc != endLoc)
                {
                    // we found a token
                    std::string token (startLoc, endLoc);
                    tokens.push_back(token);
                }
            }
            else
            {
                if (startLoc != endLoc)
                {
                    // we found a token
                    std::string token (startLoc, endLoc);
                    tokens.push_back(token);
                }
                startLoc = endLoc + 1;
                endLoc = startLoc;
            }
        }
        return tokens;
    }

    structure::AtomPtr Session::symbolLookup(std::string keyName)
    {
        structure::AtomPtr result = nullptr;
        for (auto atom : symbolTable)
        {
            if (atom->name == keyName)
            {
                result = atom;
            }
        }
        return result;
    }

    structure::AtomPtr Session::makeAtom(std::string inString)
    {
        structure::AtomPtr result;
        structure::AtomPtr lookupResult = symbolLookup(inString);
        if (lookupResult == nullptr)
        {
            result = std::make_shared<structure::Atom>(inString);
            symbolTable.push_back(result);
        }
        else
        {
            result = lookupResult;
        }
        return result;
    }

    structure::SExpressionPtr Session::parseSingleSExpression(std::vector<std::string>& tokens, int& depth)
    {
        // will consume one complete S-Expression from tokens
        if (tokens.size() == 0)
        {
            // input ended before completing the S-Expression
            // TODO: handle this
        }
        std::string token = tokens[0];
        tokens.erase(tokens.begin()); //TODO: this is an inefficient operation on vectors
        if (token == "(")
        {
            // this is a list, so we will recursively pull out all contained S-Expressions
            // S-Expression lists take the form of linked lists

            /*

            (): Nil

            (a b c): 0-> 0-> 0-> Nil
                     |   |   |
                     v   V   V
                     a   b   c

            */

            // first make our list root, this will be cut off from the returned value

            structure::SPairPtr rootPtr = std::make_shared<structure::SPair>();
            structure::SPairPtr lastPtr = rootPtr;

            while (tokens[0] != ")")
            {
                // this loop runs through once for each contained S-Expression

                // add a new list unit
                structure::SPairPtr newUnitPtr = std::make_shared<structure::SPair>();
                lastPtr->next = newUnitPtr;

                // move the pointer along
                lastPtr = newUnitPtr;

                // fill our new list unit and remove this contained S-Expression
                lastPtr->data = parseSingleSExpression(tokens, depth);
            }
            //finally make the end of the list point to NIL
            lastPtr->next = nilAtomPtr;

            // now we remove our matching )
            tokens.erase(tokens.begin());

            // this algorithm prepends a superfluous pair, so we remove it now
            return rootPtr->next;
        }
        else if (token == "'")
        {
            // is quote shorthand
            auto arg = std::make_shared<structure::SPair>(parseSingleSExpression(tokens, depth), nilAtomPtr);
            return std::make_shared<structure::SPair>(quoteAtomPtr, arg);
        }
        else if (token == ")")
        {
            // unexpected )
            // TODO: handle this
        }
        else
        {
            return makeAtom(token);
        }
        std::cout << "\nParsing Problem - reached end of non-void parse function\n";
        return errorAtomPtr;
    }

    structure::SExpressionPtr Session::read(std::istream& inStream)
    {
        structure::SExpressionString inString;
        inStream >> inString;
        std::vector<std::string> tokens = tokenise(inString.content);
        int depth = 0;
        structure::SExpressionPtr SExpressionPtr = parseSingleSExpression(tokens, depth);
        return SExpressionPtr;
    }

    structure::SExpressionPtr Session::atomLisp(structure::SExpressionPtr argList)
    {
        if (car0(argList)->isAtom())
        {
            return trueAtomPtr;
        }
        else
        {
            return nilAtomPtr;
        }
    }

    structure::SExpressionPtr Session::eqLisp(structure::SExpressionPtr argList)
    {
        if (!(car0(argList)->isAtom())
            || !(car0(cdr0(argList))->isAtom()))
        {
            std::cout << "error in eq, given ";
            LOG(argList);
            return errorAtomPtr;
        }
        std::cout << "eqing ";
        LOG(argList);
        if (car0(argList) == car0(cdr0(argList)))
        {
            return trueAtomPtr;
        }
        else
        {
            std::cout << "returning ";
            LOG(nilAtomPtr);
            return nilAtomPtr;
        }
    }

    structure::SExpressionPtr Session::carLisp(structure::SExpressionPtr argList)
    {
        return car0(car0(argList));
    }

    structure::SExpressionPtr Session::cdrLisp(structure::SExpressionPtr argList)
    {
        return cdr0(car0(argList));
    }

    structure::SExpressionPtr Session::car0(structure::SExpressionPtr arg)
    {
        structure::SPairPtr maybePair = std::dynamic_pointer_cast<structure::SPair>(arg);
        if (maybePair == nullptr)
        {
            return errorAtomPtr;
        }
        else
        {
            return maybePair->data;
        }
    }

    structure::SExpressionPtr Session::cdr0(structure::SExpressionPtr arg)
    {
        structure::SPairPtr maybePair = std::dynamic_pointer_cast<structure::SPair>(arg);
        if (maybePair == nullptr)
        {
            return errorAtomPtr;
        }
        else
        {
            return maybePair->next;
        }
    }

    structure::SExpressionPtr Session::consLisp(structure::SExpressionPtr argList)
    {
        std::cout << "consing ";
        LOG(argList);
        auto result = std::make_shared<structure::SPair>(car0(argList), car0(cdr0(argList)));
        std::cout << "consed ";
        LOG(result);
        return result;
    }

    structure::SExpressionPtr Session::defLisp(structure::SExpressionPtr argList)
    {
        // (a b)
        auto bindPair = std::make_shared<structure::SPair>(car0(cdr0(argList)), nilAtomPtr); // (b)
        auto bindList = std::make_shared<structure::SPair>(car0(argList), bindPair); // (a b)
        auto newGlobalEnvironmentPtr = std::make_shared<structure::SPair>(bindList, globalEnvironmentPtr);
        globalEnvironmentPtr = newGlobalEnvironmentPtr;
        return car0(argList);
    }

    structure::SExpressionPtr Session::assoc0(structure::SExpressionPtr key, structure::SExpressionPtr environment)
    {
        auto i = environment;
        while (car0(car0(i)) != key && i != nilAtomPtr)
        {
            i = cdr0(i);
        }
        if (i == nilAtomPtr)
        {
            return errorAtomPtr;
        }
        else
        {
            return car0(cdr0(car0(i)));
        }
    }

    structure::SExpressionPtr Session::evalList(structure::SExpressionPtr expression, structure::SExpressionPtr environment)
    {
        if (expression == nilAtomPtr)
        {
            return nilAtomPtr;
        }
        else
        {
            return std::make_shared<structure::SPair>(eval0(car0(expression), environment), evalList(cdr0(expression), environment));
        }
    }

    structure::SExpressionPtr Session::eval0(structure::SExpressionPtr expression, structure::SExpressionPtr environment)
    {
        std::cout << "evaluating ";
        printS(expression, std::cout);
        std::cout << "\nwith environment ";
        printS(environment, std::cout);
        std::cout << std::endl;
        if (expression->isAtom())
        {
            // look up the atom in the environment
            return assoc0(expression, environment);
        }

        auto carExpression = car0(expression);
        if (carExpression->isAtom())
        {
            if (carExpression == quoteAtomPtr)
            {
                return car0(cdr0(expression));
            }
            else if (carExpression == lambdaAtomPtr)
            {
                // a lambda-expression should be evaluated with arguments, if it
                // is evaluated without arguments make a closure out of it
                return expression;
            }
            else if (carExpression == condAtomPtr)
            {
                /*OUT(cdr0(expression));*/
                auto loc = cdr0(expression);
                while (loc != nilAtomPtr)
                {
                    if (eval0(car0(car0(loc)), environment) != nilAtomPtr)
                    {
                        return eval0(car0(cdr0(car0(loc))), environment);
                    }
                    loc = cdr0(loc);
                }
                return nilAtomPtr;
            }
            else if (carExpression == defineAtomPtr)
            {
                /*
                we want to append the definition to the global environment
                (define a b)
                the question is do we evaluate a / b?
                what does label do? it does no pre-evaluation - .
                evaluate both: it is not hard to just quote the b, and evaluate a because why not
                */

                auto evaluatedArgs = evalList(cdr0(expression), environment);
                auto newPair = std::make_shared<structure::SPair>(evaluatedArgs, globalEnvironmentPtr->next);
                globalEnvironmentPtr->next = newPair;
                return car0(cdr0(evaluatedArgs));
            }
            else if (carExpression == errorAtomPtr)
            {
                return errorAtomPtr;
            }
            else
            {
                auto evaluatedArgs = evalList(cdr0(expression), environment);
                structure::PrimaryFunctionPtr maybePrimaryFunction = std::dynamic_pointer_cast<structure::PrimaryFunction>(carExpression);
                if (maybePrimaryFunction)
                {
                    return (this->*(maybePrimaryFunction->functionPtr))(evaluatedArgs);
                }
                else
                {
                    auto lambda = assoc0(car0(expression), environment);
                    return eval0(std::make_shared<structure::SPair>(lambda, evaluatedArgs), environment);
                }
            }
        }
        else
        {
            auto caarExpression = car0(carExpression);
            /*
            ((label name function) arg1 arg2 arg3) becomes (function arg1 arg2 arg3) evaluated with (name (label name function)) added to the environment
            */
            if (caarExpression == labelAtomPtr)
            {
                auto functionSexp = car0(cdr0(cdr0(carExpression))); // function
                auto bindList = std::make_shared<structure::SPair>(carExpression, nilAtomPtr); // ((label name function))
                auto newBinding = std::make_shared<structure::SPair>(car0(cdr0(carExpression)), bindList); // (name (label name function))
                auto newEnvironment = std::make_shared<structure::SPair>(newBinding, environment);
                auto newArgList = cdr0(expression);
                return eval0(std::make_shared<structure::SPair>(functionSexp, newArgList), newEnvironment);
            }
            /*
            ((lambda (p1 p2 p3) sexp) a1 a2 a3) becomes sexp with (p1 a1), (p2 a2), (p3 a3) added to the environment
            */
            if (caarExpression == lambdaAtomPtr)
            {
                auto parameters = car0(cdr0(carExpression)); // (p1 p2 p3)
                auto arguments = cdr0(expression); // (a1 a2 a3)
                auto newEnvironment = environment;
                while (parameters != nilAtomPtr && arguments != nilAtomPtr)
                {
                    auto evaluatedArg = eval0(car0(arguments), environment);
                    auto bindValue = std::make_shared<structure::SPair>(evaluatedArg, nilAtomPtr); // (a)
                    auto bindPair = std::make_shared<structure::SPair>(car0(parameters), bindValue); // (p a)
                    newEnvironment = std::make_shared<structure::SPair>(bindPair, newEnvironment);
                    arguments = cdr0(arguments);
                    parameters = cdr0(parameters);
                }
                if (parameters == nilAtomPtr && arguments == nilAtomPtr)
                {
                    return eval0(car0(cdr0(cdr0(carExpression))), newEnvironment);
                }
                else
                {
                    std::cout << "wrong number of args\n";
                    // wrong number of arguments
                    return errorAtomPtr;
                }
            }
            else
            {
                return eval0(std::make_shared<structure::SPair>(eval0(carExpression, environment), cdr0(expression)), environment);
            }
        }
        std::cout << "reached end of eval0\n";
        return errorAtomPtr;
    }

    structure::SExpressionPtr Session::appq0(structure::SExpressionPtr argList)
    {
        if (argList == nilAtomPtr)
        {
            return nilAtomPtr;
        }
        else
        {
            structure::SExpressionPtr sPair = std::make_shared<structure::SPair>(car0(argList), nilAtomPtr);
            structure::SExpressionPtr quotedPair = std::make_shared<structure::SPair>(quoteAtomPtr, sPair);
            auto result = std::make_shared<structure::SPair>(quotedPair, appq0(cdr0(argList)));
            return result;
        }
    }

    structure::SExpressionPtr Session::apply0(structure::SExpressionPtr argList)
    {
        structure::SExpressionPtr quotedArgList = /*appq0(cdr0(argList))*/cdr0(argList);
        structure::SExpressionPtr functionArgList = std::make_shared<structure::SPair>(car0(argList), quotedArgList);
        return eval0(functionArgList, globalEnvironmentPtr);
    }
}
