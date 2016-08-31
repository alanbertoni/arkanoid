#pragma once

#include "ActorComponent.h"
#include "../utilities/Math.h"

namespace actor
{
	class AudioListenerComponent : public ActorComponent
	{
	public:
		AudioListenerComponent(Actor* pActor);
		~AudioListenerComponent() = default;

		virtual bool Init(const tinyxml2::XMLElement* pData) override;
		virtual void PostInit() override;
		virtual LuaPlus::LuaObject GetLuaObject() override;

		bool IsMute() const;
		float GetVolume() const;
		const Vec3& GetPosition() const;
		const Vec3& GetVelocity() const;
		const Vec3& GetDirection() const;

		void ToggleMute();
		void SetVolume(float fVolume);
		void SetPosition(const Vec3& oPosition);
		void SetVelocity(const Vec3& oVelocity);
		void SetDirection(const Vec3& oDirection, const Vec3& oUp);

		static void RegisterScriptFunction();
		static const char* sk_szTypeName;

	protected:
		virtual std::string GetTypeName() const override;

	private:
		AudioListenerComponent(const AudioListenerComponent&) = delete;
		AudioListenerComponent(AudioListenerComponent&&) = delete;
		AudioListenerComponent& operator=(const AudioListenerComponent&) = delete;
		AudioListenerComponent& operator=(AudioListenerComponent&&) = delete;

		void SetPositionFromScript(LuaPlus::LuaObject oVec3LuaObject);
		void SetVelocityFromScript(LuaPlus::LuaObject oVec3LuaObject);
		void SetDirectionFromScript(LuaPlus::LuaObject oVec3LuaObject, LuaPlus::LuaObject oVec3UpLuaObject);
		LuaPlus::LuaObject GetPositionFromScript();
		LuaPlus::LuaObject GetVelocityFromScript();
		LuaPlus::LuaObject GetDirectionFromScript();

		void ApplyVolume();
		void ApplyPosition();
		void ApplyVelocity();
		void ApplyDirection();

		bool InitAudioListenerInfo(const tinyxml2::XMLElement* pData);

		void CreateLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;

		Vec3 m_vPosition;
		Vec3 m_vVelocity;
		Vec3 m_vDirection;
		Vec3 m_vUp;
		float m_fVolume;
	};

	inline std::string AudioListenerComponent::GetTypeName() const
	{
		return sk_szTypeName;
	}

	inline LuaPlus::LuaObject AudioListenerComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline bool AudioListenerComponent::IsMute() const
	{
		return m_fVolume == 0.0f;
	}

	inline const Vec3& AudioListenerComponent::GetPosition() const
	{
		return m_vPosition;
	}

	inline const Vec3& AudioListenerComponent::GetVelocity() const
	{
		return m_vVelocity;
	}

	inline const Vec3& AudioListenerComponent::GetDirection() const
	{
		return m_vDirection;
	}

	inline float AudioListenerComponent::GetVolume() const
	{
		return m_fVolume;
	}
}