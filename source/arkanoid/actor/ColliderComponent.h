#pragma once

#include "ActorComponent.h"
#include "../collision/AABB2D.h"
#include "../event/EventPublisher.h"
#include "../utilities/Types.h"
#include "../utilities/Math.h"

namespace actor
{
	class TransformComponent;
}

namespace actor
{
	class ColliderComponent : public ActorComponent
	{
	public:
		struct CollisionData
		{
			enum ECollisionDirection
			{
				NONE = 0,
				TOP = 1,
				BOTTOM = 2,
				LEFT = 4,
				RIGHT = 8
			};

			CollisionData();

			LuaPlus::LuaObject GetLuaObject();
			void SetupLuaObject();

			static void RegisterScriptFunction();
			static const char* sk_szMetatableName;

			LuaPlus::LuaObject m_oLuaObject;
			ColliderComponent* m_pCollider1;
			ColliderComponent* m_pCollider2;
			Vec3 m_vPos1;
			Vec3 m_vPos2;
			Vec3 m_vHalfExtents1;
			Vec3 m_vHalfExtents2;
			size_t m_uiCollisionDirections;
		};

		enum class EColliderType
		{
			STATIC,
			DYNAMIC,
			KINEMATIC
		};

		enum EColliderTag
		{
			NONE = 0,
			PLAYER = 1,
			ITEM = 2,
			ENEMY = 4,
			BONUS = 8,
			ALL = 15
		};

		ColliderComponent(Actor* pActor);
		~ColliderComponent();

		virtual bool Init(const tinyxml2::XMLElement* pData) override;
		virtual void PostInit() override;
		virtual LuaPlus::LuaObject GetLuaObject() override;

		EColliderType GetColliderType() const;
		EColliderTag GetColliderTag() const;
		size_t GetColliderTagMask() const;
		const collision::AABB2D& GetAABB() const;
		const collision::AABB2D& GetLastAABB() const;

		void UpdateAABB();
		void PostUpdate();
		void OnCollision(ColliderComponent* pOther);

		static void RegisterScriptFunction();
		static const char* sk_szTypeName;

	protected:
		virtual std::string GetTypeName() const override;

	private:
		ColliderComponent(const ColliderComponent&) = delete;
		ColliderComponent(ColliderComponent&&) = delete;
		ColliderComponent& operator=(const ColliderComponent&) = delete;
		ColliderComponent& operator=(ColliderComponent&&) = delete;

		bool InitColliderType(const tinyxml2::XMLElement* pData);
		bool InitColliderTag(const tinyxml2::XMLElement* pData);
		bool InitColliderTagMask(const tinyxml2::XMLElement* pData);
		bool InitColliderSize(const tinyxml2::XMLElement* pData);
		void CreateLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;

		event::EventPublisher<CollisionData> m_oOnCollisionEvent;
		CollisionData m_oCollisionData;
		TransformComponent* m_pTransformComponent;
		collision::AABB2D m_oCurrentAABB;
		collision::AABB2D m_oLastAABB;
		EColliderType m_eColliderType;
		EColliderTag m_eColliderTag;
		size_t m_uiColliderTagMask;
		Vec3 m_vHalfExtents;
		bool m_bIsTrigger;
		bool m_bCollided;
	};

	inline std::string ColliderComponent::GetTypeName() const
	{
		return sk_szTypeName;
	}

	inline LuaPlus::LuaObject ColliderComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline ColliderComponent::EColliderType ColliderComponent::GetColliderType() const
	{
		return m_eColliderType;
	}

	inline ColliderComponent::EColliderTag ColliderComponent::GetColliderTag() const
	{
		return m_eColliderTag;
	}

	inline size_t ColliderComponent::GetColliderTagMask() const
	{
		return m_uiColliderTagMask;
	}

	inline LuaPlus::LuaObject ColliderComponent::CollisionData::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline const collision::AABB2D& ColliderComponent::GetAABB() const
	{
		return m_oCurrentAABB;
	}

	inline const collision::AABB2D& ColliderComponent::GetLastAABB() const
	{
		return m_oLastAABB;
	}

}