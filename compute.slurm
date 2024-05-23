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

if [ -n "$3" ]; then
  mkdir -p $2/$SLURM_NTASKS\n$omp_threads\c
  TIMEFORMAT="value:%R"
  time mpirun -np $SLURM_NTASKS -x OMP_NUM_THREADS=$omp_threads singularity exec hpc.sif /opt/out/build/pi_calculator $1 $2/$SLURM_NTASKS\n$omp_threads\c
  rm -rf $2/$SLURM_NTASKS\n$omp_threads\c
else
  mpirun -np $SLURM_NTASKS -x OMP_NUM_THREADS=$omp_threads --bind-to core singularity exec hpc.sif /opt/out/build/pi_calculator $1 $2
fi