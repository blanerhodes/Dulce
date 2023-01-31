#include <stdint.h>

#define PI32 3.14159265395f
#define F32Max FLT_MAX
#define U32Max ((uint32_t)-1)
#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
#define KiloBytes(value) ((value)*1024LL)
#define MegaBytes(value) (KiloBytes(value) * 1024LL)
#define GigaBytes(value) (MegaBytes(value) * 1024LL)
#define TeraBytes(value) (GigaBytes(value) * 1024LL)

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef int32_t b32;
typedef uint8_t b8;

#define TRUE 1
#define FALSE 0

#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))
#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

#if defined(__clang__) || defined(__gcc__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
    #define DPLATFORM_WINDOWS 1
    #ifndef _WIN64
        #error "64-bit is required on Windows!"alignas
    #endif
#endif

#ifdef DEXPORT
    #ifdef _MSC_VER
        #define DAPI __declspec(dllexport)
    #else
        #define DAPI __attribute__((visibility("default")))
    #endif
#else
    #ifdef _MSC_VER
        #define DAPI __declspec(dllimport)
    #else
        #define DAPI
    #endif
#endif
