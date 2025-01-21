#VERBOSE=-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON
THREADS=12
#BUILD_TYPE=Debug
BUILD_TYPE=Release

all:
	mkdir -p build
	cd build && \
	cmake -GNinja \
        $(VERBOSE) \
	    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
        .. && \
	cmake --build . \
	    -j$(THREADS)

clean:
	rm -rf build
