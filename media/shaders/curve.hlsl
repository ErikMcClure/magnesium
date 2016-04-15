cbuffer PSobj : register(b0) {
  float4x4 matViewProj;
  float4x4 matWorld;
}

struct VS_INPUT
{
	float4 p : POSITION0;
  float4 p0 : TEXCOORD0;
  float4 p1 : TEXCOORD1;
  float4 p2 : TEXCOORD2;
  float4 color : COLOR0;
};

struct PS_INPUT
{
	float4 p : SV_POSITION; 
  float3 texCoord : TEXCOORD0;
  float4 p0 : TEXCOORD1;
  float4 p1 : TEXCOORD2;
  float4 p2 : TEXCOORD3;
  float4 color : COLOR0;
};

PS_INPUT mainVS(VS_INPUT input)
{
  PS_INPUT o;
  float4x4 m = mul(matViewProj, matWorld);
  o.p = mul(m, input.p);
  o.texCoord = input.p.xyz;
  o.p0 = input.p0;
  o.p1 = input.p1;
  o.p2 = input.p2;
  o.color = input.color;
  return o;
}

// Proper implementation of a cube root that preserves the sign
inline float cbrt(float x)
{
   return pow(abs(x), 1.0/3.0)*sign(x);
}

// Solves a cubic of the form t^3 + at² + bt + c
inline float4 solveCubic(float a, float b, float c)
{
   float4 r = float4(0,0,0,0);
   float p = b - a*a / 3;
   float q = a * (2 * a*a - 9 * b) / 27 + c;
   float p3 = p*p*p;
   float d = q*q + 4 * p3 / 27;
   float offset = -a / 3;
   if(d >= 0) { // Single solution
      float z = sqrt(d);
      float u = (-q + z) / 2;
      float v = (-q - z) / 2;
      u = cbrt(u);
      v = cbrt(v);
      r.x = offset + u + v;
      r.w = 1;
      return r;
   }
   float u = sqrt(-p / 3);
   float v = acos(-sqrt(-27 / p3) * q / 2) / 3;
   float m = cos(v), n = sin(v)*((float)1.732050808);
   r.x = offset + u * (m + m);
   r.y = offset - u * (n + m);
   r.z = offset + u * (n - m);
   r.w = 3;
   return r;
}

float4 quadraticCurve(float t, float4 p0, float4 p1, float4 p2)
{
   float inv = 1.0 - t;
   return (inv*inv*p0) + (2*inv*t*p1) + (t*t*p2);
}

// First derivative of quadratic curve
float4 quadraticCurveD(float t, float4 p0, float4 p1, float4 p2)
{
   return 2*(1.0 - t)*(p1 - p0) + 2*t*(p2 - p1);
}

// Calculates f'(x)/f''(x) where f(x) is the distance f(x) = x(t)² + y(t)²
float quaditer(float t, float4 p0, float4 p1, float4 p2, float4 qqq, out float o)
{
   float4 q = quadraticCurve(t, p0, p1, p2);
   float4 qq = quadraticCurveD(t, p0, p1, p2);
   float d = (dot(q.xyz,qqq.xyz) + dot(qq.xyz,qq.xyz));
   o = dot(q.xyz,qq.xyz);
   return (d < 0.001)?0:(o/d);
}

float curvedist(float t, float4 p0, float4 p1, float4 p2)
{
  //if(cap.x > 0 && t < 0) t = 0;
  //if(cap.y > 0 && t > 1) t = 1;
  //if(t < 0 || t > 1) return 1e9;
  t = saturate(t);
  float4 q = quadraticCurve(t, p0, p1, p2);
  return length(q.xyz) - q.w;
}

float segmentdistance(float4 p0, float4 p1, out float b)
{
   float4 v = p1 - p0;

   float c1 = dot(-p0,v);
   if(c1 <= 0)
        return length(p0);

   float c2 = dot(v,v);
   if(c2 <= c1)
        return length(p1);

   b = c1 / c2;
   return length(p0 + b * v);
}

float2 calculatepixel(float4 p0, float4 p1, float4 p2)
{
   float3 A = p1.xyz - p0.xyz;
   float3 B = p2.xyz - p1.xyz - A;
   float3 M = p0.xyz;
   
   p0.w = abs(p0.w);
   p2.w = abs(p2.w);
   
   float a = dot(B, B);
   float b = 3 * dot(A, B);
   float c = 2 * dot(A, A) + dot(M, B);
   float d = dot(M, A);
   
   if(abs(a) < 0.001) a = 0.001; // This trick turns a perfectly linear line into an almost linear line that doesn't blow up the cubic.
   float4 r = solveCubic(b/a, c/a, d/a);
   float4 dd2 = 2*(p2 - 2*p1 + p0); // calculate the second derivative
   r.x = r.x - quaditer(r.x, p0, p1, p2, dd2, d); // Do a single newton-raphson iteration to smooth out floating point accuracy errors
   
   float dist = curvedist(r.x, p0, p1, p2);
   
   if(r.w > 1)
   {
      r.y = r.y - quaditer(r.y, p0, p1, p2, dd2, d);
      r.z = r.z - quaditer(r.z, p0, p1, p2, dd2, d);
      float dist2 = curvedist(r.y, p0, p1, p2);
      float dist3 = curvedist(r.z, p0, p1, p2);
      if(dist3 < dist2) { r.y = r.z; dist2 = dist3; }
      if(dist2 < dist) { r.x = r.y; dist = dist2; }
      //dist = min(dist, min(dist2, dist3));
   }
   return float2(dist, r.x);
}

float4 mainPS( PS_INPUT input ) : SV_Target
{
   float4 X = float4(input.texCoord, 0);
   float4 p0 = input.p0 - X; // Subtract X from all our points so that we only have to find the distance to the origin
   float4 p1 = input.p1 - X;
   float4 p2 = input.p2 - X;
   float2 cap = float2(input.p0.w, input.p2.w);
  
  float2 dist = calculatepixel(p0, p1, p2);
  float sd = dist.x/(abs(ddx(input.texCoord.x)) + abs(ddy(input.texCoord.y)));
    
  //return input.color;
  float alpha = -sd;  
  if(alpha < 0)
    clip(-1);
  if(alpha < 1 && ((cap.x < 0 && dist.y < 0) || (cap.y < 0 && dist.y > 1)))
    alpha*=0.35;
    
  return float4(input.color.rgb, smoothstep(0, 1, alpha)*input.color.a);
}

// Possible alternative implementation using line approximations
   /*float t = 0;
   const int slices = 4;
   const float slicesf = slices;
   float dist = length(p0.xyz) - p0.w;
   float4 lastp = p0;
   
   for(int i = 1; i <= slices; ++i)
   {
      float4 q = quadraticCurve(i/slicesf, p0, p1, p2);
     
      float db;
      float d = segmentdistance(lastp, q, db);
      if(d < dist)
      {
         dist = d;
         t = (i - 1 + db)/slicesf;
      }
      lastp = p1;
   }
   
  float4 dd2 = 2*(p2 - 2*p1 + p0);
  for(int i = 0; i < 4; ++i)
  {
    float dt;
    t = t - quaditer(t, p0, p1, p2, dd2, dt);
  }
  
  dist = curvedist(t, p0, p1, p2, cap);*/
