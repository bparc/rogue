typedef struct
{
	f32 x;
	f32 y;
} v2;

typedef struct
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} v4;

typedef struct
{
	s32 x;
	s32 y;
} v2s;

fn b32 IsZero(v2s x)
{
	b32 result = (x.x == 0 && x.y == 0);
	return result;
}
// NOTE(): Constructors.
fn v2s V2S(s32 x, s32 y);
fn v2 V2(f32 x, f32 y);
fn v2 SV2(s32 x, s32 y);
fn v4 V4(f32 x, f32 y, f32 z, f32 w);
fn v4 RGB(u8 r, u8 g, u8 b); // NOTE(): Converts each component from 0-255 to 0.0-1.0 range.

// NOTE(): Swizzling.
fn v2 YX(v2 v);
fn v2s YXs(v2s v);

// NOTE(): Interpolation.
fn v2 Lerp2(v2 a, v2 b, f32 t);
fn v4 Lerp4(v4 a, v4 b, f32 t);

// NOTE(): Boolean masks.
fn v2 GreaterThan(v2 a, v2 b);
fn v2 GreaterThanEqual(v2 a, v2 b);
fn v2 LessThan(v2 a, v2 b);
fn v2 LessThanEqual(v2 a, v2 b);
fn v2 Equal(v2 a, v2 b);
fn v2 NotEqual(v2 a, v2 b);

fn b32 CompareVectors(v2s a, v2s b);
// NOTE(): Arithmetic.
fn v2 Add(v2 a, v2 b);
fn v2 Mul(v2 a, v2 b);
fn v2 Sub(v2 a, v2 b);
fn v2 Div(v2 a, v2 b);
fn v2 Scale(v2 v, f32 scalar);

fn v2s AddS(v2s a, v2s b);
fn v2s MulS(v2s a, v2s b);
fn v2s SubS(v2s a, v2s b);
fn v2s DivS(v2s a, v2s b);
fn v2s ScaleS(v2 v, s32 scalar);

// NOTE(): Vector Calculus/Geometric.
fn f32 Dot(v2 a, v2 b);
fn f32 Cross(v2 a, v2 b);

fn f32 LengthSq(v2 v); // NOTE(): LengthSquared
fn f32 Length(v2 v);
fn f32 Distance(v2 a, v2 b);
fn f32 DistanceV2S(v2s a, v2s b);

fn v2 Perp(v2 v); // NOTE(): Perpendicular
fn v2 Invert(v2 v);
fn v2 Normalize(v2 v);
fn v2 GetDirection(v2 a, v2 b);

fn v2 ClampLength(v2 v, float max);
fn v2 ClampToLine(v2 a, v2 b, v2 p);

typedef struct
{
	v2 min;
	v2 max;
} bb_t;

fn bb_t Bounds(v2 min, v2 max);
fn bb_t RectToBounds(v2 p, v2 sz);
fn v2 GetCenter(bb_t bb);

fn int IsInsideCircle(v2s realPos, v2s size, v2s center, s32 radius);