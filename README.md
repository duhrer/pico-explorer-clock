# Pico Explorer Clock

This project displays a clock on a
[Pico Explorer Base](https://shop.pimoroni.com/products/pico-explorer-base). I used this project to better understand:

1. Using [the pimoroni graphics library](https://github.com/pimoroni/pimoroni-pico/tree/main/libraries/pico_graphics),
including drawing lines and text.
2. Using timing functions like `add_repeating_timer_ms`.
3. Using a more complex project structure and CMake setup.

## Prerequisites

First, you need the right hardware, i.e. a Raspberry Pi Pico and a Pico Explorer Base. It should not matter whether you
have a Pico that supports wireless or not.

You also need a working development environment. First, you need to set up the basics, i.e. the tools to compile code
and the libraries you need. I followed the steps in the
[setup guide for the Pico](https://github.com/pimoroni/pimoroni-pico/blob/main/setting-up-the-pico-sdk.md).

If you want to work with VS Code, you'll also need to go through something like
[this guide from Digikey](https://www.digikey.be/en/maker/projects/raspberry-pi-pico-and-rp2040-cc-part-2-debugging-with-vs-code/470abc7efb07432b82c95f6f67f184c0),
which helpfully covers the non-obvious bits required to get openocd working.

If you're interested, the [first part in the DigiKey series](https://www.digikey.be/en/maker/projects/raspberry-pi-pico-and-rp2040-cc-part-1-blink-and-vs-code/7102fb8bca95452e9df6150f39ae8422)
provides a nice alternative to the Pico guide.

## Building and Installing

### VS Code and a PiProbe

If you have a PiProbe, you should be able to use the debugger configuration in this project to build, install (and
debug) the code here.

First, you should check the paths in `.vscode/launch.json`and `.vscode/settings.json` and update them as
needed to match your system. Then, you should be able to just hit the debugger icon and choose the configuration
defined in `.vscode/launch.json`.

The application will be built, deployed, and will pause execution at the beginning of the `main()` function.

### Command Line Tools

To build the application from the command line, you can use commands like the following, starting at the root the
repository:

```
mkdir build
cd build
cmake ..
make -j4
```

The last command assumes you have four cores, adjust as needed. Once the build completes, there are two ways to install
the application.

If you don't have a PiProbe, reboot your Pico while holding the "Bootsel" button, then drag the generated UF2 file
onto the USB drive that appears.

If you have a PiProbe, you can install the program without resetting your Pico using a command like:

```
sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program pico-explorer-clock.elf verify reset exit"
```

## Usage

Once you've build and installed the binary, the display on the Pico Explorer Base will show an analog clock with a
digital clock overlaid at the bottom. In the tradition of generations of dumb devices, the time starts at 12:00:00
when the device is powered up.

The left buttons (a and b) set the hour. The right buttons (x and y) set the minute. Hold both right buttons to set
the seconds to zero.  Hold both left buttons to switch the display between 24 hour and 12 hour time.

As I don't have an
[RTC breakout board](https://shop.pimoroni.com/products/rv3028-real-time-clock-rtc-breakout), the time is kept up to
date by manually incrementing the second every 1000 milliseconds. On my setup, this seems to run slow by about five
seconds an hour.
