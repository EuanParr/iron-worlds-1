#ifndef LISP_H_INCLUDED
#define LISP_H_INCLUDED

#include "platform.h"

#include <iostream>
#include <list>
#include <string>
#include <vector>

namespace lisp
{
    typedef char SymbolChar;
    typedef std::string SymbolString;

    struct ListNode;
    struct BasicSymbol;

    union LispTypePtr_Uni
    {
        // a pointer to one of the Lisp entity types
        ListNode* listNode;
        BasicSymbol* basicSymbol;
    };

    enum class LispTypeFlag : char
    {
        // specifies a Lisp entity type
        ListT = 0,
        BasicSymbolT = 1,
    };

    struct ListNode
    {
        // Points to 2 Lisp entities and remembers their types
        LispTypeFlag firstFlags;
        LispTypeFlag secondFlags;

        LispTypePtr_Uni firstValue;
        LispTypePtr_Uni secondValue;
    };

    struct LispHandle
    {
        // Points to a Lisp entity and remembers its type
        LispTypeFlag flags;
        LispTypePtr_Uni value;
    };

    struct BasicSymbol
    {
        SymbolString name;
        LispTypeFlag valueFlags;
        LispTypePtr_Uni value;
    };

    class LispListMemory
    {
    public:
        ListNode* nodesArray;
        unsigned int arraySize;
        unsigned int defaultSize = 0x10000;

        LispListMemory()
        {
            arraySize = defaultSize;
            nodesArray = new ListNode[arraySize];
        }

        ~LispListMemory()
        {
            delete[] nodesArray;
        }
    };

    typedef ListNode* HandleListNode;

    class LispVirtualMachine
    {
    public:
        LispListMemory listMemory;

        LispHandle read(std::istream& readStream);
        LispHandle readList(std::istream& readStream);
        LispHandle readSymbol(SymbolChar startChar, std::istream& readStream);
        LispHandle stringToSymbol(SymbolString name);
    };
}

#endif // LISP_H_INCLUDED
