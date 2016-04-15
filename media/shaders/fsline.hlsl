cbuffer PSobj : register(b0) {
  float4x4 matViewProj;
  float4x4 matWorld;
}

struct VS_INPUT
{
	float4 pos : POSITION0;
  float4 color : COLOR0;
};

struct PS_INPUT
{
	float4 p : SV_POSITION; 
  float4 color : COLOR;
};

PS_INPUT mainVS(VS_INPUT input)
{
  float4x4 m = mul(matViewProj, matWorld);
  PS_INPUT o;
  o.p = mul(m, input.pos);
  o.color = input.color;
	return o;
}

float4 mainPS(PS_INPUT input) : SV_Target
{ 
	return input.color;
}