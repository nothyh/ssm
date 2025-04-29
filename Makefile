.PHONY: build run clean
build:
	cmake -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
run:
	./build/ssm
clean:
	rm -rf build