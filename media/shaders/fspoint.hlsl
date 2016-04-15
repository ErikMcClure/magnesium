cbuffer PSobj : register(b0) {
  float4x4 matViewProj;
  float4x4 matWorld;
}

//cbuffer PTobj : register(b1) {
//  float size;
//}

struct VS_INPUT
{
	float4 pos : POSITION0;
  float4 color : COLOR0;
};

struct PS_INPUT
{
	float4 p : SV_POSITION; 
  float4 color : COLOR;
	float2 t : TEXCOORD;
};

VS_INPUT mainVS(VS_INPUT input)
{
  return input;
}

[maxvertexcount(4)]
void mainGS(point VS_INPUT sprite[1], inout TriangleStream<PS_INPUT> triStream)
{
  float4 pos = sprite[0].pos;
  float4x4 m = mul(matViewProj, matWorld);
  float hsize = pos[3]*0.5;

  PS_INPUT v;
  v.color = sprite[0].color;

  //top left
  v.p = float4(pos[0] - hsize, pos[1] - hsize, pos[2], 1);
  v.p = mul(m, v.p);
  v.t = float2(0,0);
  triStream.Append(v);
  
  //bottom left
  v.p = float4(pos[0] - hsize, pos[1] + hsize, pos[2], 1);
  v.p = mul(m, v.p);
  v.t = float2(0,1);
  triStream.Append(v);

  //top right
  v.p = float4(pos[0] + hsize, pos[1] - hsize, pos[2], 1);
  v.p = mul(m, v.p);
  v.t = float2(1,0);
  triStream.Append(v);

  //bottom right
  v.p = float4(pos[0] + hsize, pos[1] + hsize, pos[2], 1);
  v.p = mul(m, v.p);
  v.t = float2(1,1);
  triStream.Append(v);
}

Texture2D<float4> diffuse : register(t0);
SamplerState samp : register(s0);

float4 mainPS(PS_INPUT input) : SV_TARGET
{
	return diffuse.Sample(samp, input.t)*input.color;
}