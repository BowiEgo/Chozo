#define HALF_PI           1.5707963267948966
#define PI                3.141592653589793
#define TWO_PI            6.283185307179586
#define RECIPROCAL_PI     0.3183098861837907
#define RECIPROCAL_TWO_PI 0.15915494309189535
#define EPSILON           1e-6

#define MEDIUMP_FLT_MAX    65504.0
#define MEDIUMP_FLT_MIN    0.00006103515625

#ifdef TARGET_MOBILE
#define FLT_EPS            MEDIUMP_FLT_MIN
#define saturateMediump(x) min(x, MEDIUMP_FLT_MAX)
#else
#define FLT_EPS            1e-5
#define saturateMediump(x) x
#endif

float Saturate(float x) { return clamp(x, 0.0, 1.0); }

float pow2(const in float x) { return x * x; }
float pow3(const in float x) { return x * x * x; }
float pow4(const in float x) { float x2 = x * x; return x2 * x2; }
float pow5(const in float x) { float x2 = x * x; return x2 * x2 * x; }

highp float rand(const in vec2 uv)
{
    const highp float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );

    return fract( sin( sn ) * c );
}

// TODO: Move to common file
vec3 InverseTransformDirection( in vec3 dir, in mat4 matrix )
{
    // dir can be either a direction vector or a normal vector
    // upper-left 3x3 of matrix is assumed to be orthogonal

    return normalize( ( vec4( dir, 0.0 ) * matrix ).xyz );

}