cbuffer PSobj : register(b0) {
  float4x4 matViewProj;
  float4x4 matWorld;
}

struct VS_INPUT
{
  float4 pos : POSITION0;
  float4 dimpivot : TEXCOORD0;
  float4 corners : TEXCOORD1;
  float outline : TEXCOORD2; // thickness of the outline (subtracted from total dimensions to get appropriate starting point
  float4 color : COLOR0;
  float4 outlinecolor : COLOR1;
};

struct PS_INPUT
{
  float4 p : SV_POSITION;
  float4 xywh : TEXCOORD0;
  float4 corners : TEXCOORD1;
  float outline : TEXCOORD2;
  float4 color : COLOR0;
  float4 outlinecolor : COLOR1;
};

VS_INPUT mainVS(VS_INPUT input)
{
  return input;
}

[maxvertexcount(4)]
void mainGS(point VS_INPUT sprite[1], inout TriangleStream<PS_INPUT> triStream)
{
  float4 pos = sprite[0].pos;
  float2 pivot = sprite[0].dimpivot.zw;
  float2 dim = sprite[0].dimpivot.xy;
  float c = cos(pos[3]);
  float s = sin(pos[3]);
  float4x4 matTransform ={
    c, -s, 0, pos[0] - c*pivot[0] + pivot[0] + pivot[1]*s,
    s, c, 0, pos[1] - c*pivot[1] + pivot[1] - pivot[0]*s,
    0, 0, 1, pos[2],
    0, 0, 0, 1
  };
  float4x4 m = mul(matViewProj, matTransform);
    m = mul(m, matWorld);

  PS_INPUT v;
  v.corners = sprite[0].corners;
  v.outline = sprite[0].outline;
  v.color = sprite[0].color;
  v.outlinecolor = sprite[0].outlinecolor;

  //top left
  v.p = float4(0, 0, 0, 1);
  v.p = mul(m, v.p);
  v.xywh = float4(0, 0, dim.x, dim.y);
  triStream.Append(v);

  //bottom left
  v.p = float4(0, dim.y, 0, 1);
  v.p = mul(m, v.p);
  v.xywh = float4(0, dim.y, dim.x, dim.y);
  triStream.Append(v);

  //top right
  v.p = float4(dim.x, 0, 0, 1);
  v.p = mul(m, v.p);
  v.xywh = float4(dim.x, 0, dim.x, dim.y);
  triStream.Append(v);

  //bottom right
  v.p = float4(dim.x, dim.y, 0, 1);
  v.p = mul(m, v.p);
  v.xywh = float4(dim.x, dim.y, dim.x, dim.y);
  triStream.Append(v);
}

float4 mainPS(PS_INPUT input) : SV_Target
{
  float2 p = input.xywh.xy;
  float2 d = input.xywh.zw;
  float4 r = input.corners;
  float2 dist;
  float outline = input.outline;
  float w;
  if(p.x < r[0] && p.y < r[0]) {
    dist = float2(distance(p, float2(r[0],r[0])), input.outline) / r[0];
    w = fwidth(dist.x)*0.5;
  } else if(p.x > d.x - r[1] && p.y < r[1]) {
    dist = float2(distance(p, float2(d.x - r[1],r[1])), input.outline) / r[1];
    w = fwidth(dist.x)*0.5;
  } else if(p.x < r[2] && p.y > d.y - r[2]) {
    dist = float2(distance(p, float2(r[2],d.y - r[2])), input.outline) / r[2];
    w = fwidth(dist.x)*0.5;
  } else if(p.x > d.x - r[3] && p.y > d.y - r[3]) {
    dist = float2(distance(p, float2(d.x - r[3],d.y - r[3])), input.outline) / r[3];
    w = fwidth(dist.x)*0.5;
  } else {
    dist = min(p, d - p);
    dist.x = 1 - (min(dist.x,dist.y) / input.outline);
    dist.y = 1;
    w = (fwidth((p.x + p.y)/2) / input.outline)*0.5; // Don't use dist.x here because it's not continuous, which breaks ddx and ddy
  }
  
  float s = 1 - smoothstep(1 - dist.y - w, 1 - dist.y + w, dist.x);
  float alpha = smoothstep(1+w, 1-w, dist.x);
  float4 fill = float4(input.color.rgb, 1);
  float4 edge = float4(input.outlinecolor.rgb, 1);
  return (fill*input.color.a*s) + (edge*input.outlinecolor.a*saturate(alpha-s));
}