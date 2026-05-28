#include "BoonBuildGenerator.h"

#include <filesystem>
#include <iostream>

int main(int argc, char** argv)
{
    std::filesystem::path engineRoot;

    if (argc >= 2)
        engineRoot = std::filesystem::absolute(argv[1]);
    else
        engineRoot = std::filesystem::current_path();

    BoonBuild::BoonBuildGenerator generator;

    if (!generator.Generate(engineRoot))
    {
        std::cerr << "BoonBuild failed.\n";
        return 1;
    }

    return 0;
}