#include "UnlitMaterial.h"
#include "IRenderer.h"
#include "../actor/CameraComponent.h"
#include "../actor/RendererComponent.h"
#include "../actor/TransformComponent.h"
#include "../scene/Scene.h"
#include "../scene/SceneNode.h"

struct CBChangesOnResize
{
	Mat4x4 m_oViewProj;
};

struct CBChangesEveryObject
{
	Mat4x4 m_oWorld;
	Vec4 m_oTextTransform;
};

struct CBMaterial
{
	Color4F m_oDiffuseColor;
};

const char* graphics::UnlitMaterial::s_szShaderFilePath = "data/assets/shader/UnlitTextured.hlsl";

graphics::UnlitMaterial::UnlitMaterial()
	: Material{}
	, m_oVSConstantBuffers{}
	, m_oPSConstantBuffers{}
	, m_oTextureId{}
{
}

void graphics::UnlitMaterial::SetupForRendering(IRenderer& oRenderer, const actor::RendererComponent& oRendererComponent)
{
	m_oVertexShaderId = oRenderer.CreateVertexShaderFromFile(s_szShaderFilePath, EVertexType::POSITION_TEXCOORD);
	m_oPixelShaderId = oRenderer.CreatePixelShaderFromFile(s_szShaderFilePath);
	CreateConstantBuffers(oRenderer);

	if (!oRendererComponent.GetTextureFilename().empty())
	{
		m_oTextureId = oRenderer.CreateTextureFromFile(oRendererComponent.GetTextureFilename());
	}
}

void graphics::UnlitMaterial::CreateConstantBuffers(IRenderer& oRenderer)
{
	std::string l_oBufferName = s_szShaderFilePath;
	CBufferId l_oBufferId = oRenderer.CreateConstantBuffer(l_oBufferName + "CBChangesEveryObject", sizeof(CBChangesEveryObject));
	m_oVSConstantBuffers.push_back(l_oBufferId);

	l_oBufferId = oRenderer.CreateConstantBuffer(l_oBufferName + "CBChangesOnResize", sizeof(CBChangesOnResize));
	m_oVSConstantBuffers.push_back(l_oBufferId);

	l_oBufferId = oRenderer.CreateConstantBuffer(l_oBufferName + "CBMaterial", sizeof(CBMaterial));
	m_oPSConstantBuffers.push_back(l_oBufferId);
}

void graphics::UnlitMaterial::UpdateMaterial(const scene::Scene& oScene, const scene::SceneNode& oSceneNode, const actor::RendererComponent& oRendererComponent)
{
	CBChangesEveryObject l_oVSCb;
	utilities::MathHelper::MatrixTranspose(l_oVSCb.m_oWorld, oScene.GetTopMatrix());
	l_oVSCb.m_oTextTransform = oRendererComponent.GetTextureScaleAndOffset();
	oScene.GetRenderer().UpdateConstantBuffer(m_oVSConstantBuffers[0], reinterpret_cast<const void*>(&l_oVSCb), sizeof(CBChangesEveryObject));

	actor::CameraComponent* l_pCameraComponent =  oScene.GetCameraNode().GetActor()->GetComponent<actor::CameraComponent>(actor::CameraComponent::sk_szTypeName);
	if ((l_pCameraComponent && l_pCameraComponent->IsChanged()) || oScene.GetCameraNode().GetActorsTransformComponent()->IsChanged())
	{
		CBChangesOnResize l_oVSCBOnResize;
		utilities::MathHelper::MatrixMultiply(l_oVSCBOnResize.m_oViewProj, oScene.GetCameraNode().GetActorsTransformComponent()->GetFromWorldMatrix(), l_pCameraComponent->GetProjectionMatrix());
		utilities::MathHelper::MatrixTranspose(l_oVSCBOnResize.m_oViewProj, l_oVSCBOnResize.m_oViewProj);
		oScene.GetRenderer().UpdateConstantBuffer(m_oVSConstantBuffers[1], reinterpret_cast<const void*>(&l_oVSCBOnResize), sizeof(CBChangesOnResize));
	}

	CBMaterial l_oPSCb;
	utilities::MathHelper::LoadColor(l_oPSCb.m_oDiffuseColor, oRendererComponent.GetDiffuseColor());
	oScene.GetRenderer().UpdateConstantBuffer(m_oPSConstantBuffers[0], reinterpret_cast<const void*>(&l_oPSCb), sizeof(CBMaterial));
}

void graphics::UnlitMaterial::BindMaterial(IRenderer& oRenderer)
{
	oRenderer.BindVertexShader(m_oVertexShaderId);
	oRenderer.BindPixelShader(m_oPixelShaderId);
	oRenderer.BindConstantBuffers(EShaderType::VertexShader, m_oVSConstantBuffers);
	oRenderer.BindConstantBuffers(EShaderType::PixelShader, m_oPSConstantBuffers);
	if (m_oTextureId)
	{
		oRenderer.BindShaderResource(m_oTextureId);
	}
}

void graphics::UnlitMaterial::ReleaseMaterial(IRenderer& oRenderer)
{
	oRenderer.ReleaseVertexShader(m_oVertexShaderId);
	oRenderer.ReleasePixelShader(m_oPixelShaderId);
	if (m_oTextureId)
	{
		oRenderer.ReleaseShaderResource(m_oTextureId);
	}
	for (auto l_oCb : m_oVSConstantBuffers)
	{
		oRenderer.ReleaseConstantBuffer(l_oCb);
	}
	for (auto l_oCb : m_oPSConstantBuffers)
	{
		oRenderer.ReleaseConstantBuffer(l_oCb);
	}
}
