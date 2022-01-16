# The Environment Required for Running the Artifacts
 
For simplicity, we provide shell scripts for the whole installation. But we still strongly recommend installing and running our tool under a Docker container.
 
### Install based on docker (simple and easy)

- **Hardware Requirements:** Please use workstations/PC with multi-core processors, as PERIOD is a concurrency testing tool
- **Operating System:** >= Ubuntu 18.04 LTS (Requires kernel version >= 4.x due to the scheduling policy)
- **Docker**: The only requirement is to install Docker (version higher than 18.09.7). You can use the command `sudo apt-get install docker.io` to install the docker on your Linux machine. (If you have any questions on docker, you can see [Docker's Documentation](https://docs.docker.com/engine/install/ubuntu/)).
 
 
### Install from source code on your host system (step by step)
 
If you really want to install the tool from source code on your local environment, please ensure that your environment meets the following requirements.

- **Hardware Requirements:** Please use workstations/PC with multi-core processors, as PERIOD is a concurrency testing tool 
- **Operating System:** Ubuntu 18.04 LTS (Requires kernel version >= 4.x due to the scheduling policy)
- **Package:**: Our artifact depends on some packages, please run the following command to install required packages.
  ```sh
  sudo apt-get install -y wget git build-essential python3 python python-pip python3-pip tmux cmake libtool libtool-bin automake autoconf autotools-dev m4 autopoint libboost-dev help2man gnulib bison flex texinfo zlib1g-dev libexpat1-dev libfreetype6 libfreetype6-dev libbz2-dev liblzo2-dev libtinfo-dev libssl-dev pkg-config libswscale-dev libarchive-dev liblzma-dev liblz4-dev doxygen vim intltool gcc-multilib sudo --fix-missing
  ```
  ```sh
  pip install numpy && pip3 install numpy && pip3 install sysv_ipc
  ```
