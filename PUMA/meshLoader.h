#pragma once
#include "vertexDef.h"
#include "mesh_structures.h"
#include <tuple>
#include <vector>

namespace mini
{
	class MeshLoader
	{
	public:
		using vpn_verts_t = std::vector<VertexPositionNormal>;
		using vp_verts_t = std::vector<DirectX::XMFLOAT3>;
		using indices_t = std::vector<unsigned short>;
		using vpn_mesh_t = std::tuple<vpn_verts_t, indices_t>;
		using vp_mesh_t = std::tuple<vp_verts_t, indices_t>;

		static vpn_mesh_t CreateBox(float side = 1.0f)
		{
			return CreateBox(side, side, side);
		}

		static vpn_mesh_t CreateBox(float width, float height, float depth)
		{
			return{ BoxVertices(width, height, depth), BoxIndices() };
		}

		static vpn_mesh_t CreatePentagon(float radius = 1.0f);

		static vpn_mesh_t CreateDoubleSidedSquare(float side = 1.0f)
		{
			return CreateDoubleSidedRectangle(side, side);
		}
		static vpn_mesh_t CreateDoubleSidedRectangle(float width, float height);

		static vp_mesh_t CreateSquareBilboard(float side = 1.0f)
		{
			return CreateRectangleBilboard(side, side);
		}

		static vp_mesh_t CreateRectangleBilboard(float width, float height);

		static vpn_mesh_t CreateSquare(float side = 1.0f)
		{
			return CreateRectangle(side, side);
		}
		static vpn_mesh_t CreateRectangle(float width, float height);

		static vpn_mesh_t CreateSphere(float radius, unsigned int stacks, unsigned int slices);

		static vpn_mesh_t CreateCylinder(float radius, float height, unsigned int stacks, unsigned int slices);

		static vpn_mesh_t CreateDisk(float radius, unsigned int slices);

		static vpn_mesh_t LoadMesh(const std::wstring& fileName);

		static vpn_mesh_t LoadMeshFromTXT(const std::wstring& filename, std::vector<Edge> &edges);

	private:

		static vpn_verts_t BoxVertices(float width, float height, float depth);
		static indices_t BoxIndices();
	};
}
