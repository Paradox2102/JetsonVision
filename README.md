# Vision Server Example

This is an in-progress example of a program that processes real time video from a V4L-compatible webcam and streams both the image and the extracted data. It is the same algorithm as that used by Team Paradox during the Stronghold season.

The program has been tested on the following platforms:

- **NVIDIA Jetson TK1** running **Linux For Tegra R21.4** and **OpenCV 3.1.0**
- **NVIDIA Jetson TX1** running **Linux For Tegra R24.2** and **OpenCV 3.2.0**

The **Microsoft LifeCam HD-3000** USB webcam was used, which was supported by the built-in Video4Linux2 drivers. However, this program should work without modification on any other Linux system with OpenCV and the appropriate drivers for your camera.

## Usage

1. Log in to your device
	- **Remotely:** Use SSH (through [PuTTY](http://www.putty.org/) if you are using Windows).
	- **Locally:** Plug in a monitor (HDMI port) and keyboard (USB port). After login, open a Terminal window.
2. Install build tools (GCC, C++ libraries, Make, pkg-config)
	- `apt-get install build-essential pkg-config` as root
3. Install OpenCV from source
	- Detailed instructions: http://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html
4. Build the example
	- Clone the repository: `git clone https://github.com/Paradox2102/JetsonVision.git`
	- Enter the new directory: `cd JetsonVision`
	- Build and run the program: `make run`

## Explanation

If all goes well, when you run the generated executable `vision-server.o`, it will get images from the attached USB camera in a loop and detect blobs of the specified HSV color (defaults to all-inclusive 0-255). It assumes the largest of these blobs represents the tape around the castle goal, and extracts the following useful data points from the contour:

- Bounding rectangle: X, Y, width, and height
	- If the vertical center of this rectangle is close to the *ideal target X position*, the robot is facing the castle.
- Y1 and Y2: the Y-values of the top left and the top right corners, respectively
	- If the greater of these two values is close to the *ideal target Y position*, the robot is at the correct distance from the castle.

The program also acts as two different servers, the `DataServer` and the `ImageServer`.

### Robot side

The `DataServer` listens on port 5800 by default and repeatedly broadcasts this real-time information about the vision target in the plain text format shown below, terminated by a newline. The robot program is intended to connect to this port and make decisions based on incoming lines of data.

	R [X] [Y] [width] [height] [Y1] [Y2] [ideal target Y position] [frame number] [ideal target X position]

Items shown in brackets are unsigned integers.

The robot can also send commands back to the server, however this has not been implemented in this example.

### Driver side

The `ImageServer` listens on port 5801 by default and repeatedly broadcasts each image as well as parameters for image processing. The Driver Station laptop should have an *Image Viewer* program installed on the Desktop, which connects to the ImageServer and displays the video, as well as providing a user interface for viewing and changing parameters. Each message from the ImageServer consists of

- A key, which is the bytes **AA 55 AA 55** (hexadecimal)
- A header, which is an instance of `struct ImageHeader` (see ImageServer.hpp) directly transmitted as a byte array
	- It contains the size in bytes for the ensuing image, as well as the current image processing parameters
- The image itself, encoded as JPEG

The client (through a GUI used by drivers and programmers) can also send commands back to the server (in a plain text format) to change parameters. The following commands have been implemented so far:

#### HSV commands

- `h [delta]`: Change the **minimum** value for the Hue filter by `[delta]` (a signed integer)
- `H [delta]`: Change the **maximum** value for the Hue filter by `[delta]` (a signed integer)
- `s [delta]`: Change the **minimum** value for the Saturation filter by `[delta]` (a signed integer)
- `S [delta]`: Change the **maximum** value for the Saturation filter by `[delta]` (a signed integer)
- `v [delta]`: Change the **minimum** value for the Value filter by `[delta]` (a signed integer)
- `V [delta]`: Change the **maximum** value for the Value filter by `[delta]` (a signed integer)

All of these values are clamped to the 0-255 range on the server side.
