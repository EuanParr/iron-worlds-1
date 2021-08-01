#include "structure.h"

namespace structure
{
    std::ostream& operator<<(std::ostream& output, const SExpressionPtr& e)
    {
        /// output << "<" << e.get() << ">";
        if (e->isAtom())
        {
            AtomPtr maybeAtom = std::dynamic_pointer_cast<Atom>(e);
            if (maybeAtom.get() != nullptr)
            {
                output << maybeAtom;
            }
        }
        else
        {
            SPairPtr maybeSPair = std::dynamic_pointer_cast<SPair>(e);
            if (maybeSPair.get() != nullptr)
            {
                output << maybeSPair;
            }
        }
        return output;
    }

    std::ostream& operator<<(std::ostream& output, const SPairPtr& p)
    {
        // for printing in dotted-pair form
        output << "(" << p->data_ << " . " << p->next_ << ")";
        return output;
    }

    std::ostream& operator<<(std::ostream& output, const AtomPtr& a)
    {
        output << a->name_; // << "@" << a.get();
        return output;
    }

    std::istream& operator>>(std::istream& input, SExpressionString& s)
    {
        std::string newString;
        int level = 0;
        bool incomplete = true;
        unsigned int i = 0;
        while (incomplete)
        {
            std::getline(input, newString);
            for (i = 0; i < newString.size(); ++i)
            {
                if (newString[i] == '(')
                {
                    ++level;
                }
                else if (newString[i] == ')')
                {
                    --level;
                    if (level == 0)
                    {
                        break;
                    }
                }
            }
            if (level == 0)
            {
                incomplete = false;
            }
            s.content += newString.substr(0, i+1);
        }
        return input;
    }
}
