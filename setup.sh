apt-get update
apt-get -y install build-essential git curl tar zip unzip wget pkg-config m4

# variables
CWD=$(pwd)
EXT=$CWD/ext

# setup
mkdir $EXT

# Install GMP headers
GMP_VERSION=6.3.0
cd $EXT
mkdir gmp
wget https://ftp.gnu.org/gnu/gmp/gmp-$GMP_VERSION.tar.gz
tar -xzf ./gmp-$GMP_VERSION.tar.gz -C gmp
rm gmp-$GMP_VERSION.tar.gz
cd gmp
./configure

# Install MPI headers
MPI_VERSION=5.0.3
cd $EXT
wget https://download.open-mpi.org/release/open-mpi/v5.0/openmpi-$MPI_VERSION.tar.bz2
tar -jxpf openmpi-$MPI_VERSION.tar.bz2
cd openmpi-$MPI_VERSION
./configure --prefix=$EXT/openmpi
make install
cd ..
rm openmpi-$MPI_VERSION.tar.bz2 # remove compressed file
rm -rf openmpi-$MPI_VERSION # remove source

# Install FMT headers
FMT_VERSION=10.2.1
cd $EXT
mkdir fmt
wget https://github.com/fmtlib/fmt/releases/download/$FMT_VERSION/fmt-$FMT_VERSION.zip
unzip fmt-$FMT_VERSION.zip -d fmt
rm fmt-$FMT_VERSION