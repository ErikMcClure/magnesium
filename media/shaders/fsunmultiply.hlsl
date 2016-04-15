struct PS_INPUT
{
	float4 p : SV_POSITION;
	float2 t : TEXCOORD0;
};

Texture2D<float4> diffuse : register(t0);
SamplerState samp : register(s0);

float4 mainPS(PS_INPUT input) : SV_Target
{
	float4 c = diffuse.Sample(samp, input.t);
  return float4(c.rgb/(c.a+0.004), c.a);
  return float4(c.rgb/c.a, c.a);
}