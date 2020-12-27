# drone-payload-fw


Project based on the OpenMV(c) firmware, modified to achieve real time image capturing and IMU data fusion, to be sent via Wifi to a  groundstation


## Hardware setup

** add circuit diagram **

|Part|OPENMV part|Model |
|--|--|--|
| MCU | OPENMVH7 | ST STM32H743 |
| Camera| OPENMV GS Camera  | ON MT9V034 |
| Wifi| OPENMV WifiShield | Atmel ATWINC1500 |
| IMU|  | Invensese MPU-6050 |
|Debugger/flasher||STlink V3|
|USB to UART|FTDI or similar|

## Debug info setup
uart @ baud = 115200.
win - `docklight` terminal or similar
ubuntu - `minicom`, `screen` or similar

# Defines and build configurations

- `camera_mngr.h` , frame size @  `FRAME_SIZE`,  `IMG_W`,  `IMG_H`
- `camera_mngr.h` , debug flags @  `CAMERA_BENCHMARK` - for printing acquisitions benchmarks,  `SAVE_INPUT_IMG_ON_SD`,  `SAVE_OUTPUT_IMG_ON_SD` - to save source and ouput images in SD card (~use carefully, it affects timings!)


## Dependencies
STM32CubeIDE - Version: 1.4.2 - install from `https://www.st.com/en/development-tools/stm32cubeide.html`





## Contributors
@Shenhavo
@danblana