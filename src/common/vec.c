fn v2 V2(f32 x, f32 y)
{
	v2 result = {x, y};
	return result;
}

fn v2 ToFloat(v2s v)
{
	return (v2) {(f32)v.x, (f32)v.y};
}

fn v2s ToInt(v2 v)
{
	return (v2s) {(s32)v.x, (s32)v.y};
}

fn v2 Scale(v2 v, f32 scalar)
{
	v2 result = {v.x * scalar, v.y * scalar};
	return result;
}

fn v2 Add(v2 a, v2 b)
{
	v2 result = {a.x + b.x, a.y + b.y};
	return result;
}

fn v2 Mul(v2 a, v2 b)
{
	v2 result = {a.x * b.x, a.y * b.y};
	return result;
}

fn v2 Sub(v2 a, v2 b)
{
	v2 result = {a.x - b.x, a.y - b.y};
	return result;
}

fn v2 Div(v2 a, v2 b)
{
	v2 result = {a.x / b.x, a.y / b.y};
	return result;
}

fn f32 Dot(v2 a, v2 b)
{
	f32 result = a.x * b.x + a.y * b.y;
	return result;
}

fn f32 Cross(v2 a, v2 b)
{
	f32 result = a.x * b.y - b.x * a.y;
	return result;
}

fn f32 LengthSq(v2 v)
{
	f32 result = Dot(v, v);
	return result;
}

fn f32 Length(v2 v)
{
	f32 result = SqRoot(LengthSq(v));
	return result;
}

fn f32 Distance(v2 a, v2 b)
{
	f32 result = Length(Sub(b, a));
	return result;
}

fn f32 IntDistance(v2s a, v2s b)
{
	v2s delta = IntSub(a, b);
	return sqrtf((f32)(delta.x * delta.x + delta.y * delta.y));
}

fn s32 ManhattanDistance(v2s a, v2s b)
{
	s32 Result = abs(b.x - a.x) + abs(b.y - a.y);
	return Result;
}

fn v2 Invert(v2 v)
{
	v2 result = {-v.x, -v.y};
	return result;
}

fn v2 Normalize(v2 v)
{
	v2 result = v;
	f32 length = Length(v);
	if (length > 0.0f)
	{
		result.x /= length;
		result.y /= length;
	}
	return result;
}

fn v2 GetDirection(v2 a, v2 b)
{
	v2 result = Normalize(Sub(b, a));
	return result;
}

fn v2 Sign2(v2 v)
{
	v2 result = {0};
	result.x = v.x >= 0.0f ? 1.0f : -1.0f;
	result.y = v.y >= 0.0f ? 1.0f : -1.0f;
	return result;
}

fn v2 Vec(v2 a, v2 b)
{
	v2 result = Sub(b, a);
	return result;
}

fn v4 V4(f32 x, f32 y, f32 z, f32 w)
{
	v4 result = {x, y, z, w};
	return result;
}

fn v4 RGB(u8 r, u8 g, u8 b)
{
	v4 result = {0};
	result.x = (f32)r / 255.0f;
	result.y = (f32)g / 255.0f;
	result.z = (f32)b / 255.0f;
	result.w = 1.0f;
	return result;
}

fn v2 Lerp2(v2 a, v2 b, f32 t)
{
	v2 result = {0};
	result.x = Lerp(a.x, b.x, t);
	result.y = Lerp(a.y, b.y, t);
	return result;
}

fn v2 EaseIn2(v2 a, v2 b, f32 t)
{
	v2 result = Lerp2(a, b, t * t * t);
	return result;
}

fn v2 EaseOut2(v2 a, v2 b, f32  t)
{
	t = 1.0f - t;
	v2 result = Lerp2(b, a, t * t);
	return result;
}

fn v2 Ease2(f32 min, f32 max, f32 mid, f32 t)
{
	v2 result = {0};

	f32 t0 = t;
	f32 t1 = Smoothstep(t0, 0.9f);
	t1 *= t1;

	f32 t2 = Smoothstep(1.0f - t0, 0.95f);
	t2 *= t2;
	
	f32 t3 = Smoothstep(t0, 0.2f);

	result.x = (min * t1) - (max * t2) + (t3 * mid);
	result.y = t1 + t2;

	return result;
}

fn v4 Lerp4(v4 a, v4 b, f32 t)
{
	v4 result = {0};
	result.x = Lerp(a.x, b.x, t);
	result.y = Lerp(a.y, b.y, t);
	result.z = Lerp(a.z, b.z, t);
	result.w = Lerp(a.w, b.w, t);
	return result;
}

fn b32 CompareInts(v2s a, v2s b)
{
	b32 result = (a.x == b.x) && (a.y == b.y);
	return result;
}

fn v2s V2S(s32 x, s32 y)
{
	v2s result = {x, y};
	return result;
}

fn bb_t BB(v2 min, v2 max)
{
	bb_t result = {min, max};
	return result;
}

fn bb_t RectBounds(v2 p, v2 sz)
{
	bb_t result = BB(p, Add(p, sz));
	return result;
}

fn v2 BoundsCenter(bb_t bb)
{
	v2 sz = Sub(bb.max, bb.min);
	v2 result = Add(bb.min, Scale(sz, 0.5f));
	return result;
}

fn bb_t StretchBounds(bb_t bb, f32 amount)
{
	bb.min.x -= amount;
	bb.min.y -= amount;
	bb.max.x += amount;
	bb.max.y += amount;
	return bb;
}

fn bb_t ShrinkBounds(bb_t bb, f32 amount)
{
	bb.min.x += amount;
	bb.min.y += amount;
	bb.max.x -= amount;
	bb.max.y -= amount;
	return bb;
}

fn v2s IntAdd(v2s a, v2s b)
{
	v2s result = {a.x + b.x, a.y + b.y};
	return result;
}

fn v2s IntMul(v2s a, v2s b)
{
	v2s result = {a.x * b.x, a.y * b.y};
	return result;
}

fn v2s IntDiv(v2s a, v2s b)
{
	v2s result = {a.x / b.x, a.y / b.y};
	return result;
}

fn v2s IntHalf(v2s a)
{
	v2s Result = a;
	Result.x /= 2;
	Result.y /= 2;
	return Result;
}

fn v2s IntSub(v2s a, v2s b)
{
	v2s result = {a.x - b.x, a.y - b.y};
	return result;
}

fn v2s Abs2S(v2s v)
{
	v2s result = {0};
	result.x = abs(v.x);
	result.y = abs(v.y);
	return result;
}

fn v2 Rotate(v2 v)
{
	v2 result = {v.y, v.x};
	return result;
}

fn v2s RotateSigned(v2s v)
{
	v2s result = {v.y, v.x};
	return result;
}

fn v2s IntClamp2(v2s v, v2s min, v2s max)
{
	v2s result = v;
	result.x = Clamp(result.x, min.x, max.x);
	result.y = Clamp(result.y, min.y, max.y);
	return result;
}

fn v2s Up(void)
{
	return V2S(0, -1);
}

fn v2s Down(void)
{
	return V2S(0, +1);
}

fn v2s Left(void)
{
	return V2S(-1, 0);
}

fn v2s Right(void)
{
	return V2S(+1, 0);
}

fn v4 Scale4(v4 v, f32 scalar)
{
	v4 result = { v.x * scalar, v.y * scalar,
	v.z * scalar, v.w *scalar};
	return result;
}

fn v4 Scale3(v4 v, f32 scalar)
{
	v4 result = { v.x * scalar, v.y * scalar,
	v.z * scalar, v.w};
	return result;
}

//radius calc (manhattan distance)
fn int IsInsideCircle(v2s position, v2s size, v2s center, s32 radius) {
   s32 radiusSquared = radius * radius;
    int totalPoints = size.x * size.y;  //all squares
    int insideCount = 0; 

    for (int y = 0; y < size.y; y++) {
        for (int x = 0; x < size.x; x++) {
            v2s point = {position.x + x, position.y + y};  

            s32 dx = point.x - center.x;
            s32 dy = point.y - center.y;
            s32 distanceSquared = dx * dx + dy * dy;

            if (distanceSquared < radiusSquared) {
                insideCount++; 
            }
        }
    }

    float percentageInside = (float)insideCount / totalPoints;

    //if 75% of entity is within circle then its attackable
    return percentageInside >= 0.75f;

}

fn v2 GreaterThanEqual(v2 a, v2 b)
{
	v2 result = {0};
	result.x = a.x >= b.x ? 1.0f : 0.0f;
	result.y = a.y >= b.y ? 1.0f : 0.0f;
	return result;
}

fn bb_t Bounds(v2 min, v2 max)
{
	bb_t result = {min, max};
	return result;
}

fn v2 TopMaxCorner(bb_t bb)
{
	return(V2(bb.max.x, bb.min.y));
}

fn v2 BotMinCorner(bb_t bb)
{
	return(V2(bb.min.x, bb.max.y));
}

fn v2 Ratio(v2 sides, float ratio)
{
    v2 result = sides;
    result.x = 0.0f;
    result.y = 0.0f;
    
    if(sides.x/sides.y>ratio)
    {
        result.x = (sides.y/sides.x)*ratio;
        result.y = 1.0f;
    }
    else
    {
        result.y = (sides.x/sides.y)/ratio;
        result.x = 1.0f;
    }
    
    result = Mul(result, sides);
    return result;
}

fn s32 BoundsContains(bb_t bb, v2 p)
{
	s32 result =
		(p.x >= bb.min.x && p.x < bb.max.x) &&
		(p.y >= bb.min.y && p.y < bb.max.y);
	return result;
}

fn s32 IsBoundingBoxInBounds(bb_t bb, bb_t bb2)
{
	s32 result =
		(bb2.min.x >= bb.min.x && bb2.max.x < bb.max.x) &&
		(bb2.min.y >= bb.min.y && bb2.max.y < bb.max.y);
	return result;
}

fn s32 DoBoundingBoxesOverlap(bb_t bb1, bb_t bb2) {
    // Basic overlap check (any intersection)
    b32 basicOverlap = (bb1.min.x < bb2.max.x && bb1.max.x > bb2.min.x) &&
                       (bb1.min.y < bb2.max.y && bb1.max.y > bb2.min.y);

    // Return true only if they overlap without full containment
    return basicOverlap;
}


fn b32 IsZero(v2s x)
{
	b32 result = (x.x == 0 && x.y == 0);
	return result;
}

fn v2s FloatSign(v2 a)
{
	v2s result = {0};
	result.x = a.x >= 0.0f ? 1 : -1;
	result.y = a.y >= 0.0f ? 1 : -1;
	return result;
}

fn v2s IntSign(v2s a)
{
	v2s result = {0};
	result.x = a.x >= 0 ? 1 : -1;
	result.y = a.y >= 0 ? 1 : -1;
	return result;
}

static float fclampf(float a, float min, float max)
{
	float Result = a;
	if (a <= min)
		Result = min;
	if (a >= max)
		Result = max;
	return Result;
}

static float step(float edge0, float edge1, float t)
{
	float Result = 0.0f;
	Result = fclampf((t - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	return Result;
}