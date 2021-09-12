#include "platform.h"

namespace platform
{
    #ifdef DEBUG
        std::ofstream logStream("log.txt");
    #endif // DEBUG
    std::ifstream standardLisp("programs.lsp");
}
