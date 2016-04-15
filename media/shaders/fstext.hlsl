struct PS_INPUT
{
  float4 p : SV_POSITION;
  float4 color : COLOR;
  float2 t : TEXCOORD;
};

Texture2D<float4> diffuse : register(t0);
SamplerState samp : register(s0);

struct PS_OUTPUT
{
  float4 alpha : SV_TARGET0;
  float4 color : SV_TARGET1;
};

PS_OUTPUT mainPS(PS_INPUT input) : SV_Target
{
  PS_OUTPUT r;
  r.alpha = diffuse.Sample(samp, input.t)*input.color.a; // premultiply by the alpha channel
  r.color = input.color; // alpha channel is ignored on color
  return r;
}