#!/bin/bash

CLUSTER_NAME=${PROMPT_COMMAND#echo -n \"(}
CLUSTER_NAME=${CLUSTER_NAME%) \"}

# The number of processes (equivalent to --ntasks) (this is also the # of computers)
if [ -n "$1" ]; then
  MPI_PROCESSES=$1
else
  MPI_PROCESSES=12
fi

# The number of threads per computer (process)
if [ -n "$2" ]; then
  OMP_THREADS=$2
else
  OMP_THREADS=16
fi

# How much memory each thread needs
MEM_PER_CPU=4gb

# The number of PI digits to calculate
NUM_PI_DIGITS=100000

TMP_DIR=/tmp
OUTPUT_PATH=./pi-$(numfmt --to=si $NUM_PI_DIGITS).$CLUSTER_NAME.bin

# The account to use for allocating time
ACCOUNT=your_group_name

if [ -n "$1" ] && [ -n "$2" ]; then
  COMPUTE_JOB_ID=$(sbatch --nodes=$MPI_PROCESSES --ntasks=$MPI_PROCESSES --cpus-per-task=$OMP_THREADS --mem-per-cpu=$MEM_PER_CPU --account=$ACCOUNT --output=logs/$MPI_PROCESSES\n$OMP_THREADS\c.out compute.slurm $NUM_PI_DIGITS $TMP_DIR PERFTEST)
else
  COMPUTE_JOB_ID=$(sbatch --ntasks=$MPI_PROCESSES --cpus-per-task=$OMP_THREADS --mem-per-cpu=$MEM_PER_CPU --account=$ACCOUNT --output=logs/%j-c.out compute.slurm $NUM_PI_DIGITS $TMP_DIR)
fi

echo ""
echo "[compute.slurm] JobID=${COMPUTE_JOB_ID##* }"
echo "  squeue --job ${COMPUTE_JOB_ID##* }"
echo "  seff ${COMPUTE_JOB_ID##* }"
echo ""

if ! [ -n "$1" ] || ! [ -n "$2" ]; then
  REDUCE_JOB_ID=$(sbatch --dependency=afterok:${COMPUTE_JOB_ID##* } --ntasks=1 --cpus-per-task=$OMP_THREADS --mem-per-cpu=4gb --account=$ACCOUNT reduce.slurm $NUM_PI_DIGITS $MPI_PROCESSES $TMP_DIR $OUTPUT_PATH)
  echo "[reduce.slurm] JobID=${REDUCE_JOB_ID##* }"
  echo "  squeue --job ${REDUCE_JOB_ID##* }"
  echo "  seff ${REDUCE_JOB_ID##* }"
  echo ""
fi