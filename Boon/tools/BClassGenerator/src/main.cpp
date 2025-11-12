#include <iostream>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>
#include <vector>
#include <unordered_map>

namespace fs = std::filesystem;

// ============================================================================
// Reflection data
// ============================================================================
struct MetaKV { std::string key; std::string value; };

struct Property {
    std::string type;                // textual C++ type as written
    std::string name;                // identifier
    std::vector<MetaKV> meta;        // property metadata
};

struct ReflectedClass {
    std::string name;                // unqualified type name
    std::string nsQualifiedName;     // e.g. Boon::PlayerController
    std::string headerPath;          // normalized include path
    std::vector<MetaKV> classMeta;   // class-level metadata from BCLASS(...)
    std::vector<Property> properties;
};

// ============================================================================
// Utils
// ============================================================================
static std::string normalizePath(const fs::path& p) {
    std::string s = p.string();
    std::replace(s.begin(), s.end(), '\\', '/');
    return s;
}

// Rough namespace detector (good enough for single named `namespace Foo {}` blocks)
static std::string detectNamespaceQualifiedName(const std::string& content,
    size_t pos,
    const std::string& className)
{
    std::string ns = "Boon";
    size_t last = content.rfind("namespace", pos);
    if (last != std::string::npos) {
        size_t start = content.find_first_not_of(" \t\r\n", last + 9);
        if (start != std::string::npos && std::isalpha(static_cast<unsigned char>(content[start]))) {
            size_t end = start;
            while (end < content.size() &&
                (std::isalnum(static_cast<unsigned char>(content[end])) || content[end] == '_' || content[end] == ':'))
                ++end;
            ns = content.substr(start, end - start);
        }
    }
    return ns + "::" + className;
}

// Basic type inference → BTypeId
static std::string inferBTypeId(const std::string& rawType) {
    std::string t = rawType;
    // strip spaces
    t.erase(std::remove_if(t.begin(), t.end(), [](unsigned char c) { return std::isspace(c); }), t.end());

    static const std::unordered_map<std::string, std::string> lut = {
        {"int", "BTypeId::Int"},
        {"float", "BTypeId::Float"},
        {"double", "BTypeId::Double"},
        {"bool", "BTypeId::Bool"},
        {"char", "BTypeId::Char"},
        {"std::string", "BTypeId::String"},
        {"glm::vec2", "BTypeId::Vec2"},
        {"glm::vec3", "BTypeId::Vec3"},
        {"glm::vec4", "BTypeId::Vec4"},
        {"UUID", "BTypeId::UUID"}
    };
    auto it = lut.find(t);
    if (it != lut.end()) return it->second;

    if (t.find("std::shared_ptr<") != std::string::npos) return "BTypeId::SharedPtr";
    if (t.find("std::vector<") != std::string::npos) return "BTypeId::Array";

    return "BTypeId::UserDefined";
}

// Parse metadata list inside (...) → key/value pairs
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
                cls.classMeta = parseMetadataList(m[1].str());
                cls.headerPath = normalizePath(e.path());
                cls.nsQualifiedName = detectNamespaceQualifiedName(content,
                    static_cast<size_t>(std::distance(content.cbegin(), m[0].first)),
                    cls.name);

                // collect properties in this file (simple: scan whole file)
                for (std::sregex_iterator pit(content.begin(), content.end(), propRe), pend; pit != pend; ++pit) {
                    Property p;
                    p.type = (*pit)[2].str();
                    p.name = (*pit)[3].str();
                    p.meta = parseMetadataList((*pit)[1].str());
                    cls.properties.push_back(std::move(p));
                }

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

    // Make private/protected public during includes so offsetof(Type, Member) compiles
    out << "#define private   public\n";
    out << "#define protected public\n";
    for (auto& c : classes)
        out << "#include \"" << c.headerPath << "\"\n";
    out << "#undef protected\n";
    out << "#undef private\n\n";

    out << "namespace Boon {\n";
    out << "static struct _AutoRegisterAllClasses {\n";
    out << "    _AutoRegisterAllClasses() {\n";

    for (auto& c : classes) {
        out << "        // " << c.nsQualifiedName << "\n";
        out << "        {\n";
        out << "            BClass* cls = RegisterBClass<" << c.nsQualifiedName << ">(\"" << c.name <<"\");\n";

        // class metadata
        for (auto& cm : c.classMeta) {
            if (cm.value.empty())
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
    if (argc < 3) {
        std::cout << "Usage: BClassGenerator <output.cpp> <includeDir1> [includeDir2 ...] [--verbose]\n";
        return 1;
    }

    bool verbose = false;
    const std::string output = argv[1];
    std::vector<std::string> includeDirs;
    for (int i = 2; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--verbose") verbose = true;
        else includeDirs.push_back(arg);
    }

    auto classes = parseSourceFiles(includeDirs, /*verbose*/verbose);
    emitGeneratedFile(output, classes, /*verbose*/verbose);
    return 0;
}
