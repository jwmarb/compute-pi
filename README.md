<h1 align="center">
  compute-pi
</h1>
<p align="center">
A small program that calculates up to any digit of π with HPC resources
</p>

<br>
<br>

## How to run

### Running with Singularity/Apptainer

**Note: This must be done in an HPC environment**

Build the container by running the command

```sh
singularity build hpc.sif hpc.def
```

Once the container has been built, you can run via slurm or manually:

#### Run via Slurm

Edit the variables in `run.sh`, setting them to what you need. You may need to `chmod +x ./run.sh` to execute it.

To execute:

```sh
./run.sh
```

#### Run manually

```sh
OMP_NUM_THREADS=<cpus-per-tasks>
NUM_PI_DIGITS=<num_of_pi_digits>
NUM_PROCESSES=<ntasks>
TMP_DIR=/tmp
OUTPUT_PATH=./pi.bin

mpirun -n $NUM_PROCESSES singularity exec --env OMP_NUM_THREADS=$OMP_NUM_THREADS hpc.sif /opt/out/build/pi_calculator $NUM_PI_DIGITS $TMP_DIR
singularity exec --env OMP_NUM_THREADS=$OMP_NUM_THREADS hpc.sif /opt/out/build/pi_reducer $NUM_PI_DIGITS $NUM_PROCESSES $TMP_DIR $OUTPUT_PATH
```

### Running locally

There are two ways to run this locally based on two definition files:

* `pi.def` - A command-line interface to calculate π with arguments
  * `-t` `--threads` The number of threads
  * `-p` `--processes` The number of MPI processes
  * `-d` `--digits` The number of digits to calculate
* `hpc_local.def` - A replica of the HPC environment that can be built locally

To build these containers:

```sh
singularity build pi.sif pi.def
singularity build hpc_local.sif hpc_local.def
```

To run the `pi.sif` container:

```sh
singularity run pi.sif -t 1 -p 1 -d 100000
```

This will output a `pi_out.bin` file, which the file path and tmp directory can be changed via the [Makefile](/Makefile) 

<br>

To run the `hpc_local.sif` container:

```sh
OMP_NUM_THREADS=<cpus-per-tasks>
NUM_PI_DIGITS=<num_of_pi_digits>
NUM_PROCESSES=<ntasks>
TMP_DIR=/tmp
OUTPUT_PATH=./pi.bin

# If you have a version of MPI in your system, uncomment below
# mpirun -n $NUM_PROCESSES singularity exec --env OMP_NUM_THREADS=$OMP_NUM_THREADS hpc_local.sif /opt/out/build/pi_calculator $NUM_PI_DIGITS $TMP_DIR

# If you do NOT have a local version of MPI in your system, uncomment below
singularity exec --env OMP_NUM_THREADS=$OMP_NUM_THREADS hpc_local.sif mpirun -n $NUM_PROCESSES /opt/out/build/pi_calculator $NUM_PI_DIGITS $TMP_DIR

singularity exec --env OMP_NUM_THREADS=$(nproc) hpc_local.sif /opt/out/build/pi_reducer $NUM_PI_DIGITS $NUM_PROCESSES $TMP_DIR $OUTPUT_PATH
```

#### Development

Intellisense has been configured for VSCode for this project. However, the header files of external libraries are NOT included. To automatically get the header files necessary for intellisense, run the shell script `setup.sh`:

```sh
chmod +x ./setup.sh && ./setup.sh
```

Depending on how fast your machine is, this will take a while. This is because the external libraries are also compiled to get their shared/dynamic library files.