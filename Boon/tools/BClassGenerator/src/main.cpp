#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

namespace fs = std::filesystem;

static bool g_BoonMinimal = false;

// ============================================================================
// Reflection data
// ============================================================================
struct MetaKV 
{ 
    std::string key; 
    std::string value; 
};

struct Property 
{
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

struct ReflectedFunctionMeta
{
    std::string key;
    std::string value;
};

struct ReflectedFunctionParam
{
    std::string type;
    std::string name;
};

struct ReflectedFunction
{
    std::string name;                       // function name
    std::vector<ReflectedFunctionMeta> meta;
    std::vector<ReflectedFunctionParam> params;
};

struct ReflectedClass 
{
    std::string name;                // unqualified type name
    std::string nsQualifiedName;     // e.g. Boon::PlayerController
    std::string headerPath;          // normalized include path
    std::string netSerializerPath;   // normalized net serializer include path
    std::vector<MetaKV> classMeta;   // class-level metadata from BCLASS(...)
    std::vector<Property> properties;
    std::vector<ReflectedFunction> functions;

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

static uint32_t FNV1a32Runtime(const std::string& s)
{
    const uint32_t OFFSET = 0x811C9DC5u;
    const uint32_t PRIME = 0x01000193u;
    uint32_t hash = OFFSET;
    for (unsigned char c : s)
    {
        hash ^= c;
        hash *= PRIME;
    }
    return hash;
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

static std::vector<ReflectedFunctionParam> parseFunctionParams(const std::string& paramsStr)
{
    std::vector<ReflectedFunctionParam> params;

    std::string s = paramsStr;
    size_t start = 0;
    while (start < s.size())
    {
        size_t comma = s.find(',', start);
        std::string param = (comma == std::string::npos)
            ? s.substr(start)
            : s.substr(start, comma - start);

        // trim
        auto trim = [](std::string& str) {
            auto isSpace = [](unsigned char c) { return std::isspace(c); };
            while (!str.empty() && isSpace(str.front())) str.erase(str.begin());
            while (!str.empty() && isSpace(str.back()))  str.pop_back();
            };
        trim(param);
        if (!param.empty())
        {
            // split type and name: assume last token is name
            size_t lastSpace = param.find_last_of(" \t");
            if (lastSpace != std::string::npos)
            {
                std::string type = param.substr(0, lastSpace);
                std::string name = param.substr(lastSpace + 1);
                trim(type);
                trim(name);
                if (!type.empty() && !name.empty())
                {
                    params.push_back({ type, name });
                }
            }
        }

        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }

    return params;
}

// Scan include trees for BCLASS/ BPROPERTY
static std::vector<ReflectedClass> parseSourceFiles(const std::vector<std::string>& includeDirs, bool verbose)
{
    std::vector<ReflectedClass> classes;

    // BCLASS(<meta>) struct|class Name
    std::regex classRe(R"(BCLASS\s*\(([^)]*)\)\s*(?:class|struct)\s+([A-Za-z_]\w*))");
    // BPROPERTY(<meta>) <type> <name>
    std::regex propRe(R"(BPROPERTY\s*\(([^)]*)\)\s*([\w:<> ,]+)\s+([A-Za-z_]\w*))");
    // BFUNCTION(<meta>) <return-type> <name>(<params>);
    std::regex funcRe(
        R"(BFUNCTION\s*\(([^)]*)\)\s*(?:(?:inline|static|virtual|constexpr|const)\s+)*([A-Za-z_][\w:<>\s*&]*)\s+([A-Za-z_]\w*)\s*\(([^)]*)\)\s*;?)");

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

                // collect functions in this file
                for (std::sregex_iterator fit(content.begin(), content.end(), funcRe), fend; fit != fend; ++fit)
                {
                    ReflectedFunction f;
                    f.name = (*fit)[3].str();   // function name
                    auto metaVec = parseMetadataList<Property>((*fit)[1].str());
                    for (auto& kv : metaVec)
                    {
                        f.meta.push_back({ kv.key, kv.value });
                    }
                    f.params = parseFunctionParams((*fit)[4].str());
                    cls.functions.push_back(std::move(f));
                }

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
                        << " -> " << cls.properties.size() << " properties -> " << cls.functions.size() << " functions\n";
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

        for (auto& f : c.functions)
        {
            uint32_t id = FNV1a32Runtime(f.name);
            std::stringstream paramsInit;

            // Build params initializer: { { "type", "name", BTypeId::X }, ... }
            if (!f.params.empty())
            {
                paramsInit << "{ ";
                for (size_t i = 0; i < f.params.size(); ++i)
                {
                    const auto& p = f.params[i];
                    std::string typeId = inferBTypeId(p.type); // reuse property usage
                    paramsInit << "BFunctionParam{ \"" << p.type << "\", \"" << p.name << "\", " << typeId << " }";
                    if (i + 1 < f.params.size())
                        paramsInit << ", ";
                }
                paramsInit << " }";
            }

            // Build meta initializer: { { "key", "value" }, ... }
            std::stringstream metaInit;
            if (!f.meta.empty())
            {
                metaInit << "{ ";
                for (size_t i = 0; i < f.meta.size(); ++i)
                {
                    const auto& m = f.meta[i];
                    metaInit << "BFunctionMeta{ \"" << m.key << "\", \"" << m.value << "\" }";
                    if (i + 1 < f.meta.size())
                        metaInit << ", ";
                }
                metaInit << " }";
            }

            // Emit thunk
            std::string thunkName = c.name + "_" + f.name + "_Thunk";
            out << "            auto " << thunkName << " = +[](void* obj, Variant* args, size_t argCount) {\n";
            out << "                " << c.nsQualifiedName << "* self = static_cast<" << c.nsQualifiedName << "*>(obj);\n";

            // generate parameter unpack
            for (size_t i = 0; i < f.params.size(); ++i)
            {
                const auto& p = f.params[i];
                std::string typeId = inferBTypeId(p.type);
                out << "                " << p.type << " " << p.name << " = args[" << i << "].As<" << p.type << ">();\n";
            }

            // call function
            out << "                self->" << f.name << "(";
            for (size_t i = 0; i < f.params.size(); ++i)
            {
                out << f.params[i].name;
                if (i + 1 < f.params.size())
                    out << ", ";
            }
            out << ");\n";
            out << "            };\n";

            // Emit AddFunction call
            out << "            cls->AddFunction(\n";
            out << "                0x" << std::hex << id << "u,\n" << std::dec;
            out << "                " << thunkName << ",\n";

            if (!f.meta.empty())
                out << "                " << metaInit.str() << ",\n";
            else
                out << "                {},\n";

            if (!f.params.empty())
                out << "                " << paramsInit.str() << "\n";
            else
                out << "                {}\n";

            out << "            );\n";
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
