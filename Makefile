CXXFLAGS = -Xpreprocessor -fopenmp -lomp -I"$(shell brew --prefix libomp)/include" -L"$(shell brew --prefix libomp)/lib"

.PHONY: all clean

all: build/main test/gen

build/main: main.cpp ELFFile.cpp Linker.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++17 -g

test/gen: test/test.cpp
	$(CXX) $^ -o $@ -std=c++11 -O2

clean:
	rm -f build/main test/code/*.*

gen: test/gen
	cd test && ./gen && cd code && make -j`nproc`

tarcode:
	cd test && tar czf code.tgz code

