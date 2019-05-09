cbuffer cbProj : register(b0) //Geometry Shader constant buffer slot 0
{
	matrix projMatrix;
};

struct GSInput
{
	float4 pos : POSITION0;
	float4 prev : POSITION1;
	float age : TEXCOORD0;
	float angle : TEXCOORD1;
	float size : TEXCOORD2;
};

struct PSInput
{
	float4 pos : SV_POSITION;
	float2 tex1: TEXCOORD0;
	float2 tex2: TEXCOORD1;
};

static const float TimeToLive = 4.0f;

[maxvertexcount(4)]
void main(point GSInput inArray[1], inout LineStream<PSInput> ostream)
{
	GSInput i = inArray[0];
	//float dy = 0.01f;

	PSInput o = (PSInput)0;

	o.tex2 = float2(i.age / TimeToLive, 0.5f);
	//if (i.pos.y < i.prev.y)
	//{
	//	dy = -dy;
	//}

	o.pos = i.pos;
	o.pos = mul(projMatrix, o.pos);
	o.tex1 = float2(0.0f, 1.0f);
	ostream.Append(o);

	//o.pos = i.pos + float4(dy, 0, 0.0f, 0.0f);
	//o.pos = mul(projMatrix, o.pos);
	//o.tex1 = float2(1.0f, 1.0f);
	//ostream.Append(o);

	o.pos = i.prev;
	o.pos = mul(projMatrix, o.pos);
	o.tex1 = float2(0.0f, 0.0f);
	ostream.Append(o);

	//o.pos = i.prev + float4(dy, 0, 0.0f, 0.0f);
	//o.pos = mul(projMatrix, o.pos);
	//o.tex1 = float2(1.0f, 0.0f);
	//ostream.Append(o);

	ostream.RestartStrip();
}