#pragma once

#include "../core/Common.h"
#include "../event/EventListener.h"
#include "../graphics/IRenderer.h"

#include <stack>

namespace scene
{
	class SceneNode;
}

namespace scene
{
	class Scene
	{
	public:
		Scene(graphics::IRenderer& oRenderer);
		~Scene();

		bool Render();

		SceneNode* FindSceneNode(ActorId oId);
		bool AddChild(SceneNode* pChildNode);
		bool AddChild(ActorId oId, SceneNode* pChildNode);
		bool RemoveChild(ActorId oId);

		void PushAndSetMatrix(const Mat4x4& oToWorldMatrix);
		void PopMatrix();
		const Mat4x4& GetTopMatrix() const;

		graphics::IRenderer& GetRenderer() const;

		SceneNode& GetCameraNode() const;

	private:
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) = delete;

		void OnCameraInit(const ActorId& oCameraId);

		typedef std::map<ActorId, SceneNode*> SceneActorMap;
		
		SceneActorMap m_ActorMap;
		SceneNode* m_pRootNode;
		SceneNode* m_pCamera;
		graphics::IRenderer& m_oRenderer;
		std::stack<Mat4x4> m_oMatrixStack;
		event::EventListener<Scene, ActorId> m_oOnCameraInitEvent;
	};

	inline graphics::IRenderer& Scene::GetRenderer() const
	{
		return m_oRenderer;
	}

	inline SceneNode& Scene::GetCameraNode() const
	{
		assert(m_pCamera);
		return *m_pCamera;
	}

	inline void Scene::PushAndSetMatrix(const Mat4x4& oToWorldMatrix)
	{
		Mat4x4 l_oTopMat;
		utilities::MathHelper::MatrixMultiply(l_oTopMat, oToWorldMatrix, GetTopMatrix());
		m_oMatrixStack.push(std::move(l_oTopMat));
	}

	inline void Scene::PopMatrix()
	{
		m_oMatrixStack.pop();
	}

	inline const Mat4x4& Scene::GetTopMatrix() const
	{
		return m_oMatrixStack.top();
	}
}
