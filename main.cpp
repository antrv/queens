#include <iostream>
#include <string>
#include <ctime>
#include "types.h"

extern void solve(uint size);

void printHelp()
{
    std::cout << "Usage: app <board_size>\n";
    std::cout << "  Example: app 16\n";
}

void printInvalidArguments()
{
    std::cout << "Invalid arguments\n";
    printHelp();
}

int main(const int argc, const char *const *const args)
{
    if (argc < 2)
    {
        printHelp();
        return 0;
    }

    if (argc == 2)
    {
        try
        {
            int value = std::stoi(args[1]);
            if (value >= 1)
            {
                std::clock_t start = clock();
                solve(value);
                std::clock_t end = clock();
                std::cout << "Execution time " << (1.0 * (end - start) / CLOCKS_PER_SEC) << "s\n";

                return 0;
            }
        }
        catch (std::invalid_argument)
        {
        }
    }

    printInvalidArguments();
    return 1;
}