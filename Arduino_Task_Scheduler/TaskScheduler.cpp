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
#define MCU_PIN_NB15 15 // A0 disconnected! Use by ANA (and Audio IN) 

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
                #ifdef ARI_MEASURE_LOAD_A0234     
                pinMode(MCU_PIN_NB15, OUTPUT);             
                digitalWrite(MCU_PIN_NB15, 1);         
                #endif
                tp->run(now);
                #ifdef ARI_MEASURE_LOAD_A0234                
                digitalWrite(MCU_PIN_NB15, 0);         
                #endif
                break;
            }
            tpp++;
        }
    }
}
