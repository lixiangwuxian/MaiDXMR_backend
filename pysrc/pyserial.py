#generated by gpt from SerialManager.cs
#of course, we don't use p2,so just ignore it
import selectors
import serial
import threading
import queue
import time
from bitarray import bitarray

class SerialManager:
    p1Serial = serial.Serial('COM5', 9600)
    p2Serial = serial.Serial('COM6', 9600)
    settingPacket = bytearray([40, 0, 0, 0, 0, 41])
    touchData = bytearray([40, 0, 0, 0, 0, 0, 0, 0, 41])
    touchData2 = bytearray([40, 0, 0, 0, 0, 0, 0, 0, 41])
    startUp = False  # use True for default start up state to prevent restart game
    recivData = ''
    touchQueue = queue.Queue()
    shutdown=False
    def __init__(self):
        self.touchThread = threading.Thread(target=self.touch_thread)
        self.touchThread.start()
        print("Serial Started")

    def touch_thread(self):
        while not self.shutdown:
            if self.p1Serial.isOpen():
                self.read_data(self.p1Serial)
            if self.p2Serial.isOpen():
                self.read_data(self.p2Serial)
            # if not self.touchQueue.empty():
            # self.touchQueue.get()
            self.update_touch()
            time.sleep(0.005)

    def read_data(self, serial:serial.Serial):
        if serial.inWaiting() == 6:
            self.recivData = serial.read(6).decode()
            print(self.recivData)
            self.touch_setup(serial, self.recivData)
    def touch_setup(self, serial, data):
        command = ord(data[3])
        # print("command:",command)
        if command in [76, 69]:
            self.startUp = False
        elif command in [114, 107]:
            for i in range(1, 5):
                self.settingPacket[i] = ord(data[i])
            # print("send serial:",self.settingPacket)
            serial.write(self.settingPacket)
        elif command == 65:
            self.startUp = True

    def send_touch(self,serial, data):
        serial.write(data)

    def update_touch(self):
        if not self.startUp:
            return
        self.send_touch(self.p1Serial, self.touchData)
        self.send_touch(self.p2Serial, self.touchData2)
        print("update touch")

    def change_touch(self,isP1, area:int, state):
        data = self.touchData if isP1 else self.touchData2
        bit_arr = bitarray()
        bit_arr.frombytes(data)
        area=area//8*8+(7-area%8)
        # print("changed bit in:",area+8,"to",state,"in bit arr",bit_arr)
        bit_arr[area + 8] = state
        # print("now bit arr is",bit_arr)
        # data = bit_arr.tobytes()
        if isP1:
            self.touchData = bit_arr.tobytes()
        else:
            self.touchData2 = bit_arr.tobytes()

    def shutdown_now(self):
        self.p1Serial.close()
        self.p2Serial.close()
        self.shutdown=True



# This class is not necessary in Python as bit manipulation is straightforward
# and can be handled directly in the methods where it's needed.

# 定义与C#枚举等价的Python字典
# touch_areas = {
#     'A1': 0, 'A2': 1, 'A3': 2, 'A4': 3, 'A5': 4,
#     'A6': 8, 'A7': 9, 'A8': 10, 'B1': 11, 'B2': 12,
#     'B3': 16, 'B4': 17, 'B5': 18, 'B6': 19, 'B7': 20,
#     'B8': 24, 'C1': 25, 'C2': 26, 'D1': 27, 'D2': 28,
#     'D3': 32, 'D4': 33, 'D5': 34, 'D6': 35, 'D7': 36,
#     'D8': 40, 'E1': 41, 'E2': 42, 'E3': 43, 'E4': 44,
#     'E5': 48, 'E6': 49, 'E7': 50, 'E8': 51
# }

# if __name__ == '__main__':
#     import time
#     s = SerialManager()
#     while True:
#         try:
#     # 循环遍历区域
#             for area in touch_areas.values():
#                 s.change_touch(True,area, True)  # 激活触摸区域
#                 time.sleep(1)             # 持续1秒
#                 s.change_touch(True,area, False) # 关闭触摸区域
#         except:
#             time.sleep(10)