#include <iostream>
#include <span>

int main()
{
    std::span<char> charSpan;
    std::string input;
    static_assert(sizeof(input) == 40);
    {
        std::string str = "Hello World!";
        charSpan = str;
    }
    std::cout << charSpan.data() << std::endl;
    return 0;
}