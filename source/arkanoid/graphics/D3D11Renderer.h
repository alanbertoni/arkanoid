#pragma once

#include "D3D11Includes.h"
#include "IRenderer.h"
#include "Mesh.h"
#include "../core/Common.h"

namespace graphics
{
	class D3D11Renderer : public IRenderer
	{
	public:
		D3D11Renderer(core::WindowHandle oWndHandle);
		D3D11Renderer(const D3D11Renderer&) = delete;
		D3D11Renderer(D3D11Renderer&&) = delete;
		D3D11Renderer& operator=(const D3D11Renderer&) = delete;
		D3D11Renderer& operator=(D3D11Renderer&&) = delete;

		virtual ~D3D11Renderer();
		virtual bool Init() override;
		virtual bool PreRender() override;
		virtual bool PostRender() override;
		virtual void Close() override;
		virtual void OnResize(int iWidth, int iHeight) override;

		virtual float GetAspectRatio() const override;
		virtual float GetScreenWidth() const override;
		virtual float GetScreenHeight() const override;

		virtual void SetBackgroundColor(unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha) override;

		virtual MeshId CreateMesh(const MeshData& oMeshData) override;
		virtual void RenderMesh(MeshId oMeshId) override;
		virtual void ReleaseMesh(MeshId oMeshId) override;

		virtual ShaderId CreateVertexShaderFromFile(const std::string& szShaderFilename, EVertexType eVertexType) override;
		virtual void BindVertexShader(ShaderId oShaderId) override;
		virtual void ReleaseVertexShader(ShaderId oShaderId) override;

		virtual ShaderId CreatePixelShaderFromFile(const std::string& szShaderFilename) override;
		virtual void BindPixelShader(ShaderId oShaderId) override;
		virtual void ReleasePixelShader(ShaderId oShaderId) override;

		virtual CBufferId CreateConstantBuffer(std::string szBufferName, size_t uiBufferSize) override;
		virtual void UpdateConstantBuffer(CBufferId oBufferId, const void* pConstantBufferData, size_t uiDataSize) override;
		virtual void BindConstantBuffers(EShaderType eShaderType, const std::vector<CBufferId>& oCBuffers) override;
		virtual void ReleaseConstantBuffer(CBufferId oBufferId) override;

		virtual TextureId CreateTextureFromFile(const std::string& szImageFilename) override;
		virtual void BindShaderResource(TextureId oTextureId) override;
		virtual void ReleaseShaderResource(TextureId oTextureId) override;

	private:
		template<typename K, typename V>
		using ResourceMap = std::map<K, V>;

		template<typename K, typename V>
		using ResourceMapIt = typename ResourceMap<K, V>::iterator;

		template<typename K, typename V>
		using ResourceMapConstIt = typename ResourceMap<K, V>::const_iterator;

		template<typename K, typename V>
		bool ExistsResource(const ResourceMap<K, V>& oResourceMap, const K& oKey);

		template<typename K, typename V>
		void AddReference(ResourceMap<K, V>& oResourceMap, const K& oKey);

		template<typename K, typename V>
		ResourceMapIt<K, V> ReleaseResource(ResourceMap<K, V>& oResourceMap, const K& oKey);

		template<typename K, typename V>
		ResourceMapIt<K, V> EditResource(ResourceMap<K, V>& oResourceMap, const K& oKey);

		template<typename K, typename V>
		V GetResource(ResourceMap<K, V>& oResourceMap, const K& oKey);

		bool CompileShaderFromFile(EShaderType eShaderType, std::string szFilename, ID3DBlob** ppOutBlob);

		InputLayoutId CreateInputLayout(EVertexType eVertexType, ID3DBlob* pCompiledShader);
		void BindInputLayout(ShaderId oShaderId);
		void ReleaseInputLayout(ShaderId oShaderId);

		void CreateDefaultSamplerState();

		core::WindowHandle m_oHWnd;

		D3D_DRIVER_TYPE m_oDriverType;
		D3D_FEATURE_LEVEL m_oFeatureLevel;

		ID3D11Device* m_poD3dDevice;
		ID3D11DeviceContext* m_poImmediateContext;
		IDXGISwapChain* m_poSwapChain;

		ID3D11RenderTargetView* m_poRenderTargetView;

		ID3D11Texture2D* m_poDepthStencilBuffer;
		ID3D11DepthStencilView* m_poDepthStencilView;

		D3D11_VIEWPORT m_oScreenViewport;
		float m_afBackgroundColor[4];

		ID3D11SamplerState* m_pSamplerState;

		ResourceMap<MeshId, std::pair<size_t, Mesh>> m_oMeshMap;
		ResourceMap<ShaderId, std::pair<size_t, VertexShader*>> m_oVertexShaderMap;
		ResourceMap<ShaderId, std::pair<size_t, PixelShader*>> m_oPixelShaderMap;
		ResourceMap<InputLayoutId, std::pair<size_t, InputLayout*>> m_oInputLayoutMap;
		ResourceMap<ShaderId, InputLayoutId> m_oVertexShaderToInputLayoutID;
		ResourceMap<CBufferId, std::pair<size_t, ConstantBuffer*>> m_oConstantBufferMap;
		ResourceMap<TextureId, std::pair<size_t, ShaderResourceView*>> m_oShaderResourceViewMap;

		MeshId m_oLastMeshId;
		ShaderId m_oLastVertexShaderId;
		ShaderId m_oLastPixelShaderId;
		TextureId m_oLastTextureId;
	};

	template<typename K, typename V>
	inline bool D3D11Renderer::ExistsResource(const ResourceMap<K, V>& oResourceMap, const K& oKey)
	{
		auto l_oIt = oResourceMap.find(oKey);
		return l_oIt != oResourceMap.end();
	}

	template<typename K, typename V>
	inline void D3D11Renderer::AddReference(ResourceMap<K, V>& oResourceMap, const K& oKey)
	{
		auto l_oIt = oResourceMap.find(oKey);
		auto& l_oPair = l_oIt->second;
		++l_oPair.first;
	}

	template<typename K, typename V>
	inline D3D11Renderer::ResourceMapIt<K, V> D3D11Renderer::ReleaseResource(ResourceMap<K, V>& oResourceMap, const K& oKey)
	{
		auto l_oIt = oResourceMap.find(oKey);
		assert(l_oIt != oResourceMap.end());
		auto& l_oPair = l_oIt->second;
		--l_oPair.first;
		return l_oIt;
	}

	template<typename K, typename V>
	inline D3D11Renderer::ResourceMapIt<K, V> D3D11Renderer::EditResource(ResourceMap<K, V>& oResourceMap, const K& oKey)
	{
		auto l_oIt = oResourceMap.find(oKey);
		assert(l_oIt != oResourceMap.end());
		return l_oIt;
	}

	template<typename K, typename V>
	inline V D3D11Renderer::GetResource(ResourceMap<K, V>& oResourceMap, const K& oKey)
	{
		ResourceMapIt<K, V> l_oIt = oResourceMap.find(oKey);
		assert(l_oIt != oResourceMap.end());
		return l_oIt->second;
	}
}