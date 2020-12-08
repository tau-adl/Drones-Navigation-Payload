USING_ROS = True

import socket
from datetime import datetime
import os

if not USING_ROS:
    import cv2
    import threading
    import subprocess
    import matplotlib.pyplot as plt
import time
import csv
from utils import *
import constants as c
import numpy as np

try:
    if USING_ROS:
        import rospy
        import sensor_msgs
    # from sensor_msgs.msg import Image, CompressedImage, Imu
except Exception:
    raise


def main():
    now = datetime.now()
    dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
    prnt(("date and time = " + dt_string), 0)
    # define the name of the directory to be created
    path = r"outputs\\" + now.strftime("%d%m%Y-%H%M%S")

    try:
        os.mkdir(path)

    except OSError:
        print("trying to create outputs directory")
        try:
            os.mkdir("outputs")
            os.mkdir(path)
            print("created  directory %s " % path)
        except OSError:
            print("error creating dir")
    else:
        print("created  directory %s " % path)
    # this section is tested on WIN10
    try:
        cmd = "netsh wlan connect name={0} ssid={0}".format(c.SSID)
        k = subprocess.run(cmd, capture_output=True, text=True).stdout
        # print("connection succeed: " + k)
        time.sleep(2)
        cmd = "netsh wlan show interfaces"
        k = subprocess.run(cmd, capture_output=True, text=True).stdout
        connection_result = k.find(c.SSID)
        if connection_result > 0:
            print("connected")
        else:
            print("NOT connected")
    except:
        print("could not connect AP: ")

    pm = PacketMngr(c.HOST, c.PORT)

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        pm.add_socket(s)
        pm.socket.connect((pm.host, pm.port))

        s.sendall(c.TARGET_START_SEND_CMD)

        if not USING_ROS:
            display_img_thread = threading.Thread(target=pm.display_img)
            display_img_thread.setDaemon(True)
            display_img_thread.start()

        while True:
            incoming_data = s.recv(c.SOF_SIZE_B)
            if incoming_data[0] == c.FRAME_SOF:
                incoming_data = s.recv(c.FRAME_HEADER_WO_SOF_SIZE_B)
                incoming_sys_tick = FourBytesToUint32(incoming_data, 0)
                if pm.frame_sys_tick == 0 or pm.frame_sys_tick == incoming_sys_tick:
                    pm.frame_sys_tick = incoming_sys_tick
                else:

                    pm.clear_frame_properties()
                    continue
                pm.frame_size = TwoBytesToUint16(incoming_data, 4)
                if pm.frame_size > c.FRAME_DATA_SIZE_B:  # means that there will be an additional packet
                    if pm.frame_rx_buff == bytes(0):
                        pm.frame_rx_buff = s.recv(c.FRAME_DATA_SIZE_B)
                    else:
                        pm.frame_rx_buff = pm.frame_rx_buff + s.recv(c.FRAME_DATA_SIZE_B)

                else:  # means this is the last packet
                    try:
                        pm.frame_rx_buff = pm.frame_rx_buff + s.recv(pm.frame_size)  # - FRAME_HEADER_SIZE_B)
                        Image_msg = CompressedImage()
                        Image_msg.header.stamp = rospy.Time.now()
                        Image_msg.format = "jpeg"
                        Image_msg.data = pm.frame_rx_buff
                        pub_image.publish(Image_msg)
                    except:
                        expected_frame_size = pm.frame_size - c.FRAME_HEADER_SIZE_B
                        print("bad recv size: %d" % expected_frame_size)
                    else:
                        pm.frame_ctr = pm.frame_ctr + 1
                        if pm.first_img_tick == 0:
                            pm.first_img_tick = incoming_sys_tick
                        print("frame recv size: %d" % len(pm.frame_rx_buff))
                        pm.save_jpeg(path)
                        pm.frame_rx_buff = bytes()
                        delta_tick = pm.frame_sys_tick - pm.last_valid_frame_sys_tick
                        print("sys_tick diff = %d [msec]" % delta_tick)
                        pm.last_valid_frame_sys_tick = pm.frame_sys_tick
                        pm.curr_img_tick = pm.frame_sys_tick

                    # ===== cleaning:  ======
                    pm.clear_frame_properties()

            elif incoming_data[0] == c.IMU_SOF:
                pm.imu_rx_buff = s.recv(c.IMU_PACKET_SIZE_WO_HEADER)
                pm.imu_sys_tick = FourBytesToUint32(pm.imu_rx_buff, 0) - c.IMU_SYSTICK_SHIFT_MSEC
                print("X" + str(pm.imu_sys_tick))
                pm.process_imu_data(pm.imu_rx_buff)
                pm.clear_imu_properties()
            else:
                prnt("ERR SOF", 1)

        else:
            prnt("bye bye..\r\n")
            input()
            exit(1)


class PacketMngr:

    def __init__(self, a_host, a_port):
        self.host = a_host
        self.port = a_port
        self.socket = None
        self.rx_bytes_to_read = 1
        self.frame_rx_buff = bytes()
        self.frame_sys_tick = 0
        self.first_img_tick = 0.0
        self.curr_img_tick = 0.0
        self.last_valid_frame_sys_tick = 0
        self.frame_size = 0
        self.ptr_jpeg = None
        self.last_saved_img_path = ''
        self.last_saved_cropped_img_path = ''
        self.frame_ctr = 0

        self.imu_rx_buff = bytes()
        self.imu_sys_tick = 0

        self.new_img_rec = False
        self.new_imu_rec = False

        self.checksum = 0
        self.err_ctr = 0.0
        self.total_bytes_rec = 0.0

        self.csvmngr = None
        self.gyro_x = 0
        self.gyro_y = 0
        self.gyro_z = 0
        self.acc_x = 0
        self.acc_y = 0
        self.acc_z = 0

    def add_socket(self, a_socket):
        self.socket = a_socket

    def save_jpeg(self, save_path):
        # prnt("image size =", 0)
        # self.print_buff_len()
        # prnt(("img systick = " + str(self.frame_sys_tick)), 0)
        curr_saved_file_name = save_path + "\\" + str(self.frame_sys_tick)

        self.last_saved_img_path = curr_saved_file_name + ".jpeg"
        self.ptr_jpeg = open(self.last_saved_img_path, "wb")
        self.ptr_jpeg.write(self.frame_rx_buff)
        self.ptr_jpeg.close()
        self.crop_img(self.last_saved_img_path)
        self.new_img_rec = True

    def crop_img(self, img_path):
        try:
            img = cv2.imread(img_path)
            cropped_img = img[c.IMG_H_BOTTOM_CROP:c.IMG_H - c.IMG_H_TOP_CROP, 0:c.IMG_W]
            self.last_saved_cropped_img_path = self.last_saved_img_path.replace('.jpeg', '_c.jpeg')
            # self.last_saved_img_path = cropped_img_file_name
            cropped_img = cv2.flip(cropped_img, 1)
            cropped_img = cv2.rotate(cropped_img, cv2.ROTATE_180)
            cv2.imwrite(self.last_saved_cropped_img_path, cropped_img)
        except:
            print("crop img err")

    def clear_frame_properties(self):
        self.packet_ctr = 0
        self.frame_sys_tick = 0
        self.frame_size = 0
        self.expected_num_of_packets = 0
        self.ptr_jpeg = None
        self.mode = 0  # 0 = off, 1 = Image, 2 = IMU
        self.frame_rx_buff = bytes(0)

    def clear_imu_properties(self):
        self.imu_rx_buff = bytes(0)
        self.imu_sys_tick = 0

    def process_imu_data(self, a_byte_arr):
        self.new_imu_rec = True

        for imu_call_idx in range(c.IMU_CALLS_PER_PACKET):
            if not USING_ROS:
                self.gyro_x = TwoBytesToInt16(a_byte_arr,
                                              c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.GYRO_X_SHIFT) / c.GYRO_BITS_TO_FLOAT
                self.gyro_y = TwoBytesToInt16(a_byte_arr,
                                              c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.GYRO_Y_SHIFT) / c.GYRO_BITS_TO_FLOAT
                self.gyro_z = TwoBytesToInt16(a_byte_arr,
                                              c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.GYRO_Z_SHIFT) / c.GYRO_BITS_TO_FLOAT
                self.acc_x = TwoBytesToInt16(a_byte_arr,
                                             c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.ACC_X_SHIFT) / c.ACC_BITS_TO_FLOAT
                self.acc_y = TwoBytesToInt16(a_byte_arr,
                                             c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.ACC_Y_SHIFT) / c.ACC_BITS_TO_FLOAT
                self.acc_z = TwoBytesToInt16(a_byte_arr,
                                             c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.ACC_Z_SHIFT) / c.ACC_BITS_TO_FLOAT
                self.csvmngr.appendImuData(
                    [curr_timestamp, self.gyro_x, self.gyro_y, self.gyro_z, self.acc_x, self.acc_y, self.acc_z])

            curr_timestamp = self.imu_sys_tick - c.IMU_CALLS_TIME_Delta_MSEC * (
                        c.IMU_CALLS_PER_PACKET - 1 - imu_call_idx)  # TODO: insert curr timestamp in IMU_msg
            IMU_msg.angular_velocity.x = self.gyro_x
            IMU_msg.angular_velocity.y = self.gyro_y
            IMU_msg.angular_velocity.z = self.gyro_z
            IMU_msg.linear_acceleration.x = self.acc_x
            IMU_msg.linear_acceleration.y = self.acc_y
            IMU_msg.linear_acceleration.z = self.acc_z
            pub_imu.publish(IMU_msg)


if __name__ == "__main__":
    rospy.init_node('ground_station_node')
    # pub = rospy.Publisher('/comp_image', CompressedImage, queue_size=1)
    pub_image = rospy.Publisher("/output/image_raw/compressed", CompressedImage)
    pub_imu = rospy.Publisher("/imu", Imu)
main()


