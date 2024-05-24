SRC_DIR=.
OUT_DIR=out/build
CMAKE_ARGS=-G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -D CMAKE_C_COMPILER=/usr/local/bin/mpicc -D CMAKE_CXX_COMPILER=/usr/local/bin/mpic++
RUN=/usr/local/bin/mpirun

out/build:
	cmake -S $(SRC_DIR) -B $(OUT_DIR) $(CMAKE_ARGS)

build:
	cd out/build && make

run:
	mkdir -p tmp
	cd out/build && $(RUN) -n $(OMPI_COMM_WORLD_SIZE) --allow-run-as-root ./pi_calculator $(PI_DIGITS) ../../tmp
	# cd out/build && $(RUN) -n $(OMPI_COMM_WORLD_SIZE) --allow-run-as-root ./pi_reducer_mpi $(PI_DIGITS) $(OMPI_COMM_WORLD_SIZE) ../../tmp ./pi_out.bin; rm ../../tmp/[TQP]AB[0123456789+]*.bin
	cd out/build && export OMP_NUM_THREADS=1 && ./pi_reducer_omp $(PI_DIGITS) $(OMPI_COMM_WORLD_SIZE) ../../tmp ./pi_out.bin

clean:
	rm -rf out ext