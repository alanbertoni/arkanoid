#pragma once

#include "ActorComponent.h"
#include "../utilities/Math.h"
#include "../audio/OpenALUtilities.h"

namespace actor
{
	class AudioSourceComponent : public ActorComponent
	{
	public:
		AudioSourceComponent(Actor* pActor);
		~AudioSourceComponent();

		virtual bool Init(const tinyxml2::XMLElement* pData) override;
		virtual void PostInit() override;
		virtual LuaPlus::LuaObject GetLuaObject() override;

		bool IsPlaying() const;
		bool IsPaused() const;
		bool IsStopped() const;

		void Play(float fVolume = 1.0f, bool bLoop = false);
		void Pause();
		void Stop();
		void Rewind();

		const std::string& GetAudioFilename() const;
		float GetVolume() const;
		bool IsLooping() const;
		float GetPitch() const;
		const Vec3& GetPosition() const;
		const Vec3& GetVelocity() const;
		const Vec3& GetDirection() const;

		void SetVolume(float fVolume);
		void SetLoop(bool bLoop);
		void SetPitch(float fPitch);
		void SetPosition(const Vec3& i_oPosition);
		void SetVelocity(const Vec3& i_oVelocity);
		void SetDirection(const Vec3& i_oDirection);

		static void RegisterScriptFunction();
		static const char* sk_szTypeName;

	protected:
		virtual std::string GetTypeName() const override;

	private:
		AudioSourceComponent(const AudioSourceComponent&) = delete;
		AudioSourceComponent(AudioSourceComponent&&) = delete;
		AudioSourceComponent& operator=(const AudioSourceComponent&) = delete;
		AudioSourceComponent& operator=(AudioSourceComponent&&) = delete;

		void SetPositionFromScript(LuaPlus::LuaObject i_oVec3LuaObject);
		void SetVelocityFromScript(LuaPlus::LuaObject i_oVec3LuaObject);
		void SetDirectionFromScript(LuaPlus::LuaObject i_oVec3LuaObject);
		LuaPlus::LuaObject GetPositionFromScript();
		LuaPlus::LuaObject GetVelocityFromScript();
		LuaPlus::LuaObject GetDirectionFromScript();

		void ApplyPitch();
		void ApplyPosition();
		void ApplyVelocity();
		void ApplyDirection();
		void DeleteSource();
		void DeleteBuffer();

		bool InitAudioSourceInfo(const tinyxml2::XMLElement* pData);

		void CreateLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;

		std::string m_szAudioFilename;
		Vec3 m_vPosition;
		Vec3 m_vVelocity;
		Vec3 m_vDirection;
		ALuint m_uiBuffer;
		ALuint m_uiSource;
		float m_fVolume;
		float m_fPitch;
		bool m_bIsLooping;
		bool m_bIsInit;
	};

	inline std::string AudioSourceComponent::GetTypeName() const
	{
		return sk_szTypeName;
	}

	inline LuaPlus::LuaObject AudioSourceComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline const std::string& AudioSourceComponent::GetAudioFilename() const
	{
		return m_szAudioFilename;
	}

	inline bool AudioSourceComponent::IsPlaying() const
	{
		if (m_bIsInit)
		{
			ALint       iState;
			alGetSourcei(m_uiSource, AL_SOURCE_STATE, &iState);
			OpenAlUtilities::CheckOpenALError("IsPlaying");
			return iState == AL_PLAYING;
		}
		return false;
	}

	inline bool AudioSourceComponent::IsPaused() const
	{
		if (m_bIsInit)
		{
			ALint       iState;
			alGetSourcei(m_uiSource, AL_SOURCE_STATE, &iState);
			OpenAlUtilities::CheckOpenALError("IsPaused");
			return iState == AL_PAUSED;
		}
		return false;
	}

	inline bool AudioSourceComponent::IsStopped() const
	{
		if (m_bIsInit)
		{
			ALint       iState;
			alGetSourcei(m_uiSource, AL_SOURCE_STATE, &iState);
			OpenAlUtilities::CheckOpenALError("IsStopped");
			return iState == AL_STOPPED;
		}
		return false;
	}

	inline const Vec3& AudioSourceComponent::GetPosition() const
	{
		return m_vPosition;
	}

	inline const Vec3& AudioSourceComponent::GetVelocity() const
	{
		return m_vVelocity;
	}

	inline const Vec3& AudioSourceComponent::GetDirection() const
	{
		return m_vDirection;
	}

	inline float AudioSourceComponent::GetVolume() const
	{
		return m_fVolume;
	}

	inline bool AudioSourceComponent::IsLooping() const
	{
		return m_bIsLooping;
	}

	inline float AudioSourceComponent::GetPitch() const
	{
		return m_fPitch;
	}
}