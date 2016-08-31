#pragma once

#include "Geometry.h"
#include "ShaderEnum.h"
#include "../utilities/Types.h"

namespace graphics
{
	class IRenderer
	{
	public:
		IRenderer() = default;
		IRenderer(const IRenderer&) = delete;
		IRenderer(IRenderer&&) = delete;
		IRenderer& operator=(const IRenderer&) = delete;
		IRenderer& operator=(IRenderer&&) = delete;

		virtual ~IRenderer() = default;
		virtual bool Init() = 0;
		virtual bool PreRender() = 0;
		virtual bool PostRender() = 0;
		virtual void Close() = 0;
		virtual void OnResize(int iWidth, int iHeight) = 0;

		virtual float GetAspectRatio() const = 0;
		virtual float GetScreenWidth() const = 0;
		virtual float GetScreenHeight() const = 0;

		virtual void SetBackgroundColor(unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha) = 0;

		virtual MeshId CreateMesh(const MeshData& oMeshData) = 0;
		virtual void RenderMesh(MeshId oMeshId) = 0;
		virtual void ReleaseMesh(MeshId oMeshId) = 0;

		virtual ShaderId CreateVertexShaderFromFile(const std::string& szShaderFilename, EVertexType eVertexType) = 0;
		virtual void BindVertexShader(ShaderId oShaderId) = 0;
		virtual void ReleaseVertexShader(ShaderId oShaderId) = 0;

		virtual ShaderId CreatePixelShaderFromFile(const std::string& szShaderFilename) = 0;
		virtual void BindPixelShader(ShaderId oShaderId) = 0;
		virtual void ReleasePixelShader(ShaderId oShaderId) = 0;

		virtual CBufferId CreateConstantBuffer(std::string szBufferName, size_t uiBufferSize) = 0;
		virtual void UpdateConstantBuffer(CBufferId oBufferId, const void* pConstantBufferData, size_t uiDataSize) = 0;
		virtual void BindConstantBuffers(EShaderType eShaderType, const std::vector<CBufferId>& oCBuffers) = 0;
		virtual void ReleaseConstantBuffer(CBufferId oBufferId) = 0;

		virtual TextureId CreateTextureFromFile(const std::string& szImageFilename) = 0;
		virtual void BindShaderResource(TextureId oTextureId) = 0;
		virtual void ReleaseShaderResource(TextureId oTextureId) = 0;
	};
}