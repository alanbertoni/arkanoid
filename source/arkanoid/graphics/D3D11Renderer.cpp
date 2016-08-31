
#include "D3D11Renderer.h"
#include "Geometry.h"
#include "../utilities/String.h"
#include "../3rdParty/stb_image/stb_image.h"

#include <d3dcompiler.h>
#include <sstream>

graphics::D3D11Renderer::D3D11Renderer(core::WindowHandle oWndHandle)
	: IRenderer{}
	, m_oDriverType{ D3D_DRIVER_TYPE_NULL }
	, m_oFeatureLevel{ D3D_FEATURE_LEVEL_11_0 }
	, m_poD3dDevice{ nullptr }
	, m_poImmediateContext{ nullptr }
	, m_poSwapChain{ nullptr }
	, m_poRenderTargetView{ nullptr }
	, m_poDepthStencilBuffer{ nullptr }
	, m_poDepthStencilView{ nullptr }
	, m_oScreenViewport{}
	, m_oHWnd{ oWndHandle }
	, m_oMeshMap{}
	, m_oVertexShaderMap{}
	, m_oPixelShaderMap{}
	, m_oInputLayoutMap{}
	, m_oVertexShaderToInputLayoutID{}
	, m_oConstantBufferMap{}
	, m_oShaderResourceViewMap{}
	, m_pSamplerState{ nullptr }
	, m_oLastMeshId{}
	, m_oLastPixelShaderId{}
	, m_oLastTextureId{}
	, m_oLastVertexShaderId{}
{
	SetBackgroundColor(0, 0, 0, 255);
}

graphics::D3D11Renderer::~D3D11Renderer()
{
	Close();
}

bool graphics::D3D11Renderer::Init()
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(m_oHWnd, &rc);
	UINT _iWidth = rc.right - rc.left;
	UINT _iHeight = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _iWidth;
	sd.BufferDesc.Height = _iHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_oHWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_oDriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(NULL, m_oDriverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &m_poSwapChain, &m_poD3dDevice, &m_oFeatureLevel, &m_poImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create D3D11 Device and SwapChain");
		return false;
	}

	OnResize(_iWidth, _iHeight);

	m_poImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CreateDefaultSamplerState();
	LOG_INFO("D3D11 Renderer initialized successfully");
	return true;
}

bool graphics::D3D11Renderer::PreRender()
{
	m_poImmediateContext->ClearRenderTargetView(m_poRenderTargetView, m_afBackgroundColor);
	m_poImmediateContext->ClearDepthStencilView(m_poDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	return true;
}

bool graphics::D3D11Renderer::PostRender()
{
	HR(m_poSwapChain->Present(0, 0));
	return true;
}

void graphics::D3D11Renderer::Close()
{
	SAFE_RELEASE(m_pSamplerState);

	SAFE_RELEASE(m_poRenderTargetView);
	SAFE_RELEASE(m_poDepthStencilView);
	SAFE_RELEASE(m_poDepthStencilBuffer);

	SAFE_RELEASE(m_poSwapChain);
	if (m_poImmediateContext)
	{
		m_poImmediateContext->ClearState();
	}
	SAFE_RELEASE(m_poImmediateContext);
	SAFE_RELEASE(m_poD3dDevice);
}

void graphics::D3D11Renderer::OnResize(int iWidth, int iHeight)
{
	assert(m_poImmediateContext);
	assert(m_poD3dDevice);
	assert(m_poSwapChain);

	// Release the old views, as they hold references to the buffers we
	// will be destroying.  Also release the old depth/stencil buffer.
	SAFE_RELEASE(m_poRenderTargetView);
	SAFE_RELEASE(m_poDepthStencilView);
	SAFE_RELEASE(m_poDepthStencilBuffer);

	// Resize the swap chain and recreate the render target view.
	HR(m_poSwapChain->ResizeBuffers(1, iWidth, iHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D11Texture2D* backBuffer;
	HR(m_poSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(m_poD3dDevice->CreateRenderTargetView(backBuffer, 0, &m_poRenderTargetView));
	SAFE_RELEASE(backBuffer);

	// Create the depth/stencil buffer and view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = iWidth;
	depthStencilDesc.Height = iHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(m_poD3dDevice->CreateTexture2D(&depthStencilDesc, 0, &m_poDepthStencilBuffer));
	HR(m_poD3dDevice->CreateDepthStencilView(m_poDepthStencilBuffer, 0, &m_poDepthStencilView));

	// Bind the render target view and depth/stencil view to the pipeline.
	m_poImmediateContext->OMSetRenderTargets(1, &m_poRenderTargetView, m_poDepthStencilView);

	// Set the viewport transform.
	m_oScreenViewport.TopLeftX = 0;
	m_oScreenViewport.TopLeftY = 0;
	m_oScreenViewport.Width = static_cast<float>(iWidth);
	m_oScreenViewport.Height = static_cast<float>(iHeight);
	m_oScreenViewport.MinDepth = 0.0f;
	m_oScreenViewport.MaxDepth = 1.0f;

	m_poImmediateContext->RSSetViewports(1, &m_oScreenViewport);
}

float graphics::D3D11Renderer::GetAspectRatio() const
{
	return m_oScreenViewport.Width / m_oScreenViewport.Height;
}

float graphics::D3D11Renderer::GetScreenWidth() const
{
	return m_oScreenViewport.Width;
}

float graphics::D3D11Renderer::GetScreenHeight() const
{
	return m_oScreenViewport.Height;
}

void graphics::D3D11Renderer::SetBackgroundColor(unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha)
{
	m_afBackgroundColor[0] = static_cast<float>(ucRed) / 255.0f;
	m_afBackgroundColor[1] = static_cast<float>(ucGreen) / 255.0f;
	m_afBackgroundColor[2] = static_cast<float>(ucBlue) / 255.0f;
	m_afBackgroundColor[3] = static_cast<float>(ucAlpha) / 255.0f;
}

MeshId graphics::D3D11Renderer::CreateMesh(const MeshData & oMeshData)
{
	if (ExistsResource(m_oMeshMap, oMeshData.m_uiId))
	{
		AddReference(m_oMeshMap, oMeshData.m_uiId);
		return oMeshData.m_uiId;
	}

	Mesh l_oMesh;
	l_oMesh.m_uiIndecesCount = oMeshData.m_uiIndecesCount;
	l_oMesh.m_uiVertexStride = oMeshData.m_uiVertexStride;
	l_oMesh.m_uiVerticesCount = oMeshData.m_uiVerticesCount;

	HRESULT hr;
	// Create vertex buffer
	D3D11_BUFFER_DESC vb{};
	vb.Usage = D3D11_USAGE_DEFAULT;
	vb.ByteWidth = oMeshData.m_uiVertexStride * oMeshData.m_uiVerticesCount;
	vb.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vb.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vd{};
	vd.pSysMem = oMeshData.m_oVertices.data();
	hr = m_poD3dDevice->CreateBuffer(&vb, &vd, &l_oMesh.m_pVertexBuffer);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create the VertexBuffer");
		return MeshId{};
	}

	// Create index buffer
	D3D11_BUFFER_DESC ib{};
	ib.Usage = D3D11_USAGE_DEFAULT;
	ib.ByteWidth = sizeof(oMeshData.m_oIndices[0]) * oMeshData.m_uiIndecesCount;
	ib.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ib.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA id{};
	id.pSysMem = oMeshData.m_oIndices.data();
	hr = m_poD3dDevice->CreateBuffer(&ib, &id, &l_oMesh.m_pIndexBuffer);
	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create the IndexBuffer");
		return MeshId{};
	}

	m_oMeshMap.emplace(oMeshData.m_uiId, std::make_pair(1, l_oMesh));

	return oMeshData.m_uiId;
}

void graphics::D3D11Renderer::RenderMesh(MeshId oMeshId)
{
	auto& l_oPair = GetResource(m_oMeshMap, oMeshId);
	auto& l_oMesh = l_oPair.second;

	if (oMeshId != m_oLastMeshId)
	{
		UINT offset = 0;
		m_poImmediateContext->IASetVertexBuffers(0, 1, &l_oMesh.m_pVertexBuffer, &l_oMesh.m_uiVertexStride, &offset);
		m_poImmediateContext->IASetIndexBuffer(l_oMesh.m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
	}
	m_poImmediateContext->DrawIndexed(l_oMesh.m_uiIndecesCount, 0, 0);
}

void graphics::D3D11Renderer::ReleaseMesh(MeshId oMeshId)
{
	auto l_oIt = ReleaseResource(m_oMeshMap, oMeshId);
	auto& l_oPair = l_oIt->second;
	if (l_oPair.first < 1)
	{
		auto& l_oMesh = l_oPair.second;
		assert(l_oMesh.m_pVertexBuffer);
		l_oMesh.m_pVertexBuffer->Release();
		l_oMesh.m_pVertexBuffer = nullptr;

		assert(l_oMesh.m_pIndexBuffer);
		l_oMesh.m_pIndexBuffer->Release();
		l_oMesh.m_pIndexBuffer = nullptr;

		m_oMeshMap.erase(l_oIt);
	}
}

ShaderId graphics::D3D11Renderer::CreateVertexShaderFromFile(const std::string& szShaderFilename, EVertexType eVertexType)
{
	ShaderId l_oId = utilities::HashString(szShaderFilename);
	if (ExistsResource(m_oVertexShaderMap, l_oId))
	{
		AddReference(m_oVertexShaderMap, l_oId);
		return l_oId;
	}

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	if (!CompileShaderFromFile(EShaderType::VertexShader, szShaderFilename, &pVSBlob))
	{
		std::stringstream l_oErrorMessage;
		l_oErrorMessage << "Failed to compile shader file: " << szShaderFilename;
		LOG_ERROR(l_oErrorMessage.str());
		return ShaderId{};
	}

	// Create the vertex shader
	ID3D11VertexShader* l_pVertexShader = nullptr;
	HRESULT hr = S_OK;
	hr = m_poD3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &l_pVertexShader);
	if (FAILED(hr))
	{
		std::stringstream l_oErrorMessage;
		l_oErrorMessage << "Failed to create VertexShader from file: " << szShaderFilename;
		LOG_ERROR(l_oErrorMessage.str());
		SAFE_RELEASE(pVSBlob);
		return ShaderId{};
	}

	m_oVertexShaderMap.emplace(l_oId, std::make_pair(1, l_pVertexShader));

	InputLayoutId l_oInputLayoutId = CreateInputLayout(eVertexType, pVSBlob);

	m_oVertexShaderToInputLayoutID.emplace(l_oId, l_oInputLayoutId);

	return l_oId;
}

void graphics::D3D11Renderer::BindVertexShader(ShaderId oShaderId)
{
	if (oShaderId == m_oLastVertexShaderId)
	{
		return;
	}

	auto l_oPair = GetResource(m_oVertexShaderMap, oShaderId);
	BindInputLayout(oShaderId);
	m_poImmediateContext->VSSetShader(l_oPair.second, nullptr, 0);
	m_oLastVertexShaderId = oShaderId;
}

void graphics::D3D11Renderer::ReleaseVertexShader(ShaderId oShaderId)
{
	auto l_oIt = ReleaseResource(m_oVertexShaderMap, oShaderId);
	auto& l_oPair = l_oIt->second;
	if (l_oPair.first < 1)
	{
		ReleaseInputLayout(oShaderId);
		assert(l_oPair.second);
		l_oPair.second->Release();

		m_oVertexShaderMap.erase(l_oIt);
		m_oVertexShaderToInputLayoutID.erase(oShaderId);
	}
}

ShaderId graphics::D3D11Renderer::CreatePixelShaderFromFile(const std::string & szShaderFilename)
{
	ShaderId l_oId = utilities::HashString(szShaderFilename);
	if (ExistsResource(m_oPixelShaderMap, l_oId))
	{
		AddReference(m_oPixelShaderMap, l_oId);
		return l_oId;
	}

	// Compile the vertex shader
	ID3DBlob* pPSBlob = nullptr;
	if (!CompileShaderFromFile(EShaderType::PixelShader, szShaderFilename, &pPSBlob))
	{
		std::stringstream l_oErrorMessage;
		l_oErrorMessage << "Failed to compile shader file: " << szShaderFilename;
		LOG_ERROR(l_oErrorMessage.str());
		return ShaderId{};
	}

	// Create the pixel shader
	ID3D11PixelShader* l_pPixelShader = nullptr;
	HRESULT hr = S_OK;
	hr = m_poD3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &l_pPixelShader);
	SAFE_RELEASE(pPSBlob);
	if (FAILED(hr))
	{
		std::stringstream l_oErrorMessage;
		l_oErrorMessage << "Failed to create PixelShader from file: " << szShaderFilename;
		LOG_ERROR(l_oErrorMessage.str());
		return ShaderId{};
	}

	m_oPixelShaderMap.emplace(l_oId, std::make_pair(1, l_pPixelShader));

	return l_oId;
}

void graphics::D3D11Renderer::BindPixelShader(ShaderId oShaderId)
{
	if (oShaderId == m_oLastPixelShaderId)
	{
		return;
	}

	auto l_oPair = GetResource(m_oPixelShaderMap, oShaderId);
	m_poImmediateContext->PSSetShader(l_oPair.second, nullptr, 0);
	m_oLastPixelShaderId = oShaderId;
}

void graphics::D3D11Renderer::ReleasePixelShader(ShaderId oShaderId)
{
	auto l_oIt = ReleaseResource(m_oPixelShaderMap, oShaderId);
	auto& l_oPair = l_oIt->second;
	if (l_oPair.first < 1)
	{
		assert(l_oPair.second);
		l_oPair.second->Release();

		m_oPixelShaderMap.erase(l_oIt);
	}
}

CBufferId graphics::D3D11Renderer::CreateConstantBuffer(std::string szBufferName, size_t uiBufferSize)
{
	CBufferId l_oId = utilities::HashString(szBufferName);
	if (ExistsResource(m_oConstantBufferMap, l_oId))
	{
		AddReference(m_oConstantBufferMap, l_oId);
		return l_oId;
	}

	assert(uiBufferSize > 0 && "Buffer size is 0");

	size_t l_uiAlignMask = 15;
	size_t l_uiBufferSize = (uiBufferSize + l_uiAlignMask) & ~l_uiAlignMask;

	D3D11_BUFFER_DESC l_oBufferDesc{};
	l_oBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	l_oBufferDesc.ByteWidth = l_uiBufferSize;
	l_oBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	l_oBufferDesc.Usage = D3D11_USAGE_DYNAMIC;

	ID3D11Buffer* l_pConstantBuffer = nullptr;
	HRESULT hr = m_poD3dDevice->CreateBuffer(&l_oBufferDesc, nullptr, &l_pConstantBuffer);
	if (FAILED(hr))
	{
		std::stringstream l_oErrorMessage;
		l_oErrorMessage << "Failed to create ConstantBuffer: " << szBufferName;
		LOG_ERROR(l_oErrorMessage.str());
		return CBufferId{};
	}

	m_oConstantBufferMap.emplace(l_oId, std::make_pair(1, l_pConstantBuffer));

	return l_oId;
}

void graphics::D3D11Renderer::UpdateConstantBuffer(CBufferId oBufferId, const void * pConstantBufferData, size_t uiDataSize)
{
	auto& l_oPair = GetResource(m_oConstantBufferMap, oBufferId);

	D3D11_MAPPED_SUBRESOURCE l_oMappedConstantBuffer;
	HR(m_poImmediateContext->Map(l_oPair.second, 0, D3D11_MAP_WRITE_DISCARD, 0, &l_oMappedConstantBuffer));
	std::memcpy(l_oMappedConstantBuffer.pData, pConstantBufferData, uiDataSize);
	m_poImmediateContext->Unmap(l_oPair.second, 0);
}

void graphics::D3D11Renderer::BindConstantBuffers(EShaderType eShaderType, const std::vector<CBufferId>& oCBuffers)
{
	size_t l_uiBuffersCount = oCBuffers.size();
	std::vector<ConstantBuffer*> l_oCbuffers;
	l_oCbuffers.reserve(l_uiBuffersCount);

	auto l_oIt = oCBuffers.begin();
	while (l_oIt != oCBuffers.end())
	{
		auto l_oCb = GetResource(m_oConstantBufferMap, (*l_oIt));
		l_oCbuffers.push_back(l_oCb.second);
		++l_oIt;
	}

	switch (eShaderType)
	{
	case EShaderType::VertexShader:
	{
		m_poImmediateContext->VSSetConstantBuffers(0, l_uiBuffersCount, l_oCbuffers.data());
		break;
	}
	case EShaderType::PixelShader:
	{
		m_poImmediateContext->PSSetConstantBuffers(0, l_uiBuffersCount, l_oCbuffers.data());
		break;
	}
	}
}

void graphics::D3D11Renderer::ReleaseConstantBuffer(CBufferId oBufferId)
{
	auto l_oIt = ReleaseResource(m_oConstantBufferMap, oBufferId);
	auto& l_oPair = l_oIt->second;
	if (l_oPair.first < 1)
	{
		assert(l_oPair.second);
		l_oPair.second->Release();

		m_oConstantBufferMap.erase(l_oIt);
	}
}

TextureId graphics::D3D11Renderer::CreateTextureFromFile(const std::string& szImageFilename)
{
	TextureId l_oId = utilities::HashString(szImageFilename);
	if (ExistsResource(m_oShaderResourceViewMap, l_oId))
	{
		AddReference(m_oShaderResourceViewMap, l_oId);
		return l_oId;
	}

	int l_iWidth;
	int l_iHeight;
	int l_iComponentsCount;
	int l_iReqComponent = 4;
	unsigned char* l_pTextureData = stbi_load(szImageFilename.c_str(), &l_iWidth, &l_iHeight, &l_iComponentsCount, l_iReqComponent);

	if (!l_pTextureData)
	{
		std::stringstream l_oErrorMessage;
		l_oErrorMessage << "Error while trying to load texture: " << szImageFilename;
		LOG_ERROR(l_oErrorMessage.str());
		return TextureId{};
	}

	UINT l_uiWidth = static_cast<UINT>(l_iWidth);
	UINT l_uiHeight = static_cast<UINT>(l_iHeight);

	D3D11_TEXTURE2D_DESC l_oTextureDesc{};
	l_oTextureDesc.ArraySize = 1;
	l_oTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	l_oTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	l_oTextureDesc.Width = l_uiWidth;
	l_oTextureDesc.Height = l_uiHeight;
	l_oTextureDesc.MipLevels = 1;
	l_oTextureDesc.SampleDesc.Count = 1;
	l_oTextureDesc.Usage = D3D11_USAGE_IMMUTABLE;


	D3D11_SUBRESOURCE_DATA l_oTextureMostDetailedMipData{};
	l_oTextureMostDetailedMipData.pSysMem = l_pTextureData;
	l_oTextureMostDetailedMipData.SysMemPitch = sizeof(unsigned char) * l_iReqComponent * l_uiWidth;
	l_oTextureMostDetailedMipData.SysMemSlicePitch = l_oTextureMostDetailedMipData.SysMemPitch * l_uiHeight;

	ID3D11Texture2D* l_pTexture = nullptr;
	HR(m_poD3dDevice->CreateTexture2D(&l_oTextureDesc, &l_oTextureMostDetailedMipData, &l_pTexture));

	D3D11_SHADER_RESOURCE_VIEW_DESC l_oSrvDesc{};
	l_oSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	l_oSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	l_oSrvDesc.Texture2D.MostDetailedMip = 0;
	l_oSrvDesc.Texture2D.MipLevels = static_cast<UINT>(-1);

	ID3D11ShaderResourceView* l_pTexture2DSRV = nullptr;
	HR(m_poD3dDevice->CreateShaderResourceView(l_pTexture, &l_oSrvDesc, &l_pTexture2DSRV));

	SAFE_RELEASE(l_pTexture);

	m_oShaderResourceViewMap.emplace(l_oId, std::make_pair(1, l_pTexture2DSRV));

	return l_oId;
}

void graphics::D3D11Renderer::BindShaderResource(TextureId oTextureId)
{
	if (oTextureId == m_oLastTextureId)
	{
		return;
	}

	auto l_oPair = GetResource(m_oShaderResourceViewMap, oTextureId);
	m_poImmediateContext->PSSetShaderResources(0, 1, &l_oPair.second);
	m_oLastTextureId = oTextureId;
}

void graphics::D3D11Renderer::ReleaseShaderResource(TextureId oTextureId)
{
	auto l_oIt = ReleaseResource(m_oShaderResourceViewMap, oTextureId);
	auto& l_oPair = l_oIt->second;
	if (l_oPair.first < 1)
	{
		assert(l_oPair.second);
		l_oPair.second->Release();

		m_oShaderResourceViewMap.erase(l_oIt);
	}
}

bool graphics::D3D11Renderer::CompileShaderFromFile(EShaderType eShaderType, std::string szFilename, ID3DBlob** ppOutBlob)
{
	HRESULT hr = S_OK;

	DWORD l_dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

#if defined( DEBUG ) || defined( _DEBUG )
	l_dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* l_pErrorBlob = nullptr;

	switch (eShaderType)
	{
	case graphics::EShaderType::VertexShader:
	{
		std::wstring l_szFilename{ szFilename.begin(), szFilename.end() };
		hr = D3DCompileFromFile(l_szFilename.c_str(), NULL, NULL, "VS", "vs_5_0", l_dwShaderFlags, 0, ppOutBlob, &l_pErrorBlob);
		break;
	}
	case graphics::EShaderType::PixelShader:
	{
		std::wstring l_szFilename{ szFilename.begin(), szFilename.end() };
		hr = D3DCompileFromFile(l_szFilename.c_str(), NULL, NULL, "PS", "ps_5_0", l_dwShaderFlags, 0, ppOutBlob, &l_pErrorBlob);
		break;
	}
	default:
	{
		LOG_ERROR("Unknown Shader Type");
		break;
	}
	}

	if (FAILED(hr))
	{
		if (l_pErrorBlob != nullptr)
		{
			char* l_pErrorMessage = reinterpret_cast<char*>(l_pErrorBlob->GetBufferPointer());
			LOG_ERROR(l_pErrorMessage);
		}
		SAFE_RELEASE(l_pErrorBlob);
		return false;
	}

	if (l_pErrorBlob != nullptr)
	{
		char* l_pErrorMessage = reinterpret_cast<char*>(l_pErrorBlob->GetBufferPointer());
		LOG_ERROR(l_pErrorMessage);
	}
	SAFE_RELEASE(l_pErrorBlob);
	return true;
}

InputLayoutId graphics::D3D11Renderer::CreateInputLayout(EVertexType eVertexType, ID3DBlob* pCompiledShader)
{
	InputLayoutId l_oId = static_cast<InputLayoutId>(eVertexType);
	if (ExistsResource(m_oInputLayoutMap, l_oId))
	{
		AddReference(m_oInputLayoutMap, l_oId);
		return l_oId;
	}

	InputLayout* l_pInputLayout = nullptr;
	HRESULT hr = S_OK;

	switch (eVertexType)
	{
	case graphics::EVertexType::POSITION_TEXCOORD:
	{
		l_oId = static_cast<InputLayoutId>(graphics::EVertexType::POSITION_TEXCOORD);
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		hr = m_poD3dDevice->CreateInputLayout(layout, numElements, pCompiledShader->GetBufferPointer(), pCompiledShader->GetBufferSize(), &l_pInputLayout);
		break;
	}
	default:
	{
		assert("Unknown Vertex Type");
		break;
	}
	}

	SAFE_RELEASE(pCompiledShader);

	if (FAILED(hr))
	{
		LOG_ERROR("Failed to create the InputLayout");
		return InputLayoutId{};
	}

	m_oInputLayoutMap.emplace(l_oId, std::make_pair(1, l_pInputLayout));

	return l_oId;
}

void graphics::D3D11Renderer::BindInputLayout(ShaderId oShaderId)
{
	auto l_oInputLayoutId = GetResource(m_oVertexShaderToInputLayoutID, oShaderId);
	auto l_oPair = GetResource(m_oInputLayoutMap, l_oInputLayoutId);
	m_poImmediateContext->IASetInputLayout(l_oPair.second);
}

void graphics::D3D11Renderer::ReleaseInputLayout(ShaderId oShaderId)
{
	auto l_oInputLayoutId = GetResource(m_oVertexShaderToInputLayoutID, oShaderId);
	auto l_oIt = ReleaseResource(m_oInputLayoutMap, l_oInputLayoutId);
	auto& l_oPair = l_oIt->second;
	if (l_oPair.first < 1)
	{
		assert(l_oPair.second);
		l_oPair.second->Release();

		m_oInputLayoutMap.erase(l_oIt);
	}
}

void graphics::D3D11Renderer::CreateDefaultSamplerState()
{
	D3D11_SAMPLER_DESC l_oSamplerDesc{};
	l_oSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	l_oSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	l_oSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	l_oSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	l_oSamplerDesc.MinLOD = 0.0f;
	l_oSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	l_oSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	HR(m_poD3dDevice->CreateSamplerState(&l_oSamplerDesc, &m_pSamplerState));

	m_poImmediateContext->PSSetSamplers(0, 1, &m_pSamplerState);
}
