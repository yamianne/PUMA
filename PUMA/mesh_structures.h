#pragma once
struct Edge {
	int vertexIdx1;
	int vertexIdx2;
	int triangleIdx1;
	int triangleIdx2;
};

struct Triangle {
	Triangle() {}
	Triangle(unsigned short v1, unsigned short v2, unsigned short v3)
	{
		vertexIdx1 = v1; vertexIdx2 = v2; vertexIdx3 = v3;
	}
	unsigned short vertexIdx1;
	unsigned short vertexIdx2;
	unsigned short vertexIdx3;
};

struct VertexTriangle {
	DirectX::XMVECTOR v1;
	DirectX::XMVECTOR v2;
	DirectX::XMVECTOR v3;
};