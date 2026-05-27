#pragma once

#include "Renderer/ShaderCompiler/ShaderReflection.h"

namespace Boon
{
    class GLSLReflectionProvider final : public IShaderReflectionProvider
    {
    public:
        ShaderReflection Reflect(const std::string& vertexSource, const std::string& fragmentSource) override;
    };
}