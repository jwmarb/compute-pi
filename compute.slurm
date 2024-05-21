#!/bin/bash
#SBATCH --output=compute.out
#SBATCH --partition=standard
#SBATCH --time=01:00:00

module load gnu8 openmpi3

if [ -n "$SLURM_CPUS_PER_TASK" ]; then
  omp_threads=$SLURM_CPUS_PER_TASK
else
  omp_threads=1
fi

mpirun -np $SLURM_NTASKS -x OMP_NUM_THREADS=$omp_threads --bind-to core singularity exec hpc.sif /opt/out/build/pi_calculator $1 $2