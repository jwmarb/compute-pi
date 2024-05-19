#!/bin/bash

# The number of processes (equivalent to --ntasks) (this is also the # of computers)
MPI_PROCESSES=4

# The number of threads per computer (process)
OMP_THREADS=12

# How much memory each thread needs
MEM_PER_CPU=4gb

# The number of PI digits to calculate
NUM_PI_DIGITS=100000

TMP_DIR=/tmp
OUTPUT_PATH=./pi-$NUM_PI_DIGITS.bin

# The account to use for allocating time
ACCOUNT=your_group_name

COMPUTE_JOB_ID=$(sbatch --ntasks=$MPI_PROCESSES --cpus-per-task=$OMP_THREADS --mem-per-cpu=$MEM_PER_CPU --account=$ACCOUNT compute.slurm $NUM_PI_DIGITS $TMP_DIR)
echo ""
echo "[compute.slurm] JobID=${COMPUTE_JOB_ID##* }"
echo "  squeue --job ${COMPUTE_JOB_ID##* }"
echo "  seff ${COMPUTE_JOB_ID##* }"
echo ""

REDUCE_JOB_ID=$(sbatch --dependency=afterok:${COMPUTE_JOB_ID##* } --ntasks=1 --cpus-per-task=1 --mem-per-cpu=4gb --account=$ACCOUNT reduce.slurm $NUM_PI_DIGITS $MPI_PROCESSES $TMP_DIR $OUTPUT_PATH)
echo "[reduce.slurm] JobID=${REDUCE_JOB_ID##* }"
echo "  squeue --job ${REDUCE_JOB_ID##* }"
echo "  seff ${REDUCE_JOB_ID##* }"
echo ""