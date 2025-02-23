
#include "context.h"

namespace LIBC_NAMESPACE_DECL
{
    namespace Ime
    {

        std::unique_ptr<Context> Context::g_context = nullptr;

        auto                     Context::GetInstance() noexcept -> Context *
        {
            if (g_context == nullptr)
            {
                g_context.reset(new Context());
            }
            return g_context.get();
        }
    }
}
