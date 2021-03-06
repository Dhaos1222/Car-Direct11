#ifndef GAMEAPP_H
#define GAMEAPP_H

#include "d3dApp.h"
#include "Geometry.h"
#include "LightHelper.h"
#include "Camera.h"
#include "SkyRender.h"

class GameApp : public D3DApp
{
public:

	struct CBChangesEveryDrawing
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInvTranspose;
	};

	struct CBChangesEveryFrame
	{
		DirectX::XMMATRIX view;
		DirectX::XMFLOAT4 eyePos;
	};

	struct CBChangesOnResize
	{
		DirectX::XMMATRIX proj;
	};

	struct CBChangesRarely
	{
		DirectionalLight dirLight[10];
		PointLight pointLight[10];
		SpotLight spotLight[10];
		Material material;
		int numDirLight;
		int numPointLight;
		int numSpotLight;
		float pad;		// 打包保证16字节对齐
	};

	// 一个尽可能小的游戏对象类
	class GameObject
	{
	public:
		// 子物体类型
		enum class ObjectType { Default, Mixed };

		GameObject();

		// 获取位置
		DirectX::XMFLOAT3 GetPosition() const;
		// 设置缓冲区
		template<class VertexType, class IndexType>
		void SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData);
		// 设置纹理
		void SetTexture(ID3D11ShaderResourceView * texture);
		// 设置矩阵
		//void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
		void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world);
		// 基于模型空间坐标的世界矩阵
		void SetWorldMatrixFixed(const DirectX::XMFLOAT4X4& world);
		void XM_CALLCONV SetWorldMatrixFixed(DirectX::FXMMATRIX world);
		void SetModelMatrix(const DirectX::XMFLOAT4X4& world);
		void XM_CALLCONV SetModelMatrix(DirectX::FXMMATRIX world);
		// 获取坐标矩阵
		DirectX::XMFLOAT4X4 GetPositionM() const;
		// 旋转
		void Rotation(float rad);
		// 绘制
		void Draw(ID3D11DeviceContext * deviceContext);
		void Draw(ID3D11DeviceContext * deviceContext, BasicEffect & effect);
		// 设置物体类型
		void SetObjectType(ObjectType type);
		ObjectType GetObjectType();
		// 设置对象运动状态
		void SetMoveState(float state);
		float GetMoveState();

		// 设置调试对象名
		// 若缓冲区被重新设置，调试对象名也需要被重新设置
		void SetDebugObjectName(const std::string& name);
	private:

		DirectX::XMFLOAT4X4 m_WorldMatrix;				    // 世界矩阵
		DirectX::XMFLOAT4X4 m_ModelMatrix;					// 模型空间矩阵

		ComPtr<ID3D11ShaderResourceView> m_pTexture;		// 纹理
		ComPtr<ID3D11Buffer> m_pVertexBuffer;				// 顶点缓冲区
		ComPtr<ID3D11Buffer> m_pIndexBuffer;				// 索引缓冲区
		UINT m_VertexStride;								// 顶点字节大小
		UINT m_IndexCount;								    // 索引数目	
		Material material;

		ObjectType o_type;		     // 对象类型
		float m_rotation;				// 转向角度
		float moveFlag;              // 移动状态
	};

	// 一个尽可能小的游戏对象控制类
	class GameObjectD
	{
	public:
		GameObjectD();

		// 获取位置
		DirectX::XMFLOAT3 GetPosition() const;
		// 设置矩阵
		//void SetWorldMatrix(const DirectX::XMFLOAT4X4& world);
		void XM_CALLCONV SetWorldMatrix(DirectX::FXMMATRIX world);
		// 获取坐标矩阵
		DirectX::XMFLOAT4X4 GetPositionM() const;
		// 旋转(原地)
		void Rotation(float rad);
		// 绘制
		void Draw(ID3D11DeviceContext * deviceContext, BasicEffect & effect);
		// 添加子对象
		void AddObj(GameObject &gameObject);
		// 设置对象运动状态
		void SetMoveState(float state);
		float GetMoveState();
	private:
		DirectX::XMFLOAT4X4 m_WorldMatrix;				    // 世界矩阵
		std::vector<GameObject> gameObjs;  //所有的子对象
		float m_rotation;				//转向角度
		float moveFlag;
	};

	// 摄像机模式
	enum class CameraMode { FirstPerson, ThirdPerson, Free };
	
public:
	GameApp(HINSTANCE hInstance);
	~GameApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	bool InitResource();

private:
	
	ComPtr<ID2D1SolidColorBrush> m_pColorBrush;				    // 单色笔刷
	ComPtr<IDWriteFont> m_pFont;								// 字体
	ComPtr<IDWriteTextFormat> m_pTextFormat;					// 文本格式

	ComPtr<ID3D11InputLayout> m_pVertexLayout2D;				// 用于2D的顶点输入布局
	ComPtr<ID3D11InputLayout> m_pVertexLayout3D;				// 用于3D的顶点输入布局
	ComPtr<ID3D11Buffer> m_pConstantBuffers[4];				    // 常量缓冲区

	GameObjectD m_Car;									    // 车车
	GameObject m_Body;										// 车身
	std::vector<GameObject> m_Wheels;			//轮子
	GameObject m_Floor;										    // 地板
	std::vector<GameObject> m_Walls;							// 墙壁

	ComPtr<ID3D11VertexShader> m_pVertexShader3D;				// 用于3D的顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader3D;				    // 用于3D的像素着色器
	ComPtr<ID3D11VertexShader> m_pVertexShader2D;				// 用于2D的顶点着色器
	ComPtr<ID3D11PixelShader> m_pPixelShader2D;				    // 用于2D的像素着色器

	CBChangesEveryFrame m_CBFrame;							    // 该缓冲区存放仅在每一帧进行更新的变量
	CBChangesOnResize m_CBOnResize;							    // 该缓冲区存放仅在窗口大小变化时更新的变量
	CBChangesRarely m_CBRarely;								    // 该缓冲区存放不会再进行修改的变量

	ComPtr<ID3D11SamplerState> m_pSamplerState;				    // 采样器状态

	std::shared_ptr<Camera> m_pCamera;						    // 摄像机
	CameraMode m_CameraMode;									// 摄像机模式

	BasicEffect m_BasicEffect;								    // 对象渲染特效管理
	SkyEffect m_SkyEffect;									    // 天空盒特效管理
	std::unique_ptr<SkyRender> m_pSunset; // 天空盒
};


#endif