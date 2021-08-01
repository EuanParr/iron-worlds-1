#include "lisp.h"

#include <limits>

namespace lisp
{
    std::ostream& operator<<(std::ostream& output, const Symbol& sym)
    {
        output << sym.name;
        return output;
    }

    bool isWhiteSpace(SymbolChar c)
    {
        switch (c)
        {
        case ' ':
        case '\t':
        case '\n':
            return true;
            break;
        default:
            return false;
            break;
        }
    }

    bool isSpecialChar(SymbolChar c)
    {
        switch (c)
        {
        case '(':
        case ')':
        case '.':
        case '\'':
            return true;
            break;
        default:
            return false;
            break;
        }
    }

    ExecutionStackFrame::~ExecutionStackFrame()
    {
        // remove this frame's bindings from each symbol
        for (Symbol* boundSym : boundSymbols)
        {
            boundSym->bindingStack.pop_back();
        }
    }

    void ExecutionStackFrame::registerBinding(Symbol* key)
    {
        // register that this symbol has a binding through this stack frame
        boundSymbols.push_back(key);
    }

    ExecutionStack::~ExecutionStack()
    {
        // unwind the stack
        while (topFrame_Ptr)
        {
            pop_back();
        }
    }

    void ExecutionStack::bind(Symbol* key, LispHandle value)
    {
        // TODO: could a symbol be bound multiple times in a stack frame? if it
        // can we need to deal with it
        assert(topFrame_Ptr);
        topFrame_Ptr->registerBinding(key);
        key->bindingStack.push_back(value);
    }

    void ExecutionStack::pop_back()
    {
        assert(topFrame_Ptr);
        ExecutionStackFrame* temp = topFrame_Ptr->getNext();
        delete topFrame_Ptr;
        topFrame_Ptr = temp;
    }

    void ExecutionStack::push_back()
    {
        topFrame_Ptr = new ExecutionStackFrame(topFrame_Ptr);
    }

    VirtualMachine::VirtualMachine()
    {
        exStack.push_back();
        atomNil = stringToSymbol("nil").basicSymbol;
        LispHandle temp;
        temp.tag = LispHandle::BasicSymbolT;
        temp.basicSymbol = atomNil;
        exStack.bind(atomNil, temp);
    }

    VirtualMachine::~VirtualMachine()
    {

    }

    void VirtualMachine::print(LispHandle expr, std::ostream& printStream)
    {
        switch (expr.tag)
        {
            case LispHandle::ListT:
            {
                printStream << '(';
                LispHandle currentHandle = expr;
                ListNode* currentListPtr = currentHandle.listNode;
                LispHandle carHandle;
                LispHandle cdrHandle;
                carHandle = currentListPtr->first;
                print(carHandle, printStream);
                cdrHandle = currentListPtr->second;
                currentHandle = cdrHandle;
                while (currentHandle.tag == LispHandle::ListT)
                {
                    currentListPtr = currentHandle.listNode;
                    carHandle = currentListPtr->first;
                    printStream << ' ';
                    print(carHandle, printStream);

                    cdrHandle = currentListPtr->second;
                    currentHandle = cdrHandle;
                }

                if (currentHandle.tag != currentHandle.BasicSymbolT || currentHandle.basicSymbol != atomNil)
                {
                    printStream << " . ";
                    print(currentHandle, printStream);
                }
                printStream << ')';

                break;
            }

            case LispHandle::BasicSymbolT:
            {
                printStream << *expr.basicSymbol;

                break;
            }

            default:
            {
                assert(false);
            }
        }
    }

    LispHandle VirtualMachine::read(std::istream& readStream)
    {
        /*SymbolChar thisChar;
        bool tokenFound = false;
        while (!tokenFound)
        {
            if (!readStream.get(thisChar))
            {
                ELOG("Unexpected end of stream");
                return LispHandle{0, nullptr};
            }
            else
            {
                if isWhiteSpace(thisChar)
                {
                    // skip white space
                    continue;
                }
                else if (thisChar == ';')
                {
                    // skip comment line
                    bool streamFail = true;
                    while (!readStream.get(thisChar))
                    {
                        if (thisChar == '\n')
                        {
                            streamFail = false;
                            break;
                        }
                    }
                    if (streamFail)
                    {
                        ELOG("Unexpected end of stream");
                        return LispHandle{0, nullptr};
                    }
                }
            }
        }*/



    }

    LispHandle VirtualMachine::readList(std::istream& readStream)
    {
        // called after the '(' of a list, consumes the rest of the list
        SymbolChar currentChar;
        LispHandle result;
        LispHandle listTail;
        while (readStream.get(currentChar))
        {
            if (currentChar == ')')
            {
                listTail.tag = listTail.BasicSymbolT;
                listTail.basicSymbol = atomNil;
                return result;
            }
            else if (currentChar == '.')
            {

            }
            else
            {

            }
        }
    }

    LispHandle VirtualMachine::readSymbol(SymbolChar& currentChar, std::istream& readStream)
    {
        // called after the first char of a symbol
        SymbolString fullName;
        while (!isWhiteSpace(currentChar) && currentChar != ')')
        {
            fullName += currentChar;
            if (!readStream.get(currentChar))
            {
                ELOG("unexpected EOF while parsing a symbol");
            }
        }
        return stringToSymbol(fullName);
    }

    SymbolString VirtualMachine::readToken(std::istream& readStream)
    {
        SymbolChar currentChar;

        while (readStream)
        {
            // clear leading white space
            readStream >> std::ws;

            currentChar = readStream.peek();
            if (!readStream)
            {
                ELOG("unexpected stream failure");
                return "";
            }
            else if (currentChar == ';')
            {
                // skip comment line
                readStream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                continue;
            }
            else if (isSpecialChar(currentChar))
            {
                // each of these characters are a token no matter the spacing

                // pop current char
                readStream.ignore();
                // make string of char
                return SymbolString(1, currentChar);
            }
            else
            {
                // normal symbol tokens
                SymbolString resultString(1, currentChar);
                readStream.ignore();
                while (readStream)
                {
                    currentChar = readStream.peek();

                    if (isWhiteSpace(currentChar) || isSpecialChar(currentChar))
                    {
                        // token ended
                        return resultString;
                    }
                    else
                    {
                        // go to next char
                        resultString += currentChar;
                        readStream.ignore();
                    }
                }
            }
        }
        ELOG("unexpected stream failure");
        return "";
    }

    LispHandle VirtualMachine::stringToSymbol(SymbolString name)
    {
        Symbol* temp = &symbolTable.emplace(name, Symbol(name)).first->second;
        LispHandle result;
        result.tag = LispHandle::BasicSymbolT;
        result.basicSymbol = temp;
        return result;
    }
}
