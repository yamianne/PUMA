#include "lightAndShadowMap.h"
#include <dxDevice.h>

using namespace mini;
using namespace gk2;
using namespace utils;
using namespace DirectX;

const float LightAndShadowMap::LIGHT_FOV_ANGLE = DirectX::XM_PI / 3.0f;

LightAndShadowMap::LightAndShadowMap(const DxDevice& device, const ConstantBuffer<XMFLOAT4X4>& cbWorld,
	const ConstantBuffer<XMFLOAT4X4, 2> cbView, const ConstantBuffer<XMFLOAT4X4>& cbProj,
	const ConstantBuffer<XMFLOAT4> cbLightPos, const ConstantBuffer<XMFLOAT4> cbSurfaceColor)
	: StaticEffect(
		PhongEffect(device.CreateVertexShader(L"lightAndShadowVS.cso"),
			device.CreatePixelShader(L"lightAndShadowPS.cso"),
			cbWorld, cbView, cbProj, cbLightPos, cbSurfaceColor),
		PSShaderResources{}, PSSamplers{}), m_cbMapMtx(device.CreateConstantBuffer<XMFLOAT4X4>())
{
	SetPSConstantBuffer(2, m_cbMapMtx);
	auto lightMap = device.CreateShaderResourceView(L"resources/textures/light_cookie.png");
	SetPSShaderResource(0, lightMap);

	SamplerDescription sd;
	// TODO : 3.04 Create sampler with appropriate addressing (border) and filtering (bilinear) modes

	SetPSSampler(0, device.CreateSamplerState(sd));

	// TODO : 3.09 Create shadow texture with appropriate width, height, format, mip levels and bind flags
	Texture2DDescription td;

	dx_ptr<ID3D11Texture2D> shadowTexture;// = device.CreateTexture(td);
	
	// TODO : 3.10 Create depth-stencil view for the shadow texture with appropriate format
	DepthViewDescription dvd;

	//m_shadowDepthBuffer = device.CreateDepthStencilView(shadowTexture, dvd);

	// TODO : 3.11 Create shader resource view for the shadow texture with appropriate format, view dimensions, mip levels and most detailed mip level
	ShaderResourceViewDescription srvd;

	dx_ptr<ID3D11ShaderResourceView> shadowMap;// = device.CreateShaderResourceView(shadowTexture, srvd);
	SetPSShaderResource(1, shadowMap);
}

XMFLOAT4 LightAndShadowMap::UpdateLightPosition(const dx_ptr<ID3D11DeviceContext>& context,
	XMMATRIX lightMtx)
{
	XMFLOAT4 lightPos{ 0.0f, -0.05f, 0.0f, 1.0f };
	XMFLOAT4 lightTarget{ 0.0f, -10.0f, 0.0f, 1.0f };
	XMFLOAT4 upDir{ 1.0f, 0.0f, 0.0f, 0.0f };

	XMFLOAT4X4 texMtx;

	// TODO : 3.01 Calculate view, inverted view and projection matrix and store them in appropriate class fields

	// TODO : 3.02 Calculate map transform matrix

	// TODO : 3.17 Modify map transform to fix z-fighting

	XMStoreFloat4x4(&texMtx, XMMatrixIdentity());

	m_cbMapMtx.Update(context, texMtx);

	// TODO : 3.03 Return light position in world coordinates
	return lightPos;
}

void LightAndShadowMap::BeginShadowRender(const dx_ptr<ID3D11DeviceContext>& context,
	ConstantBuffer<XMFLOAT4X4, 2>& cbView, ConstantBuffer<XMFLOAT4X4>& cbProj) const
{
	cbView.Update(context, m_lightViewMtx);
	cbProj.Update(context, m_lightProjMtx);
	// TODO : 3.12 Set up view port of the appropriate size
	// TODO : 3.13 Bind no render targets and the shadow map as depth buffer
	// TODO : 3.14 Clear the depth buffer

}