fn s32 StringLength(const char *string)
{
	s32 result = 0;
	while (*string++)
		result++;
	return result;
}

fn char ToUpper(char ch)
{
	char result = ch;
	if ((ch >= 'a') && (ch <= 'z'))
		result -= ('a' - 'A');
	return result;
}

fn f32 Sine(f32 x)
{
	f32 result = sinf(M_TAU32 * x);
	return result;
}

fn f32 Cosine(f32 x)
{
	f32 result = cosf(M_TAU32 * x);
	return result;
}

fn f32 SqRoot(f32 x)
{
	f32 result = sqrtf(x);
	return result;
}

fn f32 Lerp(f32 a, f32 b, f32 t)
{
	f32 result = a;
	if (t <= 0.0f)
	{
		result = a;
	}
	else
	if (t >= 1.0f)
	{
		result = b;
	}
	else
	{
		result = (b * t) + ((1.0f - t) * a);
	}
	return result;
}

fn f32 Smoothstep(f32 x, f32 edge)
{
	f32 result = 0.0f;
	if (x >= edge)
		result = (x - edge) / (1.0f - edge);
	return result;
}

fn s32 SRandIntFixed(s32 upper, s32 lower, s32 seed) {
	 if (upper < lower) { //swap upper lower if switched
        s32 temp = upper;
        upper = lower;
        lower = temp;
    }

    // LCG
    s32 rand_val = (s32)((((s32)LCG_A * (s32)seed) + LCG_C) % LCG_M);

    s32 range = upper - lower + 1;
    return lower + (rand_val % range);
}
fn s32 SRandInt(s32 seed) {
    // LCG
    s32 rand_val = (s32)((((s32)LCG_A * (s32)seed) + LCG_C) % LCG_M);
    return rand_val;
}

fn s32 RandomInt(void)
{
	s32 result = rand();
	return result;
}

fn f32 RandomFloat(void)
{
	s32 random_value = rand();
	f32 result = (f32)random_value / (f32)RAND_MAX;
	return result;
}

fn b32 RandomChance(s32 percentange)
{
	f32 random_value = RandomFloat();
	b32 result = (random_value <= ((f32)percentange / 100.0f));
	return result;
}

#define MIN(a, b) (a < b ? a : b) 
#define MAX(a, b) (a > b ? a : b)

fn s32 Clamp32(s32 value, s32 min, s32 max)
{
	if (value < min)
		value = min;
	if (value > max)
		value = max;
	return value;
}

fn s32 Min32(s32 a, s32 b)
{
	s32 result = MIN(a, b);
	return result;
}

fn u16 Min16U(u16 a, u16 b)
{
	u16 result = MIN(a, b);
	return result;
}

fn s32 MaxS32(s32 a, s32 b)
{
	s32 result = MAX(a, b);
	return result;
}

fn f32 MinF32(f32 a, f32 b)
{
	f32 result = MIN(a, b);
	return result;
}

fn f32 MaxF32(f32 a, f32 b)
{
	f32 result = MAX(a, b);
	return result;
}

static file_t OpenSystemFile(const char *path)
{
	file_t result={0};
	FILE *file = fopen(path,"rb");
	if(file)
	{
		fseek(file,0,SEEK_END);
		result.size=ftell(file);
		fseek(file,0,SEEK_SET);
		if(result.size)
		{
			result.bytes=malloc(result.size);
			fread(result.bytes,result.size,1,file);
		}
		fclose(file);
	}
	else
	{
		Error("Couldn't open a file!\n\n%s",path);
	}
	return result;
}

static void CloseSystemFile(file_t*file)
{
	if(file->bytes)
	{
		free(file->bytes);
	}
	memset(file,0,sizeof(*file));
}