# Petalinux Project for RTL Miner

# Miner

This project implements a bitcoin miner on the Xilinx ZYNQ-7000.

The software writes a block to be mined into memory. The software adds the required padding before mining commences. A 
set of mining cores will read in the block and hash it. Each mining core writes it own nonce to the block header. A 
mining core can manage multiple SHA256 implementations and will process a range of nonce's in parallel.

<https://zipcpu.com/> is an excellent source for FPGA tutorials and articles.

## Aims

* Build a distribution using first Petalinux tools.
* Understand how to use device tree, create own drivers etc
* Have memory-mapped IO and DMA transfers to memory fro PL-PS communication

## WSL Ubuntu 22.03.3 Development Environment Setup

```bash
sudo apt update
sudo apt upgrade
sudo apt install build-essential lzop u-boot-tools net-tools bison flex libssl-dev libncurses5-dev
sudo apt install libncursesw5-dev unzip chrpath xz-utils minicom 
```

Download the linux compiler `arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-linux-gnueabihf.tar.xz` from 
<https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads>. 

```bash
sudo tar -xf arm-gnu-toolchain-13.2.rel1-x86_64-arm-none-linux-gnueabihf.tar.xz -C /opt
```

I add the following function to my `.bashrc`.

<https://superuser.com/questions/39751/add-directory-to-path-if-its-not-already-there>

```bash
pathadd() {
    if [ -d "$1" ] && [[ ":$PATH:" != *":$1:"* ]]; then
        PATH="${PATH:+"$PATH:"}$1"
    fi
}

pathadd /opt/arm-gnu-toolchain-13.2.Rel1-x86_64-arm-none-linux-gnueabihf/bin
```

## Device Tree Documentation

<https://www.devicetree.org/>
<https://elinux.org/Device_Tree_What_It_Is>
<https://www.kernel.org/doc/Documentation/devicetree/usage-model.txt>

## Clang

Download a release from <https://github.com/llvm/llvm-project/releases>


## PetaLinux Tools Installation

[PetaLinux Tools Documentation: Reference Guide](https://docs.xilinx.com/r/en-US/ug1144-petalinux-tools-reference-guide/Overview)

There is a spreadsheet in the release notes for PetaLinux Tools that lists the required dependancies.

```bash
sudo apt install iproute2 gawk python3 build-essential gcc bc g++ git make net-tools libncurses5-dev tftpd zlib1g-dev 
sudo apt install libssl-dev flex bison libselinux1 gnupg wget git-core diffstat chrpath socat xterm autoconf libtool 
sudo apt install tar unzip texinfo zlib1g-dev gcc-multilib automake screen pax gzip cpio python3-pip python3-pexpect 
sudo apt install xz-utils debianutils iputils-ping python3-git python3-jinja2 libegl1-mesa libsdl1.2-dev libtinfo5

sudo locale-gen "en_US.UTF-8"
sudo update-locale LANG=en_US.UTF-8
```

Create the installation directory and make yourself the owner of the folder. Then run the petalinux installer 
(do not use root).

```bash
mkdir /opt/petalinux
chown <user_name> /opt/petalinux
chgrp <user_name> /opt/petalinux
./petalinux-v2023.1-05012318-installer.run -d /opt/petalinux
```

Add the following line to your `~/.bashrc` file:

```bash
source /opt/petalinux/settings.sh
```

Restart the terminal and run `echo $PETALINUX` to check if the environment is setup properly. This environmental
variables points to the installation directory if set.

## Setup a New Project

For WSL, ensure you are building the project in a linux directory and not one of the mounted filesystems from windows.
This is because some of the petalinux components cannot handle filesystems that are cae insensitive.

1. Create a project

Copy the bitstream (MinerCore.bit) and hardware description (top.xsa) into this directrory.

```bash
# Setup the petalinux build tools.
source /opt/petalinux/settings.sh

# `-t project` creates a project of type petalinux project.
# `project -n rtl-miner` gives the project the name rtl-miner.
# `--template zynq` structures the project for a zynq-7000 device.
petalinux-create -t project -n rtl-miner --template zynq
```

2. Configure the project with hardware description

```bash
# CD in the project directory.
cd rtl-miner

# Configure the project with the hardware description compiled with the FPGA project.
# You will be prompted with a set of configurations. You can change settings or just exit to continue.
# The top.xsa can be found in the prj directory of the rtl-miner FPGA project after the bitstream has compiled.
petalinux-config --get-hw-description ../top.xsa

# Depending on the power of your machine, you many want to limit the number or parallel jobs the yocto executes.
# See the BB_NUMBER_THREADS and PARALLEL_MAKE yocto settings
```

3. Delete reference to the second CPU

The following device tree file was add to `components/plnx_workspace/device-tree/device-tree` to remove reference
to cpu1 which doesn't exist. The file is called `system-user.dtsi`.

<https://forum.digilent.com/topic/23724-how-to-generate-device-tree-for-single-core-cora-z7/>

```
/include/ "system-conf.dtsi"
/ {
     aliases {
        /delete-property/ cpu1;
        /delete-property/ funnel0_in_port1;
    };
    /delete-node/ cpu@1;

};

&amba{
    /delete-node/ ptm@f889d000;
};

&funnel0_in_port1{
   /delete-property/ remote-endpoint; 
};
```

4. Build the project

```bash
petalinux-build

# If a python script fails with `WARNING: exit code 137 from a shell command.` then just run the build again.
```

5. Flash the SD Card

```
petalinux-package --boot --force --fsbl images/linux/zynq_fsbl.elf --fpga ../MinerCore.bit --u-boot
```

Then copy the following three files to a FAT format SD card for create a bootable OS:
* `BOOT.BIN`
* `image.ub`
* `boot.scr`

For Cora Z7-07S, make sure JP2 is on, and you can view the boot view the virtual com port of the USB that provides 
power.

## Reference

<https://docs.amd.com/r/en-US/ug1144-petalinux-tools-reference-guide/Installing-the-PetaLinux-Tool>
<https://github.com/Digilent/Petalinux-Cora-Z7-07S>
<https://doayee.co.uk/petalinux-on-windows-via-wsl-and-git/>



## Old Stuff

I have `project-spec/meta-user/recipes-bsp/device-tree/files/system-user.dtsi`

```
/include/ "system-conf.dtsi"
/ {
    chosen {
        bootargs = "console=ttyPS0,115200 earlyprintk uio_pdrv_genirq.of_id=generic-uio cma=16M";
    };
};
```

4. Add the u-dma-buf driver

```bash
petalinux-create -t modules -n u-dma-buf --enable
```

Add `./project-spec/meta-user/recipes-modules/u-dma-buf/files/u-dma-buf.c` where this source file is found on
<https://github.com/ikwzm/udmabuf>
