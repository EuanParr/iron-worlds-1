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
            ListT = 0,
            BasicSymbolT = 1,
        } flag;

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
        LispHandle readSymbol(SymbolChar startChar, std::istream& readStream);
        LispHandle stringToSymbol(SymbolString name);
    };
}

#endif // LISP_H_INCLUDED
