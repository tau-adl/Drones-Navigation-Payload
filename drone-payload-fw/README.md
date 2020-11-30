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

## Debug setup
uart @ baud = 115200

## Dependencies
STM32CubeIDE - Version: 1.4.2


## Contributors
@Shenhavo
@danblana