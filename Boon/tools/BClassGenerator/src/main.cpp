#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace fs = std::filesystem;

static bool g_BoonMinimal = false;

// ============================================================================
// Reflection data
// ============================================================================
struct MetaKV { 
    std::string key; 
    std::string value; 
};

struct Property {
    std::string type;                // textual C++ type as written
    std::string name;                // identifier
    std::vector<MetaKV> meta;        // property metadata

    static bool ValidMeta(const std::string& meta)
    {
        static const std::unordered_set<std::string> minimalMeta{ 
            "Replicated" 
        };
        static const std::unordered_set<std::string> fullMeta{ 
            "Replicated", 
            "RangeMin", 
            "RangeMax", 
            "Range", 
            "Slider", 
            "Name", 
            "Category", 
            "HideInInspector",
            "ColorPicker"
        };
        const auto& active = g_BoonMinimal ? minimalMeta : fullMeta;
        return active.find(meta) != active.end();
    }
};

struct ReflectedClass {
    std::string name;                // unqualified type name
    std::string nsQualifiedName;     // e.g. Boon::PlayerController
    std::string headerPath;          // normalized include path
    std::string netSerializerPath;   // normalized net serializer include path
    std::vector<MetaKV> classMeta;   // class-level metadata from BCLASS(...)
    std::vector<Property> properties;

    static bool ValidMeta(const std::string& meta)
    {
        static const std::unordered_set<std::string> minimalMeta{ 
            "Replicated"
        };
        static const std::unordered_set<std::string> fullMeta{ 
            "Name", 
            "Category", 
            "HideInInspector",
            "Replicated"
        };
        const auto& active = g_BoonMinimal ? minimalMeta : fullMeta;
        return active.find(meta) != active.end();
    }
};

// ============================================================================
// Utils
// ============================================================================
static std::string normalizePath(const fs::path& p) {
    std::string s = p.string();
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

std::string ReplaceFilename(const std::string& pathStr, const std::string& newFilename)
{
    fs::path p(pathStr);
    p.replace_filename(newFilename);
    return p.string();
}

// Rough namespace detector: finds "namespace Foo { ... }" blocks.
// Skips "using namespace Foo;" statements completely.

static std::string detectNamespaceQualifiedName(
    const std::string& content,
    size_t pos,
    const std::string& className)
{
    std::string ns = "Boon";

    size_t last = content.rfind("namespace", pos);
    while (last != std::string::npos)
    {
        size_t after = last + 9; // length of "namespace"

        // Skip whitespace
        size_t start = content.find_first_not_of(" \t\r\n", after);
        if (start == std::string::npos)
            break;

        // Expecting the namespace name here (unless it's anonymous)
        if (!(std::isalpha(static_cast<unsigned char>(content[start])) || content[start] == '_'))
        {
            // Could be anonymous namespace → skip
            last = content.rfind("namespace", last - 1);
            continue;
        }

        // Capture namespace identifier
        size_t end = start;
        while (end < content.size() &&
            (std::isalnum(static_cast<unsigned char>(content[end])) || content[end] == '_'))
        {
            end++;
        }

        // Now determine what comes after the namespace name
        size_t next = content.find_first_not_of(" \t\r\n", end);
        if (next == std::string::npos)
            break;

        // *** This is the critical fix ***
        if (content[next] == ';')
        {
            // This is "using namespace Foo;" → skip
            last = content.rfind("namespace", last - 1);
            continue;
        }

        if (content[next] == '{')
        {
            // This is a proper namespace block
            ns = content.substr(start, end - start);
            break;
        }

        // Any other character = invalid → keep searching
        last = content.rfind("namespace", last - 1);
    }

    return ns + "::" + className;
}



// Basic type inference → BTypeId
static std::string inferBTypeId(const std::string& rawType) {
    std::string t = rawType;
    // strip spaces
    t.erase(std::remove_if(t.begin(), t.end(), [](unsigned char c) { return std::isspace(c); }), t.end());

    static const std::unordered_map<std::string, std::string> lut = {
        {"int",         "BTypeId::Int"},
        {"glm::ivec2",  "BTypeId::Int2"},
        {"glm::ivec3",  "BTypeId::Int3"},
        {"glm::ivec4",  "BTypeId::Int4"},

        {"float",       "BTypeId::Float"},
        {"glm::vec2",   "BTypeId::Float2"},
        {"glm::vec3",   "BTypeId::Float3"},
        {"glm::vec4",   "BTypeId::Float4"},

        {"double",      "BTypeId::Double"},
        {"bool",        "BTypeId::Bool"},
        {"char",        "BTypeId::Char"},
        {"std::string", "BTypeId::String"},
        {"UUID",        "BTypeId::Int64"}
    };
    auto it = lut.find(t);
    if (it != lut.end()) return it->second;

    if (t.find("std::shared_ptr<") != std::string::npos) return "BTypeId::SharedPtr";
    if (t.find("std::vector<") != std::string::npos) return "BTypeId::Array";

    return "BTypeId::UserDefined";
}

// Parse metadata list inside (...) → key/value pairs
template <typename T>
static std::vector<MetaKV> parseMetadataList(const std::string& inside)
{
    std::vector<MetaKV> out;
    try {
        // Range(a,b) → RangeMin=a, RangeMax=b
        std::regex rangeRe(R"REG(Range\s*\(\s*([^\s,)"']+)\s*,\s*([^\s,)"']+)\s*\))REG");
        std::smatch rm;
        if (std::regex_search(inside, rm, rangeRe)) {
            out.push_back({ "RangeMin", rm[1].str() });
            out.push_back({ "RangeMax", rm[2].str() });
        }

        // Generic tokens: Flag OR Key=Value / Key="Value"
        std::regex tokenRe(R"REG(([A-Za-z_][A-Za-z0-9_]*)\s*(?:=\s*\"?([^,)\"]+)\"?)?)REG");
        for (std::sregex_iterator it(inside.begin(), inside.end(), tokenRe), end; it != end; ++it) {
            std::string key = (*it)[1].str();
            std::string value;
            if ((*it)[2].matched) value = (*it)[2].str();
            if (!T::ValidMeta(key)) continue;
            if (key == "Range") continue; // handled
            out.push_back({ key, value });
        }
    }
    catch (const std::regex_error& e) {
        std::cerr << "[BClassGenerator] regex error in parseMetadataList: " << e.what() << "\n";
    }
    return out;
}

// Scan include trees for BCLASS/ BPROPERTY
static std::vector<ReflectedClass> parseSourceFiles(const std::vector<std::string>& includeDirs, bool verbose)
{
    std::vector<ReflectedClass> classes;

    // BCLASS(<meta>) struct|class Name
    std::regex classRe(R"(BCLASS\s*\(([^)]*)\)\s*(?:class|struct)\s+([A-Za-z_]\w*))");
    // BPROPERTY(<meta>) <type> <name>
    std::regex propRe(R"(BPROPERTY\s*\(([^)]*)\)\s*([\w:<> ,]+)\s+([A-Za-z_]\w*))");

    for (const auto& dir : includeDirs) {
        for (auto& e : fs::recursive_directory_iterator(dir)) {
            if (!e.is_regular_file()) continue;
            auto ext = e.path().extension().string();
            if (ext != ".h" && ext != ".hpp") continue;

            std::ifstream in(e.path());
            if (!in.is_open()) continue;
            std::string content((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

            std::smatch m;
            auto begin = content.cbegin();
            auto end = content.cend();

            while (std::regex_search(begin, end, m, classRe)) {
                ReflectedClass cls;
                cls.name = m[2].str();
                cls.classMeta = parseMetadataList<ReflectedClass>(m[1].str());
                cls.headerPath = normalizePath(e.path());
                cls.nsQualifiedName = detectNamespaceQualifiedName(content,
                    static_cast<size_t>(std::distance(content.cbegin(), m[0].first)),
                    cls.name);

                bool replicates = false;

                // collect properties in this file (simple: scan whole file)
                for (std::sregex_iterator pit(content.begin(), content.end(), propRe), pend; pit != pend; ++pit) {
                    Property p;
                    p.type = (*pit)[2].str();
                    p.name = (*pit)[3].str();
                    p.meta = parseMetadataList<Property>((*pit)[1].str());
                    if (std::find_if(p.meta.begin(), p.meta.end(), [](const MetaKV& prop)->bool { return prop.key == "Replicated"; }) != p.meta.end())
                    {
                        replicates = true;
                    }

                    cls.properties.push_back(std::move(p));
                }
                if (replicates)
                    cls.classMeta.push_back({ "Replicated", "" });

                if (verbose) {
                    std::cout << "[BClassGenerator] " << cls.nsQualifiedName
                        << " -> " << cls.properties.size() << " properties\n";
                }

                classes.push_back(std::move(cls));
                begin = m.suffix().first;
            }
        }
    }
    return classes;
}

// Emit Generated_Components.cpp
static void emitGeneratedFile(const std::string& output, const std::vector<ReflectedClass>& classes, bool verbose)
{
    std::ofstream out(output);
    if (!out.is_open()) {
        std::cerr << "[BClassGenerator] Cannot write: " << output << "\n";
        return;
    }

    out << "// Automatically generated. Do not modify.\n";
    out << "#include \"Reflection/RegisterBClass.h\"\n\n";
    out << "#include \"Networking/NetRepRegistry.h\"\n\n";

    for (auto& c : classes)
    {
        out << "#include \"" << c.headerPath << "\"\n";
        auto it = std::find_if(c.classMeta.begin(), c.classMeta.end(), [](const MetaKV& kv) {return kv.key == "Replicated" && !kv.value.empty(); });
        if (it != c.classMeta.end())
        {
            out << "#include \"" << ReplaceFilename(c.headerPath, it->value + ".h") << "\"\n";
        }
    }

    out << "namespace Boon {\n";
    out << "static struct _AutoRegisterAllClasses {\n";
    out << "    _AutoRegisterAllClasses() {\n";

    for (auto& c : classes) {
        out << "        // " << c.nsQualifiedName << "\n";
        out << "        {\n";
        out << "            BClass* cls = RegisterBClass<" << c.nsQualifiedName << ">(\"" << c.name <<"\");\n";

        // class metadata
        for (auto& cm : c.classMeta) {
            if (cm.key == "Replicated")
            {
                if (cm.value.empty())
                    out << "            NetRepRegistry::Get().Register(cls);\n";
                else
                    out << "            NetRepRegistry::Get().Register(cls, new "<< cm.value << "());\n";
            }
            else if (cm.value.empty())
                out << "            cls->AddMeta(\"" << cm.key << "\");\n";
            else
                out << "            cls->AddMeta(\"" << cm.key << "\", \"" << cm.value << "\");\n";
        }

        for (auto& p : c.properties) {
            const std::string typeId = inferBTypeId(p.type);
            out << "            cls->AddPropertyOffset(\"" << p.name << "\", \"" << p.type << "\", "
                << "offsetof(" << c.nsQualifiedName << ", " << p.name << "), "
                << "sizeof(" << p.type << "), " << typeId;

            if (!p.meta.empty()) {
                out << ", { ";
                for (size_t i = 0; i < p.meta.size(); ++i) {
                    const auto& m = p.meta[i];
                    out << "BPropertyMeta{ \"" << m.key << "\", \"" << m.value << "\" }";
                    if (i + 1 < p.meta.size()) out << ", ";
                }
                out << " }";
            }
            out << ");\n";
        }

        out << "        }\n";
    }

    out << "    }\n";
    out << "} _autoRegisterAllClasses;\n";
    out << "} // namespace Boon\n";

    if (verbose) {
        std::cout << "[BClassGenerator] Wrote " << classes.size()
            << " classes to " << output << "\n";
    }
}

// ============================================================================
// Entry
// ============================================================================
int main(int argc, char** argv)
{
    bool verbose = false;
    const std::string usage =
        "Usage: BClassGenerator [--minimal] <output.cpp> <includeDir1> [includeDir2 ...] [--verbose]\n";

    if (argc < 3) {
        std::cout << usage;
        return 1;
    }

    const std::string firstArg = argv[1];
    int startIndex = 1;

    if (firstArg == "--minimal") {
        g_BoonMinimal = true;
        startIndex++;
        std::cout << "[BClassGenerator] Running in minimal mode\n";
    }

    std::vector<std::string> includeDirs;
    bool verboseFlag = false;

    const std::string output = argv[startIndex];
    for (int i = startIndex + 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verbose")
            verboseFlag = true;
        else
            includeDirs.push_back(arg);
    }

    auto classes = parseSourceFiles(includeDirs, verboseFlag);
    emitGeneratedFile(output, classes, verboseFlag);
    return 0;
}
