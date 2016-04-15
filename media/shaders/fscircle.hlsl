struct PS_INPUT
{
  float4 p : SV_POSITION;
  float4 xywh : TEXCOORD0;
  float4 arcs : TEXCOORD1;
  float outline : TEXCOORD2;
  float4 color : COLOR0;
  float4 outlinecolor : COLOR1;
};

float fmod(float n, float d)
{
  return n - floor(n/d)*d;
}

float getarc(float2 arcs, float angle, float anglew)
{
  const float PI = 3.14159265359;
  if(arcs.y > PI) // we can only deal with an empty circle, not a full circle, so we invert the arc if it's greater than PI
  {
    arcs.x += arcs.y; // Note: this is probably mathematically equivelent to constructing something using abs()
    arcs.y = PI*2 - arcs.y;
    // Copy paste from below becuase no recursive functions in HLSL :C
    angle = fmod(angle-arcs.x+anglew, PI*2) - anglew;
    return 1.0 - (smoothstep(-anglew, anglew, angle) - smoothstep(arcs.y-anglew, arcs.y+anglew, angle));
  }
  angle = fmod(angle-arcs.x+anglew, PI*2) - anglew; // Instead of enclosing within [0, 2PI], we shift to [-anglew, PI*2-anglew] which prevents the smoothstep from breaking
  return smoothstep(-anglew, anglew, angle) - smoothstep(arcs.y-anglew, arcs.y+anglew, angle);
}

float4 mainPS(PS_INPUT input) : SV_Target
{
  float2 d = input.xywh.zw;
  float2 p = input.xywh.xy/d;
  float angle = atan2(- p.y + 0.5, p.x - 0.5);
  float anglew = fwidth(angle)*0.5;
  float r = distance(p, float2(0.50, 0.50))*2;
  float w = fwidth(r)*0.5;
  float outline = (input.outline / d.x)*2;
  
  float s = 1 - smoothstep(1 - outline - w, 1 - outline + w, r);
  float alpha = smoothstep(1+w, 1-w, r);
  float4 fill = float4(input.color.rgb, 1);
  float4 edge = float4(input.outlinecolor.rgb, 1);
  return (fill*input.color.a*s*getarc(input.arcs.xy, angle, anglew)) + (edge*input.outlinecolor.a*saturate(alpha-s)*getarc(input.arcs.zw, angle, anglew));
}