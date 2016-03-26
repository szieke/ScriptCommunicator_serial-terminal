# ScriptCommunicator_serial-terminal

ScriptCommunicator is a scriptable cross-platform data terminal which supports serial port (RS232, USB to serial), UDP, TCP client/server (network proxy support for TCP clients), SPI master (cheetah SPI) and CAN (PCAN-USB, only on Windows)

All sent and received data can be shown in a console and can be logged in an html, a text and a custom log.

In addition to the simple sending and receiving of data ScriptCommunicator has a QtScript (similar to JavaScript) interface.

This script interface has following features:
- Scripts can send and receive data with the main interface.
- In addition to the main interface scripts can create and use own interfaces (serial port (RS232, USB to serial), UDP, TCP client, TCP server, PCAN and SPI master).
- Scripts can use their own GUI (GUI files which have been created with QtDesigner (is included) or QtCreator).
- Multiple plot windows can be created by scripts (QCustomPlot from Emanuel Eichhammer is used)

![main window](https://a.fsdn.com/con/app/proj/scriptcommunicator/screenshots/sdssdrhhhh5.png)
