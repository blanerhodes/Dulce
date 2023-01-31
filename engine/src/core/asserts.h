#pragma once

#include "defines.h"

//disable by commenting out the line below
#define DASSERTIONS_ENABLED

#ifdef DASSERTIONS_ENABLED
    #if _MSC_VER
        #include <intrin.h>
        #define debugBreak() __debugbreak()
    #else
        #define debugBreak() __builtin_trap()
    #endif

    DAPI void ReportAssertionFailure(char* expression, char* message, char* file, i32 line);

    #define DASSERT(expr)                                               \
        {                                                               \
            if(expr){                                                   \
            } else {                                                    \
                ReportAssertionFailure(#expr, "", __FILE__, __LINE__);  \
                debugBreak();                                           \
            }                                                           \
        }                                                               

    #define DASSERT_MSG(expr, message)                                      \
        {                                                                   \
            if(expr){                                                       \
            } else {                                                        \
                ReportAssertionFailure(#expr, message, __FILE__, __LINE__); \
                debugBreak();                                               \
            }                                                               \
        }                                                               
    
    #ifdef _DEBUG
        #define DASSERT_DEBUG(expr)                                      \
            {                                                                   \
                if(expr){                                                       \
                } else {                                                        \
                    ReportAssertionFailure(#expr, "", __FILE__, __LINE__); \
                    debugBreak();                                               \
                }                                                               \
            }                                                               
    #else
        #define DASSERT_DEBUG(expr)
    #endif
#else
    #define DASSERT(expr)
    #define DASSERT_MSG(expr, message)
    #define DASSERT_DEBUG(expr)
#endif