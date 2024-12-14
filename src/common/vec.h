fn float step(float edge0, float edge1, float t);

typedef struct
{
	f32 x;
	f32 y;
} v2;

fn v2 V2(f32 x, f32 y);

fn v2 Add(v2 a, v2 b);
fn v2 Mul(v2 a, v2 b);
fn v2 Sub(v2 a, v2 b);
fn v2 Div(v2 a, v2 b);

fn v2 Scale(v2 a, f32 scalar);

fn v2 Sign2(v2 a);
fn v2 Lerp2(v2 a, v2 b, f32 t);
fn v2 Ease2(f32 min, f32 max, f32 mid, f32 t);

fn f32 Dot(v2 a, v2 b);
fn f32 Cross(v2 a, v2 b);
fn f32 LengthSq(v2 a);
fn f32 Length(v2 a);
fn f32 Distance(v2 a, v2 b);
fn v2 Normalize(v2 a);
fn v2 GetDirection(v2 a, v2 b);

fn v2 Rotate(v2 a);
fn v2 Invert(v2 a);

typedef struct
{
	f32 x;
	f32 y;
	f32 z;
	f32 w;
} v4;

fn v4 V4(f32 x, f32 y, f32 z, f32 w);
fn v4 RGB(u8 r, u8 g, u8 b); // NOTE(): Converts each component from 0-255 to 0.0-1.0 range.

fn v4 Lerp4(v4 a, v4 b, f32 t);

typedef struct
{
	s32 x;
	s32 y;
} v2s;

fn v2s V2S(s32 x, s32 y);
fn b32 IsZero(v2s x);

fn b32 CompareInts(v2s a, v2s b);

fn v2s IntClamp2(v2s a, v2s min, v2s max);

fn v2s IntAdd(v2s a, v2s b);
fn v2s IntMul(v2s a, v2s b);
fn v2s IntSub(v2s a, v2s b);
fn v2s IntDiv(v2s a, v2s b);

fn v2s IntHalf(v2s a);

fn f32 IntDistance(v2s a, v2s b);
fn s32 ManhattanDistance(v2s a, v2s b);
//

fn v2 ToFloat(v2s a);
fn v2s ToInt(v2 a);

fn v2s FloatSign(v2 a);
fn v2s IntSign(v2s a);

//

fn int IsInsideCircle(v2s realPos, v2s size, v2s center, s32 radius);

typedef struct
{
	v2 min;
	v2 max;
} bb_t;

fn bb_t Bounds(v2 min, v2 max);
fn bb_t RectBounds(v2 p, v2 sz);
fn v2 BoundsCenter(bb_t bb);
fn bb_t ShrinkBounds(bb_t bb, f32 amount);
fn bb_t StretchBounds(bb_t bb, f32 amount);
fn s32 BoundsContains(bb_t bb, v2 p);
fn s32 DoBoundingBoxesOverlap(bb_t bb1, bb_t bb2);