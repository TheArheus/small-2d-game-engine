#include <stdint.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef uint64_t    i64;

typedef float       r32;
typedef double      r64;
typedef int32_t     b32;

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

#define Min(a, b) ((a < b) ? a : b)
#define Max(a, b) ((a > b) ? a : b)
