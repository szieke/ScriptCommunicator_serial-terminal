# ESP32 Console

This script is based on ANSI Color Console but also adds an option to decode ESP32 backtrace if system crashes. 

Without it you would have to use `idf.py monitor` or other tools separatelly to decode the backtrace. That means you have to disconnect, set your idf environment, find correct port, start monitor and at the end find out what was that key combination to stop the monitor. 

> Btw. it is `CTRL+]` which can even translate to `CTRL+ALTgr+G` on CZ keyboard for example - try that with one hand).

Instead, when backtrace is detected, we can simply call `xtensa-esp32-elf-addr2line -pfiaC -e build/PROJECT.elf ADDRESS` command directly from this script and see the decoded messages in console without having to switch to other tools. 

## Requirements

### No backtrace (default)

Of course you can use this as serial terminal with no backtrace decoding in which case this script will take care of removing ANSI color sequences and colorize your esp console output accordingly.


### Autodetect tools (recommended)

If you have corectly set up and working ESP-IDF Cmake build environment the only requirement would be to select project **ELF file that is running on your ESP MCU**. You should find this file in `/build` subfolder of your project. With default config it should be the only `.elf` file in build folder. Autodetect then finds correct xtensa tools that will be used for backtrace decoding by parsing contents of `CmakeCache.txt` from the same folder.

#### Requires:
- Completely set-up and working ESP-IDF Cmake build environment
- Project output `.elf` file and `CmakeCache.txt` in same folder
- Project successfully built, same build version running on ESP MCU


### Manual tools selection (advanced)

If you, for example, run the console on different system without build tools or your IDF project is using **legacy make**, you can still decode the backtraces. In this case you have to find correct decoding tools manually, which may be a bit complicated as there are multiple versions of the tools, each for specific target and ESP-IDF version. 

#### Requires:
- Project output `.elf` file
- `elf-addr2line` executable to decode backtraces (*xtensa-esp32-elf-addr2line.exe* for example)
- Optional: `elf-readelf` executable to decode firmware information (*xtensa-esp32-elf-readelf.exe* for example) 
- Project successfully built, same build version running on ESP MCU


----

## Notes

- Tested on Ubuntu 20 with autodetect and manual, IDF v4.4.1
- Tested on Windows with manual tools selection, IDF tools v4.4.4
- Backtrace decoding of of project build with *legacy make* not tested

#### TODO:

- Add tooltips and/or link to this readme

