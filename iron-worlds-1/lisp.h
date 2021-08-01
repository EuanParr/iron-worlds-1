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
    struct Symbol;

    struct LispHandle
    {
        // tagged union

        enum : char
        {
            // specifies a Lisp entity type
            NullT = 0,
            ListT = 1,
            BasicSymbolT = 2,
        } tag;

        union
        {
            // a pointer to one of the Lisp entity types
            ListNode* listNode;
            Symbol* basicSymbol;
        };

    };

    struct ListNode
    {
        // Points to 2 Lisp entities and remembers their types
        LispHandle first, second;
    };



    struct Symbol
    {
        // holds its name and a typed union pointer to its lookup value
        SymbolString name;
        std::vector<LispHandle> bindingStack;

        Symbol(SymbolString newName) : name(newName) {}
    };

    std::ostream& operator<<(std::ostream& output, const Symbol& sym);

    class LispListMemory
    {
        // this pointer is a
        ListNode* nodesArray_Ptr;
        size_t arraySize = 0;
        size_t defaultSize = 0x10000;
        size_t nextFree = 0;

    public:
        LispListMemory()
        {
            arraySize = defaultSize;
            nodesArray_Ptr = new ListNode[arraySize];
        }

        ~LispListMemory()
        {
            delete[] nodesArray_Ptr;
        }

        ListNode* construct(LispHandle first, LispHandle second)
        {
            if (nextFree < arraySize)
            {
                ListNode* result_Ptr = nodesArray_Ptr + nextFree;
                result_Ptr->first = first;
                result_Ptr->second = second;
                nextFree++;
                return result_Ptr;
            }
            else
            {
                ELOG("out of memory");
                return nullptr;
            }
        }
    };

    typedef ListNode* HandleListNode;

    class ExecutionStackFrame
    {
        ExecutionStackFrame* nextFrame_PtrWeak = nullptr;
        std::vector<Symbol*> boundSymbols;
    public:
        ExecutionStackFrame(ExecutionStackFrame* next) : nextFrame_PtrWeak(next) {}
        ~ExecutionStackFrame();

        ExecutionStackFrame* getNext() {return nextFrame_PtrWeak;}
        void registerBinding(Symbol* key);
    };

    class ExecutionStack
    {
        ExecutionStackFrame* topFrame_Ptr = nullptr;

    public:
        ~ExecutionStack();

        void bind(Symbol* key, LispHandle value);
        void pop_back();
        void push_back();
    };

    class VirtualMachine
    {
        ExecutionStack exStack;
        LispListMemory listMemory;
        std::unordered_map<SymbolString, Symbol> symbolTable;
        Symbol* atomNil;

        public:
        VirtualMachine();
        ~VirtualMachine();

        void print(LispHandle expr, std::ostream& printStream);
        LispHandle read(std::istream& readStream);
        LispHandle readList(std::istream& readStream);
        LispHandle readSymbol(SymbolChar& currentChar, std::istream& readStream);
        SymbolString readToken(std::istream& readStream);
        LispHandle stringToSymbol(SymbolString name);
    };
}

#endif // LISP_H_INCLUDED
