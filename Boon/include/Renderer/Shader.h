#pragma once

#include <memory>
#include <string>

namespace Boon
{
	class Shader
	{
	public:
		Shader() = default;

		/**
		 * @brief Virtual destructor for shader implementations.
		 */
		virtual ~Shader() = default;

		Shader(const Shader& other) = delete;
		Shader(Shader&& other) = delete;
		Shader& operator=(const Shader& other) = delete;
		Shader& operator=(Shader&& other) = delete;

		/**
		 * @brief Bind the shader for rendering.
		 */
		virtual void Bind() const = 0;

		/**
		 * @brief Unbind the shader.
		 */
		virtual void Unbind() const = 0;

		/**
		 * @brief Create a shader from vertex and fragment source strings.
		 *
		 * @param vertexSrc Source code for the vertex shader stage.
		 * @param fragmentSrc Source code for the fragment shader stage.
		 * @return Shared pointer to a Shader instance, or nullptr on failure.
		 */
		static std::shared_ptr<Shader> Create(const std::string& vertexSrc, const std::string& fragmentSrc);
	};
}