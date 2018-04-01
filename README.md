# AVRClock

This project aims to provide a simple, yet comfortable API, for those trying to implement any time(r)-based applications on the Atmel ATmegaX microcontrollers.

This project was implemented using the GPLv3.

## Contribution / Warranty

Currently, this project is under strong development, without any quarantee at all - feel free to contribute any knowledge / ideas / bugfixes you want to.

## Requirements

Currently, this code is solely compiled under linux, which yields the following dependencies:
 * [avr-gcc](https://gcc.gnu.org/wiki/avr-gcc) (installing this should also install the avr standard libraries for c)
 * [make](https://www.gnu.org/software/make/) (unless you want to configure and compile everything yourself)
 * [doxygen](http://www.stack.nl/~dimitri/doxygen/) for generating the documentation

### optional

 * [clang-format](https://clang.llvm.org/docs/ClangFormat.html) for formatting the source code with the in .clang-format configured style


## How to

### Configuring the platform

There are 2 attributes in the `Makefile`, which have direct impact on compiling this software.

 * `MCU` -> The type of microcontroller, currently defaults to atmega328p
 * `F_OSC` -> frequency of the external quartz crystal, set to 16 MHz.

### Compile the object for using in another project

```
$ make
```

After successfully compiling the source, a `customtimer.o`-file is created.

### Compiling this project as a submodule, from the parent-repository's Makefile

`make` makes it fairly straightforward to include and compile projects, which are included as e.g. git submodules.

To achieve this, add the following 2 lines to your parent-repository's Makefile:
```
CLOCK_SUBMODULE = AVRClock
CLOCK_OBJECT = AVRClock/customtimer.o
```

Add an additional target to the Makefile:

```
submodules:
	$(MAKE) -C $(CLOCK_SUBMODULE)
```

and add this target to your main building-process:

```
build: subsystems elf hex ...
```

### Format the code

```
$ clang-format -i -style=file *.c *.h
```

### Generate the documentation

```
$ doxygen
```

This command will generate a folder `documentation`, which holds both a .html and a .tex version.
