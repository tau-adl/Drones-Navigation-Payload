
import socket
from datetime import datetime
import os
import subprocess
import time
import csv
from utils import *
import cv2
import threading
import constants as c
import matplotlib.pyplot as plt



def main():
    now = datetime.now()
    dt_string = now.strftime("%d/%m/%Y %H:%M:%S")
    prnt(("date and time = " + dt_string), 0)
    # define the name of the directory to be created
    path = r"outputs\\" + now.strftime("%d%m%Y-%H%M%S")

    try:
        os.mkdir(path)

    except OSError:
        print ("trying to create outputs directory")
        try:
            os.mkdir("outputs")
            os.mkdir(path)
            print ("created  directory %s " % path)
        except OSError:
            print ("error creating dir")
    else:
        print ("created  directory %s " % path)
        csvmngr = CsvMngr(path)
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
    pm.csvmngr = csvmngr
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        pm.add_socket(s)
        pm.socket.connect((pm.host, pm.port))

        s.sendall(c.TARGET_START_SEND_CMD)

        display_img_thread = threading.Thread(target=pm.display_img)
        display_img_thread.setDaemon(True)
        display_img_thread.start()

        while True:
            incoming_data = s.recv(c.SOF_SIZE_B)
            print("SOF="+str(incoming_data))
            if incoming_data[0] == c.FRAME_SOF:
                incoming_data = s.recv(c.FRAME_HEADER_WO_SOF_SIZE_B)
                incoming_sys_tick = FourBytesToUint32(incoming_data, 0)
                # print("tick=" + str(incoming_sys_tick))
                if pm.frame_sys_tick == 0 or pm.frame_sys_tick == incoming_sys_tick:
                    pm.frame_sys_tick = incoming_sys_tick
                else:
                    print("invalid systick arrived")
                    pm.clear_frame_properties()
                    continue
                pm.frame_size = TwoBytesToUint16(incoming_data, 4)
                print("tick = %d \tframe_size = %d" % (pm.frame_sys_tick, pm.frame_size))
                if pm.frame_size > c.FRAME_DATA_SIZE_B:  # means that there will be an additional packet
                    if pm.frame_rx_buff == bytes(0):
                        pm.frame_rx_buff = s.recv(c.FRAME_DATA_SIZE_B)
                    else:
                        pm.frame_rx_buff = pm.frame_rx_buff + s.recv(c.FRAME_DATA_SIZE_B)

                else: # means this is the last packet
                    try:
                        pm.frame_rx_buff = pm.frame_rx_buff + s.recv(pm.frame_size)# - FRAME_HEADER_SIZE_B)
                    except:
                        expected_frame_size = pm.frame_size - c.FRAME_HEADER_SIZE_B
                        print("bad recv size: %d" % expected_frame_size)
                    else:
                        print("frame recv size: %d" % len(pm.frame_rx_buff))
                        pm.save_jpeg(path)
                        pm.calc_ber()
                        pm.frame_rx_buff = bytes()
                        delta_tick = pm.frame_sys_tick - pm.last_valid_frame_sys_tick
                        print("sys_tick diff = %d [msec]" % delta_tick)
                        pm.last_valid_frame_sys_tick = pm.frame_sys_tick
                    # ===== cleaning:  ======
                    pm.clear_frame_properties()

            elif incoming_data[0] == c.IMU_SOF:
                pm.imu_rx_buff = s.recv(c.IMU_PACKET_SIZE_WO_HEADER)
                pm.imu_sys_tick = FourBytesToUint32(pm.imu_rx_buff, 0) - c.IMU_SYSTICK_SHIFT_MSEC
                print("X" + str(pm.imu_sys_tick))
                pm.process_imu_data(pm.imu_rx_buff)
                # pm.print_imu_data()
                # acc_x_values.append(pm.acc_x)
                # acc_y_values.append(pm.acc_y)
                # acc_z_values.append(pm.acc_z)

                pm.clear_imu_properties()
            else:
                prnt("ERR SOF", 1)

        else:
            prnt("bye bye..\r\n")
            input()
            exit(1)




class PacketMngr:

    def __init__(self, a_host, a_port):
        self.host   = a_host
        self.port   = a_port
        self.socket = None
        self.rx_bytes_to_read = 1
        self.frame_rx_buff = bytes()
        self.frame_sys_tick = 0
        self.last_valid_frame_sys_tick = 0
        self.frame_size =   0
        self.ptr_jpeg   = None
        self.last_saved_img_path = ''
        self.last_saved_cropped_img_path = ''

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
        self.socket =   a_socket

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

    def display_img(self):
        img_plt = None
        while True:
            if self.new_img_rec:
                self.new_img_rec = False
                print("displaying new img")
                try:
                    img = plt.imread(self.last_saved_cropped_img_path)
                    if img_plt is None:
                        img_plt = plt.imshow(img)
                    else:
                        img_plt.set_data(img)
                    plt.pause(c.IMG_PLOT_PAUSE)  # needs to be less then 1/15fps
                    plt.draw()
                except:
                    if c.IS_DISP_ERR_IMG:
                        img = plt.imread(c.ERR_IMG_PATH)
                        if img_plt is None:
                            img_plt = plt.imshow(img)
                        else:
                            img_plt.set_data(img)
                    plt.pause(.001)  # needs to be less then 1/15fps
                    plt.draw()
                    print("img show err")

    # def display_imu(self):
    #     imu_plt = None
    #     while True:
    #         if self.new_imu_rec:
    #             self.new_imu_rec = False
    #             print("displaying new imu")
    #             try:
    #                 if imu_plt is None:
    #                     imu_plt = plt.plot(0, 0)
    #                 else:
    #                     imu_plt.set_data(np.linspace(0, len(self.acc_x)), self.acc_x)
    #                 plt.pause(.067) # ~15fps
    #                 plt.draw()
    #             except:
    #                 print("imu show err")

    def crop_img(self, img_path):
        try:
            img = cv2.imread(img_path)
            cropped_img = img[c.IMG_H_BOTTOM_CROP:c.IMG_H-c.IMG_H_TOP_CROP, 0:c.IMG_W]
            self.last_saved_cropped_img_path = self.last_saved_img_path.replace('.jpeg', '_c.jpeg')
            # self.last_saved_img_path = cropped_img_file_name
            cropped_img = cv2.flip(cropped_img, 1)
            cropped_img = cv2.rotate(cropped_img, cv2.ROTATE_180)
            cv2.imwrite(self.last_saved_cropped_img_path, cropped_img)
        except:
            print("crop img err")


    def clear_frame_properties(self):
        self.packet_ctr =   0
        self.frame_sys_tick   =   0
        self.frame_size =   0
        self.frame_ctr  =   0
        self.expected_num_of_packets = 0
        self.ptr_jpeg   = None
        self.mode       =   0 # 0 = off, 1 = Image, 2 = IMU
        self.frame_rx_buff = bytes(0)

    def clear_imu_properties(self):
        self.imu_rx_buff = bytes(0)
        self.imu_sys_tick = 0

    def print_buff_len(self):
        try:
            print("buff len: " + str(len(self.frame_rx_buff)))
        except:
            print("buff len: 0")

    def process_imu_data(self, a_byte_arr):
        self.new_imu_rec = True

        for imu_call_idx in range(c.IMU_CALLS_PER_PACKET):
            self.gyro_x = TwoBytesToInt16(a_byte_arr,c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx*c.IMU_CALL_SIZE_B + c.GYRO_X_SHIFT) / c.GYRO_BITS_TO_FLOAT
            self.gyro_y = TwoBytesToInt16(a_byte_arr,c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.GYRO_Y_SHIFT) / c.GYRO_BITS_TO_FLOAT
            self.gyro_z = TwoBytesToInt16(a_byte_arr,c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.GYRO_Z_SHIFT) / c.GYRO_BITS_TO_FLOAT
            self.acc_x = TwoBytesToInt16(a_byte_arr,c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.ACC_X_SHIFT) / c.ACC_BITS_TO_FLOAT
            self.acc_y = TwoBytesToInt16(a_byte_arr,c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.ACC_Y_SHIFT) / c.ACC_BITS_TO_FLOAT
            self.acc_z = TwoBytesToInt16(a_byte_arr,c.IMU_DATA_SHIFT_SIZE_B + imu_call_idx * c.IMU_CALL_SIZE_B + c.ACC_Z_SHIFT) / c.ACC_BITS_TO_FLOAT
            self.csvmngr.appendImuData([self.imu_sys_tick-c.IMU_CALLS_TIME_Delta_MSEC*(c.IMU_CALLS_PER_PACKET-1-imu_call_idx),self.gyro_x,self.gyro_y,self.gyro_z,self.acc_x,self.acc_y,self.acc_z])
    def print_imu_data(self):
        prnt(("Gyro X = " + str(self.gyro_x)), 0)
        prnt(("Gyro Y = " + str(self.gyro_y)), 0)
        prnt(("Gyro Z = " + str(self.gyro_z)), 0)
        prnt(("Acc X = " + str(self.acc_x)), 0)
        prnt(("Acc Y = " + str(self.acc_y)), 0)
        prnt(("Acc Z = " + str(self.acc_z)), 0)

    def calc_ber(self):
        self.checksum = 0
        is_err = False

        for i in range(len(self.frame_rx_buff)):
            self.checksum += self.frame_rx_buff[i]
            if self.frame_rx_buff[i] != 0xFF:
                is_err |= True
                self.err_ctr += 1
            self.total_bytes_rec += 1

        print("tick = %d \tframe_size = %d, is Err = %s" % (self.frame_sys_tick,len(self.frame_rx_buff), str(is_err)))
        print("BER = %f [percent]" % (100*self.err_ctr/self.total_bytes_rec))

class CsvMngr:
    '''
    handles reading and writing from CSV files
    '''

    def __init__(self, a_path):
        self.path = a_path
        self.list = []
        with open(self.path + '/imu.csv','w', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(["TimeStamp","GyroX", "GyroY", "GyroZ","AccX", "AccY", "AccZ"])
    def appendImuData(self,a_list):
        with open(self.path + '/imu.csv','a', newline='') as file:
            writer = csv.writer(file)
            writer.writerow(a_list)
if __name__ == "__main__":
    main()


