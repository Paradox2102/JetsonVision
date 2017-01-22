class V4LController {
	int device;

public:
	V4LController(int device);
	
	int get(const char* prop);
	int set(const char* prop, int value);
};
