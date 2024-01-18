from pynput.keyboard import Controller, Key
class KeyboardSimulator:
    def __init__(self):
        self.keyboard = Controller()

    def press_or_release_key(self, key_code, press=True):
        # 根据键值获取字符或特殊键
        def get_key(key_code):
            if key_code==51:
                return '3'
            if key_code==145:
                return Key.scroll_lock
            if key_code==19:
                return Key.pause
            if key_code==13:
                return Key.enter
            return chr(key_code).lower()
        key = get_key(key_code)
        # print(f"press_or_release_key:{key},{press}")
        if key is not None:
            if press:
                self.keyboard.press(key)
            else:
                self.keyboard.release(key)
        else:
            print("无法识别的键值:", key_code)

