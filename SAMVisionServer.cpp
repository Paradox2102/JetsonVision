#include "ImageServer.hpp"
#include "DataServer.hpp"
#include "V4LController.hpp"

#include <opencv2/opencv.hpp>

#include <fstream>

#include <vector>
#include <string>

#include <algorithm>

#include <utility> // pair

#include <chrono>
#include <iostream>

#include <stdexcept> // runtime_error

class SAMVisionServer final : public ImageServer
{
	using Contour = std::vector<cv::Point>;
	using ContourArray = std::vector<Contour>;

	static constexpr int frameWidth = 640;
	static constexpr int frameHeight = 480;

	static constexpr bool rotateHSV = false; // true = better for red

	static constexpr double polyEpsilon = 10.0;

	static const cv::Scalar contourColor;

	static const cv::Scalar targetColor;
	static constexpr int targetLineWidth = 1;

	cv::VideoCapture videoSource;
	V4LController camCtrl;

	const std::string saveFilename;

	// Default values
	ImageHeader header
	{
		0,	// uint32_t imageSize;
		0,	// int16_t MinH;
		179,	// int16_t MaxH;
		0,	// int16_t MinS;
		255,	// int16_t MaxS;
		0,	// int16_t MinV;
		255,	// int16_t MaxV;
		0,	// int16_t cameraBrightness;
		0,	// int16_t cameraSaturation;
		0,	// int16_t cameraISO;
		frameHeight / 2,	// int16_t targetPosition;
		0,	// int16_t pixelX;
		0,	// int16_t pixelY;
		0,	// int16_t pixelRed;
		0,	// int16_t pixelBlue;
		0,	// int16_t pixelGreen;
		0,	// int16_t pixelHue;
		0,	// int16_t pixelSat;
		0,	// int16_t pixelVal;
		0,	// int16_t targetX;
		0,	// int16_t targetY;
		0,	// int16_t targetWidth;
		0,	// int16_t targetHeight;
		0,	// float camRedBalance;
		0,	// float camBlueBalance;
		0,	// int16_t camAutoColorBalance;
		0,	// int16_t camExposureComp;
		0,	// int16_t camContrast;
		0,	// int16_t minArea;
		0,	// int16_t camShutterSpeed;
		0,	// float fps;
		1,	// int16_t showFiltered;
		0,	// int16_t zoom;
		0,	// int16_t camAutoExposure;
		0,	// int16_t blobPosition;
		frameWidth / 2,	// int16_t centerLine;
	};

	cv::Scalar thresholdMin, thresholdMax;

	DataServer dataServer;

	int nFrames = 0;

public:
	SAMVisionServer(int camID, std::string saveFilename, const char* dataPort, const char* imagePort) :
		ImageServer(imagePort),
		videoSource(camID),
		camCtrl(camID),
		saveFilename(saveFilename),
		dataServer(dataPort)
	{
		if (!videoSource.isOpened())
		{
			throw std::runtime_error("Failed to open video capture stream");
		}

		{
			std::ifstream saveFile(saveFilename, std::ifstream::binary);
			if (saveFile.is_open())
			{
				saveFile.read(reinterpret_cast<char*>(&header), sizeof header);
			}
		}

		thresholdMin[0] = header.MinH;
		thresholdMin[1] = header.MinS;
		thresholdMin[2] = header.MinV;

		thresholdMax[0] = header.MaxH;
		thresholdMax[1] = header.MaxS;
		thresholdMax[2] = header.MaxV;
	}

	void processFrame()
	{
		auto start = std::chrono::steady_clock::now();

		cv::Mat frame;
		videoSource >> frame;

		{
			auto pixel = frame.at<cv::Vec3b>(header.pixelY, header.pixelX);
			header.pixelBlue = pixel[0];
			header.pixelGreen = pixel[1];
			header.pixelRed = pixel[2];
		}

		cv::Mat threshold;
		cv::cvtColor(frame, threshold, rotateHSV ? cv::COLOR_RGB2HSV : cv::COLOR_BGR2HSV);

		{
			auto pixel = threshold.at<cv::Vec3b>(header.pixelY, header.pixelX);
			header.pixelHue = pixel[0];
			header.pixelSat = pixel[1];
			header.pixelVal = pixel[2];
		}

		cv::inRange(threshold, thresholdMin, thresholdMax, threshold);

		ContourArray contours;
		cv::findContours(threshold, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);

		if (header.showFiltered)
		{
			cv::drawContours(frame, contours, -1, contourColor, -1);
		}

		/*
		double largestArea = 0.0;
		const Contour* pWinningContour = nullptr;
		for (const auto& contour : contours)
		{
			double area = cv::contourArea(contour);
			if (area > largestArea)
			{
				largestArea = area;
				pWinningContour = &contour;
			}
		}

		if (pWinningContour && largestArea > header.minArea)
		{
			cv::Rect rect = cv::boundingRect(*pWinningContour);

			if (header.showFiltered)
			{
				cv::rectangle(frame, rect.tl(), rect.br(), contourColor, contourThickness);
			}

			Contour quad(4);
			cv::approxPolyDP(*pWinningContour, quad, polyEpsilon, true);

			if (quad.size() == 4)
			{
				int distLine = std::max(quad[0].y, quad[3].y);
				int midLine = (quad[0].x + quad[3].x) / 2;

				cv::line(frame, { 0, distLine }, { frame.cols, distLine }, contourColor);
				cv::line(frame, { midLine, 0 }, { midLine, frame.rows }, contourColor);
			}

			dataServer.bufferData('R', {
				rect.x, rect.y,
				rect.width, rect.height,
				0, 0,// quad[0].y, quad[3].y,
				header.targetPosition,
				++nFrames,
				header.centerLine
			});
			dataServer.run(5);
		}
		*/

		for (auto it = contours.cbegin(); it != contours.cend(); ++it)
		{
			if (cv::contourArea(*it) >= header.minArea)
			{
				auto rect = cv::boundingRect(*it);

				if (header.showFiltered)
				{
					int xMid = rect.x + rect.width / 2;
					int yMid = rect.y + rect.height / 2;
					cv::line(frame, { xMid, 0 }, { xMid, frame.rows }, targetLineWidth);
					cv::line(frame, { 0, yMid }, { frame.cols, yMid }, targetLineWidth);
				}

				dataServer.bufferData('R', {
					std::distance(contours.cbegin(), it),
					rect.x, rect.y,
					rect.width, rect.height,
					0, 0,// quad[0].y, quad[3].y,
					header.targetPosition,
					nFrames,
					header.centerLine
				});
				dataServer.run(5);
			}
		}

		bufferFrame(header, frame);
		run(5);

		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> diff = end - start;
		header.fps = 1.0 / diff.count();

		++nFrames;
	}

private:
	template <typename T>
	static T limit(T n, T min, T max)
	{
		if (n > max)
		{
			return max;
		}
		else if (n < min)
		{
			return min;
		}
		else
		{
			return n;
		}
	}

	template <typename T>
	static T limit(T n, T min)
	{
		if (n < min)
		{
			return min;
		}
		else
		{
			return n;
		}
	}

	virtual void handleCmd(char cmd, int arg1, int arg2) override
	{
		switch (cmd)
		{
		case 'h':
			thresholdMin[0] = header.MinH = limit(header.MinH + arg1, 0, 179);
			break;

		case 's':
			thresholdMin[1] = header.MinS = limit(header.MinS + arg1, 0, 255);
			break;

		case 'v':
			thresholdMin[2] = header.MinV = limit(header.MinV + arg1, 0, 255);
			break;

		case 'H':
			thresholdMax[0] = header.MaxH = limit(header.MaxH + arg1, 0, 179);
			break;

		case 'S':
			thresholdMax[1] = header.MaxS = limit(header.MaxS + arg1, 0, 255);
			break;

		case 'V':
			thresholdMax[2] = header.MaxV = limit(header.MaxV + arg1, 0, 255);
			break;

		case 'E':
			header.camShutterSpeed = limit(header.camShutterSpeed + arg1, 0);
			camCtrl.set("exposure_time_absolute", header.camShutterSpeed);
			break;

		case 'm':
			header.minArea = limit(header.minArea + arg1, 0);
			break;

		case 't':
			header.pixelX = arg1;
			header.pixelY = arg2;
			break;

		case 'p':
			header.showFiltered = !header.showFiltered;
			break;

		case 'w':
			{
				std::ofstream saveFile(saveFilename, std::ofstream::binary);
				if (saveFile.is_open())
				{
					saveFile.write(reinterpret_cast<char*>(&header), sizeof header);
				}
			}
			break;
		}
	}
};

const cv::Scalar SAMVisionServer::contourColor(0, 255, 0);
const cv::Scalar SAMVisionServer::targetColor(0, 0, 255);

int main()
{
	// TODO: Get save file name from command line
	SAMVisionServer server(0, "settings", "5800", "5801");
	for (;;)
	{
		server.processFrame();
	}
}
