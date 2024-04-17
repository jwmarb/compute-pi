SRC_DIR=.
OUT_DIR=out/build
CMAKE_ARGS=-G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release

out/build:
	cmake -S $(SRC_DIR) -B $(OUT_DIR) $(CMAKE_ARGS)

build:
	cd out/build && make

run:
	cd out/build && ./pi_calculator

clean:
	rm -rf out