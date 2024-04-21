SRC_DIR=.
OUT_DIR=out/build
CMAKE_ARGS=-G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
PI_DIGITS=1000000

out/build:
	cmake -S $(SRC_DIR) -B $(OUT_DIR) $(CMAKE_ARGS)

build:
	cd out/build && make

run:
	cd out/build && ./pi_calculator $(PI_DIGITS)

clean:
	rm -rf out ext