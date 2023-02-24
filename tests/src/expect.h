#include <core/logger.h>
#include <math/dmath.h>

#define ExpectIntEquals(expected, actual)                                                               \
    if(actual != expected){                                                                             \
        DERROR("--> Expected %lld, but got: %lld. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                   \
    }

#define ExpectIntNotEquals(expected, actual)                                                                     \
    if(actual == expected){                                                                                      \
        DERROR("--> Expected %d != %d, but they are equal. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                                            \
    }

#define ExpectFloatEquals(expected, actual)                                                         \
    if(dabs(expected - actual) > 0.001f){                                                           \
        DERROR("--> Expected %f, but got: %f. File: %s:%d.", expected, actual, __FILE__, __LINE__); \
        return false;                                                                               \
    }

#define ExpectTrue(actual)                                                             \
    if(actual != true){                                                                \
        DERROR("--> Expected true, but got: false. File: %s:%d.", __FILE__, __LINE__); \
        return false;                                                                  \
    }

#define ExpectFalse(actual)                                                            \
    if(actual != false){                                                               \
        DERROR("--> Expected false, but got: true. File: %s:%d.", __FILE__, __LINE__); \
        return false;                                                                  \
    }