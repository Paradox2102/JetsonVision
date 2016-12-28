# Vision Server Example

This is a complete example of a program that processes real time video from a V4L-compatible webcam and streams both the image and the extracted data. It is the same algorithm as that used by Team Paradox during the Stronghold season.

The program has been tested on a **NVIDIA Jetson TK1** running **Linux For Tegra R21.4** (with updates) and **OpenCV 3.1.0**, with the **Microsoft LifeCam HD-3000** USB webcam. However, it should probably work without modification on any other Linux system with OpenCV and Video4Linux2 drivers.

## Usage

1. Log in to your device
	- **Remotely:** Use SSH (through [PuTTY](http://www.putty.org/) if you are using Windows).
	- **Locally:** Plug in a monitor (HDMI port) and keyboard (USB port). After login, open a Terminal window.
2. Install build tools (GCC, C++ libraries, Make, pkg-config)
	- `apt-get install build-essential pkg-config` as root
3. Install OpenCV from source
	- Detailed instructions: http://docs.opencv.org/master/d7/d9f/tutorial_linux_install.html
		- Set CMAKE_INSTALL_PREFIX to `/opt` so it will not conflict with opencv from Ubuntu's repositories
4. Build the example
	- Clone the repository: `git clone https://github.com/Paradox2102/JetsonVision.git`
	- Enter the new directory: `cd JetsonVision`
	- Build and run the program: `make run`

*If you installed somewhere other than `/opt`, you will have to edit the following line in `Makefile` and replace `/opt` with what you used for CMAKE_INSTALL_PREFIX during the OpenCV installation:*

	pc = pkg-config /opt/lib/pkgconfig/opencv.pc
