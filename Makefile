.PHONY: build run clean
build:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
run:
	LD_LIBRARY_PATH=/opt/gcc-15.1/lib64 ./build/ssm
clean:
	rm -rf build
