struct PS_INPUT
{
  float4 p : SV_POSITION;
  float4 color : COLOR;
  float2 t : TEXCOORD;
};

Texture2D<float4> diffuse : register(t0);
SamplerState samp : register(s0);

float4 mainPS(PS_INPUT input) : SV_Target
{
  float2 dt = float2(ddx(input.t.x),ddy(input.t.y));
  float2 dir = normalize(float2(1.0, 0.0));
  float dist = 32;
  int len = (int)dist;
  float4 color = float4(0,0,0,0);
  for(int i = 0; i < len; ++i)
  {
    color += diffuse.Sample(samp, input.t + dt*dir*i);
  }
  return (color/dist)*input.color;
}