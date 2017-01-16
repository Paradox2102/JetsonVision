src = $(wildcard *.cpp)
inc = $(wildcard *.hpp)
obj = vision-server.o
obj_debug = debug-vision-server.o

pc = pkg-config opencv

$(obj): $(src) $(inc)
	g++ $(src) -o $(obj) `$(pc) --cflags --libs` -O2 -std=c++14 -Wall

$(obj_debug): $(src) $(inc)
	g++ $(src) -o $(obj_debug) `$(pc) --cflags --libs` -g -Og -std=c++14 -Wall

run: $(obj)
	./$(obj)

debug: $(obj_debug)
	gdb $(obj_debug)

clean:
	rm -f $(obj) $(obj_debug)
