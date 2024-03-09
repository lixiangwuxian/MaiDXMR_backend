#generated by gpt from SerialManager.cs
#of course, we don't use p2,so just ignore it
import serial
import threading
import time
from bitarray import bitarray

class SerialManager:
    p1_serial = serial.Serial('COM5', 9600)
    p2_serial = serial.Serial('COM6', 9600)
    setting_packet = bytearray([40, 0, 0, 0, 0, 41])
    touch_data = bytearray([40, 0, 0, 0, 0, 0, 0, 0, 41])
    touch_data2 = bytearray([40, 0, 0, 0, 0, 0, 0, 0, 41])
    startup = False
    recived_data = ''
    shutdown=False
    gap=0.0
    def __init__(self,faq=120):
        self.touch_thread = threading.Thread(target=self.touch_thread)
        self.touch_thread.start()
        print("Serial Started")
        if faq <= 0:
            raise ValueError("faq cannot be less or equal to 0")
        self.gap = 1 / faq

    def touch_thread(self):
        while not self.shutdown:
            if self.p1_serial.isOpen():
                self.read_data(self.p1_serial)
            if self.p2_serial.isOpen():
                self.read_data(self.p2_serial)
            self.update_touch()
            time.sleep(self.gap)

    def read_data(self, serial:serial.Serial):
        if serial.inWaiting() == 6:
            self.recived_data = serial.read(6).decode()
            print(self.recived_data)
            self.touch_setup(serial, self.recived_data)
    def touch_setup(self, serial, data):
        command = ord(data[3])
        # print("command:",command)
        if command in [76, 69]:
            self.startup = False
        elif command in [114, 107]:
            for i in range(1, 5):
                self.setting_packet[i] = ord(data[i])
            # print("send serial:",self.settingPacket)
            serial.write(self.setting_packet)
        elif command == 65:
            self.startup = True

    def send_touch(self,serial, data):
        serial.write(data)

    def update_touch(self):
        if not self.startup:
            return
        self.send_touch(self.p1_serial, self.touch_data)
        self.send_touch(self.p2_serial, self.touch_data2)
        print("update touch")

    def change_touch(self,is_p1, area:int, state):
        data = self.touch_data if is_p1 else self.touch_data2
        bit_arr = bitarray()
        bit_arr.frombytes(data)
        area=area//8*8+(7-area%8)#wired -- why this needs to be reversed
        bit_arr[area + 8] = state
        if is_p1:
            self.touch_data = bit_arr.tobytes()
        else:
            self.touch_data2 = bit_arr.tobytes()

    def shutdown_now(self):
        self.p1_serial.close()
        self.p2_serial.close()
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