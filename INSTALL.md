# INSTALL

We provide here a snapshot of ConFuzz. For simplicity, we provide shell script for the whole installation.

### Requirements

- Operating System: Ubuntu 18.04 LTS (*This is very important, as our implementation requires higher kernel version*)
- Run the following command to install required packages
    ```sh
    $ sudo apt-get install -y wget git build-essential python3 python python-pip python3-pip tmux cmake libtool libtool-bin automake autoconf autotools-dev m4 autopoint libboost-dev help2man gnulib bison flex texinfo zlib1g-dev libexpat1-dev libfreetype6 libfreetype6-dev libbz2-dev liblzo2-dev libtinfo-dev libssl-dev pkg-config libswscale-dev libarchive-dev liblzma-dev liblz4-dev doxygen vim intltool gcc-multilib sudo --fix-missing
    $ pip install numpy && pip3 install numpy && pip3 install sysv_ipc
    ```

### Clone the Repository

```sh
$ git clone https://github.com/wcventure/ConcurrencyFuzzer.git ConFuzz --depth=1
$ cd ConFuzz
```

### Build

Run the following command to automatically configure the environment and install the tool.

```sh
# install LLVM and clang
tool/install_llvm.sh

# set up environment
export ROOT_DIR=path/to/ConFuzz
export PATH=$ROOT_DIR/clang+llvm/bin:$PATH
export LD_LIBRARY_PATH=$ROOT_DIR/clang+llvm/lib:$LD_LIBRARY_PATH

# install SVF
$ROOT_DIR/tool/install_SVF.sh

# install the fuzzer
$ROOT_DIR/tool/install_Fuzzer.sh

# install static analysis tool
$ROOT_DIR/tool/install_staticAnalysis.sh

# install wllvm
sudo pip install -e /workdir/ConFuzz/tool/wllvm/
```
