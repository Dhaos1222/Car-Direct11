//#include "SkyRender.h"
//#include "Geometry.h"
//#include "d3dUtil.h"
//
//#pragma warning(disable: 26812)
//
//using namespace DirectX;
//using namespace Microsoft::WRL;
//
//HRESULT SkyRender::InitResource(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::wstring& cubemapFilename, float skySphereRadius, bool generateMips)
//{
//	// ��ֹ�ظ���ʼ������ڴ�й©
//	m_pIndexBuffer.Reset();
//	m_pVertexBuffer.Reset();
//	m_pTextureCubeSRV.Reset();
//
//	HRESULT hr;
//	// ��պ��������
//	if (cubemapFilename.substr(cubemapFilename.size() - 3) == L"dds")
//	{
//		hr = CreateDDSTextureFromFile(device,
//			generateMips ? deviceContext : nullptr,
//			cubemapFilename.c_str(),
//			nullptr,
//			m_pTextureCubeSRV.GetAddressOf());
//	}
//	else
//	{
//		hr = CreateWICTexture2DCubeFromFile(device,
//			deviceContext,
//			cubemapFilename,
//			nullptr,
//			m_pTextureCubeSRV.GetAddressOf(),
//			generateMips);
//	}
//
//	if (FAILED(hr))
//		return hr;
//
//	return InitResource(device, skySphereRadius);
//}
//
//HRESULT SkyRender::InitResource(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<std::wstring>& cubemapFilenames, float skySphereRadius, bool generateMips)
//{
//	// ��ֹ�ظ���ʼ������ڴ�й©
//	m_pIndexBuffer.Reset();
//	m_pVertexBuffer.Reset();
//	m_pTextureCubeSRV.Reset();
//
//	HRESULT hr;
//	// ��պ��������
//	hr = CreateWICTexture2DCubeFromFile(device,
//		deviceContext,
//		cubemapFilenames,
//		nullptr,
//		m_pTextureCubeSRV.GetAddressOf(),
//		generateMips);
//	if (FAILED(hr))
//		return hr;
//
//	return InitResource(device, skySphereRadius);
//}
//
//ID3D11ShaderResourceView* SkyRender::GetTextureCube()
//{
//	return m_pTextureCubeSRV.Get();
//}
//
//void SkyRender::Draw(ID3D11DeviceContext* deviceContext, SkyEffect& skyEffect, const Camera& camera)
//{
//	UINT strides[1] = { sizeof(XMFLOAT3) };
//	UINT offsets[1] = { 0 };
//	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), strides, offsets);
//	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//	XMFLOAT3 pos = camera.GetPosition();
//	skyEffect.SetWorldViewProjMatrix(XMMatrixTranslation(pos.x, pos.y, pos.z) * camera.GetViewProjXM());
//	skyEffect.SetTextureCube(m_pTextureCubeSRV.Get());
//	skyEffect.Apply(deviceContext);
//	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
//}
//
//void SkyRender::SetDebugObjectName(const std::string& name)
//{
//#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
//	// ����տ��ܴ��ڵ�����
//	D3D11SetDebugObjectName(m_pTextureCubeSRV.Get(), nullptr);
//
//	D3D11SetDebugObjectName(m_pTextureCubeSRV.Get(), name + ".CubeMapSRV");
//	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
//	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
//#else
//	UNREFERENCED_PARAMETER(name);
//#endif
//}
//
//HRESULT SkyRender::InitResource(ID3D11Device* device, float skySphereRadius)
//{
//	HRESULT hr;
//	auto sphere = Geometry::CreateSphere<VertexPos>(skySphereRadius);
//
//	// ���㻺��������
//	D3D11_BUFFER_DESC vbd;
//	vbd.Usage = D3D11_USAGE_IMMUTABLE;
//	vbd.ByteWidth = sizeof(XMFLOAT3) * (UINT)sphere.vertexVec.size();
//	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vbd.CPUAccessFlags = 0;
//	vbd.MiscFlags = 0;
//	vbd.StructureByteStride = 0;
//
//	D3D11_SUBRESOURCE_DATA InitData;
//	InitData.pSysMem = sphere.vertexVec.data();
//
//	hr = device->CreateBuffer(&vbd, &InitData, &m_pVertexBuffer);
//	if (FAILED(hr))
//		return hr;
//
//	// ��������������
//	m_IndexCount = (UINT)sphere.indexVec.size();
//
//	D3D11_BUFFER_DESC ibd;
//	ibd.Usage = D3D11_USAGE_IMMUTABLE;
//	ibd.ByteWidth = sizeof(DWORD) * m_IndexCount;
//	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	ibd.CPUAccessFlags = 0;
//	ibd.StructureByteStride = 0;
//	ibd.MiscFlags = 0;
//
//	InitData.pSysMem = sphere.indexVec.data();
//
//	return device->CreateBuffer(&ibd, &InitData, &m_pIndexBuffer);
//}
//
////
//// SkyEffect::Impl ��Ҫ����SkyEffect�Ķ���
////
//
//class SkyEffect::Impl : public AlignedType<SkyEffect::Impl>
//{
//public:
//	//
//	// ��Щ�ṹ���ӦHLSL�Ľṹ�塣��Ҫ��16�ֽڶ���
//	//
//
//	struct CBChangesEveryFrame
//	{
//		DirectX::XMMATRIX worldViewProj;
//	};
//
//public:
//	// ������ʽָ��
//	Impl() : m_IsDirty() {}
//	~Impl() = default;
//
//public:
//	CBufferObject<0, CBChangesEveryFrame>	m_CBFrame;	        // ÿ֡���Ƶĳ���������
//
//	BOOL m_IsDirty;										        // �Ƿ���ֵ���
//	std::vector<CBufferBase*> m_pCBuffers;				        // ͳһ�����������еĳ���������
//
//	ComPtr<ID3D11VertexShader> m_pSkyVS;
//	ComPtr<ID3D11PixelShader> m_pSkyPS;
//
//	ComPtr<ID3D11InputLayout> m_pVertexPosLayout;
//
//	ComPtr<ID3D11ShaderResourceView> m_pTextureCube;			// ��պ�����
//};
//
////
//// SkyEffect
////
//
//namespace
//{
//	// SkyEffect����
//	static SkyEffect * g_pInstance = nullptr;
//}
//
//SkyEffect::SkyEffect()
//{
//	if (g_pInstance)
//		throw std::exception("SkyEffect is a singleton!");
//	g_pInstance = this;
//	pImpl = std::make_unique<SkyEffect::Impl>();
//}
//
//SkyEffect::~SkyEffect()
//{
//}
//
//SkyEffect::SkyEffect(SkyEffect && moveFrom) noexcept
//{
//	pImpl.swap(moveFrom.pImpl);
//}
//
//SkyEffect & SkyEffect::operator=(SkyEffect && moveFrom) noexcept
//{
//	pImpl.swap(moveFrom.pImpl);
//	return *this;
//}
//
//SkyEffect & SkyEffect::Get()
//{
//	if (!g_pInstance)
//		throw std::exception("SkyEffect needs an instance!");
//	return *g_pInstance;
//}
//
//bool SkyEffect::InitAll(ID3D11Device * device)
//{
//	if (!device)
//		return false;
//
//	if (!pImpl->m_pCBuffers.empty())
//		return true;
//
//	if (!RenderStates::IsInit())
//		throw std::exception("RenderStates need to be initialized first!");
//
//	ComPtr<ID3DBlob> blob;
//
//	// ******************
//	// ����������ɫ��
//	//
//
//	HR(CreateShaderFromFile(L"HLSL\\Sky_VS.cso", L"HLSL\\Sky_VS.hlsl", "VS", "vs_5_0", blob.ReleaseAndGetAddressOf()));
//	HR(device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pSkyVS.GetAddressOf()));
//	// �������㲼��
//	HR(device->CreateInputLayout(VertexPos::inputLayout, ARRAYSIZE(VertexPos::inputLayout),
//		blob->GetBufferPointer(), blob->GetBufferSize(), pImpl->m_pVertexPosLayout.GetAddressOf()));
//
//	// ******************
//	// ����������ɫ��
//	//
//
//	HR(CreateShaderFromFile(L"HLSL\\Sky_PS.cso", L"HLSL\\Sky_PS.hlsl", "PS", "ps_5_0", blob.ReleaseAndGetAddressOf()));
//	HR(device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, pImpl->m_pSkyPS.GetAddressOf()));
//
//
//	pImpl->m_pCBuffers.assign({
//		&pImpl->m_CBFrame,
//		});
//
//	// ��������������
//	for (auto& pBuffer : pImpl->m_pCBuffers)
//	{
//		HR(pBuffer->CreateBuffer(device));
//	}
//
//	// ���õ��Զ�����
//	D3D11SetDebugObjectName(pImpl->m_pVertexPosLayout.Get(), "SkyEffect.VertexPosLayout");
//	D3D11SetDebugObjectName(pImpl->m_pCBuffers[0]->cBuffer.Get(), "SkyEffect.CBFrame");
//	D3D11SetDebugObjectName(pImpl->m_pSkyVS.Get(), "SkyEffect.Sky_VS");
//	D3D11SetDebugObjectName(pImpl->m_pSkyPS.Get(), "SkyEffect.Sky_PS");
//
//	return true;
//}
//
//void SkyEffect::SetRenderDefault(ID3D11DeviceContext * deviceContext)
//{
//	deviceContext->IASetInputLayout(pImpl->m_pVertexPosLayout.Get());
//	deviceContext->VSSetShader(pImpl->m_pSkyVS.Get(), nullptr, 0);
//	deviceContext->PSSetShader(pImpl->m_pSkyPS.Get(), nullptr, 0);
//
//	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	deviceContext->GSSetShader(nullptr, nullptr, 0);
//	deviceContext->RSSetState(RenderStates::RSNoCull.Get());
//
//	deviceContext->PSSetSamplers(0, 1, RenderStates::SSLinearWrap.GetAddressOf());
//	deviceContext->OMSetDepthStencilState(RenderStates::DSSLessEqual.Get(), 0);
//	deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);
//}
//
//void XM_CALLCONV SkyEffect::SetWorldViewProjMatrix(DirectX::FXMMATRIX W, DirectX::CXMMATRIX V, DirectX::CXMMATRIX P)
//{
//	auto& cBuffer = pImpl->m_CBFrame;
//	cBuffer.data.worldViewProj = XMMatrixTranspose(W * V * P);
//	pImpl->m_IsDirty = cBuffer.isDirty = true;
//}
//
//void XM_CALLCONV SkyEffect::SetWorldViewProjMatrix(DirectX::FXMMATRIX WVP)
//{
//	auto& cBuffer = pImpl->m_CBFrame;
//	cBuffer.data.worldViewProj = XMMatrixTranspose(WVP);
//	pImpl->m_IsDirty = cBuffer.isDirty = true;
//}
//
//void SkyEffect::SetTextureCube(ID3D11ShaderResourceView * m_pTextureCube)
//{
//	pImpl->m_pTextureCube = m_pTextureCube;
//}
//
//void SkyEffect::Apply(ID3D11DeviceContext * deviceContext)
//{
//	auto& pCBuffers = pImpl->m_pCBuffers;
//	// ���������󶨵���Ⱦ������
//	pCBuffers[0]->BindVS(deviceContext);
//
//	// ����SRV
//	deviceContext->PSSetShaderResources(0, 1, pImpl->m_pTextureCube.GetAddressOf());
//
//	if (pImpl->m_IsDirty)
//	{
//		pImpl->m_IsDirty = false;
//		for (auto& pCBuffer : pCBuffers)
//		{
//			pCBuffer->UpdateBuffer(deviceContext);
//		}
//	}
//}