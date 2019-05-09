#pragma once
#include "gk2ExampleBase.h"
#include "constantBuffer.h"
#include "mesh.h"
#include "PhongEffect.h"
#include "particleSystem.h"
#include "mesh_structures.h"
#include "vertexDef.h"

namespace mini::gk2
{
	class Puma : public Gk2ExampleBase
	{
	public:
		explicit Puma(HINSTANCE appInstance);

	protected:
		void Update(const Clock& dt) override;
		void Render() override;

	private:
#pragma region CONSTANTS
		static const unsigned int VB_STRIDE;
		static const unsigned int VB_OFFSET;
		static const unsigned int BS_MASK;

		static constexpr DirectX::XMFLOAT4 LIGHT_POS = {-1.0f, 2.0f, 1.0f, 1.0f};
		static constexpr int ROBOT_PARTS_NUMBER = 6;
		static constexpr float ROOM_SIZE = 5.0f;
		static constexpr float MIRROR_WIDTH = 1.0f;
		static constexpr float MIRROR_HEIGHT = 2.0f;
		static constexpr float MIRROR_ANGLE = DirectX::XM_PI / 6;
		static constexpr DirectX::XMVECTOR MIRROR_POSITION = {-1.7f, 0.0f, -0.27f};
		static constexpr float CYLINDER_RADIUS = 0.2f;
		static constexpr float CYLINDER_HEIGHT = 1.0f;
		static constexpr int CYLINDER_STACKS = 8;
		static constexpr int CYLINDER_SLICES = 20;
		static constexpr DirectX::XMVECTOR CYLINDER_POSITION = {1.0f, -1.0f + CYLINDER_RADIUS, -0.7f};

#pragma endregion
		ConstantBuffer<DirectX::XMFLOAT4X4> m_cbWorldMtx, //vertex shader constant buffer slot 0
			m_cbProjMtx;	//vertex shader constant buffer slot 2 & geometry shader constant buffer slot 0
		ConstantBuffer<DirectX::XMFLOAT4X4, 2> m_cbViewMtx; //vertex shader constant buffer slot 1
		ConstantBuffer<DirectX::XMFLOAT4> m_cbSurfaceColor;	//pixel shader constant buffer slot 0
		ConstantBuffer<DirectX::XMFLOAT4> m_cbLightPos; //pixel shader constant buffer slot 1

		//dx_ptr<ID3D11Texture2D> m_backBufferTexture;
		//dx_ptr<ID3D11RenderTargetView> m_backBuffer;
		//dx_ptr<ID3D11Texture2D> m_depthStencilTexture;
		//dx_ptr<ID3D11DepthStencilView> m_depthStencilView;

		std::vector<Edge> m_robotPartEdges[ROBOT_PARTS_NUMBER];
		std::vector<VertexPositionNormal> m_robotPartVertices[ROBOT_PARTS_NUMBER];
		std::vector<Triangle> m_robotPartTriangles[ROBOT_PARTS_NUMBER];
		Mesh m_robotPart[ROBOT_PARTS_NUMBER]; // 6 parts of robot
		Mesh m_wall; //uses m_wallsMtx[6]
		Mesh m_mirror;
		Mesh m_cylinder;
		Mesh m_cylinderBase;
		Mesh m_helpPoint;

		DirectX::XMFLOAT4X4 m_robotPartMtx[ROBOT_PARTS_NUMBER];
		DirectX::XMFLOAT4X4 m_wallsMtx[6];
		DirectX::XMFLOAT4X4 m_mirrorMtx;
		DirectX::XMFLOAT4X4 m_mirroredWorldMtx;
		DirectX::XMFLOAT4X4 m_cylinderMtx;
		DirectX::XMFLOAT4X4 m_cylinderBaseMtx[2];
		DirectX::XMFLOAT4X4 m_helpPointMtx;

		DirectX::XMFLOAT4X4 m_projMtx;

		dx_ptr<ID3D11RasterizerState> m_rsCullNone;
		dx_ptr<ID3D11BlendState> m_bsAlpha;
		dx_ptr<ID3D11DepthStencilState> m_dssNoWrite;

		// DSS for reflected world in mirror
		dx_ptr<ID3D11DepthStencilState> m_dssWrite;
		dx_ptr<ID3D11DepthStencilState> m_dssTest;
		dx_ptr<ID3D11DepthStencilState> m_dssTestNoWrite;
		dx_ptr<ID3D11RasterizerState> m_rsCCW;

		//DSS for Shadow Volumes
		dx_ptr<ID3D11DepthStencilState> m_dsShadowWriteFront;
		dx_ptr<ID3D11DepthStencilState> m_dsShadowWriteBack;
		dx_ptr<ID3D11DepthStencilState> m_dsShadowTest;
		dx_ptr<ID3D11DepthStencilState> m_dsShadowTestComplement;

		dx_ptr<ID3D11InputLayout> m_inputlayout;

		PhongEffect m_phongEffect;
		ParticleSystem m_particles;

		DirectX::XMVECTOR m_swivelPos;
		DirectX::XMVECTOR m_swivelNorm;

		int m_contourEdges;
		std::shared_ptr<ID3D11Buffer> m_vbMesh[6];
		std::shared_ptr<ID3D11Buffer> m_ibMesh[6];
		std::shared_ptr<ID3D11Buffer> m_vbShadowVolume;
		std::shared_ptr<ID3D11Buffer> m_ibShadowVolume;
		bool cameraInShadow = false;
		DirectX::XMFLOAT4 v1, v2, v3;
		DirectX::XMFLOAT3 pos1, pos2, inter, inter1, inter2, inter3, inter4, inter5, inter6, dot1, dot2, dot3;
		float dotf;

		std::vector<Triangle> CreateTrianglesVector(std::vector<unsigned short> ind);
		void CreateRenderStates();
		void UpdateCameraCB(DirectX::XMMATRIX cameraMtx);
		void UpdateSwivel(float dt);
		void CalculateRobotAngles(DirectX::XMVECTOR pos, DirectX::XMVECTOR normal);

		void TurnOffLight();
		void TurnOnLight();

		void DrawMesh(const Mesh& m, DirectX::XMFLOAT4X4 worldMtx);
		void DrawParticles() const;
		void DrawSceneInMirror();

		void SetWorldMtx(DirectX::XMFLOAT4X4 mtx);

		void DrawScene();

		void UpdateShadowGeometry();
		void DrawShadowGeometry();
	};
}