#include <boost/parser/parser.hpp>

#include <iostream>
#include <string>


struct employee
{
    int age;
    std::string surname;
    std::string forename;
    double salary;
};

namespace bp = boost::parser;

int main()
{
    std::cout << "Enter a list of doubles, separated by commas.  No pressure. ";
    std::string input;
    std::getline(std::cin, input);

    auto const result = bp::parse(input, bp::double_ % ',');

    if (result) {
        std::cout << "Great! It looks like you entered:\n";
        for (double x : *result) {
            std::cout << x << "\n";
        }
    } else {
        std::cout
            << "Good job!  Please proceed to the recovery annex for cake.\n";
    }
}