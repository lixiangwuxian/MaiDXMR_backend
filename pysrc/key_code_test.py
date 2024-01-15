from pynput.keyboard import Listener, KeyCode

def on_press(key):
    try:
        # 对于常规按键，直接打印键值
        print(f"按下的键: {key.char}，键值: {key.vk}")
    except AttributeError:
        # 对于特殊按键（如功能键），打印键值
        print(f"按下的键: {key}，键值: {key.value.vk}")

def on_release(key):
    # 当按下Esc键时，停止监听
    if key == KeyCode.from_char('esc'):
        return False

# 设置监听器
with Listener(on_press=on_press, on_release=on_release) as listener:
    listener.join()
