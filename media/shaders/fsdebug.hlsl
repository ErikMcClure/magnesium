float4 mainPS(float4 p : SV_POSITION) : SV_Target
{
  int px = (int)(p.x/ddx(p.x));
  int py = (int)(p.y/ddy(p.y));
  int i = (px % 2)^(py % 2);
  return float4(i, i, i, 1.0);
}