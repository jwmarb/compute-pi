SRC_DIR=.
OUT_DIR=out/build
CMAKE_ARGS=-G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
RUN=/usr/local/openmpi/bin/mpirun

out/build:
	cmake -S $(SRC_DIR) -B $(OUT_DIR) $(CMAKE_ARGS)

build:
	cd out/build && make

run:
	cd out/build && $(RUN) -n $(OMPI_COMM_WORLD_SIZE) --allow-run-as-root ./pi_calculator $(PI_DIGITS)

clean:
	rm -rf out ext