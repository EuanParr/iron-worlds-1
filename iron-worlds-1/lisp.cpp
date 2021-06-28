#include "lisp.h"

namespace lisp
{
    bool isWhiteSpace(SymbolChar c)
    {
        switch (c)
        {
        case ' ':
        case '\t':
        case '\n':
            return true;
        default:
            return false;
        }
    }

    void copyLispHandle(LispTypeFlag& fromType, LispTypePtr_Uni& fromValue,
                        LispTypeFlag& toType, LispTypePtr_Uni& toValue)
    {
        toType = fromType;
        switch (fromType)
        {
        case LispTypeFlag::ListT:
            toValue.listNode = fromValue.listNode;
            break;
        case LispTypeFlag::BasicSymbolT:
            toValue.basicSymbol = fromValue.basicSymbol;
            break;
        default:
            ELOG("Unhandled type");
        }
    }

    LispHandle LispVirtualMachine::read(std::istream& readStream)
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

    LispHandle LispVirtualMachine::readList(std::istream& readStream)
    {
        SymbolChar thisChar;
        while (readStream.get(thisChar))
        {

        }
    }

    LispHandle LispVirtualMachine::readSymbol(SymbolChar startChar, std::istream& readStream)
    {

    }

    LispHandle LispVirtualMachine::stringToSymbol(SymbolString name)
    {

    }
}
