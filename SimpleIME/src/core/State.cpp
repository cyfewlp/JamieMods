#include "core/State.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime::Core
    {
        std::unique_ptr<State> State::g_instance = nullptr;
    }
}