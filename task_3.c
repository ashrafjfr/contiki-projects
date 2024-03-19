#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "buzzer.h"
#include "board-peripherals.h"

#define STATE_IDLE 0
#define STATE_BUZZ 1
#define STATE_WAIT 2

#define LUX_THRESHOLD 3000
#define IMU_THRESHOLD 200 // need to check IMU threshold
#define BUZZ_DURATION (CLOCK_SECOND * 2)
#define WAIT_DURATION (CLOCK_SECOND * 2)

PROCESS(process_task_2, "Task 2");
AUTOSTART_PROCESSES(&process_task_2);

static struct rtimer poll_timer, interval_timer;//, pre_idle_timer;
static rtimer_clock_t poll_sensor_timeout = RTIMER_SECOND / 100;  // 100 Hz
static rtimer_clock_t light_timeout = RTIMER_SECOND / 4;  // 4 Hz
static int currState = STATE_IDLE;
// static int prevState = STATE_IDLE;
static int lastLightLux = 0;
static int lastAccelMag = 0;
static int clock = 0;
static int clockCount = 0;
int buzzerFrequency = 2000;

static bool triggerIMU = false;
static bool triggerLight = false;
static bool triggerIDLE = false;

static void init_opt_reading(void);
static int get_light_reading(void);
static void init_mpu_reading(void);
static int get_mpu_reading(void);
static void poll_sensor(struct rtimer *timer, void *ptr);
static void interval_wait(struct rtimer *timer, void *ptr);
static void interval_buzz(struct rtimer *timer, void *ptr);

PROCESS_THREAD(process_task_2, ev, data) {
    PROCESS_BEGIN();
    init_opt_reading();
    init_mpu_reading();
    buzzer_init();
    
    rtimer_set(&poll_timer, RTIMER_NOW() + poll_sensor_timeout, 0, poll_sensor, NULL); // call poll sensor once

    while (1) {
        printf("Current State: %d\n", currState);
        switch (currState) {
            case STATE_BUZZ:
                printf("STATE_BUZZ\n");
                // rtimer_set(&interval_timer, RTIMER_NOW() + light_timeout, 0, interval_buzz, NULL);
                break;
            case STATE_WAIT:
                printf("STATE_WAIT\n");
                // rtimer_set(&interval_timer, RTIMER_NOW() + light_timeout, 0, interval_wait, NULL);
                break;
            case STATE_IDLE:
                printf("STATE_IDLE\n");
                break;
        }
        PROCESS_YIELD();
    }

    PROCESS_END();
}

static void interval_wait(struct rtimer *timer, void *ptr) {
    clockCount++;
    printf("Clock Count: %d\n", clockCount);
    if (clockCount > 16) {  // 4s / 4Hz = 16
        printf("4s elapsed, turning on buzzer\n");
        clockCount = 0;
        if (triggerIDLE) { // back to poll sensor
            printf("Trigger idle\n");
            currState = STATE_IDLE;

            rtimer_set(&poll_timer, RTIMER_NOW() + poll_sensor_timeout, 0, poll_sensor, NULL);  // go to IDLE
        } else {
            printf("4s elapsed, turning on buzzer\n");
            currState = STATE_BUZZ;
            buzzer_start(buzzerFrequency);
            rtimer_set(&interval_timer, RTIMER_NOW() + light_timeout, 0, interval_buzz, NULL);  // go to 4s wait state
        }
    } else {
        int currLightLux = get_light_reading();
        if(abs(currLightLux - lastLightLux) > LUX_THRESHOLD) {
            printf("light intensity change during interval\n");
            triggerIDLE = true;
        }     
        lastLightLux = currLightLux;   
        rtimer_set(&interval_timer, RTIMER_NOW() + light_timeout, 0, interval_wait, NULL); // continue on 4Hz
    }
}

static void interval_buzz(struct rtimer *timer, void *ptr) {
    clockCount++;
    printf("Clock Count: %d\n", clockCount);

    if (clockCount > 8) {  // 2s / 4Hz = 8
        printf("2s elapsed, turning off buzzer\n");
        clockCount = 0;
        buzzer_stop();
        currState = STATE_WAIT;
        rtimer_set(&interval_timer, RTIMER_NOW() + light_timeout, 0, interval_wait, NULL);  // go to 4s wait state
    } else {
        int currLightLux = get_light_reading();
        if (abs(currLightLux - lastLightLux) > LUX_THRESHOLD) {
            printf("light intensity change during interval\n");
            triggerIDLE = true;
        }
        lastLightLux = currLightLux;
        rtimer_set(&interval_timer, RTIMER_NOW() + light_timeout, 0, interval_buzz, NULL);
    }
}

static void poll_sensor(struct rtimer *timer, void *ptr) {
    triggerIDLE = false;
    if (clock % 25 == 0) {  // poll at 100Hz/25 = 4Hz if IMU triggered
        int currLightLux = get_light_reading();
        // printf("Light Intensity: %d\n", currLightLux);
        if (abs(currLightLux - lastLightLux) > LUX_THRESHOLD && currState == STATE_IDLE && triggerIMU) {
            triggerLight = true;
            printf("Change in Light Intensity detected.\n");
        }
        lastLightLux = currLightLux;
    }
    if (clock % 2 == 0 && !triggerIMU) {  // poll at 100Hz/2 = 50Hz if IMU not triggered
        int currAccelMag = get_mpu_reading();
        // printf("Accel Magnitude: %d\n", currAccelMag);
        if (abs(currAccelMag - lastAccelMag) > IMU_THRESHOLD) {
            triggerIMU = true;
            printf("Significant Motion detected.\n");
        }
        lastAccelMag = currAccelMag;
    }
    
    clock++;
    if (triggerLight) {
        printf("Trigger Light\n");
        currState = STATE_BUZZ;
        triggerLight = false;
        triggerIMU = false;
        buzzer_start(buzzerFrequency);
        rtimer_set(&interval_timer, RTIMER_NOW() + light_timeout, 0, interval_buzz, NULL); //4Hz
    } else {
        rtimer_set(&poll_timer, RTIMER_NOW() + poll_sensor_timeout, 0, poll_sensor, NULL); // continue on 100Hz
    }
}

static int get_light_reading() {
    int value;
    value = opt_3001_sensor.value(0);
    if (value == CC26XX_SENSOR_READING_ERROR) {
        printf("OPT: Light Sensor's Warming Up\n\n");
    }
    init_opt_reading();

    return value;
}

static void init_opt_reading(void) {
    SENSORS_ACTIVATE(opt_3001_sensor);
}

static int get_mpu_reading() {
    int currAccelx = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
    int currAccely = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
    int currAccelz = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);

    return sqrt(currAccelx * currAccelx + currAccely * currAccely + currAccelz * currAccelz);
}

static void init_mpu_reading(void) {
    mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}
