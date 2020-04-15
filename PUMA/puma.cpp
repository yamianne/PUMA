#include "puma.h"
#include <array>
#include "meshLoader.h"

using namespace mini;
using namespace utils;
using namespace gk2;
using namespace DirectX;
using namespace std;

const unsigned int Puma::VB_STRIDE = sizeof(VertexPositionNormal);
const unsigned int Puma::VB_OFFSET = 0;
const unsigned int Puma::BS_MASK = 0xffffffff;

Puma::Puma(HINSTANCE appInstance)
	: Gk2ExampleBase(appInstance, 1280, 720, L"PUMA - Anna Kozłowska"), 
	//Constant Buffers
	m_cbWorldMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbProjMtx(m_device.CreateConstantBuffer<XMFLOAT4X4>()),
	m_cbViewMtx(m_device.CreateConstantBuffer<XMFLOAT4X4, 2>()),
	m_cbSurfaceColor(m_device.CreateConstantBuffer<XMFLOAT4>()),
	m_cbLightPos(m_device.CreateConstantBuffer<XMFLOAT4>())
{
	//m_backBufferTexture = m_device.swapChain().GetBuffer();
	//auto windowSize = m_window.getClientSize();
	////auto backBufferTexture = m_device.swapChain().GetBuffer();
	//m_backBuffer = m_device.CreateRenderTargetView(m_backBufferTexture);
	//m_depthStencilView = m_device.CreateDepthStencilView(windowSize);
	//ID3D11RenderTargetView* backBuffer = m_backBuffer.get();
	//m_device.context()->OMSetRenderTargets(1, &backBuffer, m_depthStencilView.get());

	//Projection matrix
	auto s = m_window.getClientSize();
	auto ar = static_cast<float>(s.cx) / s.cy;
	XMStoreFloat4x4(&m_projMtx, XMMatrixPerspectiveFovLH(XM_PIDIV4, ar, 0.01f, 100.0f));
	m_cbProjMtx.Update(m_device.context(), m_projMtx);
	UpdateCameraCB(m_camera.getViewMatrix());

	//Meshes
	vector<VertexPositionNormal> vertices;
	vector<unsigned short> indices;
	for (auto i = 0; i < ROBOT_PARTS_NUMBER; i++)
	{
		tie(vertices, indices) = MeshLoader::LoadMeshFromTXT(L"resources/robot_parts/mesh" + to_wstring(i + 1) + L".txt", m_robotPartEdges[i]);
		m_robotPart[i] = m_device.CreateMesh(indices, vertices);
		m_robotPartVertices[i] = vertices;
		m_robotPartTriangles[i] = CreateTrianglesVector(indices);
	}
	tie(vertices, indices) = MeshLoader::CreateSquare(ROOM_SIZE);
	m_wall = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateRectangle(MIRROR_WIDTH, MIRROR_HEIGHT);
	m_mirror = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateCylinder(CYLINDER_RADIUS, CYLINDER_HEIGHT, CYLINDER_STACKS, CYLINDER_SLICES);
	m_cylinder = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateDisk(CYLINDER_RADIUS, CYLINDER_SLICES);
	m_cylinderBase = m_device.CreateMesh(indices, vertices);
	tie(vertices, indices) = MeshLoader::CreateSphere(0.05f, 10, 10);
	m_helpPoint = m_device.CreateMesh(indices, vertices);


	// World matrices of all objects
	auto temp = XMMatrixTranslation(0.0f, 0.0f, ROOM_SIZE / 2);
	auto translation = XMMatrixTranslation(0.0f, ROOM_SIZE / 2 - 1, 0.0f);
	auto a = 0.f;
	for (auto i = 0U; i < 4U; ++i, a += XM_PIDIV2)
		XMStoreFloat4x4(&m_wallsMtx[i], temp * XMMatrixRotationY(a) * translation);
	XMStoreFloat4x4(&m_wallsMtx[4], temp * XMMatrixRotationX(XM_PIDIV2) * translation);
	XMStoreFloat4x4(&m_wallsMtx[5], temp * XMMatrixRotationX(-XM_PIDIV2) * translation);
	
	XMMATRIX mirror = XMMatrixRotationY(-XM_PIDIV2) * XMMatrixRotationZ(MIRROR_ANGLE) * XMMatrixTranslationFromVector(MIRROR_POSITION);
	m_swivelNorm = XMVector3TransformNormal(XMVECTOR{ 0.0f, 0.0f, -1.0f }, mirror);

	XMVECTOR det;
	XMStoreFloat4x4(&m_mirrorMtx, mirror);
	XMStoreFloat4x4(&m_mirroredWorldMtx, XMMatrixInverse(&det, mirror) * XMMatrixScaling(1, 1, -1) * mirror);

	XMMATRIX cylinder_translation = XMMatrixRotationZ(XM_PIDIV2) * XMMatrixRotationY(-XM_PI / 6) * XMMatrixTranslationFromVector(CYLINDER_POSITION);
	XMStoreFloat4x4(&m_cylinderMtx, cylinder_translation);
	XMStoreFloat4x4(&m_cylinderBaseMtx[0], XMMatrixTranslation(0.0f, CYLINDER_HEIGHT / 2, 0.0f) * cylinder_translation);
	XMStoreFloat4x4(&m_cylinderBaseMtx[1], XMMatrixRotationZ(XM_PI) * XMMatrixTranslation(0.0f, -CYLINDER_HEIGHT / 2, 0.0f) * cylinder_translation);

	for (auto& m_robotPart : m_robotPartMtx)
		XMStoreFloat4x4(&m_robotPart, XMMatrixIdentity());

	//Constant buffers content
	m_cbSurfaceColor.Update(m_device.context(), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	m_cbLightPos.Update(m_device.context(), LIGHT_POS);
	//Render states
	CreateRenderStates();

	auto vsCode = m_device.LoadByteCode(L"phongVS.cso");
	auto psCode = m_device.LoadByteCode(L"phongPS.cso");
	m_phongEffect = PhongEffect(m_device.CreateVertexShader(vsCode), m_device.CreatePixelShader(psCode),
		m_cbWorldMtx, m_cbViewMtx, m_cbProjMtx, m_cbLightPos, m_cbSurfaceColor);
	m_inputlayout = m_device.CreateInputLayout(VertexPositionNormal::Layout, vsCode);

	m_particles = ParticleSystem(m_device, m_cbViewMtx, m_cbProjMtx, XMFLOAT3(-1.3f, -0.6f, -0.14f), mirror);

	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	UpdateSwivel(0.0f);
}

std::vector<Triangle> Puma::CreateTrianglesVector(std::vector<unsigned short> ind)
{
	std::vector<Triangle> triangles;
	for (int i = 0; i < ind.size(); i+=3)
		triangles.push_back(Triangle(ind[i], ind[i + 1], ind[i + 2]));
	return triangles;
}

void Puma::CreateRenderStates()
{
	DepthStencilDescription dssDesc;
	//dssDesc.StencilEnable = true; //Enable stencil operations
	//dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //Disable writing to depth buffer
	////dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; //Disable writing to depth buffer
	//dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER; //Back faces should never pass stencil test
	//dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; //Front faces should always pass stencil test
	//dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE; //when pixel passes depth and stencil test write to stencil buffer
	//m_dssWrite = m_device.CreateDepthStencilState(dssDesc);

	//dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//dssDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	//dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	//m_dssTest = m_device.CreateDepthStencilState(dssDesc);

	dssDesc.StencilEnable = true; 
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
	dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER; 
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS; 
	dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	m_dssWrite = m_device.CreateDepthStencilState(dssDesc);

	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER; 
	dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	m_dssTest = m_device.CreateDepthStencilState(dssDesc);

	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssTestNoWrite = m_device.CreateDepthStencilState(dssDesc);

	m_rsCCW = m_device.CreateRasterizerState(RasterizerDescription(true));

	RasterizerDescription rsDesc;
	rsDesc.CullMode = D3D11_CULL_NONE;
	m_rsCullNone = m_device.CreateRasterizerState(rsDesc);

	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//dssDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dssDesc.StencilEnable = false;
	m_dssNoWrite = m_device.CreateDepthStencilState(dssDesc);

	m_bsAlpha = m_device.CreateBlendState(BlendDescription::AlphaBlendDescription());

	DepthStencilDescription dss2Desc;
	//Setup depth stencil state for writing
	dss2Desc.StencilEnable = true;
	dss2Desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dss2Desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_INCR;
	dss2Desc.BackFace.StencilFunc = D3D11_COMPARISON_NEVER;
	dss2Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dsShadowWriteFront = m_device.CreateDepthStencilState(dss2Desc);
	dss2Desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_DECR;
	m_dsShadowWriteBack = m_device.CreateDepthStencilState(dss2Desc);

	//Setup depth stencil state for testing
	dss2Desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dss2Desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dss2Desc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	dss2Desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	m_dsShadowTest = m_device.CreateDepthStencilState(dss2Desc);
	dss2Desc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;
	m_dsShadowTestComplement = m_device.CreateDepthStencilState(dss2Desc);

	//BlendDescription bs2Desc;
	//bs2Desc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
	//bs2Desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	//bs2Desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	////bs2Desc.RenderTarget[0].BlendOp = false;
	//bs2Desc.RenderTarget[0].RenderTargetWriteMask = 0;
	//m_bsNoColorWrite = m_device.CreateBlendState(bs2Desc);
}

void Puma::UpdateCameraCB(DirectX::XMMATRIX cameraMtx)
{
	XMVECTOR det;
	XMMATRIX invViewMtx = XMMatrixInverse(&det, cameraMtx);
	XMFLOAT4X4 view[2];
	XMStoreFloat4x4(view, cameraMtx);
	XMStoreFloat4x4(view + 1, invViewMtx);
	m_cbViewMtx.Update(m_device.context(), view);
}

void Puma::UpdateSwivel(float dt)
{
	static auto time = 0.0f;
	time += dt;
	float circleX = 0.3f * XMScalarSin(XM_2PI * time / 4);
	float circleY = 0.3f * XMScalarCos(XM_2PI * time / 4);
	XMVECTOR pos = { circleX, circleY, 0.0f };
	XMMATRIX mirrorMtx;
	mirrorMtx = XMLoadFloat4x4(&m_mirrorMtx);
	m_swivelPos = XMVector3TransformCoord(pos, mirrorMtx);
	XMStoreFloat4x4(&m_helpPointMtx, XMMatrixTranslationFromVector(m_swivelPos));
}

void Puma::CalculateRobotAngles(XMVECTOR pos, XMVECTOR normal)
{
	float a1, a2, a3, a4, a5;
	float l1 = .91f, l2 = .81f, l3 = .33f, dy = .27f, dz = .26f;
	normal = XMVector3Normalize(normal);
	XMVECTOR pos1vec = pos + normal * l3;
	XMFLOAT4 pos1;
	XMStoreFloat4(&pos1, pos1vec);
	float e = sqrtf(pos1.z * pos1.z + pos1.x * pos1.x - dz * dz);
	a1 = atan2(pos1.z, -pos1.x) + atan2(dz, e);
	XMVECTOR pos2vec = { e, pos1.y - dy, .0f };
	XMFLOAT4 pos2;
	XMStoreFloat4(&pos2, pos2vec);
	a3 = -acosf(min(1.0f, (pos2.x * pos2.x + pos2.y * pos2.y - l1 * l1 - l2 * l2)
		/ (2.0f * l1 * l2)));
	float k = l1 + l2 * cosf(a3), l = l2 * sinf(a3);
	a2 = -atan2(pos2.y, sqrtf(pos2.x*pos2.x + pos2.z*pos2.z)) - atan2(l, k);
	XMVECTOR normal1 = XMVector3TransformNormal(normal, XMMatrixRotationY(-a1));
	normal1 = XMVector3TransformNormal(normal1, XMMatrixRotationZ(-(a2 + a3)));
	XMFLOAT4 normal1val;
	XMStoreFloat4(&normal1val, normal1);
	a5 = acosf(normal1val.x);
	a4 = atan2(normal1val.z, normal1val.y);

	XMMATRIX mtx = XMMatrixRotationY(a1);
	XMStoreFloat4x4(&m_robotPartMtx[1], mtx);
	mtx = XMMatrixTranslation(0.0f, -dy, 0.0f) * XMMatrixRotationZ(a2) * XMMatrixTranslation(0.0f, dy, 0.0f) * mtx;
	XMStoreFloat4x4(&m_robotPartMtx[2], mtx);
	mtx = XMMatrixTranslation(l1, -dy, 0.0f) * XMMatrixRotationZ(a3) * XMMatrixTranslation(-l1, dy, 0.0f) * mtx;
	XMStoreFloat4x4(&m_robotPartMtx[3], mtx);
	mtx = XMMatrixTranslation(0.0f, -dy, dz) * XMMatrixRotationX(a4) * XMMatrixTranslation(0.0f, dy, -dz) * mtx;
	XMStoreFloat4x4(&m_robotPartMtx[4], mtx);
	mtx = XMMatrixTranslation((l1+l2), -dy, 0.0f) * XMMatrixRotationZ(a5) * XMMatrixTranslation(-(l1 + l2), dy, 0.0f) * mtx;
	XMStoreFloat4x4(&m_robotPartMtx[5], mtx);
}

void Puma::TurnOffLight()
{
	m_cbLightPos.Update(m_device.context(), XMFLOAT4{0.0f, 0.0f, 0.0f, 1.0f});
}

void Puma::TurnOnLight()
{
	m_cbLightPos.Update(m_device.context(), LIGHT_POS);
}


void Puma::Update(const Clock& c)
{
	double dt = c.getFrameTime();
	if (HandleCameraInput(dt))
		UpdateCameraCB(m_camera.getViewMatrix());
	UpdateSwivel(static_cast<float>(dt));
	CalculateRobotAngles(m_swivelPos, m_swivelNorm);
	m_particles.UpdateEmitterPosition(m_swivelPos);
	m_particles.Update(m_device.context(), static_cast<float>(dt), m_camera.getCameraPosition());
	UpdateShadowGeometry();
}

void Puma::SetWorldMtx(DirectX::XMFLOAT4X4 mtx)
{
	m_cbWorldMtx.Update(m_device.context(), mtx);
}

void Puma::DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx)
{
	SetWorldMtx(worldMtx);
	m.Render(m_device.context());
}

void Puma::DrawParticles() const
{
	m_particles.Render(m_device.context());
	m_device.context()->GSSetShader(nullptr, nullptr, 0);
	m_device.context()->IASetInputLayout(m_inputlayout.get());
	m_device.context()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Puma::DrawSceneInMirror()
{
	m_device.context()->OMSetDepthStencilState(m_dssWrite.get(), 1);
	DrawMesh(m_mirror, m_mirrorMtx);
	m_device.context()->OMSetDepthStencilState(m_dssTest.get(), 1);

	XMMATRIX viewMtx = m_camera.getViewMatrix();
	XMMATRIX mirrorViewMtx = XMMatrixMultiply(XMLoadFloat4x4(&m_mirroredWorldMtx), viewMtx);
	UpdateCameraCB(mirrorViewMtx);

	m_device.context()->RSSetState(m_rsCCW.get());
	
	m_cbSurfaceColor.Update(m_device.context(), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

	DrawMesh(m_cylinder, m_cylinderMtx);
	for (auto& mtx : m_cylinderBaseMtx)
		DrawMesh(m_cylinderBase, mtx);

	for (int idx = 0; idx < ROBOT_PARTS_NUMBER; idx++)
		DrawMesh(m_robotPart[idx], m_robotPartMtx[idx]);

	//m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(m_dssTestNoWrite.get(), 1);
	DrawParticles();
	//m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	//m_device.context()->OMSetDepthStencilState(m_dssTest.get(), 0);
	m_device.context()->OMSetDepthStencilState(nullptr, 0);

	for (auto& wallMtx : m_wallsMtx)
		DrawMesh(m_wall, wallMtx);
	m_device.context()->RSSetState(nullptr);

	UpdateCameraCB(viewMtx);

	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

void Puma::DrawScene()
{
	for (int idx = 0; idx < ROBOT_PARTS_NUMBER; idx++)
		DrawMesh(m_robotPart[idx], m_robotPartMtx[idx]);
	for (auto& wallMtx : m_wallsMtx)
		DrawMesh(m_wall, wallMtx);
	DrawMesh(m_cylinder, m_cylinderMtx);
	for (auto& mtx : m_cylinderBaseMtx)
		DrawMesh(m_cylinderBase, mtx);
	//DrawMesh(m_helpPoint, m_helpPointMtx);

	//m_device.context()->RSSetState(nullptr);
}

void Puma::Render()
{
	Gk2ExampleBase::Render();

	getDefaultRenderTarget().Begin(m_device.context());

	m_phongEffect.Begin(m_device.context());
	DrawSceneInMirror();
	m_phongEffect.Begin(m_device.context());
	m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, BS_MASK);
	m_cbSurfaceColor.Update(m_device.context(), XMFLOAT4(0.4f, 0.2f, 0.2f, 0.4f));
	DrawMesh(m_mirror, m_mirrorMtx);

	m_device.context()->OMSetBlendState(nullptr, nullptr, BS_MASK);
	m_cbSurfaceColor.Update(m_device.context(), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	DrawScene();
	//TurnOffLight();
	//DrawShadowGeometry();

	// Draw shadow volumes
	m_renderTarget.ClearDepthStencil(m_device.context(), D3D11_CLEAR_STENCIL);
	m_device.context()->OMSetDepthStencilState(m_dsShadowWriteFront.get(), 0);
	DrawShadowGeometry();
	m_device.context()->RSSetState(m_rsCCW.get());
	m_device.context()->OMSetDepthStencilState(m_dsShadowWriteBack.get(), 0);
	DrawShadowGeometry();
	m_device.context()->RSSetState(nullptr);

	m_device.context()->OMSetDepthStencilState(m_dsShadowTestComplement.get(), 0);
	DrawMesh(m_mirror, m_mirrorMtx);
	m_device.context()->OMSetDepthStencilState(m_dsShadowTest.get(), 0);
	DrawScene();
	m_device.context()->OMSetDepthStencilState(nullptr, 0);


	// Particles
	//m_device.context()->OMSetBlendState(m_bsAlpha.get(), nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
	DrawParticles();
	//m_device.context()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
	m_device.context()->OMSetDepthStencilState(nullptr, 0);
}

void Puma::UpdateShadowGeometry()
{
	vector<VertexPositionNormal> vertices;
	vector<unsigned short> indices;
	vector<vector<VertexTriangle>> triangles;
	m_contourEdges = 0;
	float GO_TO_INFINITY = 1000.0f;
	for (int partIdx = 0; partIdx < 6; ++partIdx) {
		XMMATRIX mat = XMLoadFloat4x4(&m_robotPartMtx[partIdx]);
		vector<VertexTriangle> part_triangles;
		for (int i = 0; i < m_robotPartEdges[partIdx].size(); ++i) {
			auto edge = m_robotPartEdges[partIdx][i];
			auto v1 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][edge.vertexIdx1].position), mat);
			auto v2 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][edge.vertexIdx2].position), mat);
			auto t1 = m_robotPartTriangles[partIdx][edge.triangleIdx1];
			auto t2 = m_robotPartTriangles[partIdx][edge.triangleIdx2];
			auto lightPos = XMLoadFloat4(&LIGHT_POS);
			auto t1v1 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][t1.vertexIdx1].position), mat);
			auto t1v2 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][t1.vertexIdx2].position), mat);
			auto t1v3 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][t1.vertexIdx3].position), mat);
			auto t2v1 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][t2.vertexIdx1].position), mat);
			auto t2v2 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][t2.vertexIdx2].position), mat);
			auto t2v3 = XMVector3Transform(XMLoadFloat3(&m_robotPartVertices[partIdx][t2.vertexIdx3].position), mat);
			auto t1lightDir = (t1v1 + t1v2 + t1v3) / 3 - lightPos;
			auto t2lightDir = (t2v1 + t2v2 + t2v3) / 3 - lightPos;
			auto t1normal = XMVector3Normalize(XMVector3Cross(t1v2 - t1v1, t1v3 - t1v2));
			auto t2normal = XMVector3Normalize(XMVector3Cross(t2v2 - t2v1, t2v3 - t2v2));
			auto t1dot = XMVectorGetX(XMVector3Dot(t1lightDir, t1normal));
			auto t2dot = XMVectorGetX(XMVector3Dot(t2lightDir, t2normal));
			bool first = t1dot >= 0.0 && t2dot < 0;
			bool second = t2dot >= 0.0 && t1dot < 0;
			if (first || second) {
				++m_contourEdges;
				VertexPositionNormal vertex1;
				VertexPositionNormal vertex2;
				VertexPositionNormal vertex3;
				VertexPositionNormal vertex4;
				XMStoreFloat3(&vertex1.position, v1);
				XMStoreFloat3(&vertex2.position, v2);
				XMStoreFloat3(&vertex3.position, v1 + XMVector3Normalize(v1 - lightPos) * GO_TO_INFINITY);
				XMStoreFloat3(&vertex4.position, v2 + XMVector3Normalize(v2 - lightPos) * GO_TO_INFINITY);
				vertex1.normal = XMFLOAT3(1, 0, 0);
				vertex2.normal = XMFLOAT3(1, 0, 0);
				vertex3.normal = XMFLOAT3(1, 0, 0);
				vertex4.normal = XMFLOAT3(1, 0, 0);
				int idx = vertices.size();
				vertices.push_back(vertex1);
				vertices.push_back(vertex2);
				vertices.push_back(vertex3);
				vertices.push_back(vertex4);
				if (first) {
					indices.push_back(idx);
					indices.push_back(idx + 1);
					indices.push_back(idx + 2);
					indices.push_back(idx + 2);
					indices.push_back(idx + 1);
					indices.push_back(idx + 3);
					VertexTriangle tri1, tri2;
					tri1.v1 = XMLoadFloat3(&vertex1.position);
					tri1.v2 = XMLoadFloat3(&vertex2.position);
					tri1.v3 = XMLoadFloat3(&vertex3.position);
					tri2.v1 = XMLoadFloat3(&vertex3.position);
					tri2.v2 = XMLoadFloat3(&vertex2.position);
					tri2.v3 = XMLoadFloat3(&vertex4.position);
					part_triangles.push_back(tri1);
					part_triangles.push_back(tri2);
				}
				if (second) {
					indices.push_back(idx + 1);
					indices.push_back(idx);
					indices.push_back(idx + 2);
					indices.push_back(idx + 1);
					indices.push_back(idx + 2);
					indices.push_back(idx + 3);
					VertexTriangle tri1, tri2;
					tri1.v1 = XMLoadFloat3(&vertex2.position);
					tri1.v2 = XMLoadFloat3(&vertex1.position);
					tri1.v3 = XMLoadFloat3(&vertex3.position);
					tri2.v1 = XMLoadFloat3(&vertex2.position);
					tri2.v2 = XMLoadFloat3(&vertex3.position);
					tri2.v3 = XMLoadFloat3(&vertex4.position);
					part_triangles.push_back(tri1);
					part_triangles.push_back(tri2);
				}
			}
		}
		triangles.push_back(part_triangles);
	}
	m_vbShadowVolume = m_device.CreateVertexBuffer(vertices);
	m_ibShadowVolume = m_device.CreateIndexBuffer(indices);

	auto pl = XMPlaneFromPoints(XMVECTOR{ 0, 2, 0 }, XMVECTOR{ 0, 2, 1 }, XMVECTOR{ 1, 2, 0 });
	dotf = XMVectorGetX(XMPlaneDotCoord(pl, XMVECTOR{ 0, 3, 0 }));

	cameraInShadow = false;
	int plus = 0;
	int zero = 0;
	int minus = 0;
	XMVECTOR pos = XMLoadFloat4(&m_camera.getCameraPosition());
	for (int part = 0; part < triangles.size(); ++part) {
		bool cameraInPartShadow = true;
		for (int i = 0; i < triangles[part].size(); ++i) {
			VertexTriangle tri = triangles[part][i];
			XMStoreFloat4(&v1, tri.v1);
			XMStoreFloat4(&v2, tri.v2);
			XMStoreFloat4(&v3, tri.v3);
			auto plane = XMPlaneFromPoints(tri.v1, tri.v2, tri.v3);
			dotf = XMVectorGetX(XMPlaneDotCoord(plane, pos));
			if (dotf > 0) {
				cameraInPartShadow = false;
			}
		}
		if (cameraInPartShadow) {
			cameraInShadow = true;
			break;
		}
	}
}

void Puma::DrawShadowGeometry()
{
	XMFLOAT4X4 ident;
	XMStoreFloat4x4(&ident, XMMatrixIdentity());
	SetWorldMtx(ident);
	auto b = m_vbShadowVolume.get();
	m_device.context()->IASetVertexBuffers(0, 1, &b, &VB_STRIDE, &VB_OFFSET);
	m_device.context()->IASetIndexBuffer(m_ibShadowVolume.get(), DXGI_FORMAT_R16_UINT, 0);
	m_device.context()->DrawIndexed(m_contourEdges * 6, 0, 0);
}
