#include "ModuleDependencyGraph.h"

#include <functional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace BoonBuild
{
    bool ModuleDependencyGraph::Sort(const std::vector<ModuleRules>& modules, std::vector<ModuleRules>& outSortedModules, std::string& outError)
    {
        outSortedModules.clear();
        outError.clear();

        std::unordered_map<std::string, const ModuleRules*> moduleMap;

        for (const ModuleRules& module : modules)
        {
            if (moduleMap.contains(module.Name))
            {
                outError = "Duplicate module name: " + module.Name;
                return false;
            }

            moduleMap[module.Name] = &module;
        }

        enum class VisitState
        {
            Unvisited,
            Visiting,
            Visited
        };

        std::unordered_map<std::string, VisitState> states;
        std::vector<std::string> stack;

        for (const ModuleRules& module : modules)
            states[module.Name] = VisitState::Unvisited;

        std::function<bool(const ModuleRules&)> visit;

        visit = [&](const ModuleRules& module) -> bool
            {
                VisitState& state = states[module.Name];

                if (state == VisitState::Visited)
                    return true;

                if (state == VisitState::Visiting)
                {
                    std::stringstream ss;
                    ss << "Circular module dependency detected: ";

                    for (const std::string& item : stack)
                        ss << item << " -> ";

                    ss << module.Name;

                    outError = ss.str();
                    return false;
                }

                state = VisitState::Visiting;
                stack.push_back(module.Name);

                for (const std::string& dependencyName : module.ModuleDependencies)
                {
                    auto it = moduleMap.find(dependencyName);

                    if (it == moduleMap.end())
                    {
                        outError =
                            "Module '" + module.Name +
                            "' depends on unknown module '" + dependencyName + "'";

                        return false;
                    }

                    if (!visit(*it->second))
                        return false;
                }

                stack.pop_back();

                state = VisitState::Visited;
                outSortedModules.push_back(module);

                return true;
            };

        for (const ModuleRules& module : modules)
        {
            if (!visit(module))
                return false;
        }

        return true;
    }
}