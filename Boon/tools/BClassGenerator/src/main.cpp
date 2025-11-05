#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct FoundClass {
    std::string typeName;
    std::string includePath;
};

int main(int argc, char** argv)
{
    if (argc < 3) {
        std::cerr << "Usage: BClassGenerator <include_dir> <output_file>\n";
        return 1;
    }

    fs::path includeDir = argv[1];
    fs::path outputFile = argv[2];

    std::vector<FoundClass> found;

    std::regex classRegex(
        R"(BCLASS\s*\(\s*\)\s*(class|struct)\s+([A-Za-z_][A-Za-z0-9_]*)\b)"
    );

    for (auto& file : fs::recursive_directory_iterator(includeDir))
    {
        if (!file.is_regular_file())
            continue;

        auto ext = file.path().extension().string();
        if (ext != ".h" && ext != ".hpp")
            continue;

        std::ifstream in(file.path());
        std::string content((std::istreambuf_iterator<char>(in)), {});

        std::smatch match;
        auto begin = content.cbegin();

        while (std::regex_search(begin, content.cend(), match, classRegex))
        {
            FoundClass c;
            c.typeName = match[2];
            fs::path rel = fs::relative(file.path(), includeDir);
            c.includePath = rel.generic_string();
            found.push_back(c);

            begin = match.suffix().first;
        }
    }

    std::cout << "[BClassGenerator] Found " << found.size() << " reflected classes:\n";
    for (auto& c : found)
        std::cout << "  - " << c.typeName << " (" << c.includePath << ")\n";

    std::ofstream out(outputFile);
    out << "// Automatically generated. Do not modify.\n";
    out << "#include \"Reflection/RegisterBClass.h\"\n";

    for (auto& c : found)
        out << "#include \"" << c.includePath << "\"\n";

    out << "\nnamespace Boon {\n";
    out << "static struct _AutoRegisterAllClasses {\n";
    out << "    _AutoRegisterAllClasses() {\n";

    for (auto& c : found)
        out << "        RegisterBClass<" << c.typeName << ">();\n";

    out << "    }\n} _autoRegisterAllClasses;\n}\n";

    return 0;
}
