#VERBOSE=-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON

all:
	mkdir -p build
	cd build && cmake -GNinja $(VERBOSE) .. && cmake --build .

clean:
	rm -rf build
