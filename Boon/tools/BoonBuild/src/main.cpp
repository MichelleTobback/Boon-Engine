#include "BoonBuildGenerator.h"

#include <filesystem>
#include <iostream>

int main(int argc, char** argv)
{
    std::filesystem::path engineRoot;

    std::string profileName;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--profile" && i + 1 < argc)
        {
            profileName = argv[++i];
        }
        else
        {
            engineRoot = arg;
        }
    }

    BoonBuild::BoonBuildGenerator generator;

    if (!generator.Generate(engineRoot, profileName))
    {
        std::cerr << "BoonBuild failed.\n";
        return 1;
    }

    return 0;
}