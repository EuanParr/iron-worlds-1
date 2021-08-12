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

        union
        {
            // a pointer to one of the Lisp entity types
            ListNode* listNode;
            Symbol* basicSymbol;
        };

        enum : char
        {
            // specifies a Lisp entity type
            NullT = 0,
            ListT = 1,
            BasicSymbolT = 2,
        } tag;

        LispHandle() : listNode(nullptr), tag(NullT) {}
        LispHandle(ListNode* node) : listNode(node), tag(ListT) {}
        LispHandle(Symbol* symbol) : basicSymbol(symbol), tag(BasicSymbolT) {}
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

        ListNode* construct()
        {
            if (nextFree < arraySize)
            {
                ListNode* result_Ptr = nodesArray_Ptr + nextFree;
                nextFree++;
                return result_Ptr;
            }
            else
            {
                ELOG("out of memory");
                return nullptr;
            }
        }

        ListNode* construct(LispHandle first, LispHandle second)
        {
            ListNode* result_Ptr = construct();
            result_Ptr->first = first;
            result_Ptr->second = second;
            return result_Ptr;
        }
    };

    typedef ListNode* HandleListNode;

    class ExecutionStackFrame
    {
        ExecutionStackFrame* nextFrame_PtrWeak = nullptr;
        std::vector<Symbol*> boundSymbols;
        std::string contextString_;
    public:
        ExecutionStackFrame(ExecutionStackFrame* next, std::string contextString)
            : nextFrame_PtrWeak(next),
            contextString_(contextString) {}
        ~ExecutionStackFrame();

        ExecutionStackFrame* getNext() {return nextFrame_PtrWeak;}
        void registerBinding(Symbol* key);
    };

    class ExecutionStack
    {
        ExecutionStackFrame* topFrame_Ptr = nullptr;

    public:
        ExecutionStack();
        ~ExecutionStack();

        void bind(Symbol* key, LispHandle value);
        void pop_back();
        void push_back(std::string contextString);
    };

    class VirtualMachine;

    class Builtins
    {
    public:
        // TODO: references would be better, but that would complicate
        // construction. For now handle this in the VirtualMachine constructor.
        // Also pointers can be easily used where handles are expected.
        Symbol* nil;
        Symbol* quote;
        Symbol* def;
        Symbol* let;
        Symbol* lambda;

        std::vector<std::pair<Symbol*, SymbolString>> bindings =
        {
            {nil, "nil"},
            {quote, "quote"},
            {def, "def"},
            {let, "let"},
            {lambda, "lambda"},
        };

        Builtins(VirtualMachine& parentVM);
    };

    class VirtualMachine
    {
        ExecutionStack exStack;
        LispListMemory listMemory;
        std::unordered_map<SymbolString, Symbol> symbolTable;
        Builtins builtins;

        public:
        VirtualMachine() : builtins(*this) {};
        /*~VirtualMachine();

        // forbid copying
        // TODO: is there a better way to rule-of-3 this class?
        VirtualMachine(const VirtualMachine&) = delete;
        VirtualMachine& operator=(const VirtualMachine&) = delete;*/

        void print(LispHandle expr, std::ostream& printStream);
        void printLn(LispHandle expr, std::ostream& printStream)
        {
            print(expr, printStream);
            printStream << std::endl;
        }
        LispHandle read(std::istream& readStream);
        LispHandle read(std::istream& readStream, SymbolString thisToken);
        LispHandle evaluate(LispHandle expr);
        ListNode* readList(std::istream& readStream);
        Symbol* readSymbol(SymbolChar& currentChar, std::istream& readStream);
        SymbolString readToken(std::istream& readStream);
        Symbol* stringToSymbol(SymbolString name);
    };
}

#endif // LISP_H_INCLUDED
