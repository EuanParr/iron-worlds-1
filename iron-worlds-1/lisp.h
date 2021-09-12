#ifndef LISP_H_INCLUDED
#define LISP_H_INCLUDED

#include "platform.h"

#include <functional>
#include <iostream>
#include <list>
#include <stdexcept>
#include <string>
#include <vector>

namespace lisp
{
    typedef char SymbolChar;
    typedef std::string SymbolString;

    struct ListNode;
    struct Symbol;
    struct LispHandle;
    class VirtualMachine;
    class Lambda;
    class Closure;
    template <class T>
    class SpecialisedMemory;
    class SymbolTable;

    typedef LispHandle (*NativeFunctionPtr) (VirtualMachine&, LispHandle);

    LispHandle quote_SF(VirtualMachine& vm, LispHandle args);
    LispHandle def_SF(VirtualMachine& vm, LispHandle args);
    LispHandle let_SF(VirtualMachine& vm, LispHandle args);
    LispHandle lambda_SF(VirtualMachine& vm, LispHandle args);
    LispHandle progn_SF(VirtualMachine& vm, LispHandle args);
    LispHandle cond_SF(VirtualMachine& vm, LispHandle args);
    LispHandle closure_SF(VirtualMachine& vm, LispHandle args);

    struct LispHandle
    {
        // tagged union

        union
        {
            // a pointer to one of the Lisp entity types
            ListNode* listNode;
            Symbol* basicSymbol;
            NativeFunctionPtr nativeFunction;
            NativeFunctionPtr specialForm;
            Lambda* lambda;
            Closure* closure;
        };

        enum : char
        {
            // specifies a Lisp entity type
            NullT = 0,
            ListT = 1,
            BasicSymbolT = 2,
            NativeFunctionT = 3,
            SpecialFormT = 4,
            LambdaT = 5,
            ClosureT = 6
        } tag;

        LispHandle() : listNode(nullptr), tag(NullT) {}
        LispHandle(ListNode* node) : listNode(node), tag(ListT) {}
        LispHandle(Symbol* symbol) : basicSymbol(symbol), tag(BasicSymbolT) {}
        LispHandle(NativeFunctionPtr func) : nativeFunction(func), tag(NativeFunctionT) {}
        LispHandle(NativeFunctionPtr form, int) : specialForm(form), tag(SpecialFormT) {}
        LispHandle(Lambda* lam) : lambda(lam), tag(LambdaT) {}
        LispHandle(Closure* clo) : closure(clo), tag(ClosureT) {}

        LispHandle car();
        LispHandle cdr();
    };

    struct ListNode
    {
        // Points to 2 Lisp entities and remembers their types
        LispHandle first, second;

    private:
        ListNode() {}
    public:
        friend class SpecialisedMemory<ListNode>;
    };

    struct Symbol
    {
        // holds its name and a typed union pointer to its lookup value
        SymbolString name;
        std::vector<LispHandle> bindingStack;

    private:
        Symbol(SymbolString newName) : name(newName) {}

    public:
        friend class SymbolTable;
    };
    std::ostream& operator<<(std::ostream& output, const Symbol& sym);

    struct Lambda
    {
        std::vector<Symbol*> parameters;
        LispHandle body;
    protected:
        Lambda() {}
    public:
        friend class SpecialisedMemory<Lambda>;
    };

    struct Closure : public Lambda
    {
        std::vector<std::pair<Symbol, LispHandle>> environment;
    };

    class BigInt
    {
        std::vector<long long int> nums;
    };

    template <class T>
    class SpecialisedMemory
    {
        // this pointer is a
        T* nodesArray_Ptr;
        size_t arraySize = 0;
        size_t defaultSize = 0x10000;
        size_t nextFree = 0;

    public:
        SpecialisedMemory()
        {
            arraySize = defaultSize;
            nodesArray_Ptr = new T[arraySize];
        }

        ~SpecialisedMemory()
        {
            delete[] nodesArray_Ptr;
        }

        T* construct()
        {
            if (nextFree < arraySize)
            {
                T* result_Ptr = nodesArray_Ptr + nextFree;
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

    template <>
    class SpecialisedMemory <ListNode>
    {
        // this pointer is a
        ListNode* nodesArray_Ptr;
        size_t arraySize = 0;
        size_t defaultSize = 0x10000;
        size_t nextFree = 0;

    public:
        SpecialisedMemory()
        {
            arraySize = defaultSize;
            nodesArray_Ptr = new ListNode[arraySize];
        }

        ~SpecialisedMemory()
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

    class SymbolTable
    {
        public:
         std::unordered_map<SymbolString, Symbol> hashTable;

         Symbol* stringToSymbol(SymbolString name);
    };

    class Memory
    {
    public:
        SpecialisedMemory<ListNode> lists;
        SpecialisedMemory<Lambda> lambdas;
        SymbolTable symbols;
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

    class NativeCalledLisp
    {
        // hook from native code to lisp function
    };

    class LispCalledNative
    {
        //hook from lisp code to native function
        std::function<int(int)> fun;
    };

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
        Symbol* progn;
        Symbol* cond;
        Symbol* closure;

        std::vector<std::pair<Symbol*&, SymbolString>> bindings =
        {
            {nil, "nil"},
            {quote, "quote"},
            {def, "def"},
            {let, "let"},
            {lambda, "lambda"},
            {progn, "progn"},
            {cond, "cond"},
            {closure, "closure"},
        };

        Builtins(VirtualMachine& parentVM);
    };

    class VirtualMachine
    {
        ExecutionStack exStack;
        Memory memory;
        Builtins builtins;

        public:
        VirtualMachine();
        /*~VirtualMachine();

        // forbid copying
        // TODO: is there a better way to rule-of-3 this class?
        VirtualMachine(const VirtualMachine&) = delete;
        VirtualMachine& operator=(const VirtualMachine&) = delete;*/

        void bind(Symbol* key, LispHandle value) {exStack.bind(key, value);}
        void print(LispHandle expr, std::ostream& printStream);
        void printLn(LispHandle expr, std::ostream& printStream)
        {
            print(expr, printStream);
            printStream << std::endl;
        }
        LispHandle read(std::istream& readStream);
        LispHandle read(std::istream& readStream, SymbolString thisToken);
        LispHandle evaluate(LispHandle expr);
        void readFile(std::string path);
        ListNode* readList(std::istream& readStream);
        SymbolString readToken(std::istream& readStream);
        Symbol* stringToSymbol(SymbolString name);
        bool isAtom(LispHandle expr); // nil is considered an atom
        bool isList(LispHandle expr); // nil is considered a list
        bool isNil(LispHandle expr);

        friend LispHandle quote_SF(VirtualMachine& vm, LispHandle args);
        friend LispHandle def_SF(VirtualMachine& vm, LispHandle args);
        friend LispHandle let_SF(VirtualMachine& vm, LispHandle args);
        friend LispHandle lambda_SF(VirtualMachine& vm, LispHandle args);
        friend LispHandle progn_SF(VirtualMachine& vm, LispHandle args);
        friend LispHandle cond_SF(VirtualMachine& vm, LispHandle args);
        friend LispHandle closure_SF(VirtualMachine& vm, LispHandle args);
    };
}

#endif // LISP_H_INCLUDED
