CXXFLAGS = -Xpreprocessor -fopenmp -lomp -I"$(shell brew --prefix libomp)/include" -L"$(shell brew --prefix libomp)/lib"

.PHONY: all clean

all: build/main

build/main: main.cpp ELFFile.cpp Linker.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ -std=c++17 -g

clean:
	rm build/main

