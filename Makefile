CONTIKI_PROJECT = etimer-buzzer rtimer-lightSensor rtimer-IMUSensor task_2 task_2a
all: $(CONTIKI_PROJECT)

CONTIKI = ../
include $(CONTIKI)/Makefile.include
