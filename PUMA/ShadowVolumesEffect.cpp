#include "ShadowVolumesEffect.h"
#include <dxDevice.h>

using namespace mini;
using namespace gk2;
using namespace utils;
using namespace DirectX;

ShadowVolumes::ShadowVolumes(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbWorld,
	const ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj,
	const ConstantBuffer<DirectX::XMFLOAT4>& cbLightPos, const ConstantBuffer<DirectX::XMFLOAT4>& cbSurfaceColor)
	: StaticEffect(PhongEffect( move(vs), move(ps), cbWorld, cbView, cbProj, cbLightPos, cbSurfaceColor), PSShaderResources{}, PSSamplers{})
{}

void ShadowVolumes::BeginShadowRender(const dx_ptr<ID3D11DeviceContext>& context,
	ConstantBuffer<XMFLOAT4X4, 2>& cbView, ConstantBuffer<XMFLOAT4X4>& cbProj) const
{
	cbView.Update(context, m_lightViewMtx);
	cbProj.Update(context, m_lightProjMtx);
	// TODO : 3.12 Set up view port of the appropriate size
	ViewportDescription vd;
	vd.Height = vd.Width = TEXTURE_SIZE;
	context->RSSetViewports(1, &vd);
	// TODO : 3.13 Bind no render targets and the shadow map as depth buffer
	context->OMSetRenderTargets(0, 0, m_shadowDepthBuffer.get());
	// TODO : 3.14 Clear the depth buffer
	context->ClearDepthStencilView(m_shadowDepthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	//context->ClearDepthStencilView(nullptr, 0, 0, 0);
}