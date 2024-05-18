apt-get update
apt-get -y install build-essential git curl tar zip unzip wget pkg-config m4

# variables
CWD=$(pwd)
EXT=$CWD/ext

# setup
mkdir -p $EXT

# Install GMP
GMP_VERSION=6.3.0
cd $EXT
wget https://ftp.gnu.org/gnu/gmp/gmp-$GMP_VERSION.tar.gz
tar -xzf ./gmp-$GMP_VERSION.tar.gz
rm gmp-$GMP_VERSION.tar.gz
mv gmp-$GMP_VERSION gmp
cd gmp
./configure
make -j$(nproc)


# Install MPI

MPI_VERSION=3.1.4
cd $EXT
wget https://download.open-mpi.org/release/open-mpi/v3.1/openmpi-$MPI_VERSION.tar.bz2
tar -jxpf openmpi-$MPI_VERSION.tar.bz2
cd openmpi-$MPI_VERSION
./configure --prefix=$EXT/ompi
make -j$(nproc) all
make install
cd ..
rm openmpi-$MPI_VERSION.tar.bz2 # remove compressed file
rm -rf openmpi-$MPI_VERSION # remove source

# Install FMT
# FMT_VERSION=10.2.1
# cd $EXT
# wget https://github.com/fmtlib/fmt/releases/download/$FMT_VERSION/fmt-$FMT_VERSION.zip
# unzip fmt-$FMT_VERSION.zip
# rm fmt-$FMT_VERSION.zip
# mv fmt-$FMT_VERSION fmt
# cd fmt
# cmake -DBUILD_SHARED_LIBS=TRUE -S . -B out/build
# cd out/build
# make -j$(nproc)