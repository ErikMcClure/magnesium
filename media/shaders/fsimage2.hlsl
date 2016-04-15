cbuffer PSobj : register(b0) {
  float4x4 matViewProj;
  float4x4 matWorld;
}

struct VS_INPUT
{
	float4 pos : POSITION0;
	float4 dimpivot : TEXCOORD0;
  float4 uv : TEXCOORD1;
  float4 uv2 : TEXCOORD2;
  float4 color : COLOR0;
};

struct PS_INPUT
{
	float4 p : SV_POSITION; 
  float4 color : COLOR;
  float2 t : TEXCOORD0;
  float2 t2 : TEXCOORD1;
};

VS_INPUT mainVS(VS_INPUT input)
{
	return input;  
}

[maxvertexcount(4)]
void mainGS(point VS_INPUT sprite[1], inout TriangleStream<PS_INPUT> triStream)
{
  float4 pos = sprite[0].pos;
  float4 uv = sprite[0].uv;
  float4 uv2 = sprite[0].uv2;
  float2 pivot = sprite[0].dimpivot.zw;
  float2 dim = sprite[0].dimpivot.xy;
  float c = cos(pos[3]);
  float s = sin(pos[3]);
  float4x4 matTransform = { 
    c, -s, 0, pos[0] - c*pivot[0] + pivot[0] + pivot[1]*s,
    s,  c, 0, pos[1] - c*pivot[1] + pivot[1] - pivot[0]*s,
    0,  0, 1, pos[2],
    0,  0, 0, 1
    };
  float4x4 m = mul(matViewProj, matTransform);
  m = mul(m, matWorld);
  
  PS_INPUT v;
  v.color = sprite[0].color;

  //top left
  v.p = float4(0,0,0,1);
  v.p = mul(m, v.p);
  v.t = uv.xy;
  v.t2 = uv2.xy;
  triStream.Append(v);
  
  //bottom left
  v.p = float4(0,dim.y,0,1);
  v.p = mul(m, v.p);
  v.t = uv.xw;
  v.t2 = uv2.xw;
  triStream.Append(v);

  //top right
  v.p = float4(dim.x,0,0,1);
  v.p = mul(m, v.p);
  v.t = uv.zy;
  v.t2 = uv2.zy;
  triStream.Append(v);

  //bottom right
  v.p = float4(dim.x,dim.y,0,1);
  v.p = mul(m, v.p);
  v.t = uv.zw;
  v.t2 = uv2.zw;
  triStream.Append(v);
}

Texture2D<float4> diffuse : register(t0);
SamplerState samp : register(s0);
Texture2D<float4> diffuse2 : register(t1);
SamplerState samp2 : register(s1);

float4 mainPS(PS_INPUT input) : SV_Target
{ 
  return diffuse2.Sample(samp2, input.t2)*diffuse.Sample(samp, input.t)*input.color;
}