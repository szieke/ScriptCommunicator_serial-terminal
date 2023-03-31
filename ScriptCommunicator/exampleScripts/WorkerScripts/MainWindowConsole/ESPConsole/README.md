# ESP Console

This script is based on ANSI Color Console but also adds an option to decode ESP32 backtrace if system crashes. 

Without it you would have to use `idf.py monitor` or other tools separately to decode the backtrace. That means you have to disconnect, set your idf environment, find correct port, start monitor and at the end find out what was that key combination to stop the monitor. 

> Btw. it is `CTRL+]` which can even translate to `CTRL+ALTgr+G` on CZ keyboard for example - try that with one hand.

Instead, when backtrace is detected, we can simply call, for example, `xtensa-esp32-elf-addr2line -pfiaC -e build/PROJECT.elf ADDRESS` command directly from this script and see the decoded messages in console without having to switch to other tools. 

## Usage

### Option 1: No backtrace (default)

Of course you can use this as serial terminal with no backtrace decoding in which case this script will take care of removing ANSI color sequences (for example those `[0;32m` at the beginning of the lines) and colorize your ESP log output accordingly.

### Option 2: Autodetect tools (recommended)

If you have correctly set up and working ESP-IDF Cmake build environment the only requirement would be to select project **ELF file that is running on your ESP MCU**. You should find this file in `/build` subfolder of your IDF project. 
With default config it should be the only `.elf` file in build folder. * **Autodetect Tools from ELF** * then finds correct xtensa tools that will be used for backtrace decoding by parsing contents of `CmakeCache.txt` from the same folder.

#### Requires:
- Completely set-up and working ESP-IDF Cmake build environment
- Project output `.elf` file and `CmakeCache.txt` in same folder
- Project successfully built, same build version running on ESP MCU 

### Option 3: Manual tools selection (advanced)

If you, for example, run the console on different system without build tools or your IDF project is using **legacy make**, you can still decode the backtraces. In this case you can * **uncheck Autodetect Tools from ELF** * and then find correct decoding tools manually. That may be a bit complicated as there are multiple versions of the tools, each for specific target and ESP-IDF version. 

#### Requires:
- Project output `.elf` file
- `elf-addr2line` executable to decode backtraces (*xtensa-esp32-elf-addr2line.exe* for example)
- Optional: `elf-readelf` executable to decode firmware information (*xtensa-esp32-elf-readelf.exe* for example) 
- Project successfully built, same build version running on ESP MCU 

### Other functions 

- Main window's **Clear** button is not only used for clearing the console, but also resets status of *'Backtrace detected!'* info label and refreshes current project firmware info.
- You can also use functionality on *'Manual Address Decode'* tab in settings dialog to decode selected address/es manually. Works for single address or whole block - each address starting with 0x gets decoded.

----

## Notes

To issue bug or feature request for the ESP Console script your best bet would be to do it in script author's fork directly: [https://github.com/mmrein/ScriptCommunicator_serial-terminal/tree/ESPConsole](https://github.com/mmrein/ScriptCommunicator_serial-terminal/tree/ESPConsole).  
Alternatively you can try original ScriptCommunicator repo: [https://github.com/szieke/ScriptCommunicator_serial-terminal/tree/ESPConsole](https://github.com/szieke/ScriptCommunicator_serial-terminal/tree/ESPConsole).

- Tested on Ubuntu 20 with autodetect and manual, IDF v4.4.1, v4.4.4, v5.0.1
- Tested on Windows with manual tools selection only, IDF tools v4.4.4
- Backtrace decoding of project built with *legacy make* not tested

#### Notes on markdown formatters:

> Although `QTextEdit` class does support markdown formatting, `setMarkdown()` function is not implemented in ScriptCommunicator. Adding it should not be very difficult, but using external libraries may offer more variability and their output may even look better than current QT markdown formatter.  

> For basic MD-to-HTML conversion, results using [marked](https://github.com/markedjs/marked) library were visually not too different than using [snarkdown](https://github.com/bpmn-io/snarkdown) lib, but sizes differ significantly. While **marked** with its 50kB of compacted inline text is already quite small, it would still almost double the size of ESPConsole script. **Snarkdown** with its minimalist approach is 5kB while still in human-readable form, which fits much better.

### Changelog

#### ESP Console v1.1.1 (30.03.2023)

- Bugfix: only show the "ELF not found" dialog on start if decode was enabled on previous exit. Otherwise it may be disturbing if user just wants to use console without backtrace.
- Also save settings on script exit, not just after OK button click.

#### ESP Console v1.1 (26.03.2023)

- Firmware info can now be refreshed by clicking **Clear** button in main window.
- Added internal "Backtrace:" string search to signalize the backtrace was detected even if it could not be decoded.
- Added checkbox to Show invalid results too (decoded backtrace result is '`?? ??:0`' or '`??:?`')
- Added *Manual Address Decode* tab where user can write selected address(es) and decode them directly. 
- Added *Readme* tab which shows this file. Snarkdown JS source is used for markdown formatting. 
- Version info label added

#### ESP Console v1.0 (17.03.2023)

- Initial release

### TODO:

- Maybe some tooltips here and there

