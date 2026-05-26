#include <Renderer/Pipeline.h>

#include <glad/glad.h>

namespace Boon
{
	struct Pipeline::Impl final
	{
	public:
		Impl(const PipelineDescriptor& desc)
			: m_Desc(desc) { }

		void Bind()
		{
			switch (m_Desc.Blend)
			{
			case BlendMode::None:
				glDisable(GL_BLEND);
				break;

			case BlendMode::Alpha:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				break;

			case BlendMode::Additive:
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				break;
			}

			switch (m_Desc.Depth)
			{
			case DepthMode::Disabled:
				glDisable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				break;

			case DepthMode::Read:
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				break;

			case DepthMode::ReadWrite:
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_TRUE);
				break;
			}

			switch (m_Desc.Cull)
			{
			case CullMode::None:
				glDisable(GL_CULL_FACE);
				break;

			case CullMode::Back:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;

			case CullMode::Front:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			}

			if (m_Desc.Shader)
				m_Desc.Shader->Bind();
		}

		void Unbind()
		{
			if (m_Desc.Shader)
				m_Desc.Shader->Unbind();
		}

		const PipelineDescriptor& GetDescriptor() const
		{
			return m_Desc;
		}
		std::shared_ptr<Shader> GetShader() const
		{
			return m_Desc.Shader;
		}

	private:
		PipelineDescriptor m_Desc;
	};


	Pipeline::Pipeline(const PipelineDescriptor& desc)
		: m_pImpl(std::make_unique<Pipeline::Impl>(desc)) { }
	Pipeline::~Pipeline(){}

	void Pipeline::Bind(){ m_pImpl->Bind(); }
	void Pipeline::Unbind(){ m_pImpl->Unbind(); }
	const PipelineDescriptor& Pipeline::GetDescriptor() const { return m_pImpl->GetDescriptor(); }
	std::shared_ptr<Shader> Pipeline::GetShader() const { return m_pImpl->GetShader(); }

	std::shared_ptr<Pipeline> Pipeline::Create(const PipelineDescriptor& desc)
	{
		return std::make_shared<Pipeline>(desc);
	}
}