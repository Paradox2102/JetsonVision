#include "ImageServer.hpp"

#include <opencv2/opencv.hpp>

#include <unistd.h>

#include <cstdio> // sscanf
#include <cinttypes>

#include <cstdint>
#include <cstddef> // size_t

const uint32_t ImageServer::key = 0xaa55aa55;
const size_t ImageServer::cmdBufSize = 64;

void ImageServer::bufferFrame(const ImageHeader& header, const cv::Mat& image)
{
	cv::imencode(".jpg", image, imgBuf);

	headerBuf = header;
	headerBuf.imageSize = imgBuf.size();
}

void ImageServer::readData(int fd)
{
	char buf[cmdBufSize];
	int len = check(::read(fd, buf, sizeof buf - 1));
	buf[len] = '\0';

	char cmd;
	int16_t arg;
	if (std::sscanf(buf, "%c %" SCNd16, &cmd, &arg) >= 1)
	{
		handleCmd(cmd, arg);
	}
}

void ImageServer::writeData(int fd)
{
	::write(fd, &key, sizeof key);
	::write(fd, &headerBuf, sizeof headerBuf);
	::write(fd, imgBuf.data(), imgBuf.size());
}
