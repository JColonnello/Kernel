This is a simple kernel built upon x64BareBones

x64BareBones is a basic setup to develop operating systems for the Intel 64 bits architecture.
The final goal of the project is to provide an entry point for a kernel and the possibility to load extra binary modules separated from the main kernel.

Environment setup:
1- Install the following packages before building the Toolchain and Kernel:

nasm qemu gcc make

2- Build the disk Image

From the main project directory run:

  user@linux:$ make all
  
To choose a memory manager between Free-list and bit buddy [default] use:

  user@linux:$ make all -DVIRT_MEM_LINKED
  or
  user@linux:$ make all -DVIRT_MEM_BUDDY
  
You can clean the build directories or rebuild everything using make clean and make rebuild

3- Run the kernel

From the main project directory run:

  user@linux:$ ./run.sh

Author: Joaquín Colonnello

x64BareBones:
Author: Rodrigo Rearden (RowDaBoat)
Collaborator: Augusto Nizzo McIntosh

Basic usage:

F1-F12: Change TTY
Ctrl-Alt-Supr: Store registers value
Alt-Space: Change keyboard layout (US / Latin American)
Esc: Erase whole line
Backspace: Erase characters
Ctrl-D: Send end-of-file (EOF)

"help" command list available programs
