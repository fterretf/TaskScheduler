/*
 * Copyright 2011 Alan Burlison, alan@bleaklow.com.  All rights reserved.
 * Use is subject to license terms.
 */

/*
 * Simple task classes.
 */

#ifndef Task_h
#define Task_h

#include <stdint.h>
#include <Arduino.h>
#include "pin.h"
#include "cppQueue.h"

// Maximum time into the future - approximately 50 days.
#define MAX_TIME UINT32_MAX

/*
 * A simple (abstract) task - base class for all other tasks.
 */
class Task {
/*
*	Newbie Note: The "= 0;" following each virtual method 
*	in the Task Class makes it an "abstract" or "pure virtual"
*	method in C++. These methods *must* be defined/implemented in the
*	derived class. These are *only* Task Class. -KG 3-19-2019
*/

public:

    Task(uint8_t taskId ):_id(taskId)
    {};

    /*
     * Can the task currently run?
     * now - current time, in milliseconds.
     */
    virtual bool canRun(uint32_t now) = 0;		//<--ABSTRACT

    /*
     * Run the task,
     * now - current time, in milliseconds.
     */
    virtual void run(uint32_t now) = 0;			//<--ABSTRACT

    virtual void taskId2Dso(){};
    virtual void task0Dso(){};    

protected:
    uint8_t _id;
};

/*
 * A task that is triggered by an external event.
 */
class TriggeredTask : public Task {

public:
    /*
     * Can the task currently run?
     * now - current time, in milliseconds.
     */
    virtual bool canRun(uint32_t now);

    /*
     * Mark the task as runnable.
     */
    inline void setRunnable() { runFlag = true; }

    /*
     * Mark the task as non-runnable.
     */
    inline void resetRunnable() { runFlag = false; }

protected:
    bool runFlag;   // True if the task is currently runnable.
};

/*
 * A task that is run on a periodic basis.
 */
class TimedTask : public Task {

public:
    /*
     * Create a periodically executed task.
     * when - the system clock tick when the task should run, in milliseconds.
     */
    inline TimedTask(uint8_t id, uint32_t when): Task(id) {
        runTime = when; 

        #ifdef ARI_MEASURE_LOAD  
        //                     size_rec          nb_recs                       cppQueueType overwrite pQDat     sizeof pQDat 
        _queue = new cppQueue(sizeof(uint32_t), sizeof(_queueItems)/sizeof(uint32_t), FIFO,true, _queueItems, sizeof(_queueItems));
        memset(_queueItems,0xFF,sizeof(_queueItems));
        #endif        
    }

    /*
     * Can the task currently run?
     * now - current system clock tick, in milliseconds.
     */
    virtual bool canRun(uint32_t now);

    /*
     * Set the system clock tick when the task can next run.
     * when - the system clock tick when the task should run, in milliseconds.
     */
    inline void setRunTime(uint32_t when) { runTime = when; }

    /*
     * Increment the system clock tick when the task can next run.
     * inc - system clock increment, in milliseconds.
     */
    inline void incRunTime(uint32_t inc) { runTime += inc; }

    /*
     * Get the system clock tick when the task can next run.
     * return - system clock tick when the task is next due to run.
     */
    inline uint32_t getRunTime() { return runTime; }

    /*
    +------------+------------------+--------+-----------------+--------+-----------------------+---------+---------+--------+--------+----------+----------+
    | Pin number |  MKR  Board pin  |  PIN   | Notes           | Peri.A |     Peripheral B      | Perip.C | Perip.D | Peri.E | Peri.F | Periph.G | Periph.H |
    |            |                  |        |                 |   EIC  | ADC |  AC | PTC | DAC | SERCOMx | SERCOMx |  TCCx  |  TCCx  |    COM   | AC/GLCK  |
    |            |                  |        |                 |(EXTINT)|(AIN)|(AIN)|     |     | (x/PAD) | (x/PAD) | (x/WO) | (x/WO) |          |          |
    +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    | 00         | D0               |  PA22  |                 |  *06   |     |     | X10 |     |   3/00  |   5/00  |* TC4/0 | TCC0/4 |          | GCLK_IO6 |
    | 01         | D1               |  PA23  |                 |  *07   |     |     | X11 |     |   3/01  |   5/01  |* TC4/1 | TCC0/5 | USB/SOF  | GCLK_IO7 |
    | 02         | D2               |  PA10  |                 |   10   | *18 |     | X02 |     |   0/02  |   2/02  |*TCC1/0 | TCC0/2 | I2S/SCK0 | GCLK_IO4 |
    | 03         | D3               |  PA11  |                 |   11   | *19 |     | X03 |     |   0/03  |   2/03  |*TCC1/1 | TCC0/3 | I2S/FS0  | GCLK_IO5 |
    | 04         | D4               |  PB10  |                 |  *10   |     |     |     |     |         |   4/02  |* TC5/0 | TCC0/4 | I2S/MCK1 | GCLK_IO4 |
    | 05         | D5               |  PB11  |                 |  *11   |     |     |     |     |         |   4/03  |* TC5/1 | TCC0/5 | I2S/SCK1 | GCLK_IO5 |
    | 06         | D6               |  PA20  | LED_BUILTIN     |  *04   |     |     | X08 |     |   5/02  |   3/02  |        |*TCC0/6 | I2S/SCK0 | GCLK_IO4 |
    | 07         | D7               |  PA21  |                 |  *05   |     |     | X09 |     |   5/03  |   3/03  |        |*TCC0/7 | I2S/FS0  | GCLK_IO5 |
    +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    |            |       SPI        |        |                 |        |     |     |     |     |         |         |        |        |          |          |
    | 08         | MOSI             |  PA16  |                 |  *00   |     |     | X04 |     |  *1/00  |   3/00  |*TCC2/0 | TCC0/6 |          | GCLK_IO2 |
    | 09         | SCK              |  PA17  |                 |  *01   |     |     | X05 |     |  *1/01  |   3/01  | TCC2/1 | TCC0/7 |          | GCLK_IO3 |
    | 10         | MISO             |  PA19  |                 |   03   |     |     | X07 |     |  *1/03  |   3/03  |* TC3/1 | TCC0/3 | I2S/SD0  | AC/CMP1  |
    +------------+------------------+--------+-----------------+--------------------+-----+-----+---------+---------+--------+--------+----------+----------+
    |            |       Wire       |        |                 |        |     |     |     |     |         |         |        |        |          |          |
    | 11         | SDA              |  PA08  |                 |   NMI  | *16 |     | X00 |     |  *0/00  |   2/00  | TCC0/0 | TCC1/2 | I2S/SD1  |          |
    | 12         | SCL              |  PA09  |                 |   09   | *17 |     | X01 |     |  *0/01  |   2/01  | TCC0/1 | TCC1/3 | I2S/MCK0 |          |
    +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    |            |      Serial1     |        |                 |        |     |     |     |     |         |         |        |        |          |          |
    | 13         | RX               |  PB23  |                 |   07   |     |     |     |     |         |  *5/03  |        |        |          | GCLK_IO1 |
    | 14         | TX               |  PB22  |                 |   06   |     |     |     |     |         |  *5/02  |        |        |          | GCLK_IO0 |
 +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    | 15         | A0 / DAC0        |  PA02  |                 |   02   | *00 |     | Y00 | OUT |         |         |        |        |          |          |
    | 16         | A1               |  PB02  |                 |  *02   | *10 |     | Y08 |     |         |   5/00  |        |        |          |          |
    | 17         | A2               |  PB03  |                 |  *03   | *11 |     | Y09 |     |         |   5/01  |        |        |          |          |
    | 18         | A3               |  PA04  |                 |   04   | *04 |  00 | Y02 |     |         |   0/00  |*TCC0/0 |        |          |          |
    | 19         | A4               |  PA05  |                 |   05   | *05 |  01 | Y03 |     |         |   0/01  |*TCC0/1 |        |          |          |
    | 20         | A5               |  PA06  |                 |   06   | *06 |  02 | Y04 |     |         |   0/02  | TCC1/0 |        |          |          |
    | 21         | A6               |  PA07  |                 |   07   | *07 |  03 | Y05 |     |         |   0/03  | TCC1/1 |        | I2S/SD0  |          |
    |            |       USB        |        |                 |        |     |     |     |     |         |         |        |        |          |          |
    | 22         |                  |  PA24  | USB N           |   12   |     |     |     |     |   3/02  |   5/02  |  TC5/0 | TCC1/2 | USB/DM   |          |
    | 23         |                  |  PA25  | USB P           |   13   |     |     |     |     |   3/03  |   5/03  |  TC5/1 | TCC1/3 | USB/DP   |          |
    | 24         |                  |  PA18  | USB ID          |   02   |     |     | X06 |     |   1/02  |   3/02  |  TC3/0 | TCC0/2 |          | AC/CMP0  |
    +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    | 25         | AREF             |  PA03  |                 |   03   |  01 |     | Y01 |     |         |         |        |        |          |          |
    |            | WiFi SPI         |        |                 |        |     |     |     |     |         |         |        |        |          |          |
    | 26         |                  |  PA12  | NINA_MOSI       |   12   |     |     |     |     |  *2/00  |   4/00  | TCC2/0 | TCC0/6 |          | AC/CMP0  |
    | 27         |                  |  PA13  | NINA_MISO       |   13   |     |     |     |     |  *2/01  |   4/01  | TCC2/1 | TCC0/7 |          | AC/CMP1  |
    | 28         |                  |  PA14  | NINA_CS         |   14   |     |     |     |     |   2/02  |   4/02  |  TC3/0 | TCC0/4 |          | GCLK_IO0 |
    | 29         |                  |  PA15  | NINA_SCK        |   15   |     |     |     |     |  *2/03  |   4/03  |  TC3/1 | TCC0/5 |          | GCLK_IO1 |
    | 30         |                  |  PA27  | NINA_GPIO0      |  *15   |     |     |     |     |         |         |        |        |          | GCLK_IO0 |
    | 31         |                  |  PB08  | NINA_RESETN     |   08   |  02 |     | Y14 |     |         |   4/00  |  TC4/0 |        |          |          |
    | 32         |                  |  PB09  | ADC_VBAT        |  *09   |  03 |     | Y15 |     |         |   4/01  |  TC4/1 |        |          |          |
    +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    |            | 32768Hz Crystal  |        |                 |        |     |     |     |     |         |         |        |        |          |          |
    | 33         |                  |  PA00  | XIN32           |   00   |     |     |     |     |         |   1/00  | TCC2/0 |        |          |          |
    | 34         |                  |  PA01  | XOUT32          |   01   |     |     |     |     |         |   1/01  | TCC2/1 |        |          |          |
    +------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    | 35         |                  |  PA28  | NINA_ACK        |   01   |     |     |     |     |         |   1/01  | TCC2/1 |        |          |          |

+------------+------------------+--------+-----------------+--------+-----+-----+-----+-----+---------+---------+--------+--------+----------+----------+
    */
   #define PWRTWO(EXP) (1 << (EXP))   
   #define WB(x,s1,s2)  (((x&PWRTWO(s1))>>s1)<<s2) //write bit s1 of x to position s2
    void taskId2Dso(){
        #ifdef ARI_MEASURE_LOAD  
        // set pin#16 to 1 and pin# 15 18 20 21 to task _id
        // shift is pin nb on the PORT PA or PB ! see variants\mkrwifi1010\variant.cpp g_APinDescription
        // ex A0 PA02->shift 2 on Group[0], 0 is A, 1 is B
        uint32_t volatile rA =  WB(_id,0,7) | WB(_id,1,6) | WB(_id,2,4) | WB(_id,3,2);
        PORT->Group[0].OUTSET.reg = rA; 
        uint32_t volatile rB = (0x01 << 2);  
        PORT->Group[1].OUTSET.reg = rB; 
        #endif        
    }
    void task0Dso(){
        #ifdef ARI_MEASURE_LOAD  
        //set to 0
        uint32_t rA =  WB(1,0,7) | WB(1,0,6) | WB(1,0,4)| WB(1,0,2);
        PORT->Group[0].OUTCLR.reg = rA;
        uint32_t rB = (0x01 << 2);  
        PORT->Group[1].OUTCLR.reg = rB;
        #endif        
    }

protected:
    uint32_t runTime;   // The  system clock tick when the task can next run.
    #ifdef ARI_MEASURE_LOAD  
    cppQueue * _queue; //(new cppQueue(sizeof(MsgCan), 64)  
    uint32_t _queueItems[64];  
    #endif
};

#endif
