build:
	cmake -B build
	cmake --build build
clean:
	rm ./build -rf

run:
	./build/ssm -s f1 -l 03 
.PHONY: build clean