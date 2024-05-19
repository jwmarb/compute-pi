#!/bin/bash
#SBATCH --output=reduce.out
#SBATCH --partition=standard
#SBATCH --time=00:05:00

singularity exec hpc.sif /opt/out/build/pi_reducer $1 $2 $3 $4