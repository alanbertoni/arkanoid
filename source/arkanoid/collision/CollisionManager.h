#pragma once

#include "../actor/ColliderComponent.h"
#include "../core/Common.h"
#include "../core/Singleton.h"

namespace collision
{
	class CollisionManager : public core::Singleton<CollisionManager>
	{
	public:
		static void CreateInstance();
		~CollisionManager() = default;

		void AddCollider(actor::ColliderComponent* pCollider);
		void RemoveCollider(actor::ColliderComponent* pCollider);
		void Update();

	private:
		typedef std::vector<actor::ColliderComponent*> ColliderList;
		typedef std::map<actor::ColliderComponent::EColliderType, ColliderList> ColliderMap;
		
		CollisionManager();
		CollisionManager(const CollisionManager&) = delete;
		CollisionManager(CollisionManager&&) = delete;
		CollisionManager& operator=(const CollisionManager&) = delete;
		CollisionManager& operator=(CollisionManager&&) = delete;

		bool CheckCollision(actor::ColliderComponent* pCollider1, actor::ColliderComponent* pCollider2) const;

		ColliderList& GetColliderList(actor::ColliderComponent::EColliderType eType);

		ColliderMap m_oColliderMap;
		ColliderList m_oColliderList;
	};

	inline collision::CollisionManager::ColliderList& collision::CollisionManager::GetColliderList(actor::ColliderComponent::EColliderType eType)
	{
		auto l_oIt = m_oColliderMap.find(eType);
		assert(l_oIt != m_oColliderMap.end());
		return l_oIt->second;
	}
}