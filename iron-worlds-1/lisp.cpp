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

    LispHandle listGet(LispHandle expr, unsigned int index)
    {
        for (unsigned int i = 0; i < index; i++)
        {
            if (expr.tag == expr.ListT)
            {
                expr = expr.listNode->second;
            }
            else
            {
                throw std::range_error("Lisp list ended prematurely");
            }
        }
        if (expr.tag == expr.ListT)
        {
            return expr.listNode->first;
        }
        else
        {
            throw std::range_error("Lisp list ended prematurely");
        }
    }

    LispHandle quote_SF(VirtualMachine&, LispHandle args)
    {
        return listGet(args, 0);
    }

    LispHandle def_SF(VirtualMachine& vm, LispHandle args)
    {
        LispHandle key = listGet(args, 0);
        if (key.tag == key.BasicSymbolT)
        {
            vm.bind(key.basicSymbol, vm.evaluate(listGet(args, 1)));
            return key.basicSymbol->bindingStack.back();
        }
        else
        {
            throw std::domain_error("attempt to bind a non-symbol");
        }
    }

    LispHandle let_SF(VirtualMachine& vm, LispHandle args)
    {

    }

    LispHandle lambda_SF(VirtualMachine& vm, LispHandle args)
    {
        Lambda* result = vm.memory.lambdas.construct();
        LispHandle parameters = listGet(args, 0);
        while (true)
        {
            if (!vm.isAtom(parameters))
            {
                LispHandle param = parameters.car();
                if(param.tag == param.BasicSymbolT)
                {
                    result->parameters.push_back(param.basicSymbol);
                }
                else
                {
                    throw std::domain_error("non-symbol in lambda parameter list");
                }
                parameters = parameters.cdr();
            }
            else
            {
                // parameter list ended
                break;
            }
        }

        if (vm.isAtom(args.cdr().cdr()))
        {
            // body is 1 list
            result->body = listGet(args, 1);
        }
        else
        {
            // body is multiple lists, insert implicit progn
            result->body = vm.memory.lists.construct(vm.builtins.progn, args.cdr());
        }

        return result;
    }

    LispHandle progn_SF(VirtualMachine& vm, LispHandle args)
    {
        LispHandle result;
        while (args.tag == args.ListT)
        {
            result = vm.evaluate(args.car());
            args = args.cdr();
        }
        return result;
    }

    LispHandle cond_SF(VirtualMachine& vm, LispHandle args)
    {
        while (true)
        {
            if (vm.isList(args))
            {
                LispHandle clause = args.car();
                if (!vm.isNil(vm.evaluate(listGet(clause, 0))))
                {
                    return vm.evaluate(listGet(clause, 1));
                }
            }
            else
            {
                // reached end of condition list, return the terminating
                // symbol, which is nil unless otherwise specified by user
                return args;
            }
            args = args.cdr();
        }
    }

    LispHandle closure_SF(VirtualMachine& vm, LispHandle args)
    {

    }

    LispHandle LispHandle::car()
    {
        if (tag == ListT)
        {
            return listNode->first;
        }
        else
        {
            throw std::domain_error("attempt to car an atom");
        }
    }

    LispHandle LispHandle::cdr()
    {
        if (tag == ListT)
        {
            return listNode->second;
        }
        else
        {
            throw std::domain_error("attempt to cdr an atom");
        }
    }

    Symbol* SymbolTable::stringToSymbol(SymbolString name)
    {
        // creates symbol if does not exist, returns pointer to it either way
        return &hashTable.emplace(name, Symbol(name)).first->second;
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

    ExecutionStack::ExecutionStack()
    {
        push_back("global");
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

    Builtins::Builtins(VirtualMachine& parentVM)
    {
        for (std::pair<Symbol*&, SymbolString> bindPair : bindings)
        {
            bindPair.first = parentVM.stringToSymbol(bindPair.second);
        }
        nil->bindingStack.push_back(nil);
    }

    VirtualMachine::VirtualMachine() : builtins(*this)
    {
        exStack.bind(builtins.quote, LispHandle(quote_SF, 0));
        exStack.bind(builtins.def, LispHandle(def_SF, 0));
        exStack.bind(builtins.let, LispHandle(let_SF, 0));
        exStack.bind(builtins.lambda, LispHandle(lambda_SF, 0));
        exStack.bind(builtins.progn, LispHandle(progn_SF, 0));
        exStack.bind(builtins.cond, LispHandle(cond_SF, 0));
        exStack.bind(builtins.closure, LispHandle(closure_SF, 0));
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

                if (currentHandle.tag != currentHandle.BasicSymbolT || currentHandle.basicSymbol != builtins.nil)
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
                //printStream << expr.basicSymbol;
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
                break;
            case ')':
                ELOG("Unexpected ')'");
                assert(false);
                break;
            case '.':
                assert(false);
                break;
            case '\'':
                return memory.lists.construct(builtins.quote, memory.lists.construct(read(readStream), builtins.nil));
                break;
            default:
                assert(false);
            }
        }
        else
        {
            return memory.symbols.stringToSymbol(thisToken);
        }

    }

    void VirtualMachine::readFile(std::string path)
    {
        std::ifstream fileStream(path);
        while ((fileStream >> std::ws).peek()!=EOF)
        {
            evaluate(read(fileStream));
        }
    }

    LispHandle VirtualMachine::evaluate(LispHandle expr)
    {
        switch (expr.tag)
        {
        case (expr.NullT):
            ELOG("Null-type Lisp handle");
            assert(false);
            break;

        case (expr.ListT):
            {
                LispHandle first = evaluate(expr.listNode->first);

                switch(first.tag)
                {
                case (first.NullT):
                    ELOG("expected function, got null");
                    assert(false);
                    break;

                case (first.ListT):
                    ELOG("expected function, got list");
                    assert(false);
                    break;

                case (first.BasicSymbolT):
                    ELOG("expected function, got symbol");
                    assert(false);
                    break;

                case (first.SpecialFormT):
                    {
                        return first.specialForm(*this, expr.listNode->second);
                    }
                    break;

                case (first.NativeFunctionT):
                    {
                        LispHandle args = expr.listNode->second;
                        LispHandle evaluatedArgs;
                        LispHandle* evListTail_Ptr = &evaluatedArgs;

                        while (!isAtom(args))
                        {
                            // read in list member, passing in first token
                            LispHandle thisItem = evaluate(args.car());
                            // construct a cell
                            ListNode* newCons = memory.lists.construct();
                            // point the cell's cdr to the new member
                            newCons->first = thisItem;
                            // point the list tail's next handle to the new cell
                            *evListTail_Ptr = LispHandle(newCons);
                            // update the pointer to the tail
                            evListTail_Ptr = &(newCons->second);

                            args = args.cdr();
                        }

                        // list ended, point last cdr to nil
                        *evListTail_Ptr = LispHandle(builtins.nil);

                        return first.nativeFunction(*this, evaluatedArgs);
                    }
                    break;

                case (first.LambdaT):
                    {
                        LispHandle args = expr;
                        for (Symbol* parameter : first.lambda->parameters)
                        {
                            args = args.cdr();
                            bind(parameter, evaluate(args.car()));
                        }

                        return evaluate(first.lambda->body);
                    }
                    break;

                default:
                    ELOG("expected function, got unaccounted for handle type");
                    assert(false);
                    break;
                }
            }

            break;

        case (expr.BasicSymbolT):
            if (expr.basicSymbol->bindingStack.empty())
            {
                ELOG("Unbound symbol (" << expr.basicSymbol->name << ")");
                assert(false);
            }
            else
            {
                return expr.basicSymbol->bindingStack.back();
            }
            break;

        default:
            ELOG("unaccounted for handle type");
            assert(false);
            break;
        }
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
                ListNode* newCons = memory.lists.construct();
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
        *listTail_Ptr = LispHandle(builtins.nil);

        return resultHandle.listNode;
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
                return "<Invalid-Token>";
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
        return "<Invalid-Token>";
    }

    Symbol* VirtualMachine::stringToSymbol(SymbolString name)
    {
        // creates symbol if does not exist, returns pointer to it either way
        return memory.symbols.stringToSymbol(name);
    }

    bool VirtualMachine::isAtom(LispHandle expr)
    {
        return (expr.tag != expr.ListT);
    }

    bool VirtualMachine::isList(LispHandle expr)
    {
        return (expr.tag == expr.ListT) || (isNil(expr));
    }

    bool VirtualMachine::isNil(LispHandle expr)
    {
        return (expr.tag == expr.BasicSymbolT) && (expr.basicSymbol == builtins.nil);
    }
}
