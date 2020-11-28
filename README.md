# Drones-Navigation-Payload
The goal of the project is to build a payload taken by a drone that transmit both IMU and Images to a python script on a computer. 
Later, this information will be processed by ROS environment for navigation with SLAM algorithm. 

The Hardware for this project is based on the OpenMV(c) cH7 camera circuit loaded with global shutter camera, wifi shield and MPU6050 IMU sensor.

The repo cantains python script to be run as "ground-station" and the original firmware that runs on the main MCU.
