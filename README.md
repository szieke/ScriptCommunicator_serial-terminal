# ScriptCommunicator
ScriptCommunicator is a scriptable cross-platform data terminal that supports serial port (RS232, USB to serial), UDP, TCP client/server, SPI, I2C, and CAN.
All sent and received data can be shown in a console and can be logged in an html and a text log.

In addition to the simple sending and receiving of data, ScriptCommunicator has a QtScript (similar to JavaScript) interface.
This script interface has the following features:
* Scripts can send and receive data with the main interface.
* In addition to the main interface scripts can create and use their own interfaces (serial port (RS232, USB to serial), UDP, TCP client, TCP server, PCAN, and SPI/I2C).
* Scripts can use their own GUI (GUI files which have been created with QtDesigner (is included) or QtCreator). 
* Multiple plot windows can be created by scripts (QCustomPlot  developed by Emanuel Eichhammer is used)

**Features**

* seriel port (RS232, USB to serial)
* TCP client/server (network proxy support for TCP clients)
* UDP
* SPI (Aardvark I2C/SPI)
* I2C (Aardvark I2C/SPI)
* CAN (PCAN-USB, only on windows)
* utf8, hexadecimal, decimal, binary and mixed console
* html and text log
* script interface (QtScript)->run automated test jobs, automatic device configuration scripts...
* use ui files (user interface created with QtDesigner (is included) or QtCreator) from script->building test GUIs, control GUIs...
* plotting data via script and export the generated graphs to file (png, jpg, bmp, pdf and csv)->e.g. to visualize a PID controller or a heater
* multi user and multi workspace support

**main window**

![main window](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2017-08-07_16h01_36.png/1)

**dark mode**

![dask mode](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2020-12-23_14h01_25.png/max/max/1)

**example script GUI**

![example script GUI](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2015-12-02_10h19_22.png)

# Homepage
[https://sourceforge.net/projects/scriptcommunicator/](https://sourceforge.net/projects/scriptcommunicator/)

# Downloads (release 05.19)
- [Windows](http://sourceforge.net/projects/scriptcommunicator/files/Windows/ScriptCommunicatorSetup_05_19_windows.exe/download)
- [Windows no installer](http://sourceforge.net/projects/scriptcommunicator/files/Windows/ScriptCommunicator_05_19_windows_no_installer.zip/download)
- [Linux 64 bit](http://sourceforge.net/projects/scriptcommunicator/files/Linux_64Bit/ScriptCommunicator_05_19_linux_64_bit.zip/download)
- [Source](http://sourceforge.net/projects/scriptcommunicator/files/Source/ScriptCommunicator_05_19_source.zip/download)
