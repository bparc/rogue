fn v2 V2(f32 x, f32 y)
{
	v2 result = {x, y};
	return result;
}

fn v2 SV2(s32 x, s32 y)
{
	return V2((f32)x, (f32)y);
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

fn f32 DistanceV2S(v2s a, v2s b)
{
	v2s delta = SubS(a, b);
	return sqrtf((f32)(delta.x * delta.x + delta.y * delta.y));
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

fn v4 Lerp4(v4 a, v4 b, f32 t)
{
	v4 result = {0};
	result.x = Lerp(a.x, b.x, t);
	result.y = Lerp(a.y, b.y, t);
	result.z = Lerp(a.z, b.z, t);
	result.w = Lerp(a.w, b.w, t);
	return result;
}

fn b32 CompareVectors(v2s a, v2s b)
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

fn bb_t RectToBounds(v2 p, v2 sz)
{
	bb_t result = BB(p, Add(p, sz));
	return result;
}

fn v2 GetCenter(bb_t bb)
{
	v2 sz = Sub(bb.max, bb.min);
	v2 result = Add(bb.min, Scale(sz, 0.5f));
	return result;
}

fn v2s AddS(v2s a, v2s b)
{
	v2s result = {a.x + b.x, a.y + b.y};
	return result;
}

fn v2s SubS(v2s a, v2s b)
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

fn v2 Perp(v2 v)
{
	v2 result = {v.y, -v.x};
	return result;
}

fn v2 ClampLength(v2 v, float max)
{
	v2 result = v;
	if (Length(result) >= max)
		result = Scale(Normalize(v), max);
	return result;
}

fn v2 ClampToLine(v2 a, v2 b, v2 p)
{
	v2 v = Sub(b, a);
	v2 n = Normalize(v);
	f32 dot = Dot(Sub(p, a), n);
	f32 t = dot / Length(v);
	if (t <= 0.0f)
		p = a;
	else
	if ( t >= 1.0f)
		p = b;
	else
	{
		p = Scale(n, dot);
		p = Add(a, p);
	}
	return (p);
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