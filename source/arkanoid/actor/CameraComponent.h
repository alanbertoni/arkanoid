#pragma once

#include "ActorComponent.h"
#include "../event/EventListener.h"
#include "../utilities/Types.h"
#include "../utilities/Math.h"

namespace actor
{
	class CameraComponent : public ActorComponent
	{
	public:
		CameraComponent(Actor* pOwner);
		~CameraComponent() = default;

		virtual bool Init(const tinyxml2::XMLElement* pData) override;
		virtual void PostInit() override;
		virtual LuaPlus::LuaObject GetLuaObject() override;

		bool IsChanged() const;
		void SetAspectRatio(float fAspectRatio);
		void OnResize(const Dimension& oDimension);

		const Mat4x4& GetProjectionMatrix();

		static void RegisterScriptFunction();
		static const char* sk_szTypeName;

	protected:
		virtual std::string GetTypeName() const override;

	private:
		CameraComponent(const CameraComponent&) = delete;
		CameraComponent(CameraComponent&&) = delete;
		CameraComponent& operator=(const CameraComponent&) = delete;
		CameraComponent& operator=(CameraComponent&&) = delete;

		enum ECameraType
		{
			ORTHOGRAPHIC,
			PERSPECTIVE
		};

		bool InitOrthographicInfo(const tinyxml2::XMLElement* pData);
		bool InitPerspectiveInfo(const tinyxml2::XMLElement* pData);
		void UpdateProjectionMatrix();

		void CreateLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;

		event::EventListener<CameraComponent, Dimension> m_oOnResizeEvent;
		ECameraType m_eCameraType;
		float m_fViewWidth;
		float m_fViewHeight;
		float m_fNearPlane;
		float m_fFarPlane;
		float m_fFovH;
		float m_fAspectRatio;
		float m_fScreenWidth;
		float m_fScreenHeight;

		Mat4x4 m_oProjectionMatrix;
		bool m_bIsChanged;
	};

	inline std::string CameraComponent::GetTypeName() const
	{
		return sk_szTypeName;
	}

	inline LuaPlus::LuaObject CameraComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline bool CameraComponent::IsChanged() const
	{
		return m_bIsChanged;
	}

	inline void CameraComponent::SetAspectRatio(float fAspectRatio)
	{
		m_fAspectRatio = fAspectRatio;

		if (m_eCameraType == PERSPECTIVE)
		{
			m_bIsChanged = true;
		}
	}
}
