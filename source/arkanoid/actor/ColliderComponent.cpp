
#include "ColliderComponent.h"
#include "Actor.h"
#include "TransformComponent.h"
#include "../collision/CollisionManager.h"
#include "../log/LogManager.h"
#include "../script/LuaUtilities.h"

const char* actor::ColliderComponent::sk_szTypeName = "ColliderComponent";
const char* actor::ColliderComponent::sk_szMetatableName = "ColliderComponentMetatable";
const char* actor::ColliderComponent::CollisionData::sk_szMetatableName = "CollisionDataMetatable";


actor::ColliderComponent::CollisionData::CollisionData()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<CollisionData*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

void actor::ColliderComponent::CollisionData::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
}

void actor::ColliderComponent::CollisionData::SetupLuaObject()
{
	m_oLuaObject.SetString("ActorType1", m_pCollider1->GetOwner()->GetType().c_str());
	m_oLuaObject.SetString("ActorType2", m_pCollider2->GetOwner()->GetType().c_str());
	m_oLuaObject.SetObject("Collider1", m_pCollider1->GetLuaObject());
	m_oLuaObject.SetObject("Collider2", m_pCollider2->GetLuaObject());
	LuaPlus::LuaObject l_vPos1;
	script::LuaUtilities::ConvertVec3ToLuaObject(m_vPos1, l_vPos1);
	m_oLuaObject.SetObject("Pos1", l_vPos1);
	LuaPlus::LuaObject l_vPos2;
	script::LuaUtilities::ConvertVec3ToLuaObject(m_vPos2, l_vPos2);
	m_oLuaObject.SetObject("Pos2", l_vPos2);
	LuaPlus::LuaObject l_vExt1;
	script::LuaUtilities::ConvertVec3ToLuaObject(m_vHalfExtents1, l_vExt1);
	m_oLuaObject.SetObject("Dim1", l_vExt1);
	LuaPlus::LuaObject l_vExt2;
	script::LuaUtilities::ConvertVec3ToLuaObject(m_vHalfExtents2, l_vExt2);
	m_oLuaObject.SetObject("Dim2", l_vExt2);

	std::string l_szDirection;
	if (m_uiCollisionDirections & ECollisionDirection::RIGHT)
	{
		l_szDirection = "RIGHT";
	}

	if (m_uiCollisionDirections & ECollisionDirection::LEFT)
	{
		l_szDirection = "LEFT";
	}

	m_oLuaObject.SetString("DirectionX", l_szDirection.c_str());

	l_szDirection = "";
	if (m_uiCollisionDirections & ECollisionDirection::TOP)
	{
		l_szDirection = "TOP";
	}

	if (m_uiCollisionDirections & ECollisionDirection::BOTTOM)
	{
		l_szDirection = "BOTTOM";
	}

	m_oLuaObject.SetString("DirectionY", l_szDirection.c_str());
}

actor::ColliderComponent::ColliderComponent(Actor* pOwner)
	: ActorComponent{ pOwner }
	, m_eColliderType{ EColliderType::STATIC }
	, m_vHalfExtents{ 1.0f, 1.0f, 1.0f }
	, m_eColliderTag{ EColliderTag::NONE }
	, m_uiColliderTagMask{}
	, m_bIsTrigger{ false }
{
	CreateLuaObject();
	std::string l_szOnCollisionPath = "OnCollision/";
	m_oOnCollisionEvent.SetPath(l_szOnCollisionPath + pOwner->GetName());
}

actor::ColliderComponent::~ColliderComponent()
{
	collision::CollisionManager::GetSingleton().RemoveCollider(this);
}

void actor::ColliderComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);

	l_oMetaTable.SetMetaTable(script::LuaManager::GetSingleton().GetGlobalVars().GetByName(ActorComponent::sk_szMetatableName));

	CollisionData::RegisterScriptFunction();
}

void actor::ColliderComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<ColliderComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

bool actor::ColliderComponent::Init(const tinyxml2::XMLElement* pData)
{
	if (InitColliderType(pData) && InitColliderSize(pData) && InitColliderTag(pData) && InitColliderTagMask(pData))
	{
		std::stringstream l_oSS;
		l_oSS << "ColliderComponent info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to init ColliderComponent info of Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}

void actor::ColliderComponent::PostInit()
{
	m_pTransformComponent = GetOwner()->GetComponent<TransformComponent>(TransformComponent::sk_szTypeName);
	UpdateAABB();
	collision::CollisionManager::GetSingleton().AddCollider(this);
}

void actor::ColliderComponent::UpdateAABB()
{
	m_oLastAABB = m_oCurrentAABB;
	const Vec3& l_vPos = m_pTransformComponent->GetPosition();
	m_oCurrentAABB.SetCenter(Vec2{ l_vPos.x, l_vPos.y });
	if (!m_oLastAABB.IsValid())
	{
		m_oCurrentAABB.SetHalfExtents(Vec2{ m_vHalfExtents.x, m_vHalfExtents.y });
		m_oLastAABB = m_oCurrentAABB;
	}

	m_bCollided = false;
}

void actor::ColliderComponent::PostUpdate()
{
	if (m_bCollided && !m_bIsTrigger)
	{
		const Vec3& l_vPos = m_pTransformComponent->GetPosition();
		const Vec2& l_vCurrPos = m_oCurrentAABB.GetCenter();
		m_pTransformComponent->SetPosition(Vec3{ l_vCurrPos.x, l_vCurrPos.y, l_vPos.z });
	}
}

void actor::ColliderComponent::OnCollision(ColliderComponent* pOther)
{
	size_t l_uiCollisionDirections = 0;

	if (m_eColliderType == EColliderType::DYNAMIC)
	{
		m_bCollided = true;
		const Vec2& l_vCurrMin = m_oCurrentAABB.GetMin();
		const Vec2& l_vCurrMax = m_oCurrentAABB.GetMax();
		const Vec2& l_vLastMin = m_oLastAABB.GetMin();
		const Vec2& l_vLastMax = m_oLastAABB.GetMax();
		const Vec2& l_vOtherMin = pOther->m_oCurrentAABB.GetMin();
		const Vec2& l_vOtherMax = pOther->m_oCurrentAABB.GetMax();

		const Vec2& l_oHalfExtents = m_oCurrentAABB.GetHalfExtents();
		Vec2 l_vCurrPos = m_oCurrentAABB.GetCenter();

		// collision with Other's right edge 
		if (l_vLastMax.x <= l_vOtherMin.x && l_vOtherMin.x <= l_vCurrMax.x)
		{
			l_vCurrPos.x = l_vOtherMin.x - l_oHalfExtents.x;
			l_uiCollisionDirections |= CollisionData::ECollisionDirection::RIGHT;
		}

		// collision with Other's left edge 
		if (l_vLastMin.x >= l_vOtherMax.x && l_vOtherMax.x >= l_vCurrMin.x)
		{
			l_vCurrPos.x = l_vOtherMax.x + l_oHalfExtents.x;
			l_uiCollisionDirections |= CollisionData::ECollisionDirection::LEFT;
		}

		// collision with Other's bottom edge 
		if (l_vLastMax.y <= l_vOtherMin.y && l_vOtherMin.y <= l_vCurrMax.y)
		{
			l_vCurrPos.y = l_vOtherMin.y - l_oHalfExtents.y;
			l_uiCollisionDirections |= CollisionData::ECollisionDirection::BOTTOM;
		}

		// collision with Other's top edge 
		if (l_vLastMin.y >= l_vOtherMax.y && l_vOtherMax.y >= l_vCurrMin.y)
		{
			l_vCurrPos.y = l_vOtherMax.y + l_oHalfExtents.y;
			l_uiCollisionDirections |= CollisionData::ECollisionDirection::TOP;
		}

		m_oCurrentAABB.SetCenter(l_vCurrPos);
	}

	m_oCollisionData.m_pCollider1 = this;
	m_oCollisionData.m_pCollider2 = pOther;
	m_oCollisionData.m_vPos1 = m_pTransformComponent->GetPosition();
	m_oCollisionData.m_vPos2 = pOther->m_pTransformComponent->GetPosition();
	m_oCollisionData.m_vHalfExtents1 = m_vHalfExtents;
	m_oCollisionData.m_vHalfExtents2 = pOther->m_vHalfExtents;
	m_oCollisionData.m_uiCollisionDirections = l_uiCollisionDirections;

	m_oCollisionData.SetupLuaObject();
	m_oOnCollisionEvent.Raise(m_oCollisionData);
}

bool actor::ColliderComponent::InitColliderType(const tinyxml2::XMLElement* pData)
{
	bool l_bIsTrigger = false;
	tinyxml2::XMLError l_oError = pData->QueryBoolAttribute("isTrigger", &l_bIsTrigger);
	if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
	{
		std::stringstream l_oSS;
		l_oSS << "Error while trying to set isTrigger attribute of ColliderComponentfor Actor " << GetOwner()->GetName() << ", set to default";
		LOG_ERROR(l_oSS.str());
	}
	m_bIsTrigger = l_bIsTrigger;

	const char* l_szColliderType = pData->Attribute("type");
	if (l_szColliderType)
	{
		if (_strcmpi(l_szColliderType, "DYNAMIC") == 0)
		{
			m_eColliderType = EColliderType::DYNAMIC;
		}
		else if (_strcmpi(l_szColliderType, "STATIC") == 0)
		{
			m_eColliderType = EColliderType::STATIC;
		}
		else if (_strcmpi(l_szColliderType, "KINEMATIC") == 0)
		{
			m_eColliderType = EColliderType::KINEMATIC;
		}
		else
		{
			std::stringstream l_oSS;
			l_oSS << "Unknown Collider type attribute for ColliderComponent of Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		std::stringstream l_oSS;
		l_oSS << "Collider type successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Type attribute in ColliderComponent of Actor " << GetOwner()->GetName() << " set to default (STATIC)";
		LOG_ERROR(l_oSS.str());
		return false;
	}
}

bool actor::ColliderComponent::InitColliderTag(const tinyxml2::XMLElement * pData)
{
	const char* l_szColliderTag = pData->Attribute("tag");
	if (l_szColliderTag)
	{
		if (_strcmpi(l_szColliderTag, "PLAYER") == 0)
		{
			m_eColliderTag = EColliderTag::PLAYER;
		}
		else if (_strcmpi(l_szColliderTag, "ITEM") == 0)
		{
			m_eColliderTag = EColliderTag::ITEM;
		}
		else if (_strcmpi(l_szColliderTag, "ENEMY") == 0)
		{
			m_eColliderTag = EColliderTag::ENEMY;
		}
		else if (_strcmpi(l_szColliderTag, "BONUS") == 0)
		{
			m_eColliderTag = EColliderTag::BONUS;
		}
		else
		{
			std::stringstream l_oSS;
			l_oSS << "Unknown Collider tag attribute for ColliderComponent of Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		std::stringstream l_oSS;
		l_oSS << "Collider tag successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Type attribute in ColliderComponent of Actor " << GetOwner()->GetName() << " set to default (STATIC)";
		LOG_ERROR(l_oSS.str());
		return false;
	}
}

bool actor::ColliderComponent::InitColliderTagMask(const tinyxml2::XMLElement * pData)
{
	const char* l_szColliderTagMask = pData->Attribute("tagMask");
	if (l_szColliderTagMask)
	{
		std::vector<std::string> l_oTagMaskTokens;
		utilities::SplitString(l_szColliderTagMask, l_oTagMaskTokens, '|');
		for (std::string& l_oTag : l_oTagMaskTokens)
		{
			if (_strcmpi(l_oTag.c_str(), "PLAYER") == 0)
			{
				m_uiColliderTagMask |= static_cast<size_t>(EColliderTag::PLAYER);
			}
			else if (_strcmpi(l_oTag.c_str(), "ITEM") == 0)
			{
				m_uiColliderTagMask |= static_cast<size_t>(EColliderTag::ITEM);
			}
			else if (_strcmpi(l_oTag.c_str(), "ENEMY") == 0)
			{
				m_uiColliderTagMask |= static_cast<size_t>(EColliderTag::ENEMY);
			}
			else if (_strcmpi(l_oTag.c_str(), "BONUS") == 0)
			{
				m_uiColliderTagMask |= static_cast<size_t>(EColliderTag::BONUS);
			}
			else if (_strcmpi(l_oTag.c_str(), "ALL") == 0)
			{
				m_uiColliderTagMask |= static_cast<size_t>(EColliderTag::ALL);
			}
			else
			{
				std::stringstream l_oSS;
				l_oSS << "Unknown Collider tagMask attribute for ColliderComponent of Actor " << GetOwner()->GetName();
				LOG_ERROR(l_oSS.str());
				return false;
			}
		}

		std::stringstream l_oSS;
		l_oSS << "Collider tagMask successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		m_uiColliderTagMask = static_cast<size_t>(EColliderTag::ALL);
		std::stringstream l_oSS;
		l_oSS << "Missing tagMask attribute in ColliderComponent of Actor " << GetOwner()->GetName() << " set to default (ALL)";
		LOG_ERROR(l_oSS.str());
		return false;
	}
}

bool actor::ColliderComponent::InitColliderSize(const tinyxml2::XMLElement * pData)
{
	float l_fWidth = 1.0f;
	tinyxml2::XMLError l_oError = pData->QueryFloatAttribute("width", &l_fWidth);
	if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
	{
		std::stringstream l_oSS;
		l_oSS << "Error while trying to set width attribute of ColliderComponent for Actor " << GetOwner()->GetName();
		LOG_ERROR(l_oSS.str());
		return false;
	}
	m_vHalfExtents.x = l_fWidth * 0.5f;

	float l_fHeight = 1.0f;
	l_oError = pData->QueryFloatAttribute("height", &l_fHeight);
	if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
	{
		std::stringstream l_oSS;
		l_oSS << "Error while trying to set height attribute of ColliderComponent for Actor " << GetOwner()->GetName();
		LOG_ERROR(l_oSS.str());
		return false;
	}
	m_vHalfExtents.y = l_fHeight * 0.5f;

	std::stringstream l_oSS;
	l_oSS << "Collider size successfully initialized for Actor " << GetOwner()->GetName();
	LOG("ACTOR", l_oSS.str());
	return true;
}
