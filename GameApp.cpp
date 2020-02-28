#include "GameApp.h"
#include "d3dUtil.h"
#include "DXTrace.h"
using namespace DirectX;

GameApp::GameApp(HINSTANCE hInstance)
	: D3DApp(hInstance),
	m_CameraMode(CameraMode::FirstPerson),
	m_CBFrame(),
	m_CBOnResize(),
	m_CBRarely()
{
}

GameApp::~GameApp()
{
}

bool GameApp::Init()
{
	if (!D3DApp::Init())
		return false;

	// 务必先初始化所有渲染状态，以供下面的特效使用
	RenderStates::InitAll(m_pd3dDevice.Get());


	if (!m_BasicEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!m_SkyEffect.InitAll(m_pd3dDevice.Get()))
		return false;

	if (!InitResource())
		return false;

	// 初始化鼠标，键盘不需要
	m_pMouse->SetWindow(m_hMainWnd);
	m_pMouse->SetMode(DirectX::Mouse::MODE_RELATIVE);

	return true;
}

void GameApp::OnResize()
{
	assert(m_pd2dFactory);
	assert(m_pdwriteFactory);
	// 释放D2D的相关资源
	m_pColorBrush.Reset();
	m_pd2dRenderTarget.Reset();

	D3DApp::OnResize();

	// 为D2D创建DXGI表面渲染目标
	ComPtr<IDXGISurface> surface;
	HR(m_pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), reinterpret_cast<void**>(surface.GetAddressOf())));
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));
	HRESULT hr = m_pd2dFactory->CreateDxgiSurfaceRenderTarget(surface.Get(), &props, m_pd2dRenderTarget.GetAddressOf());
	surface.Reset();

	if (hr == E_NOINTERFACE)
	{
		OutputDebugStringW(L"\n警告：Direct2D与Direct3D互操作性功能受限，你将无法看到文本信息。现提供下述可选方法：\n"
			L"1. 对于Win7系统，需要更新至Win7 SP1，并安装KB2670838补丁以支持Direct2D显示。\n"
			L"2. 自行完成Direct3D 10.1与Direct2D的交互。详情参阅："
			L"https://docs.microsoft.com/zh-cn/windows/desktop/Direct2D/direct2d-and-direct3d-interoperation-overview""\n"
			L"3. 使用别的字体库，比如FreeType。\n\n");
	}
	else if (hr == S_OK)
	{
		// 创建固定颜色刷和文本格式
		HR(m_pd2dRenderTarget->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::White),
			m_pColorBrush.GetAddressOf()));
		HR(m_pdwriteFactory->CreateTextFormat(L"宋体", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 15, L"zh-cn",
			m_pTextFormat.GetAddressOf()));
	}
	else
	{
		// 报告异常问题
		assert(m_pd2dRenderTarget);
	}
	
	// 摄像机变更显示
	if (m_pCamera != nullptr)
	{
		m_pCamera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
		m_pCamera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
		m_BasicEffect.SetProjMatrix(m_pCamera->GetProjXM());
	}
}

void GameApp::UpdateScene(float dt)
{
	// 更新鼠标事件，获取相对偏移量
	Mouse::State mouseState = m_pMouse->GetState();
	Mouse::State lastMouseState = m_MouseTracker.GetLastState();

	Keyboard::State keyState = m_pKeyboard->GetState();
	m_KeyboardTracker.Update(keyState);

	// 获取子类
	auto cam1st = std::dynamic_pointer_cast<FirstPersonCamera>(m_pCamera);
	auto cam3rd = std::dynamic_pointer_cast<ThirdPersonCamera>(m_pCamera);

	m_pCamera->SetMoveState(0);

	if (m_CameraMode == CameraMode::FirstPerson || m_CameraMode == CameraMode::Free)
	{
		// 第一人称/自由摄像机的操作
		cam1st->SetTargetM(m_Car.GetPositionM());
		// 方向移动
		if (keyState.IsKeyDown(Keyboard::W))
		{
			if (m_CameraMode == CameraMode::FirstPerson)
				cam1st->Walk(dt * 3.0f);
			else
				cam1st->MoveForward(dt * 3.0f);
		}	
		if (keyState.IsKeyDown(Keyboard::S))
		{
			if (m_CameraMode == CameraMode::FirstPerson)
				cam1st->Walk(dt * -3.0f);
			else
				cam1st->MoveForward(dt * -3.0f);
		}
		if (keyState.IsKeyDown(Keyboard::A))
			cam1st->Turn(-1);
		if (keyState.IsKeyDown(Keyboard::D))
			cam1st->Turn(1);

		// 将位置限制在[-98.9f, 98.9f]的区域内
		// 不允许穿地
		XMFLOAT3 adjustedPos;
		XMStoreFloat3(&adjustedPos, XMVectorClamp(cam1st->GetPositionXM(), XMVectorSet(-98.9f, 0.0f, -98.9f, 0.0f), XMVectorReplicate(98.9f)));
		cam1st->SetPosition(adjustedPos);

		// 仅在第一人称模式移动箱子
		if (m_CameraMode != CameraMode::Free)
		{
			XMFLOAT4X4 adjustedPosM = cam1st->GetTargetPositionM();
			adjustedPosM(3, 0) = adjustedPos.x;
			adjustedPosM(3, 1) = adjustedPos.y;
			adjustedPosM(3, 2) = adjustedPos.z;
			// 更新状态
			m_Car.SetMoveState(cam1st->GetMoveState());
			m_Car.SetWorldMatrix(XMLoadFloat4x4(&adjustedPosM));
		}
		// 视野旋转，防止开始的差值过大导致的突然旋转
		cam1st->Pitch(mouseState.y * dt * 1.25f);
		cam1st->RotateY(mouseState.x * dt * 1.25f);
	}
	else if (m_CameraMode == CameraMode::ThirdPerson)
	{
		// 第三人称摄像机的操作

		cam3rd->SetTarget(m_Car.GetPosition());
		cam3rd->SetTargetM(m_Car.GetPositionM());
		// 方向移动
		if (keyState.IsKeyDown(Keyboard::W))
			cam3rd->Walk(dt * 3.0f);
		if (keyState.IsKeyDown(Keyboard::S))
			cam3rd->Walk(dt * -3.0f);
		if (keyState.IsKeyDown(Keyboard::A))
			cam3rd->Turn(-1);
		if (keyState.IsKeyDown(Keyboard::D))
			cam3rd->Turn(1);

		// 将位置限制在[-98.9f, 98.9f]的区域内
		// 不允许穿地
		XMFLOAT3 adjustedPos;
		XMFLOAT4X4 adjustedPosM = cam3rd->GetTargetPositionM();
		XMFLOAT3 targetPos = cam3rd->GetTargetPosition();
		XMStoreFloat3(&adjustedPos, XMVectorClamp(XMLoadFloat3(&targetPos), XMVectorSet(-98.9f, 0.0f, -98.9f, 0.0f), XMVectorReplicate(98.9f)));
		adjustedPosM(3, 0) = adjustedPos.x;
		adjustedPosM(3, 1) = adjustedPos.y;
		adjustedPosM(3, 2) = adjustedPos.z;
		//m_Car.SetWorldMatrix(XMMatrixTranslation(adjustedPos.x, adjustedPos.y, adjustedPos.z));

		// 更新状态
		m_Car.SetMoveState(cam3rd->GetMoveState());
		m_Car.SetWorldMatrix(XMLoadFloat4x4(&adjustedPosM));

		// 绕物体旋转
		cam3rd->RotateX(mouseState.y * dt * 1.25f);
		cam3rd->RotateY(mouseState.x * dt * 1.25f);
		cam3rd->Approach(-mouseState.scrollWheelValue / 120 * 1.0f);
	}

	// 更新观察矩阵
	m_pCamera->UpdateViewMatrix();
	m_BasicEffect.SetViewMatrix(m_pCamera->GetViewXM());
	m_BasicEffect.SetEyePos(m_pCamera->GetPositionXM());

	// 重置滚轮值
	m_pMouse->ResetScrollWheelValue();
	
	// 摄像机模式切换
	if (m_KeyboardTracker.IsKeyPressed(Keyboard::D1) && m_CameraMode != CameraMode::FirstPerson)
	{
		if (!cam1st)
		{
			cam1st.reset(new FirstPersonCamera);
			cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam1st;
		}

		cam1st->LookTo(m_Car.GetPosition(),
			XMFLOAT3(0.0f, 0.0f, 1.0f),
			XMFLOAT3(0.0f, 1.0f, 0.0f));
		
		m_CameraMode = CameraMode::FirstPerson;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D2) && m_CameraMode != CameraMode::ThirdPerson)
	{
		if (!cam3rd)
		{
			cam3rd.reset(new ThirdPersonCamera);
			cam3rd->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam3rd;
		}
		XMFLOAT3 target = m_Car.GetPosition();
		cam3rd->SetTarget(target);
		cam3rd->SetDistance(8.0f);
		cam3rd->SetDistanceMinMax(3.0f, 20.0f);
		
		m_CameraMode = CameraMode::ThirdPerson;
	}
	else if (m_KeyboardTracker.IsKeyPressed(Keyboard::D3) && m_CameraMode != CameraMode::Free)
	{
		if (!cam1st)
		{
			cam1st.reset(new FirstPersonCamera);
			cam1st->SetFrustum(XM_PI / 3, AspectRatio(), 0.5f, 1000.0f);
			m_pCamera = cam1st;
		}
		// 从箱子上方开始
		XMFLOAT3 pos = m_Car.GetPosition();
		XMFLOAT3 to = XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		pos.y += 3;
		cam1st->LookTo(pos, to, up);

		m_CameraMode = CameraMode::Free;
	}
	// 退出程序，这里应向窗口发送销毁信息
	if (keyState.IsKeyDown(Keyboard::Escape))
		SendMessage(MainWnd(), WM_DESTROY, 0, 0);
}

void GameApp::DrawScene()
{
	assert(m_pd3dImmediateContext);
	assert(m_pSwapChain);

	m_pd3dImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), reinterpret_cast<const float*>(&Colors::Black));
	m_pd3dImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// 绘制模型
	m_BasicEffect.SetRenderDefault(m_pd3dImmediateContext.Get(), BasicEffect::RenderObject);
	m_BasicEffect.SetReflectionEnabled(false);
	m_BasicEffect.SetTextureUsed(true);
	m_Car.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	m_BasicEffect.SetReflectionEnabled(false);
	m_Floor.Draw(m_pd3dImmediateContext.Get(), m_BasicEffect);

	// 绘制天空盒
	m_SkyEffect.SetRenderDefault(m_pd3dImmediateContext.Get());
	m_pSunset->Draw(m_pd3dImmediateContext.Get(), m_SkyEffect, *m_pCamera);

	// 绘制Direct2D部分
	if (m_pd2dRenderTarget != nullptr)
	{
		m_pd2dRenderTarget->BeginDraw();
		std::wstring text = L"切换摄像机模式: 1-第一人称 2-第三人称 3-自由视角\n"
			L"W/S/A/D 前进/后退/左平移/右平移 (第三人称无效)  Esc退出\n"
			L"鼠标移动控制视野 滚轮控制第三人称观察距离\n"
			L"当前模式: ";
		if (m_CameraMode == CameraMode::FirstPerson)
			text += L"第一人称(控制箱子移动)";
		else if (m_CameraMode == CameraMode::ThirdPerson)
			text += L"第三人称";
		else
			text += L"自由视角";
		m_pd2dRenderTarget->DrawTextW(text.c_str(), (UINT32)text.length(), m_pTextFormat.Get(),
			D2D1_RECT_F{ 0.0f, 0.0f, 600.0f, 200.0f }, m_pColorBrush.Get());
		HR(m_pd2dRenderTarget->EndDraw());
	}

	HR(m_pSwapChain->Present(0, 0));
}

bool GameApp::InitResource()
{
	// 初始化天空盒
	m_pSunset = std::make_unique<SkyRender>();
	HR(m_pSunset->InitResource(m_pd3dDevice.Get(), m_pd3dImmediateContext.Get(),
		std::vector<std::wstring>{
		L"Texture\\sunset_posX.bmp", L"Texture\\sunset_negX.bmp",
			L"Texture\\sunset_posY.bmp", L"Texture\\sunset_negY.bmp",
			L"Texture\\sunset_posZ.bmp", L"Texture\\sunset_negZ.bmp", },
		5000.0f));

	m_BasicEffect.SetTextureCube(m_pSunset->GetTextureCube());

	// 初始化游戏对象
	ComPtr<ID3D11ShaderResourceView> texture;
	// 初始化车
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\WoodCrate.dds", nullptr, texture.GetAddressOf()));


	m_Body.SetBuffer(m_pd3dDevice.Get(), Geometry::CreateBox(6.0f, 1.0f, 6.0f));
	m_Body.SetTexture(texture.Get());
	m_Car.AddObj(m_Body);
	// 轮子
	m_Wheels.resize(4);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	for (int i = 0; i < 4; ++i)
	{
		m_Wheels[i].SetBuffer(m_pd3dDevice.Get(), Geometry::CreateCylinder(1.0f, 0.7f));
		XMMATRIX world = XMMatrixTranslation(i % 2 ? -3.0f *(i-2)  : 3.0f * (i-1), 0.0f, i % 2 == 0? -3.0f * (i-1) : -3.0f * (i-2));
		m_Wheels[i].SetWorldMatrix(world);
		m_Wheels[i].SetTexture(texture.Get());
		m_Wheels[i].SetModelMatrix(world);
		// 仅轮子转动
		/* 轮子坐标
		-3, 3;
		3, 3;
		3, -3;
		-3, -3;
		*/
		//if (i == 0 || i == 1)
		m_Wheels[i].SetObjectType(GameObject::ObjectType::Mixed);
		m_Car.AddObj(m_Wheels[i]);
	}

	// 初始化地板
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\floor.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	m_Floor.SetBuffer(m_pd3dDevice.Get(),
		Geometry::CreatePlane(XMFLOAT2(200.0f, 200.0f), XMFLOAT2(5.0f, 5.0f)));
	m_Floor.SetTexture(texture.Get());
	m_Floor.SetWorldMatrix(XMMatrixTranslation(0.0f, -1.0f, 0.0f));
	
	// 初始化墙体
	m_Walls.resize(4);
	HR(CreateDDSTextureFromFile(m_pd3dDevice.Get(), L"Texture\\brick.dds", nullptr, texture.ReleaseAndGetAddressOf()));
	// 这里控制墙体四个面的生成
	for (int i = 0; i < 4; ++i)
	{
		m_Walls[i].SetBuffer(m_pd3dDevice.Get(),
			Geometry::CreatePlane(XMFLOAT2(20.0f, 8.0f), XMFLOAT2(5.0f, 1.5f)));
		XMMATRIX world = XMMatrixRotationX(-XM_PIDIV2) * XMMatrixRotationY(XM_PIDIV2 * i)
			* XMMatrixTranslation(i % 2 ? -10.0f * (i - 2) : 0.0f, 3.0f, i % 2 == 0 ? -10.0f * (i - 1) : 0.0f);
		m_Walls[i].SetWorldMatrix(world);
		m_Walls[i].SetTexture(texture.Get());
	}
		
	// ******************
	// 初始化摄像机
	//
	auto camera = std::shared_ptr<FirstPersonCamera>(new FirstPersonCamera);
	m_pCamera = camera;
	camera->SetViewPort(0.0f, 0.0f, (float)m_ClientWidth, (float)m_ClientHeight);
	camera->SetFrustum(XM_PI / 3, AspectRatio(), 1.0f, 1000.0f);
	camera->LookTo(
		XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f),
		XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	// 初始化并更新观察矩阵、投影矩阵(摄像机将被固定)
	camera->UpdateViewMatrix();
	m_BasicEffect.SetViewMatrix(camera->GetViewXM());
	m_BasicEffect.SetProjMatrix(camera->GetProjXM());

	// ******************
	// 初始化不会变化的值
	//
	// 灯光
	PointLight pointLight;
	pointLight.position = XMFLOAT3(0.0f, 10.0f, 0.0f);
	pointLight.ambient = XMFLOAT4(0.8f, 0.5f, 0.5f, 1.0f);
	pointLight.diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	pointLight.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	pointLight.att = XMFLOAT3(0.0f, 0.1f, 0.0f);
	pointLight.range = 25.0f;
	m_BasicEffect.SetPointLight(0, pointLight);
	// 方向光
	DirectionalLight dirLight[2];
	dirLight[0].ambient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);
	dirLight[0].diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	dirLight[0].specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	dirLight[0].direction = XMFLOAT3(-0.577f, -0.577f, 0.577f);
	dirLight[1] = dirLight[0];
	dirLight[1].direction = XMFLOAT3(0.577f, -0.577f, 0.577f);

	for (int i = 0; i < 2; ++i)
		m_BasicEffect.SetDirLight(i, dirLight[i]);

	return true;
}

GameApp::GameObject::GameObject()
	: m_IndexCount(), m_VertexStride(), m_rotation(XM_PI/90.0f), o_type(ObjectType::Default), moveFlag(0)
{
	XMStoreFloat4x4(&m_ModelMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
}

DirectX::XMFLOAT3 GameApp::GameObject::GetPosition() const
{
	return XMFLOAT3(m_WorldMatrix(3, 0), m_WorldMatrix(3, 1), m_WorldMatrix(3, 2));
}

DirectX::XMFLOAT4X4 GameApp::GameObject::GetPositionM() const
{
	return m_WorldMatrix;
}

void GameApp::GameObject::Rotation(float rad)
{
	XMMATRIX Rotation = XMMatrixRotationZ(rad);
	XMMATRIX Translation = XMLoadFloat4x4(&GetPositionM());
	SetWorldMatrix(XMMatrixMultiply(Rotation, Translation));
}

template<class VertexType, class IndexType>
void GameApp::GameObject::SetBuffer(ID3D11Device * device, const Geometry::MeshData<VertexType, IndexType>& meshData)
{
	// 释放旧资源
	m_pVertexBuffer.Reset();
	m_pIndexBuffer.Reset();

	// 设置顶点缓冲区描述
	m_VertexStride = sizeof(VertexType);
	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));
	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = (UINT)meshData.vertexVec.size() * m_VertexStride;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	// 新建顶点缓冲区
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = meshData.vertexVec.data();
	HR(device->CreateBuffer(&vbd, &InitData, m_pVertexBuffer.GetAddressOf()));


	// 设置索引缓冲区描述
	m_IndexCount = (UINT)meshData.indexVec.size();
	D3D11_BUFFER_DESC ibd;
	ZeroMemory(&ibd, sizeof(ibd));
	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	ibd.ByteWidth = m_IndexCount * sizeof(IndexType);
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	// 新建索引缓冲区
	InitData.pSysMem = meshData.indexVec.data();
	HR(device->CreateBuffer(&ibd, &InitData, m_pIndexBuffer.GetAddressOf()));



}

void GameApp::GameObject::SetTexture(ID3D11ShaderResourceView * texture)
{
	m_pTexture = texture;
}

void XM_CALLCONV GameApp::GameObject::SetWorldMatrix(FXMMATRIX world)
{
	XMStoreFloat4x4(&m_WorldMatrix, world);
}

void GameApp::GameObject::SetWorldMatrixFixed(const XMFLOAT4X4 & world)
{
	XMFLOAT4X4 res(0);
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			res(i, j) += world(i, j);
		}
	}
	m_WorldMatrix = res;
}

void XM_CALLCONV GameApp::GameObject::SetWorldMatrixFixed(FXMMATRIX world)
{
	if(o_type == ObjectType::Default)
		XMStoreFloat4x4(&m_WorldMatrix, XMMatrixMultiply(XMLoadFloat4x4(&m_ModelMatrix), world));
	else
	{
		static float increased = 0.001f;
		XMMATRIX Rotation = XMMatrixRotationX(increased);

		if (moveFlag)
		{
			if (increased > 100)
				increased = 0.001f;
			increased += moveFlag * 0.2f;
		}

		XMStoreFloat4x4(&m_WorldMatrix, XMMatrixMultiply(Rotation, XMMatrixMultiply(XMLoadFloat4x4(&m_ModelMatrix), world)));
	}
}

void GameApp::GameObject::SetModelMatrix(const XMFLOAT4X4 & world)
{
	m_ModelMatrix = world;
}

void XM_CALLCONV GameApp::GameObject::SetModelMatrix(FXMMATRIX world)
{
	XMStoreFloat4x4(&m_ModelMatrix, world);
}

void GameApp::GameObject::Draw(ID3D11DeviceContext * deviceContext)
{
	// 设置顶点/索引缓冲区
	UINT strides = m_VertexStride;
	UINT offsets = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// 获取之前已经绑定到渲染管线上的常量缓冲区并进行修改
	ComPtr<ID3D11Buffer> cBuffer = nullptr;
	deviceContext->VSGetConstantBuffers(0, 1, cBuffer.GetAddressOf());
	CBChangesEveryDrawing cbDrawing;

	// 内部进行转置，这样外部就不需要提前转置了
	XMMATRIX W = XMLoadFloat4x4(&m_WorldMatrix);
	cbDrawing.world = XMMatrixTranspose(W);
	cbDrawing.worldInvTranspose = XMMatrixInverse(nullptr, W);	// 两次转置抵消

	// 更新常量缓冲区
	D3D11_MAPPED_SUBRESOURCE mappedData;
	HR(deviceContext->Map(cBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData));
	memcpy_s(mappedData.pData, sizeof(CBChangesEveryDrawing), &cbDrawing, sizeof(CBChangesEveryDrawing));
	deviceContext->Unmap(cBuffer.Get(), 0);

	// 设置纹理
	deviceContext->PSSetShaderResources(0, 1, m_pTexture.GetAddressOf());
	// 可以开始绘制
	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameApp::GameObject::Draw(ID3D11DeviceContext * deviceContext, BasicEffect & effect)
{
	UINT strides = m_VertexStride;
	UINT offsets = 0;

	// 设置顶点/索引缓冲区
	deviceContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &strides, &offsets);
	deviceContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	//Material material;
    material.ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	material.diffuse = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
	material.specular = XMFLOAT4(0.1f, 0.1f, 0.1f, 50.0f);
	// 更新数据并应用
	effect.SetWorldMatrix(XMLoadFloat4x4(&m_WorldMatrix));
	effect.SetTextureDiffuse(m_pTexture.Get());
	effect.SetMaterial(material);

	effect.Apply(deviceContext);

	deviceContext->DrawIndexed(m_IndexCount, 0, 0);
}

void GameApp::GameObject::SetObjectType(ObjectType type)
{
	o_type = type;
}

GameApp::GameObject::ObjectType GameApp::GameObject::GetObjectType()
{
	return o_type;
}

void GameApp::GameObject::SetMoveState(float state)
{
	moveFlag = state;
}

float GameApp::GameObject::GetMoveState()
{
	return moveFlag;
}

void GameApp::GameObject::SetDebugObjectName(const std::string& name)
{
#if (defined(DEBUG) || defined(_DEBUG)) && (GRAPHICS_DEBUGGER_OBJECT_NAME)
	D3D11SetDebugObjectName(m_pVertexBuffer.Get(), name + ".VertexBuffer");
	D3D11SetDebugObjectName(m_pIndexBuffer.Get(), name + ".IndexBuffer");
#else
	UNREFERENCED_PARAMETER(name);
#endif
}

GameApp::GameObjectD::GameObjectD()
	: m_rotation(XM_PI / 90.0f), moveFlag(0)
{
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());
}

DirectX::XMFLOAT3 GameApp::GameObjectD::GetPosition() const
{
	return XMFLOAT3(m_WorldMatrix(3, 0), m_WorldMatrix(3, 1), m_WorldMatrix(3, 2));
}

DirectX::XMFLOAT4X4 GameApp::GameObjectD::GetPositionM() const
{
	return m_WorldMatrix;
}

void GameApp::GameObjectD::Rotation(float rad)
{
	for (int i = 0; i < gameObjs.size(); i++)
	{
		XMMATRIX Rotation = XMMatrixRotationZ(rad);
		XMMATRIX Translation = XMLoadFloat4x4(&GetPositionM());
		gameObjs[i].SetWorldMatrix(XMMatrixMultiply(Rotation, Translation));
	}
}

void XM_CALLCONV GameApp::GameObjectD::SetWorldMatrix(FXMMATRIX world)
{
	for (int i = 0; i < gameObjs.size(); ++i)
	{
		gameObjs[i].SetWorldMatrixFixed(world);
	}
	XMStoreFloat4x4(&m_WorldMatrix, world);
}

void GameApp::GameObjectD::Draw(ID3D11DeviceContext * deviceContext, BasicEffect & effect)
{
	for (int i = 0; i < gameObjs.size(); i++)
	{
		gameObjs[i].Draw(deviceContext, effect);
	}
}

void GameApp::GameObjectD::AddObj(GameObject &gameObject)
{
	gameObjs.push_back(gameObject);
}

void GameApp::GameObjectD::SetMoveState(float state)
{
	moveFlag = state;
	for (int i = 0; i < gameObjs.size(); ++i)
	{
		gameObjs[i].SetMoveState(state);
	}
}

float GameApp::GameObjectD::GetMoveState()
{
	return moveFlag;
}