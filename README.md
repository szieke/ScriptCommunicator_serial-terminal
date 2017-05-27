# ScriptCommunicator

ScriptCommunicator is a scriptable cross-platform data terminal which supports several interfaces.
All sent and received data can be shown in a console and can be logged in an html, a text and a custom log.
In addition to the simple sending and receiving of data ScriptCommunicator has a QtScript (similar to JavaScript) interface.
This script interface has following features:
* Scripts can send and receive data with the main interface.
* In addition to the main interface scripts can create and use own interfaces (serial port (RS232, USB to serial), UDP, TCP client, TCP server, PCAN and SPI master).
* Scripts can use their own GUI (GUI files which have been created with QtDesigner (is included) or QtCreator).
* Multiple plot windows can be created by scripts (QCustomPlot from Emanuel Eichhammer is used).

**Features**

* seriel port (RS232, USB to serial)
* TCP client/server
* UDP
* SPI master (cheetah SPI)
* CAN (PCAN-USB, only on windows)
* ascii, hexadecimal, decimal, binary, custom and mixed console (adjustable colors)
* html, text and custom log
* script interface (QtScript)->run automated test jobs, automatic device configuration scripts...
* use ui files (user interface created with QtDesigner (is included) or QtCreator) from script->building test GUIs, control GUIs...
* plotting data via script and export the generated graphs to file (png, jpg, bmp, pdf and csv)->e.g. to visualize a PID controller or a heater
* multi user and multi workspace support

**main window**

![main window](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2016-04-26_07h31_37.png)

**settings dialog**

![settings dialog](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2016-03-31_07h54_07.png)

**example script GUI**

![example script GUI](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2015-12-02_10h19_22.png)

# Homepage
[https://sourceforge.net/projects/scriptcommunicator/](https://sourceforge.net/projects/scriptcommunicator/)

# Downloads (release 04.22)
- [Windows](http://sourceforge.net/projects/scriptcommunicator/files/Windows/ScriptCommunicatorSetup_04_22_windows.zip/download)
- [Linux 64 bit](http://sourceforge.net/projects/scriptcommunicator/files/Linux_64Bit/ScriptCommunicator_04_22_linux_64_bit.zip/download)
- [Mac OS X](http://sourceforge.net/projects/scriptcommunicator/files/Mac%20OS%20X/ScriptCommunicator_04_22_mac.zip/download)
- [Source](http://sourceforge.net/projects/scriptcommunicator/files/Source/ScriptCommunicator_04_22_source.zip/download)
