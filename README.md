Work In Progress...

Backend for maimai dx with hand tracking.

Todo List:

- [X] capture sinmai.exe display
- [X] encode texture to h264 stream
  - [ ] in low latency mode - failed on decoder side
- [ ] encode the game sound to headset
- [X] send the stream to headset in udp
- [ ] full hand hitbox
- [ ] add a user-friendly config file
- [X] receive user input from quest in udp
- [X] transform the input to proper serial data format
- [X] simulate the keyboard input
- [X] send input data to sinmai.exe
- [ ] stop streaming when hmd not active
- [ ] simulate light

## Build & Run Cpp part

```batch
mkdir build
cd build
cmake ..
msbuild DesktopCaptureApp.sln /p:Configuration=Release
```

You can find the output binary in `build/Release/` folder.

PS: If you encounts the error `msbuild` not found, you have to add the path to `msbuild.exe` to your `PATH` environment variable. See `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`, you can add this path to your `PATH` environment variable. Or you can just open the Visual Studio 2022 IDE and build the solution there.

## Debug the Cpp part

Open this project with VS Code, and press `F5` to start debugging. Make sure you have already opened the `Sinmai.exe` window before you start debugging, or the program will crash.

## Run the Python part

You need to install the following packages:

```batch
pip install pynput pyserial bitarray
```

Just double click the `pysrc\run_me.bat` file.
