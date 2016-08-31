
#include "CollisionManager.h"
#include "AABB2D.h"
#include "../actor/Actor.h"
#include "../actor/TransformComponent.h"

void collision::CollisionManager::CreateInstance()
{
	if (GetSingletonPtr() == nullptr)
	{
		new CollisionManager{};
	}
}

collision::CollisionManager::CollisionManager()
	: Singleton{}
	, m_oColliderMap{}
	, m_oColliderList{}
{
	m_oColliderMap.emplace(actor::ColliderComponent::EColliderType::DYNAMIC, ColliderList{});
	m_oColliderMap.emplace(actor::ColliderComponent::EColliderType::STATIC, ColliderList{});
	m_oColliderMap.emplace(actor::ColliderComponent::EColliderType::KINEMATIC, ColliderList{});
}

void collision::CollisionManager::AddCollider(actor::ColliderComponent* pCollider)
{
	m_oColliderList.push_back(pCollider);

	const actor::ColliderComponent::EColliderType l_eType = pCollider->GetColliderType();
	ColliderList& l_oColliderList = GetColliderList(l_eType);
	l_oColliderList.push_back(pCollider);
}

void collision::CollisionManager::RemoveCollider(actor::ColliderComponent * pCollider)
{
	auto l_oIt = m_oColliderList.begin();
	while (l_oIt != m_oColliderList.end())
	{
		auto it = l_oIt;
		++l_oIt;
		if ((*it) == pCollider)
		{
			m_oColliderList.erase(it);
			break;
		}
	}

	const actor::ColliderComponent::EColliderType l_eType = pCollider->GetColliderType();
	ColliderList& l_oColliderList = GetColliderList(l_eType);
	l_oIt = l_oColliderList.begin();
	while (l_oIt != l_oColliderList.end())
	{
		auto it = l_oIt;
		++l_oIt;
		if ((*it) == pCollider)
		{
			l_oColliderList.erase(it);
			break;
		}
	}
}

void collision::CollisionManager::Update()
{
	// Update the position of the ColliderComponent's AABB
	for (actor::ColliderComponent* l_pCollider : m_oColliderList)
	{
		l_pCollider->UpdateAABB();
	}

	ColliderList& l_oDynamicList = GetColliderList(actor::ColliderComponent::EColliderType::DYNAMIC);
	ColliderList& l_oStaticList = GetColliderList(actor::ColliderComponent::EColliderType::STATIC);
	ColliderList& l_oKinematicList = GetColliderList(actor::ColliderComponent::EColliderType::KINEMATIC);
	for (auto l_oIt = l_oDynamicList.begin(); l_oIt != l_oDynamicList.end(); ++l_oIt)
	{
		// Skip if ColliderComponent is either ColliderComponent or Actor is disabled
		auto l_pDynCollider = *l_oIt;
		if (!l_pDynCollider->IsEnable() || !l_pDynCollider->GetOwner()->IsEnable())
		{
			continue;
		}

		// check collisions between dynamic collider and static colliders
		for (auto l_pCollider : l_oStaticList)
		{
			if (CheckCollision(*l_oIt, l_pCollider))
			{
				break;
			}
		}

		// check collisions between dynamic collider and kinematic colliders
		for (auto l_pCollider : l_oKinematicList)
		{
			if (CheckCollision(*l_oIt, l_pCollider))
			{
				break;
			}
		}

		// check collisions between dynamic collider and others dynamic colliders
		auto it = l_oIt;
		++it;
		while (it != l_oDynamicList.end())
		{
			if (CheckCollision(*l_oIt, *it))
			{
				break;
			}
			++it;
		}

		// Update the position of the object due to collision response
		(*l_oIt)->PostUpdate();
	}
}

bool collision::CollisionManager::CheckCollision(actor::ColliderComponent* pCollider1, actor::ColliderComponent* pCollider2) const
{
	// Skip if ColliderComponent is either ColliderComponent or Actor is disabled
	if (!pCollider2->IsEnable() || !pCollider2->GetOwner()->IsEnable())
	{
		return false;
	}

	// Skip if ColliderTag is not included in the ColliderTagMask
	if ((pCollider2->GetColliderTag() & pCollider1->GetColliderTagMask()) == 0)
	{
		return false;
	}

	const AABB2D& l_oAABB1 = pCollider1->GetAABB();
	const AABB2D& l_oAABB2 = pCollider2->GetAABB();
	if (l_oAABB1.Overlaps(l_oAABB2))
	{
		pCollider1->OnCollision(pCollider2);
		pCollider2->OnCollision(pCollider1);
		return true;
	}
	return false;
}
