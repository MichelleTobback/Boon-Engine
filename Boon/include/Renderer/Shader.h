#pragma once

#include <memory>
#include <string>

namespace Boon
{
	class Shader
	{
	public:
		Shader() = default;
		virtual ~Shader() = default;

		Shader(const Shader& other) = delete;
		Shader(Shader&& other) = delete;
		Shader& operator=(const Shader& other) = delete;
		Shader& operator=(Shader&& other) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static std::shared_ptr<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);
	};
}