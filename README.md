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

mpirun -n $NUM_PROCESSES singularity exec --env OMP_NUM_THREADS=$OMP_NUM_THREADS hpc.sif /opt/out/build/pi_calculator $NUM_PI_DIGITS
singularity exec hpc.sif /opt/out/build/pi_reducer $NUM_PI_DIGITS $NUM_PROCESSES
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

To run the `hpc_local.sif` container:

```sh
OMP_NUM_THREADS=<cpus-per-tasks>
NUM_PI_DIGITS=<num_of_pi_digits>
NUM_PROCESSES=<ntasks>

# If you have a version of MPI in your system, uncomment below
# mpirun -n $NUM_PROCESSES singularity exec --env OMP_NUM_THREADS=$OMP_NUM_THREADS hpc_local.sif /opt/out/build/pi_calculator $NUM_PI_DIGITS

# If you do NOT have a local version of MPI in your system, uncomment below
singularity exec --env OMP_NUM_THREADS=$OMP_NUM_THREADS hpc_local.sif mpirun -n $NUM_PROCESSES 

singularity exec hpc_local.sif /opt/out/build/pi_reducer $NUM_PI_DIGITS $NUM_PROCESSES
```