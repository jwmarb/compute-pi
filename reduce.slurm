#!/bin/bash
#SBATCH --output=reduce.out
#SBATCH --partition=standard
#SBATCH --time=00:05:00

if [ -n "$SLURM_CPUS_PER_TASK" ]; then
  omp_threads=$SLURM_CPUS_PER_TASK
else
  omp_threads=1
fi

singularity exec --env OMP_NUM_THREADS=$omp_threads hpc.sif /opt/out/build/pi_reducer $1 $2 $3 $4