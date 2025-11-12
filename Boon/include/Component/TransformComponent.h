#pragma once
#include <glm/glm.hpp>

//TODO - Make wrapper class so these includes can be moved to a cpp file
#define GLM_ENABLE_EXPERIMENTAL
#pragma warning(push)
#pragma warning(disable: 4201)
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#pragma warning(pop)

#include "Reflection/BClassBase.h"

namespace Boon
{
	class SceneComponent;
	BCLASS(HideInInspector)
	class TransformComponent
	{
	public:
		enum class TransformFlag : uint32_t
		{
			None = 0,

			Position = 1 << 0,
			Rotation = 1 << 1,
			Scale = 1 << 2,

			Forward = 1 << 3,
			Up = 1 << 4,
			Right = 1 << 4,

			World = 1 << 5,

			All = 0xffffffff
		};
		TransformComponent() = default;
		TransformComponent(SceneComponent* owner);
		~TransformComponent() = default;

		TransformComponent(const TransformComponent& other) = default;
		TransformComponent(TransformComponent&& other) = default;
		TransformComponent& operator=(const TransformComponent& other) = default;
		TransformComponent& operator=(TransformComponent&& other) = default;

		const glm::mat4& GetWorld();
		const glm::vec3& GetWorldPosition();
		const glm::vec3& GetLocalPosition() const;
		const glm::quat& GetWorldRotation();
		const glm::quat& GetLocalRotation() const;
		const glm::vec3& GetWorldEulerRotation();
		const glm::vec3& GetLocalEulerRotation() const;
		const glm::vec3& GetWorldScale();
		const glm::vec3& GetLocalScale() const;

		void SetLocalPosition(float x, float y, float z);
		void SetLocalPosition(const glm::vec3& position);
		void SetLocalRotation(float x, float y, float z);
		void SetLocalRotation(const glm::quat& rotation);
		void SetLocalRotation(const glm::vec3& rotation);
		void SetLocalScale(float x, float y, float z);
		void SetLocalScale(const glm::vec3& scale);

		void Translate(float x, float y, float z);
		void Translate(const glm::vec3& translation);
		void Rotate(float x, float y, float z);
		void Rotate(const glm::quat& rotation);
		void Rotate(const glm::vec3& rotation);
		void Scale(float x, float y, float z);
		void Scale(const glm::vec3& scale);

		void RotateTowards(const glm::vec3 direction);
		void RotateToPoint(const glm::vec3 point);

		const glm::vec3& GetForward();
		const glm::vec3& GetUp();
		const glm::vec3& GetRight();

	protected:
		void SetChildrenDirty(TransformFlag flag);

	private:
		void RecalculateWorldPosition();
		void RecalculateWorldRotation();
		void RecalculateWorldScale();
		void RecalculateWorldTransform();
		void SetDirty(TransformFlag flag, bool isDirty);
		bool IsDirty(TransformFlag flag) const;

		void RecalculateForward();
		void RecalculateUp();
		void RecalculateRight();

		glm::mat4 m_WorldTransform{};
		glm::vec3 m_LocalPosition{};
		glm::vec3 m_WorldPosition{};
		glm::quat m_LocalRotQ{};
		glm::quat m_WorldRotQ{};
		glm::vec3 m_LocalEuler{};
		glm::vec3 m_WorldEuler{};
		glm::vec3 m_LocalScale{ 1.f, 1.f, 1.f };
		glm::vec3 m_WorldScale{};

		glm::vec3 m_Forward{ 0.f, 0.f, -1.f };
		glm::vec3 m_Up{ 0.f, 1.f, 0.f };
		glm::vec3 m_Right{ 1.f, 0.f, 0.f };

		TransformFlag m_DirtyFlags{ TransformFlag::All };

		friend class GameObject;
		friend class SceneComponent;
		SceneComponent* m_Owner;
	};
}