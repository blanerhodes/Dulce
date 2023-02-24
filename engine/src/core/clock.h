#pragma once

#include "defines.h"

struct Clock{
    f64 startTime;
    f64 elapsed;
};

//should be called just before checking elapsed time
DAPI void ClockUpdate(Clock* clock);

//resets elapsed time
DAPI void ClockStart(Clock* clock);

//doesnt reset elapsed time
DAPI void ClockStop(Clock* clock);