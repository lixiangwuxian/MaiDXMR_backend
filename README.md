Work In Progress...

Backend for maimai dx with hand tracking.

Todo List:

- [X] capture sinmai.exe display
- [X] encode texture to h264 stream
  - [ ] in low latency mode - failed on decoder side
- [ ] encode the game sound to headset
- [X] send the stream to headset in udp
- [ ] hand hitbox
- [ ] add a user-friendly config file
- [X] receive user input from quest in udp
- [X] transform the input to proper serial data format
- [X] simulate the keyboard input
- [ ] simulate light
- [X] send input data to sinmai.exe
- [ ] stop capture when hmd not active
