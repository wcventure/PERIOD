FROM ubuntu:18.04

RUN cp /etc/apt/sources.list /etc/apt/sources.list.bak
RUN sed -i s:/archive.ubuntu.com:/mirrors.tuna.tsinghua.edu.cn/ubuntu:g /etc/apt/sources.list
RUN apt-get clean
RUN apt-get update --fix-missing
RUN apt-get install -y wget git build-essential apt-utils python3 python python-pip python3-pip tmux cmake libtool libtool-bin automake autoconf autotools-dev m4 autopoint libboost-dev help2man gnulib bison flex texinfo zlib1g-dev libexpat1-dev libfreetype6 libfreetype6-dev libbz2-dev liblzo2-dev libtinfo-dev libssl-dev pkg-config libswscale-dev libarchive-dev liblzma-dev liblz4-dev doxygen vim intltool gcc-multilib sudo --fix-missing

RUN mkdir -p /workdir/ConFuzz

WORKDIR /workdir/ConFuzz
COPY . /workdir/ConFuzz

ENV PATH "/workdir/ConFuzz/clang+llvm/bin:$PATH"
ENV LD_LIBRARY_PATH "/workdir/ConFuzz/clang+llvm/lib:$LD_LIBRARY_PATH"
ENV ROOT_DIR "/workdir/ConFuzz"

RUN sudo pip install -e /workdir/ConFuzz/tool/wllvm/
RUN sudo pip install numpy
RUN sudo pip3 install numpy
RUN sudo pip3 install sysv_ipc

RUN cp -rf /usr/share/aclocal/* /usr/local/share/aclocal/

RUN tool/install_llvm.sh
RUN tool/install_Fuzzer.sh
RUN tool/install_SVF.sh
RUN tool/install_staticAnalysis.sh
