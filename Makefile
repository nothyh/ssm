build:
	cmake -DCMAKE_BUILD_TYPE=Debug -B build
	cmake --build build
clean:
	rm ./build -rf

run:
	./build/ssm -d -j /home/hyh/workspace/proj/ssm/ssm/resources/ssm_config.json 
.PHONY: build clean
