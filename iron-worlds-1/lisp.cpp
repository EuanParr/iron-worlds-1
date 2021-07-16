#include "lisp.h"

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
        temp.flag = LispHandle::BasicSymbolT;
        temp.basicSymbol = atomNil;
        exStack.bind(atomNil, temp);
    }

    VirtualMachine::~VirtualMachine()
    {

    }

    void VirtualMachine::print(LispHandle expr, std::ostream& printStream)
    {
        switch (expr.flag)
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
                while (currentHandle.flag == LispHandle::ListT)
                {
                    currentListPtr = currentHandle.listNode;
                    carHandle = currentListPtr->first;
                    printStream << ' ';
                    print(carHandle, printStream);

                    cdrHandle = currentListPtr->second;
                    currentHandle = cdrHandle;
                }

                if (currentHandle.flag != currentHandle.BasicSymbolT || currentHandle.basicSymbol != atomNil)
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
        }
    }

    LispHandle VirtualMachine::read(std::istream& readStream)
    {
        SymbolChar thisChar;
        while (readStream.get(thisChar))
        {
            if (isWhiteSpace(thisChar))
            {
                continue;
            }
            else if (thisChar == '(')
            {
                return readList(readStream);
            }
            else
            {
                return readSymbol(thisChar, readStream);
            }
        }
    }

    LispHandle VirtualMachine::readList(std::istream& readStream)
    {
        SymbolChar thisChar;
        while (readStream.get(thisChar))
        {

        }
    }

    LispHandle VirtualMachine::readSymbol(SymbolChar startChar, std::istream& readStream)
    {
        SymbolString fullName;
        SymbolChar thisChar = startChar;
        while (!isWhiteSpace(thisChar) && thisChar != ')')
        {
            fullName += thisChar;
            if (!readStream.get(thisChar))
            {
                ELOG("unexpected EOF while parsing a symbol");
            }
        }
        return stringToSymbol(fullName);
    }

    LispHandle VirtualMachine::stringToSymbol(SymbolString name)
    {
        Symbol* temp = &symbolTable.emplace(name, Symbol(name)).first->second;
        LispHandle result;
        result.flag = LispHandle::BasicSymbolT;
        result.basicSymbol = temp;
        return result;
    }
}
