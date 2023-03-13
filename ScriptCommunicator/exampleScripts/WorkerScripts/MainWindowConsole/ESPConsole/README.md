# ESP32 Console

This script is based on ANSI Color Console but also adds an option to decode ESP32 backtrace if system crashes. 

Without it you would have to use `idf.py monitor` or other tools separatelly to decode the backtrace. That means you have to disconnect, set your idf environment, find correct port, start monitor and at the end find out what was that key combination to stop the monitor (it is `CTRL+]` or even `CTRL+ALTgr+G` on CZ keyboard for example...).

Instead, when backtrace is detected, we can simply call `xtensa-esp32-elf-addr2line -pfiaC -e build/PROJECT.elf ADDRESS` command directly from this script and see the decoded messages in console without having to switch to other tools. 

## Requirements

### No backtrace (default)

Of course you can use this as serial terminal with no backtrace decoding in which case this script will take care of removing ANSI color sequences and colorize your esp console output accordingly.


### Autodetect tools (recommended)

- Completely set-up and working ESP-IDF Cmake build environment
- Project output `.elf` file and `CmakeCache.txt` in same folder
- Project successfully built, same build version running on ESP MCU

If you have corectly set up and working ESP-IDF Cmake build environment the only requirement would be to select project **ELF file that is running on your ESP MCU**. You should find this file in `/build` subfolder of your project. With default config it should be the only `.elf` file in build folder. Autodetect then should be able to find correct xtensa tools that should be used for backtrace decoding. `CmakeCache.txt` file from build folder is necessary for that.


### Manual tools selection (advanced users)

- Project output `.elf` file
- `elf-addr2line` executable to decode backtraces (required, `xtensa-esp32-elf-addr2line.exe` for example)
- Optional: `elf-readelf` executable to decode backtraces (required, `xtensa-esp32-elf-readelf.exe` for example) 
- Project successfully built, same build version running on ESP MCU

If you for example run the console on different system without build tools, you can still decode the backtraces. In this case you have to find correct decoding tools manually, which may be a bit problematic as there are multiple versions of the tools, each for specific target and ESP-IDF version. 

----

## Notes

- Tested on Ubuntu 20 with autodetect, IDF v4.4.1
- Tested on Windows with manual tools selection, IDF tools v4.4.4

#### TODO:

- **Handling of incomplete addresses (store to buffer for next round of data)**
- Reload FW info after manual tool was selected
- Show warning if CmakeCache.txt was not found while autodetect was on
- Add tooltips and/or link to this readme
- Some more testing and debug
