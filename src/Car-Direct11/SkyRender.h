// 天空盒加载与渲染类

#ifndef SKYRENDER_H
#define SKYRENDER_H

#include <vector>
#include <string>
#include "Camera.h"
#include <wrl/client.h>
#include "RenderStates.h"
#include "LightHelper.h"

class BasicEffect
{
public:

	enum RenderType { RenderObject, RenderInstance };

	BasicEffect();
	virtual ~BasicEffect();

	BasicEffect(BasicEffect&& moveFrom) noexcept;
	BasicEffect& operator=(BasicEffect&& moveFrom) noexcept;

	// 获取单例
	static BasicEffect& Get();



	// 初始化所需资源
	bool InitAll(ID3D11Device * device);


	// 
	// 渲染模式的变更
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext * deviceContext, RenderType type);

	//
	// 矩阵设置
	//

	void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX W);
	void XM_CALLCONV SetViewMatrix(DirectX::FXMMATRIX V);
	void XM_CALLCONV SetProjMatrix(DirectX::FXMMATRIX P);

	//
	// 光照、材质和纹理相关设置
	//

	// 各种类型灯光允许的最大数目
	static const int maxLights = 5;

	void SetDirLight(size_t pos, const DirectionalLight& dirLight);
	void SetPointLight(size_t pos, const PointLight& pointLight);
	void SetSpotLight(size_t pos, const SpotLight& spotLight);

	void SetMaterial(const Material& material);


	void SetTextureUsed(bool isUsed);

	void SetTextureDiffuse(ID3D11ShaderResourceView * textureDiffuse);
	void SetTextureCube(ID3D11ShaderResourceView * textureCube);

	void XM_CALLCONV SetEyePos(DirectX::FXMVECTOR eyePos);

	//
	// 状态开关设置
	//

	void SetReflectionEnabled(bool isEnable);


	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext * deviceContext);

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};

class SkyEffect
{
public:
	SkyEffect();
	~SkyEffect();

	SkyEffect(SkyEffect&& moveFrom) noexcept;
	SkyEffect& operator=(SkyEffect&& moveFrom) noexcept;

	// 获取单例
	static SkyEffect& Get();

	// 初始化所需资源
	bool InitAll(ID3D11Device * device);

	// 
	// 渲染模式的变更
	//

	// 默认状态来绘制
	void SetRenderDefault(ID3D11DeviceContext * deviceContext);

	//
	// 矩阵设置
	//

	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX W, DirectX::CXMMATRIX V, DirectX::CXMMATRIX P);
	void XM_CALLCONV SetWorldViewProjMatrix(DirectX::FXMMATRIX WVP);

	//
	// 纹理立方体映射设置
	//

	void SetTextureCube(ID3D11ShaderResourceView * textureCube);


	// 应用常量缓冲区和纹理资源的变更
	void Apply(ID3D11DeviceContext * deviceContext);

private:
	class Impl;
	std::unique_ptr<Impl> pImpl;
};


class SkyRender
{
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	SkyRender() = default;
	~SkyRender() = default;
	// 不允许拷贝，允许移动
	SkyRender(const SkyRender&) = delete;
	SkyRender& operator=(const SkyRender&) = delete;
	SkyRender(SkyRender&&) = default;
	SkyRender& operator=(SkyRender&&) = default;


	// 需要提供完整的天空盒贴图 或者 已经创建好的天空盒纹理.dds文件
	HRESULT InitResource(ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		const std::wstring& cubemapFilename,
		float skySphereRadius,		// 天空球半径
		bool generateMips = false);	// 默认不为静态天空盒生成mipmaps

	// 需要提供天空盒的六张正方形贴图
	HRESULT InitResource(ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		const std::vector<std::wstring>& cubemapFilenames,
		float skySphereRadius,		// 天空球半径
		bool generateMips = false);	// 默认不为静态天空盒生成mipmaps

	ID3D11ShaderResourceView* GetTextureCube();

	void Draw(ID3D11DeviceContext* deviceContext, SkyEffect& skyEffect, const Camera& camera);

	// 设置调试对象名
	void SetDebugObjectName(const std::string& name);

private:
	HRESULT InitResource(ID3D11Device* device, float skySphereRadius);


private:
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;

	UINT m_IndexCount = 0;

	ComPtr<ID3D11ShaderResourceView> m_pTextureCubeSRV;
};

#endif