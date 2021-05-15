#ifndef LISP_H_INCLUDED
#define LISP_H_INCLUDED

#include "bottom_portability_bookend.h"

#include <string>
#include <vector>

namespace lisp
{
    class SpecialSymbol
    {
    public:
        std::string name;

        // needs a virtual destructor because SymbolTableEntryValue deletes it
        // polymorphically
        virtual ~SpecialSymbol() {};
    };

    /*
        SymbolTableEntry is a discriminated union: it stores a union and a
        variable recording the type currently active in the union.
        This allows space-efficient storage of different types without dealing
        with class polymorphism overhead.

        The cost is that we must manually ensure type safety.

        An alternative would be to use a void* pointer, but that is not smaller
        than the union and the intent is less clear. Also it would require ugly
        casting.
    */

    enum class SymbolTableEntryType
    {
        Empty, Basic, Special
    };

    union SymbolTableEntryValue
    {
        std::string* name_Ptr;
        SpecialSymbol* specialSymbol_Ptr;
    };

    class SymbolTableEntry
    {
    public:
        SymbolTableEntryType typeOfValue;
        SymbolTableEntryValue value;

        SymbolTableEntry()
        {
            typeOfValue = SymbolTableEntryType::Empty;
        }

        ~SymbolTableEntry()
        {
            // determine type of stored value and destroy appropriately
            switch (typeOfValue)
            {
            case (SymbolTableEntryType::Basic):
                delete value.name_Ptr;
                break;
            case (SymbolTableEntryType::Special):
                delete value.specialSymbol_Ptr;
                break;
            default:
                ELOG("Symbol Entry type not accounted for");
                break;
            }
        }
    };

    class SymbolTable
    {
        SymbolTableEntry* data_ArrayPtrOwned;
        std::hash<std::string> standardHash;
        int tableSize;
        int containedItems;

    public:
        int hashFunction(std::string input)
        {
            return standardHash(input) % tableSize;
        }

        SymbolTable(int capacity)
        {
            tableSize = capacity;
            data_ArrayPtrOwned = new SymbolTableEntry[tableSize];
            containedItems = 0;
        }

        ~SymbolTable()
        {
            delete[] data_ArrayPtrOwned;
        }
    };

    class Environment
    {
    public:
        SymbolTable symbols;
    };
}

#endif // LISP_H_INCLUDED
