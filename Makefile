src = $(wildcard *.cpp)
inc = $(wildcard *.hpp)
obj = robovision.o
obj_debug = debug.o

pc = pkg-config /opt/lib/pkgconfig/opencv.pc

$(obj): $(src) $(inc)
	g++ `$(pc) --cflags` -o $(obj) $(src) `$(pc) --libs` -lpthread -std=c++11 -Wall -Wextra -Wno-missing-field-initializers

$(obj_debug): $(src) $(inc)
	g++ -g `$(pc) --cflags` -o $(obj_debug) $(src) `$(pc) --libs` -lpthread -std=c++11 -Wall -Wextra

run: $(obj)
	./$(obj)

debug: $(obj_debug)
	gdb $(obj_debug)

clean:
	-rm $(obj) $(obj_debug)
