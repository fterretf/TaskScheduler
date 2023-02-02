/*
 * Copyright 2011 Alan Burlison, alan@bleaklow.com.  All rights reserved.
 * Use is subject to license terms.
 */

#include "Task.h"

// Virtual.
bool TriggeredTask::canRun(uint32_t now) {
    return runFlag;
}

// Virtual.
bool TimedTask::canRun(uint32_t now) {
    bool run = (now >= runTime);
    #ifdef ARI_MEASURE_LOAD  
    if (run){
        uint32_t diff;
        diff = now - runTime;
        _queue->push(&diff);
    }
    #endif
    return run;
}
