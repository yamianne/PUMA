#pragma once
#include "PhongEffect.h"
#include <DirectXMath.h>

namespace mini
{
	namespace gk2
	{
		class ShadowVolumes : public StaticEffect<PhongEffect, PSShaderResources, PSSamplers>
		{
			ShadowVolumes() = default;

			ShadowVolumes(dx_ptr<ID3D11VertexShader>&& vs, dx_ptr<ID3D11PixelShader>&& ps, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbWorld,
				const ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView, const ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj,
				const ConstantBuffer<DirectX::XMFLOAT4>& cbLightPos, const ConstantBuffer<DirectX::XMFLOAT4>& cbSurfaceColor);

			DirectX::XMFLOAT4 UpdateLightPosition(const dx_ptr<ID3D11DeviceContext>& context, DirectX::XMMATRIX lightMtx);

			void BeginShadowRender(const dx_ptr<ID3D11DeviceContext>& context, ConstantBuffer<DirectX::XMFLOAT4X4, 2>& cbView,
				ConstantBuffer<DirectX::XMFLOAT4X4>& cbProj) const;
		private:
			dx_ptr<ID3D11RenderTargetView> m_shadowRenderTarget;
			dx_ptr<ID3D11DepthStencilView> m_shadowDepthBuffer;
		};
	}
}