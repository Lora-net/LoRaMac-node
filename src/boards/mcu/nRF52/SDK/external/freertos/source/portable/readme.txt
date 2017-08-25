Each real time kernel port consists of three files that contain the core kernel
components and are common to every port, and one or more files that are
specific to a particular microcontroller and/or compiler.


+ The FreeRTOS/Source/Portable/MemMang directory contains the three sample
memory allocators as described on the http://www.FreeRTOS.org WEB site.

+ The other directories each contain files specific to a particular
microcontroller or compiler.



For example, if you are interested in the GCC port for the ATMega323
microcontroller then the port specific files are contained in
FreeRTOS/Source/Portable/GCC/ATMega323 directory.  If this is the only
port you are interested in then all the other directories can be
ignored.

