apt-get update
apt-get -y install build-essential git curl tar zip unzip wget pkg-config m4

# CMake 3.29.2 installation
CMAKE_VERSION=3.29.2

cd /usr/local
wget https://github.com/Kitware/CMake/releases/download/v$CMAKE_VERSION/cmake-$CMAKE_VERSION-linux-x86_64.sh
chmod +x ./cmake-$CMAKE_VERSION-linux-x86_64.sh
./cmake-$CMAKE_VERSION-linux-x86_64.sh --skip-license
rm cmake-$CMAKE_VERSION-linux-x86_64.sh

# Install VCPKG
cd /usr/local
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
chmod +x ./bootstrap-vcpkg.sh
./bootstrap-vcpkg.sh

# Install FMT
./vcpkg install fmt

# Install GMP
cd /usr/local
wget https://ftp.gnu.org/gnu/gmp/gmp-6.3.0.tar.gz
tar -xzf ./gmp-6.3.0.tar.gz
rm gmp-6.3.0.tar.gz
cd gmp-6.3.0
./configure
make