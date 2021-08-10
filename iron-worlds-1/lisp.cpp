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

    void ExecutionStack::push_back(std::string contextString)
    {
        topFrame_Ptr = new ExecutionStackFrame(topFrame_Ptr, contextString);
    }

    VirtualMachine::VirtualMachine()
    {
        exStack.push_back("global");
        atomNil = stringToSymbol("nil");
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
                if (expr.basicSymbol)
                {
                    printStream << *expr.basicSymbol;
                }
                else
                {
                    printStream << "<nullptr>";
                }

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
        return read(readStream, readToken(readStream));
    }

    LispHandle VirtualMachine::read(std::istream& readStream, SymbolString thisToken)
    {


        if (isSpecialChar(thisToken[0]))
        {
            switch (thisToken[0])
            {
            case '(':
                return readList(readStream);
            case ')':
                ELOG("Unexpected ')'");
                assert(false);
            case '.':

            case '\'':

            default:
                assert(false);
            }
        }
        else
        {
            return stringToSymbol(thisToken);
        }

    }

    LispHandle VirtualMachine::evaluate(LispHandle expr)
    {

    }

    ListNode* VirtualMachine::readList(std::istream& readStream)
    {
        // called after the '(' of a list, consumes the rest of the list

        // points to the root cell of the list,
        // exists to avoid special case of list start,
        // discarded at function end
        LispHandle resultHandle;
        // a pointer to the last cell
        LispHandle* listTail_Ptr = &resultHandle;

        SymbolString currentToken = readToken(readStream);
        SymbolChar& currentChar = currentToken[0];

        while (currentChar != ')')
        {
            switch (currentChar)
            {
            case '.':
                assert(false); // TODO
                break;
            default:
                // read in list member, passing in first token
                LispHandle thisItem = read(readStream, currentToken);
                // construct a cell
                ListNode* newCons = listMemory.construct();
                // point the cell's cdr to the new member
                newCons->first = thisItem;
                // point the list tail's next handle to the new cell
                *listTail_Ptr = LispHandle(newCons);
                // update the pointer to the tail
                listTail_Ptr = &(newCons->second);
            }
            currentToken = readToken(readStream);
        }

        // list ended, point last cdr to nil
        *listTail_Ptr = LispHandle(atomNil);

        return resultHandle.listNode;
    }

    Symbol* VirtualMachine::readSymbol(SymbolChar& currentChar, std::istream& readStream)
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
                        // do not consume the char after the symbol
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

    Symbol* VirtualMachine::stringToSymbol(SymbolString name)
    {
        Symbol* temp = &symbolTable.emplace(name, Symbol(name)).first->second;
        /*LispHandle result;
        result.tag = LispHandle::BasicSymbolT;
        result.basicSymbol = temp;*/
        return temp;
    }
}
