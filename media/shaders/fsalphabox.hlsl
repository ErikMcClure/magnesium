struct PS_INPUT
{
  float4 p : SV_POSITION;
  float2 t : TEXCOORD;
};

Texture2D<float4> diffuse : register(t0);
SamplerState samp : register(s0);

float4 mainPS(PS_INPUT input) : SV_Target
{
  float2 step = float2(ddx(input.t.x)*0.5, ddy(input.t.y)*0.5);
  uint4 c0 = diffuse.Sample(samp, input.t) * 255;
  uint4 c1 = diffuse.Sample(samp, input.t + float2(step.x, 0)) * 255;
  uint4 c2 = diffuse.Sample(samp, input.t + float2(0, step.y)) * 255;
  uint4 c3 = diffuse.Sample(samp, input.t + step) * 255;
  
  uint a = c0.a + c1.a + c2.a + c3.a;
  return float4(((c0.rgb*c0.a) + (c1.rgb*c1.a) + (c2.rgb*c2.a) + (c3.rgb*c3.a)) / a, a / 4) / float4(255, 255, 255, 255);
}