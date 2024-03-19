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

#define LUX_THRESHOLD 500
#define IMU_THRESHOLD 180 // need to check IMU threshold
#define BUZZ_DURATION (CLOCK_SECOND * 2)
#define WAIT_DURATION (CLOCK_SECOND * 2)

PROCESS(process_task_2, "Task 2");
AUTOSTART_PROCESSES(&process_task_2);

static struct rtimer poll_timer, interval_timer;//, pre_idle_timer;
static rtimer_clock_t poll_sensor_timeout = RTIMER_SECOND / 100;  // 100 Hz
static rtimer_clock_t interval_timeout = RTIMER_SECOND * 2;      // 2 seconds
static int currState = STATE_IDLE;
static int prevState = STATE_IDLE;
volatile static int lastLightLux = 0;
static int lastAccelMag = 0;
static int clock = 0;
static int buzzerCount = 0;
int buzzerFrequency = 3156;

static void init_opt_reading(void);
static int get_light_reading(void);
static void init_mpu_reading(void);
static int get_mpu_reading(void);
static void poll_sensor(struct rtimer *timer, void *ptr);
static void interval_wait();
static void interval_buzz();

PROCESS_THREAD(process_task_2, ev, data) {
    PROCESS_BEGIN();
    init_opt_reading();
    init_mpu_reading();
    buzzer_init();

    while (1) {
        printf("Current State: %d\n", currState);
        switch (currState) {
            case STATE_BUZZ:
                printf("STATE_BUZZ\n");
                buzzer_start(buzzerFrequency);
                rtimer_set(&interval_timer, RTIMER_NOW() + interval_timeout, 0, interval_wait, NULL);
                break;
            case STATE_IDLE:
                printf("STATE_IDLE\n");
                rtimer_set(&poll_timer, RTIMER_NOW() + poll_sensor_timeout, 0, poll_sensor, NULL);
                break;
        }
        PROCESS_YIELD();
    }

    PROCESS_END();
}

static void interval_wait() {
    printf("Turn buzzing off\n");
    buzzer_stop();
    buzzerCount++;
    printf("BuzzerCount = %d\n", buzzerCount);
    if(buzzerCount < 7) {
        rtimer_set(&interval_timer, RTIMER_NOW() + interval_timeout, 0, interval_buzz, NULL);
        if (buzzerCount == 4) {
            get_light_reading();
        }
    } else {
        printf("Reset buzzer\n");
        buzzerCount = 0;
        currState = STATE_IDLE;
        prevState = STATE_BUZZ;
        lastLightLux = 0;
        lastAccelMag = 0;
        get_light_reading();
        rtimer_set(&poll_timer, RTIMER_NOW() + poll_sensor_timeout, 0, poll_sensor, NULL);
    }
}

static void interval_buzz() {
    printf("Turn buzzing on\n");
    buzzer_start(buzzerFrequency);
    buzzerCount++;
    printf("BuzzerCount = %d\n", buzzerCount);
    if(buzzerCount < 15) {
        rtimer_set(&interval_timer, RTIMER_NOW() + interval_timeout, 0, interval_wait, NULL);
        if (buzzerCount == 4) {
            get_light_reading();
        }
    } else {
        printf("Reset buzzer\n");
        buzzerCount = 0;
        currState = STATE_IDLE;
        prevState = STATE_BUZZ;
        lastLightLux = 0;
        lastAccelMag = 0;
        rtimer_set(&poll_timer, RTIMER_NOW() + poll_sensor_timeout, 0, poll_sensor, NULL);
    }
}

static void poll_sensor(struct rtimer *timer, void *ptr) {
    bool trigger = false;
    if (clock % 25 == 0) {  // poll at 100Hz/25 = 4Hz
        int currLightLux = get_light_reading();
        if (currLightLux < 0) {
            currLightLux = 0;
        }
        printf("Light Intensity: %d\n", currLightLux);
        if (abs(abs(currLightLux) - abs(lastLightLux)) > LUX_THRESHOLD && currState == STATE_IDLE) {
            trigger = true;
            printf("currLightLux: %d\n", currLightLux);
            printf("lastLightLux: %d\n", lastLightLux);
            printf("Delta: %d\n", abs(abs(currLightLux) - abs(lastLightLux)));
            printf("Change in Light Intensity detected.\n");
        }
        lastLightLux = currLightLux;
    }
    if (clock % 2 == 0) {  // poll at 100Hz/2 = 50Hz
        int currAccelMag = get_mpu_reading();
        printf("Accel Magnitude: %d\n", currAccelMag);
        if (abs(currAccelMag - lastAccelMag) > IMU_THRESHOLD && currState == STATE_IDLE) {
            trigger = true;
            printf("Significant Motion detected.\n");
        }
        lastAccelMag = currAccelMag;
    }
    clock++;
    if (trigger) {
        printf("Trigger\n");
        currState = STATE_BUZZ;
        prevState = STATE_IDLE;
    } else {
        rtimer_set(&poll_timer, RTIMER_NOW() + poll_sensor_timeout, 0, poll_sensor, NULL);
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
