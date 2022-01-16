# INSTALL

The easiest way to use PERIOD is to use Docker. We strongly recommend installing and running our tool based on Docker.
 
## Installing based on docker (simple and easy)

**Hardware Requirements:** Please use workstations/PC with multi-core processors, as PERIOD is a concurrency testing tool
**Operating System:** >= Ubuntu 18.04 LTS (Requires kernel version >= 4.x due to the scheduling policy)
**Docker**: The only requirement is to install Docker (version higher than 18.09.7). You can use the command `sudo apt-get install docker.io` to install the docker on your Linux machine. (If you have any questions on docker, you can see [Docker's Documentation](https://docs.docker.com/engine/install/ubuntu/)).

**Add `period:latest` image on your system**. There are two ways of doing this:
- `docker pull wcventure/period:stable && docker tag wcventure/period:stable period:latest`
- Alternatively, you can build your own image with `sudo docker build -t period:latest --no-cache ./`

**running a privileged container**
- `sudo docker run --privileged -it period:latest /bin/bash`

Then you will start a docker container and enter the configured environment of PERIOD.

 
## Installing from source code on local environment (for development)
 
If you really want to install the tool from source code on your host system, please ensure that your environment meets the following requirements. 
 
- **CPU:** Please use workstations/PC with multi-core processors, as PERIOD is a concurrency testing tool
- **Operating System:** >= Ubuntu 18.04 LTS (Requires kernel version >= 4.x due to the scheduling policy)
- **install Package:**: Our artifact depends on some packages, please run the following command to install required packages.
  - ```sh
    sudo apt-get install -y wget git build-essential python3 python python-pip python3-pip tmux cmake libtool libtool-bin automake autoconf autotools-dev m4 autopoint libboost-dev help2man gnulib bison flex texinfo zlib1g-dev libexpat1-dev libfreetype6 libfreetype6-dev libbz2-dev liblzo2-dev libtinfo-dev libssl-dev pkg-config libswscale-dev libarchive-dev liblzma-dev liblz4-dev doxygen vim intltool gcc-multilib sudo --fix-missing```
  - ```sh
    pip install numpy && pip3 install numpy && pip3 install sysv_ipc
    ```
- **Clone the Repository**

    ```sh
    git clone https://github.com/wcventure/PERIOD.git PERIOD --depth=1
    cd PERIOD
    ```

- **Configure the environment and install the tool.**

    For simplicity, we provide shell scripts for the whole installation. Run the following command to automatically configure the environment and install the tool.
    
    ```sh
    # install LLVM and clang
    tool/install_llvm.sh

    # set up environment
    source tool/init_env.sh

    # install SVF
    $ROOT_DIR/tool/install_SVF.sh

    # install static analysis tool
    $ROOT_DIR/tool/install_staticAnalysis.sh

    # install wllvm
    sudo pip install -e $ROOT_DIR/ConFuzz/tool/wllvm/
    ```

- **Usage**
	
	The only difference between "Installing based on docker" is that everytime you open a new terminal you need to set up an environment variable.
	
	```
	# set up environment
    source tool/init_env.sh
	```
