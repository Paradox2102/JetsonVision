# Vision Server Example

This is a complete example of a program that processes real time video from a V4L-compatible webcam and streams both the image and the extracted data. It is the same algorithm as that used by Team Paradox during the Stronghold season.

The program has been tested on a **NVIDIA Jetson TK1** running **Linux For Tegra R21.4** (with updates) and **OpenCV 3.1.0**, with the **Microsoft LifeCam HD-3000** USB webcam. However, it should probably work without modification on any other Linux system with OpenCV and Video4Linux2 drivers.
