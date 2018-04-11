# AVRClock [![Build Status](https://travis-ci.org/LuTri/AVRClock.svg?branch=master)](https://travis-ci.org/LuTri/AVRClock)

This project aims to provide a simple, yet comfortable API, for those trying to implement any time(r)-based applications on the Atmel ATmegaX microcontrollers.

This project was implemented using the GPLv3.

## Documentation

The documentation for the APIs is available [here](http://lutri.github.io/AVRClock/).

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

There are 5 attributes, which have direct impact on compiling this software. All of them have their default values defined in the Makefile, but may be overwritten by passing parameters to make (or by exporting them from a parent's Makefile):

 * `MCU` (default: **atmega328p**) -> The type of microcontroller.
 * `F_OSC` (default: **16MHz**) -> frequency of the clock's source.
 * `TIMERNR` (default: **1**) -> The number of the timer. Usually, it's one of [1,2,3,4]. Refer to your ATmega's datasheet to choose the timer suitable for your needs.
 * `TIMERBITS` (default: **16**) -> Internal bitsize of your chosen timer, can be 8bits or 16bits. Refer to your ATmega's datasheet to determine to correct bitsize for your chosen timer.
 * `PRESCALER` (default: **1024**) -> Determines the fraction of the clocks speed, at which the timer should run (and therefore directly controls the accuracy and the amount of time your countdown can handle).

The above configuration results in a maximum of ~ **274869.53 seconds**, which a countdown can run, with an accuracy of +- **64 micro seconds**.

**Compile the object for using in another project**

```
$ make
```

Invoking `make` with no parameters causes a build with the above mentioned default values.

To overwrite those values, simply pass them to `make`:

```
$ make MCU=atmega8 TIMERNR=0 TIMERBITS=8 PRESCALER=256 F_OSC=1000000
```

After successfully compiling the source, a `customtimer.o`-file is created.

### Running unit tests

This project has some basic unit tests provided. Those tests are executed by
compiling the source to a native C-object and mocking some of the ATmegaX internals.

Execute the tests by running

```
$ make test
```

`make test` also accepts overwriting the parameters.

### Compiling this project as a submodule, from the parent-repository's Makefile

`make` makes it fairly straightforward to include and compile projects, which are included as e.g. git submodules.

To achieve this, add the following 2 lines to your parent-repository's Makefile:
```
CLOCK_SUBMODULE = AVRClock
CLOCK_OBJECT = AVRClock/customtimer.o
```

If you want your parent's Makefile to export parameters to AVRClock, simply mark them as exported:

```
MCU=atmega16p
export MCU
```

Add an additional target to the Makefile:

```
submodules:
	$(MAKE) -C $(CLOCK_SUBMODULE)
```

Call the target `submodules` wherever it's needed.

### Format the code

```
$ clang-format -i -style=file *.c *.h
```

### Generate the documentation

```
$ doxygen
```

This command will generate a folder `docs`, which holds both a .html and a .tex version.
(The html version is also published to [github pages](http://lutri.github.io/AVRClock/), always build from the latest master branch.)

### License information

This file is part of the AVRClock distribution
(https://github.com/LuTri/AVRClock).
Copyright (c) 2016 Tristan Lucas.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
