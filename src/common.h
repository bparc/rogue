#include "types.h"
#include <stdarg.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdio.h>
#define fn static
#define ArraySize(Array) (sizeof(Array) / sizeof(*Array))

#define KB(Bytes) (Bytes*1024)
#define MB(Bytes) (Bytes*1024*1024)
#define GB(Bytes) (Bytes*1024*1024*1024)

#define ZeroStruct(Struct) memset(Struct, 0, sizeof(*Struct))

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif

typedef struct 
{
	int32_t size;
	uint8_t *bytes;
} file_t;

fn file_t OpenSystemFile(const char *path);
fn void CloseSystemFile(file_t*file);
fn void Error(const char *format, ...);

fn void _Assert(const char *message, const char *file, const char *function, int32_t line);
#define Assert(expression) (void) ((!!(expression)) || \
(_Assert(#expression, __FILE__, __FUNCTION__, __LINE__), 0))
#ifdef _DEBUG
#define DebugAssert(...) Assert(__VA_ARGS__)
#else
#define DebugAssert(...)
#endif

#ifdef _DEBUG
#include <stdio.h>
#else
#endif

// NOTE(): ANSI
fn s32 StringLength(const char *string);
fn char ToUpper(char ch);

#define _USE_MATH_DEFINES
#include <math.h>
#define M_TAU32 (f32)(M_PI * 2.0)

// NOTE(): Numerical
fn f32 Sine(f32 x);
fn f32 Cosine(f32 x);
fn f32 SqRoot(f32 x);

// NOTE(): Interpolation
fn f32 Lerp(f32 a, f32 b, f32 t);
fn f32 Smoothstep(f32 x, f32 edge);

// Constants for LCG
#define LCG_A 1664525
#define LCG_C 1013904223
#define LCG_M 4294967296 // 2^32

fn s32 SRandIntFixed(int upper, int lower, int seed); //rand in range with seed
fn s32 SRandInt(int seed); //rand in 0 - 2^32 range with seed
fn b32 RandomChance(s32 percentange);

fn s32 RandomInt(void);
fn u16 Min16U(u16 a, u16 b);
fn s32 Min32(s32 a, s32 b);
fn s32 Clamp32(s32 value, s32 min, s32 max);