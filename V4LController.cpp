#include "V4LController.hpp"

#include <cstdlib>

#include <sstream>

V4LController::V4LController(int device) :
	device(device)
{
}

int V4LController::set(const char* prop, int value)
{
	std::ostringstream cmd;
	cmd << "v4l2-ctl -d" << device << " -c " << prop << '=' << value;
	return std::system(cmd.str().c_str());
}
