#pragma once

#include "ActorComponent.h"
#include "../utilities/Math.h"

namespace actor
{
	class TransformComponent : public ActorComponent
	{
	public:
		TransformComponent(Actor* pOwner);
		~TransformComponent() = default;

		virtual bool Init(const tinyxml2::XMLElement* pData) override;
		virtual LuaPlus::LuaObject GetLuaObject() override;

		bool IsChanged() const;

		const Mat4x4& GetToWorldMatrix();
		const Mat4x4& GetFromWorldMatrix();

		const Vec3& GetPosition() const;
		void SetPosition(Vec3 oPosition);
		void Translate(float fX, float fY, float fZ);
		void Translate(Vec3 oOffset);
		const Vec3& GetScale() const;
		void SetScale(Vec3 oScale);
		void SetRotation(const Vec3& oAxis, float fDegrees);
		void Rotate(const Vec3& oAxis, float fDegrees);

		static void RegisterScriptFunction();
		static const char* sk_szTypeName;

	protected:
		virtual std::string GetTypeName() const override;

	private:
		TransformComponent(const TransformComponent&) = delete;
		TransformComponent(TransformComponent&&) = delete;
		TransformComponent& operator=(const TransformComponent&) = delete;
		TransformComponent& operator=(TransformComponent&&) = delete;

		bool InitPositionInfo(const tinyxml2::XMLElement* pData);
		bool InitScaleInfo(const tinyxml2::XMLElement* pData);
		bool InitRotationInfo(const tinyxml2::XMLElement* pData);
		void UpdateWorldMatrices();

		LuaPlus::LuaObject GetScaleFromScript();
		void SetScaleFromScript(float fScaleFactor);
		LuaPlus::LuaObject GetPositionFromScript();
		void SetPositionFromScript(LuaPlus::LuaObject oVec3LuaObject);
		void TranslateFromScript(float fX, float fY, float fZ);
		void SetRotationFromScript(LuaPlus::LuaObject oVec3LuaObject, float fDegrees);
		void RotateFromScript(LuaPlus::LuaObject oVec3LuaObject, float fDegrees);
		void CreateLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;
		
		Vec3 m_oPosition;
		Quat m_oRotation;
		Vec3 m_oScale;

		Mat4x4 m_oToWorldMatrix;
		Mat4x4 m_oFromWorldMatrix;
		bool m_bIsChanged;
	};

	inline std::string TransformComponent::GetTypeName() const
	{
		return sk_szTypeName;
	}

	inline LuaPlus::LuaObject TransformComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline bool TransformComponent::IsChanged() const
	{
		return m_bIsChanged;
	}

	inline const Vec3& TransformComponent::GetPosition() const
	{
		return m_oPosition;
	}

	inline const Vec3& TransformComponent::GetScale() const
	{
		return m_oScale;
	}
}