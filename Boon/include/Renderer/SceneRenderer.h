
namespace Boon
{
	class SceneRenderer final
	{
	public:
		SceneRenderer() = default;
		virtual ~SceneRenderer() = default;

		SceneRenderer(const SceneRenderer& other) = delete;
		SceneRenderer(SceneRenderer&& other) = delete;
		SceneRenderer& operator=(const SceneRenderer& other) = delete;
		SceneRenderer& operator=(SceneRenderer&& other) = delete;

		void Render();
	};
}