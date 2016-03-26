# ScriptCommunicator_serial-terminal

ScriptCommunicator is a scriptable cross-platform data terminal which supports:
* serial port (RS232, USB to serial)
* UDP
* TCP client/server (network proxy support for TCP clients)
* SPI master (cheetah SPI)
* CAN (PCAN-USB, only on Windows)

All sent and received data can be shown in a console and can be logged in an html, a text and a custom log.
In addition to the simple sending and receiving of data ScriptCommunicator has a QtScript (similar to JavaScript) interface.
This script interface has following features:
* Scripts can send and receive data with the main interface.
* In addition to the main interface scripts can create and use own interfaces (serial port (RS232, USB to serial), UDP, TCP client, TCP server, PCAN and SPI master).
* Scripts can use their own GUI (GUI files which have been created with QtDesigner (is included) or QtCreator).
* Multiple plot windows can be created by scripts (QCustomPlot from Emanuel Eichhammer is used)

**main window**
![main window](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/sdssdrhhhh5.png)

**settings dialog**
![settings dialog](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2015-12-02_10h19_22.png)

**plotting via script**

![plotting via script](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/2015-12-02_09h50_12.png)

# Downloads (prebuild versions)
- [Windows](http://sourceforge.net/projects/scriptcommunicator/files/Windows/ScriptCommunicatorSetup_04_08_windows.zip/download)
- [Linux 32 bit](http://sourceforge.net/projects/scriptcommunicator/files/Linux_32Bit/ScriptCommunicator_04_08_linux_32_bit.zip/download)
- [Linux 64 bit](http://sourceforge.net/projects/scriptcommunicator/files/Linux_64Bit/ScriptCommunicator_04_08_linux_64_bit.zip/download)
- [Mac OS X](http://sourceforge.net/projects/scriptcommunicator/files/Mac%20OS%20X/ScriptCommunicator_04_08_mac.zip/download)

