#include "common/toml++/toml.hpp"

#include <array>
#include <iostream>
#include <ostream>

int main()
{
    struct ImGuiScope
    {
        ImGuiScope()
        {
            std::cout << __func__ << std::endl;
        }

        virtual ~ImGuiScope()
        {
            std::cout << __func__ << std::endl;
        }
    };

    struct ImGuiColorScope : ImGuiScope
    {
        ImGuiColorScope(uint32_t colorIndex)
        {
            std::cout << "ImGuiColorScope() " << colorIndex << std::endl;
        }

        ~ImGuiColorScope() override
        {
            std::cout << "~ImGuiColorScope" << std::endl;
        }
    };

    std::array<ImGuiScope, 3> scopes = {};

    scopes[0] = ImGuiScope();
    scopes[1] = ImGuiScope();
    scopes[2] = ImGuiScope();

    {
        std::cout << "enter block\n";
        scopes[1] = ImGuiColorScope(1);
        scopes[2] = ImGuiColorScope(3);
        std::cout << "exit block\n";
    }

    return 0;
}