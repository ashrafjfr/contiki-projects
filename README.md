# Assignment 2

### Names:
- Leow Yuan Yang (A0217538Y)
- Muhammad Ashraf Bin Mohamad Jaffar (A0217396W)
- Ng Andre (A0217937U)

### Instructions to execute program:
Enter the names of the programs you want to compile in the Makefile, in `CONTIKI_PROJECT = ` .
Compile the program using this command: `make TARGET=cc26x0-cc13x0 BOARD=sensortag/cc2650 all` .
Open **UniFlash**. Choose your device by typing **CC2650** in the search field and select **CC2650F128**.
Next, choose the **Texas Instruments XDS110 USB Debug Probe** and hit the Start button.
Select the compiled files, **file-name.cc26x0-cc13x0** in your working directory and click on load image.
Open a serial monitor to display the outputs from the SensorTag.

### Task 1 Values:
- Value of CLOCK_SECOND = 128
- Number of clock ticks per second in 1s (real time) using etimer  = 128
- Value of RTIMER_SECOND = 65536
- Number of clock ticks per second in 1s (real time) using rtimer = 65544

### For Task 2:
1. The SensorTag will begin in an IDLE state.
2. When significant motion or change in light intensity is detected, the SensorTag will enter a BUZZ state where it will play a sound.
3. This sound will turn on and off every 2 seconds, where the SensorTag will alternate between BUZZ and WAIT states.
4. After 16 seconds, the SensorTag will return to IDLE state.

### For Task 3:
1. The SensorTag will begin in an IDLE state.
2. When a significant motion is detected, the SensorTag will start checking for changes in light intensity.
3. When a significant change in light intensity is detected, the SensorTag will enter a BUZZ state where it will play a sound.
4. This sound will play for 2 seconds before the SensorTag enters the WAIT state where the sound will be off for 4 seconds.
5. The SensorTag will continue this cycle until it detects a significant change in light intensity which it will continuously check during the BUZZ/WAIT cycles.
5. After detecting a significant light, the SensorTag will return to IDLE state.


