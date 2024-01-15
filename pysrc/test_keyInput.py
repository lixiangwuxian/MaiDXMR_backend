from pynput.keyboard import Key, Controller
import time

keyboard = Controller()

while True:
    # 模拟按下并释放'a'键
    keyboard.press('A')
    time.sleep(3)
    keyboard.release('A')
    time.sleep(3)
