.PHONY: build run clean
build:
	cmake -B build
	cmake --build build
run:
	./build/ssm
clean:
	rm -rf build