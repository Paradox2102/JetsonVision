#include "ImageServer.hpp"
#include "DataServer.hpp"

#include <opencv2/opencv.hpp>

#include <fstream>

#include <vector>
#include <string>

#include <utility> // pair

#include <stdexcept> // runtime_error

class SAMVisionServer final : public ImageServer
{
	using Contour = std::vector<cv::Point>;
	using ContourArray = std::vector<Contour>;

	static constexpr bool rotateHSV = false; // true = better for red

	static constexpr double polyEpsilon = 10.0;

	static const cv::Scalar contourColor;
	static constexpr int contourThickness = 2;

	cv::VideoCapture videoSource;

	const std::string saveFilename;

	ImageHeader header
	{
		0,
		0, 255,
		0, 255,
		0, 255,
	};

	cv::Scalar thresholdMin, thresholdMax;

	DataServer dataServer;

	int nFrames = 0;

public:
	SAMVisionServer(int camID, std::string saveFilename, const char* dataPort, const char* imagePort) :
		ImageServer(imagePort),
		videoSource(camID),
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
		cv::Mat frame;
		videoSource >> frame;

		cv::Mat threshold;
		cv::cvtColor(frame, threshold, rotateHSV ? cv::COLOR_RGB2HSV : cv::COLOR_BGR2HSV);
		cv::inRange(threshold, thresholdMin, thresholdMax, threshold);

		ContourArray contours;
		cv::findContours(threshold, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_TC89_KCOS);

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

		if (pWinningContour)
		{
			Contour quad(4);
			cv::approxPolyDP(*pWinningContour, quad, polyEpsilon, true);

			if (quad.size() == 4)
			{
				auto distLine = std::max(quad[0].y, quad[3].y);
				auto midLine = (quad[0].x + quad[3].x) / 2;

				cv::line(frame, { 0, distLine }, { frame.cols, distLine }, contourColor);
				cv::line(frame, { midLine, 0 }, { midLine, frame.rows }, contourColor);

				// cv::drawContours(frame, contours, -1, { 0, 255, 0 });

				auto rect = cv::boundingRect(*pWinningContour);
				cv::rectangle(frame, rect.tl(), rect.br(), contourColor, contourThickness);

				dataServer.bufferData('R', { rect.x, rect.y, rect.width, rect.height, quad[0].y, quad[3].y, distLine, ++nFrames, midLine });
				dataServer.run(17);
			}
		}

		bufferFrame(header, frame);
		run(17);
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

	virtual void handleCmd(char cmd, int arg) override
	{
		switch (cmd)
		{
		case 'h':
			thresholdMin[0] = header.MinH = limit(header.MinH + arg, 0, 255);
			break;

		case 's':
			thresholdMin[1] = header.MinS = limit(header.MinS + arg, 0, 255);
			break;

		case 'v':
			thresholdMin[2] = header.MinV = limit(header.MinV + arg, 0, 255);
			break;

		case 'H':
			thresholdMax[0] = header.MaxH = limit(header.MaxH + arg, 0, 255);
			break;

		case 'S':
			thresholdMax[1] = header.MaxS = limit(header.MaxS + arg, 0, 255);
			break;

		case 'V':
			thresholdMax[2] = header.MaxV = limit(header.MaxV + arg, 0, 255);
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

int main()
{
	SAMVisionServer server(0, "settings", "5800", "5801");
	for (;;)
	{
		server.processFrame();
	}
}
