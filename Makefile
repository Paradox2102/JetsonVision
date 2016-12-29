src = $(wildcard *.cpp)
inc = $(wildcard *.hpp)
obj = vision-server.o
obj_debug = debug-vision-server.o

pc = pkg-config /opt/lib/pkgconfig/opencv.pc

$(obj): $(src) $(inc)
	g++ `$(pc) --cflags` -O2 -o $(obj) $(src) `$(pc) --libs` -lpthread -std=c++11 -Wall

$(obj_debug): $(src) $(inc)
	g++ -g `$(pc) --cflags` -Og -o $(obj_debug) $(src) `$(pc) --libs` -lpthread -std=c++11 -Wall

run: $(obj)
	./$(obj)

debug: $(obj_debug)
	gdb $(obj_debug)

clean:
	rm -f $(obj) $(obj_debug)
