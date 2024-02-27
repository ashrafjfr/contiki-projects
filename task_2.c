#include <stdio.h>
#include <stdint.h>
#include "contiki.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "buzzer.h"
#include "board-peripherals.h"

#define STATE_IDLE 0
#define STATE_BUZZ 1
#define STATE_WAIT 2

#define LUX_THRESHOLD 300
#define IMU_THRESHOLD 1 // need to check IMU threshold
#define BUZZ_DURATION (CLOCK_SECOND * 2)
#define WAIT_DURATION (CLOCK_SECOND * 2)
#define PRE_IDLE_DURATION (CLOCK_SECOND * 16)

PROCESS(process_task_2, "Task 2");
AUTOSTART_PROCESS(&process_task_2);

static struct rtimer state_timer, lightSensor_timer, IMU_timer;
int buzzerFrequency[8]={2093,2349,2637,2794,3156,3520,3951,4186};

static void init_opt_reading(void);
static void get_light_reading(void);
static void init_mpu_reading(void);
static void get_mpu_reading(void);


static void get_light_reading()
{
  int value;

  value = opt_3001_sensor.value(0);
  if(value == CC26XX_SENSOR_READING_ERROR) {
    printf("OPT: Light=%d.%02d lux\n", value / 100, value % 100);
  } else {
    printf("OPT: Light Sensor's Warming Up\n\n");
  }
  init_opt_reading();
}

static void init_opt_reading(void)
{
  SENSORS_ACTIVATE(opt_3001_sensor);
}

static void get_mpu_reading()
{
  int value;

  printf("MPU Gyro: X=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_X);
  print_mpu_reading(value);
  printf(" deg/sec\n");

  printf("MPU Gyro: Y=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Y);
  print_mpu_reading(value);
  printf(" deg/sec\n");

  printf("MPU Gyro: Z=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_GYRO_Z);
  print_mpu_reading(value);
  printf(" deg/sec\n");

  printf("MPU Acc: X=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_X);
  print_mpu_reading(value);
  printf(" G\n");

  printf("MPU Acc: Y=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Y);
  print_mpu_reading(value);
  printf(" G\n");

  printf("MPU Acc: Z=");
  value = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
  print_mpu_reading(value);
  printf(" G\n");
}

static void init_mpu_reading(void)
{
  mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}

