/*
 * Copyright 2011 Alan Burlison, alan@bleaklow.com.  All rights reserved.
 * Use is subject to license terms.
 */

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#include "TaskScheduler.h"

TaskScheduler::TaskScheduler(Task **_tasks, uint8_t _numTasks) :
  tasks(_tasks),
  numTasks(_numTasks) {
}

void TaskScheduler::runTasks() {
    while (1) {
        uint32_t now = millis();
        Task **tpp = tasks;
        for (int t = 0; t < numTasks; t++) {
            Task *tp = *tpp;
            if (tp->canRun(now)) {
                #ifdef ARI_MEASURE_LOAD  
                tp->taskId2Dso();      
                #endif
                //check how late is the task
                tp->run(now);
                #ifdef ARI_MEASURE_LOAD             
                tp->task0Dso();      
                #endif
                break;
            }
            tpp++;
        }
    }
}
