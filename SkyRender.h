////***************************************************************************************
//// SkyRender.h by X_Jun(MKXJun) (C) 2018-2020 All Rights Reserved.
//// Licensed under the MIT License.
////
//// 天空盒加载与渲染类
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
//	// 不允许拷贝，允许移动
//	SkyRender(const SkyRender&) = delete;
//	SkyRender& operator=(const SkyRender&) = delete;
//	SkyRender(SkyRender&&) = default;
//	SkyRender& operator=(SkyRender&&) = default;
//
//
//	// 需要提供完整的天空盒贴图 或者 已经创建好的天空盒纹理.dds文件
//	HRESULT InitResource(ID3D11Device* device,
//		ID3D11DeviceContext* deviceContext,
//		const std::wstring& cubemapFilename,
//		float skySphereRadius,		// 天空球半径
//		bool generateMips = false);	// 默认不为静态天空盒生成mipmaps
//
//	// 需要提供天空盒的六张正方形贴图
//	HRESULT InitResource(ID3D11Device* device,
//		ID3D11DeviceContext* deviceContext,
//		const std::vector<std::wstring>& cubemapFilenames,
//		float skySphereRadius,		// 天空球半径
//		bool generateMips = false);	// 默认不为静态天空盒生成mipmaps
//
//	ID3D11ShaderResourceView* GetTextureCube();
//
//	void Draw(ID3D11DeviceContext* deviceContext, SkyEffect& skyEffect, const Camera& camera);
//
//	// 设置调试对象名
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
//	// 获取单例
//	static SkyEffect& Get();
//
//	// 初始化所需资源
//	bool InitAll(ID3D11Device * device);
//
//	// 
//	// 渲染模式的变更
//	//
//
//	// 默认状态来绘制
//	void SetRenderDefault(ID3D11DeviceContext * deviceContext);
//
//	//
//	// 矩阵设置
//	//
//
//	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX W, DirectX::CXMMATRIX V, DirectX::CXMMATRIX P);
//	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX WVP);
//
//	//
//	// 纹理立方体映射设置
//	//
//
//	void SetTextureCube(ID3D11ShaderResourceView * textureCube);
//
//
//	// 应用常量缓冲区和纹理资源的变更
//	void Apply(ID3D11DeviceContext * deviceContext);
//
//private:
//	class Impl;
//	std::unique_ptr<Impl> pImpl;
//};
//
//#endif