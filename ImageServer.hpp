#pragma once

#include "Server.hpp"

#include <functional>

#include <cstdint>
#include <cstddef> // size_t

struct ImageHeader
{
	uint32_t imageSize;
	int16_t MinH;
	int16_t MaxH;
	int16_t MinS;
	int16_t MaxS;
	int16_t MinV;
	int16_t MaxV;
	int16_t cameraBrightness;
	int16_t cameraSaturation;
	int16_t cameraISO;
	int16_t targetPosition;
	int16_t pixelX;
	int16_t pixelY;
	int16_t pixelRed;
	int16_t pixelBlue;
	int16_t pixelGreen;
	int16_t pixelHue;
	int16_t pixelSat;
	int16_t pixelVal;
	int16_t targetX;
	int16_t targetY;
	int16_t targetWidth;
	int16_t targetHeight;
	float camRedBalance;
	float camBlueBalance;
	int16_t camAutoColorBalance;
	int16_t camExposureComp;
	int16_t camContrast;
	int16_t minArea;
	int16_t camShutterSpeed;
	float fps;
	int16_t showFiltered;
	int16_t zoom;
	int16_t camAutoExposure;
	int16_t blobPosition;
	int16_t centerLine;
};

// Forward declaration
namespace cv
{
	class Mat;
}

class ImageServer : public Server
{
	static const uint32_t key;
	static const size_t cmdBufSize;

	ImageHeader headerBuf;
	std::vector<unsigned char> imgBuf;

public:
	using Server::Server;

	void bufferFrame(const ImageHeader& header, const cv::Mat& image);

protected:
	virtual void handleCmd(char cmd, int arg1, int arg2) = 0;

	virtual void readData(int fd) override;
	virtual void writeData(int fd) override;
};
