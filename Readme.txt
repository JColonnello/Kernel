This is a simple kernel built upon x64BareBones

x64BareBones is a basic setup to develop operating systems for the Intel 64 bits architecture.
The final goal of the project is to provide an entry point for a kernel and the possibility to load extra binary modules separated from the main kernel.

Environment setup:
1- Install the following packages before building the Toolchain and Kernel:

nasm qemu gcc make

2- Build the disk Image

From the main project directory run:

  user@linux:$ make all

3- Run the kernel

From the main project directory run:

  user@linux:$ ./run.sh

Author: Joaquín Colonnello

x64BareBones:
Author: Rodrigo Rearden (RowDaBoat)
Collaborator: Augusto Nizzo McIntosh