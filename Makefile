CONTIKI_PROJECT = etimer-buzzer rtimer-lightSensor rtimer-IMUSensor task_2
all: $(CONTIKI_PROJECT)

CONTIKI = ../
include $(CONTIKI)/Makefile.include
