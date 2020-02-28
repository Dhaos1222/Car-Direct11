////***************************************************************************************
//// SkyRender.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
//// Licensed under the MIT License.
////
//// ��պм�������Ⱦ��
//// Skybox loader and render classes.
////***************************************************************************************
//
//#ifndef SKYRENDER_H
//#define SKYRENDER_H
//
//#include <vector>
//#include <string>
//#include "Camera.h"
//#include <wrl/client.h>
//
//
//class SkyRender
//{
//public:
//	template<class T>
//	using ComPtr = Microsoft::WRL::ComPtr<T>;
//
//	SkyRender() = default;
//	~SkyRender() = default;
//	// ���������������ƶ�
//	SkyRender(const SkyRender&) = delete;
//	SkyRender& operator=(const SkyRender&) = delete;
//	SkyRender(SkyRender&&) = default;
//	SkyRender& operator=(SkyRender&&) = default;
//
//
//	// ��Ҫ�ṩ��������պ���ͼ ���� �Ѿ������õ���պ�����.dds�ļ�
//	HRESULT InitResource(ID3D11Device* device,
//		ID3D11DeviceContext* deviceContext,
//		const std::wstring& cubemapFilename,
//		float skySphereRadius,		// �����뾶
//		bool generateMips = false);	// Ĭ�ϲ�Ϊ��̬��պ�����mipmaps
//
//	// ��Ҫ�ṩ��պе�������������ͼ
//	HRESULT InitResource(ID3D11Device* device,
//		ID3D11DeviceContext* deviceContext,
//		const std::vector<std::wstring>& cubemapFilenames,
//		float skySphereRadius,		// �����뾶
//		bool generateMips = false);	// Ĭ�ϲ�Ϊ��̬��պ�����mipmaps
//
//	ID3D11ShaderResourceView* GetTextureCube();
//
//	void Draw(ID3D11DeviceContext* deviceContext, SkyEffect& skyEffect, const Camera& camera);
//
//	// ���õ��Զ�����
//	void SetDebugObjectName(const std::string& name);
//
//private:
//	HRESULT InitResource(ID3D11Device* device, float skySphereRadius);
//
//
//private:
//	ComPtr<ID3D11Buffer> m_pVertexBuffer;
//	ComPtr<ID3D11Buffer> m_pIndexBuffer;
//
//	UINT m_IndexCount = 0;
//
//	ComPtr<ID3D11ShaderResourceView> m_pTextureCubeSRV;
//};
//
//class SkyEffect
//{
//public:
//	SkyEffect();
//	~SkyEffect();
//
//	SkyEffect(SkyEffect&& moveFrom) noexcept;
//	SkyEffect& operator=(SkyEffect&& moveFrom) noexcept;
//
//	// ��ȡ����
//	static SkyEffect& Get();
//
//	// ��ʼ��������Դ
//	bool InitAll(ID3D11Device * device);
//
//	// 
//	// ��Ⱦģʽ�ı��
//	//
//
//	// Ĭ��״̬������
//	void SetRenderDefault(ID3D11DeviceContext * deviceContext);
//
//	//
//	// ��������
//	//
//
//	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX W, DirectX::CXMMATRIX V, DirectX::CXMMATRIX P);
//	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX WVP);
//
//	//
//	// ����������ӳ������
//	//
//
//	void SetTextureCube(ID3D11ShaderResourceView * textureCube);
//
//
//	// Ӧ�ó�����������������Դ�ı��
//	void Apply(ID3D11DeviceContext * deviceContext);
//
//private:
//	class Impl;
//	std::unique_ptr<Impl> pImpl;
//};
//
//#endif