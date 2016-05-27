all: test

CFLAGS=-fPIC -g -Wall `pkg-config --cflags opencv`
LIBS = `pkg-config --libs opencv`
INCLUDE = -I/usr/local/include/libfreenect -I/usr/include/libusb-1.0
FREE_LIBS = -L/usr/local/lib -lfreenect -lpthread

#you may need to run this in the terminal we may want to try
#making a shell script that launches and manages this stuff
#or come up with a more permanent solution.
test:  test.cpp
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
	export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
	$(CXX) $(INCLUDE) $(CFLAGS) $? -o $@  $(LIBS) $(FREE_LIBS)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o test

