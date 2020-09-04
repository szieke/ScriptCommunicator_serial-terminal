/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the use of the following classes:
- UDP socket
- TCP client
- TCP server
- serial Port

All received data from the main interface is sent with the second interface (can be selcted in the gui)
and vice versa.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
	saveUiSettings()
    scriptThread.appendTextToConsole("script sniff_convert stopped");
}

//Is called if the dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

function clearConsoleButtonClicked()
{
	UI_scriptConsole.clear();
}

function updateConsole()
{
	if(consoleBuffer != "")
	{
		consoleTimer.stop();
		var pos = UI_scriptConsole.verticalScrollBarValue();
		UI_scriptConsole.moveTextPositionToEnd();
		UI_scriptConsole.append(consoleBuffer);
		UI_scriptConsole.moveTextPositionToEnd();
		
		if(UI_scrollLockCheckBox.isChecked())
		{
			UI_scriptConsole.verticalScrollBarSetValue(pos);
		}
		consoleBuffer = "";
		consoleTimer.start();
	}
}

function appendToConsoleBuffer(text)
{
	consoleBuffer += text;
}

//Data has been received with the main interface.
function dataReceivedSlot(data)
{
	if(UI_showReceivedCheckBox.isChecked())
	{
		var consoleString = ""
		if(UI_showAsAsciiCheckBox.isChecked())
		{
			consoleString = conv.byteArrayToString(data);
		}
		else
		{
			consoleString = data.join("");
		}
		
		appendToConsoleBuffer("<span style=\"color:#00ff00;\">" + UI_scriptConsole.replaceNonHtmlChars(consoleString) + "</span>");
	}
	
	if(isConnectedSecondInterface)
	{
		if("SERIAL_PORT" == connectionType)
		{
			serialPort.write(data);
		}
		else if("TCP_CLIENT" == connectionType)
		{
			tcpClient.write(data);
		}
		else if("TCP_SERVER" == connectionType)
		{
			tcpServerClient .write(data);
		}
		else if("UDP_SOCKET" == connectionType)
		{
			udpSocket.write(data, UI_socketAdressLineEdit.text() , UI_socketPortLineEdit.text());
		}
		else
		{
			appendToConsoleBuffer("<br>invalid connection type in dataReceivedSlot<br>");
		}
	}
	else
	{
		appendToConsoleBuffer("<br>second Interface is not connected<br>");
	}
}

//Connection closed.
function secondInterfaceDisconnectedSlot()
{
	isConnectedSecondInterface = false;
	appendToConsoleBuffer('<br>sniff_convert: second interface is disconnected<br>');
	
	serialPort.close();
	tcpClient.close();
	tcpServer.close();
	udpSocket.close();
	
	if (typeof tcpServerClient  != 'undefined') 
	{
		tcpServerClient.close();
		//Delete the TCP server client.
		tcpServerClient  = undefined;
	}
		
	UI_disconnectButton.setEnabled(false);
	UI_connectButton.setEnabled(true);
	textFromGuiElementChanged("");
	UI_connectionTypeComboBox.setEnabled(true);
}

//An error has been ocurred (TCP client).
function tcpClientErrorSlot(error)
{
	if(error != 1)
	{//The error is not QAbstractSocket::RemoteHostClosedError.
		secondInterfaceDisconnectedSlot();
	}

	if(error == 0) //QAbstractSocket::ConnectionRefusedError
	{
		scriptThread.messageBox("Critical", "TCP error",
								 "The connection was refused. " +
									"Make sure the server is running, " +
									"and check that the host name and port " +
									"settings are correct.");
	}
	else if(error == 1) //QAbstractSocket::RemoteHostClosedError
	{
		//The connection has been closed. Do nothing.
	}
	else if(error == 2) //QAbstractSocket::HostNotFoundError
	{
		scriptThread.messageBox("Critical", "TCP error",
								 "The server was not found. Please check the " +
									"host name and port settings.");
	}
	else
	{
		scriptThread.messageBox("Critical", "TCP error",
								 "The following error occurred: " +
								 tcpClient.getErrorString());
	}
    
}

//Connection established.
function secondInterfaceConnectedSlot()
{
	isConnectedSecondInterface = true;
	appendToConsoleBuffer('<br>sniff_convert: second interface is connected<br>');
	
	if("SERIAL_PORT" == connectionType)
	{
		appendToConsoleBuffer('<br>serial port signals: ' + serialPort.getSerialPortSignals().toString(16) + '<br>');
	}
	
	if("TCP_SERVER" == connectionType)
	{
		tcpServerClient  = tcpServer.nextPendingConnection();
		tcpServer.close();
		
		tcpServerClient .disconnectedSignal.connect(secondInterfaceDisconnectedSlot);
		tcpServerClient .readyReadSignal.connect(readyReadSlot);
	}
}

//Data has been received with the second interface.
function readyReadSlot()
{
	var data = Array();
	
	if("SERIAL_PORT" == connectionType)
	{
		data = serialPort.readAll();
	}
	else if("TCP_CLIENT" == connectionType)
	{
		data = tcpClient.readAll();
	}
	else if("TCP_SERVER" == connectionType)
	{
		data = tcpServerClient .readAll();
	}
	else if("UDP_SOCKET" == connectionType)
	{
		data = udpSocket.readAll();
	}
	else
	{
		appendToConsoleBuffer("<br>invalid connection type in readyReadSlot<br>");
	}
	
	if(UI_showReceivedCheckBox.isChecked())
	{
		var consoleString = "";
		if(UI_showAsAsciiCheckBox.isChecked())
		{
			consoleString = conv.byteArrayToString(data);
		}
		else
		{
			consoleString = data.join("");
		}
		appendToConsoleBuffer("<span style=\"color:#ff0000;\">" + UI_scriptConsole.replaceNonHtmlChars(consoleString) + "</span>");
	}
	
	if(scriptInf.isConnected())
	{
		scriptInf.sendDataArray(data);
	}
	else
	{
		appendToConsoleBuffer("<br>main Interface is not connected<br>");
	}
}

//Save the GUI settings.
function saveUiSettings()
{
	var settings;
	
	settings = "UI_dataBitsBox=" + UI_dataBitsBox.currentText() + "\r\n";
	settings += "UI_parityBox=" + UI_parityBox.currentText() + "\r\n";
	settings += "UI_baudRateBox=" + UI_baudRateBox.currentText() + "\r\n";
	settings += "UI_stopBitsBox=" + UI_stopBitsBox.currentText() + "\r\n";
	settings += "UI_flowControlBox=" + UI_flowControlBox.currentText() + "\r\n";
	settings += "UI_serialPortInfoListBox=" + UI_serialPortInfoListBox.currentText() + "\r\n";


	settings += "UI_socketsTypeComboBox=" + UI_socketsTypeComboBox.currentText() + "\r\n";
	settings += "UI_socketAdressLineEdit=" + UI_socketAdressLineEdit.text() + "\r\n";
	settings += "UI_socketPortLineEdit=" + UI_socketPortLineEdit.text() + "\r\n";
	settings += "UI_socketUdpOwnPortLineEdit=" + UI_socketUdpOwnPortLineEdit.text() + "\r\n";
	
	settings += "UI_connectionTypeComboBox=" + UI_connectionTypeComboBox.currentText() + "\r\n";
	
	settings += "UI_showReceivedCheckBox=" + UI_showReceivedCheckBox.isChecked().toString() + "\r\n";
	settings += "UI_showAsAsciiCheckBox=" + UI_showAsAsciiCheckBox.isChecked().toString() + "\r\n";
	settings += "UI_scrollLockCheckBox=" + UI_scrollLockCheckBox.isChecked().toString() + "\r\n";
	
	scriptFile.writeFile("snif_convertUiSettings.txt", true, settings, true);
}

function getValueOfStringArray(stringArray, key)
{
	for (var i=0; i < stringArray.length; i++)
	{
		var subStringArray = stringArray[i].split("=");
		if(subStringArray[0] == key)
		{
			return subStringArray[1];
		}
	}
}

//Load the GUI settings.
function loadUiSettings()
{
	if(scriptFile.checkFileExists("snif_convertUiSettings.txt"))
	{
		var settings = scriptFile.readFile("snif_convertUiSettings.txt");
		var stringArray = settings.split("\r\n");
		
		UI_dataBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_dataBitsBox"));
		UI_parityBox.setCurrentText(getValueOfStringArray(stringArray, "UI_parityBox"));
		UI_baudRateBox.setCurrentText(getValueOfStringArray(stringArray, "UI_baudRateBox"));
		UI_stopBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_stopBitsBox"));
		UI_flowControlBox.setCurrentText(getValueOfStringArray(stringArray, "UI_flowControlBox"));
		UI_serialPortInfoListBox.setCurrentText(getValueOfStringArray(stringArray, "UI_serialPortInfoListBox"));
		
		UI_socketsTypeComboBox.setCurrentText(getValueOfStringArray(stringArray, "UI_socketsTypeComboBox"));
		UI_socketAdressLineEdit.setText(getValueOfStringArray(stringArray, "UI_socketAdressLineEdit"));
		UI_socketPortLineEdit.setText(getValueOfStringArray(stringArray, "UI_socketPortLineEdit"));
		UI_socketUdpOwnPortLineEdit.setText(getValueOfStringArray(stringArray, "UI_socketUdpOwnPortLineEdit"));
		
		UI_connectionTypeComboBox.setCurrentText(getValueOfStringArray(stringArray, "UI_connectionTypeComboBox"));
		
		if(getValueOfStringArray(stringArray, "UI_showReceivedCheckBox") == "true"){UI_showReceivedCheckBox.setChecked(true);}
		else{UI_showReceivedCheckBox.setChecked(false);}
		
		if(getValueOfStringArray(stringArray, "UI_showAsAsciiCheckBox") == "true"){UI_showAsAsciiCheckBox.setChecked(true);}
		else{UI_showAsAsciiCheckBox.setChecked(false);}
		
		if(getValueOfStringArray(stringArray, "UI_scrollLockCheckBox") == "true"){UI_scrollLockCheckBox.setChecked(true);}
		else{UI_scrollLockCheckBox.setChecked(false);}
		
		textFromGuiElementChanged("");
	}	
}

function connectButtonPressed()
{
	if(UI_connectionTypeComboBox.currentText() == "serial port")
	{
		connectionType = "SERIAL_PORT";
		
		serialPort.setDTR(true);
		serialPort.setPortName(UI_serialPortInfoListBox.currentText());
		if (serialPort.open())
		{
			if (serialPort.setBaudRate(Number(UI_baudRateBox.currentText()))
			 && serialPort.setDataBits(Number(UI_dataBitsBox.currentText()))
			 && serialPort.setParity(UI_parityBox.currentText())
			 && serialPort.setStopBits(UI_stopBitsBox.currentText())
			 && serialPort.setFlowControl(UI_flowControlBox.currentText()))
			 {
				serialPort.setRTS(true);
				
				isConnectedSecondInterface = true;
				secondInterfaceConnectedSlot()

				UI_disconnectButton.setEnabled(true);
				UI_connectButton.setEnabled(false);
				
				UI_baudRateBox.setEnabled(false);
				UI_dataBitsBox.setEnabled(false);
				UI_parityBox.setEnabled(false);
				UI_stopBitsBox.setEnabled(false);
				UI_flowControlBox.setEnabled(false);
				UI_serialPortInfoListBox.setEnabled(false);

				UI_socketsTypeComboBox.setEnabled(false);
				UI_socketAdressLineEdit.setEnabled(false);
				UI_socketPortLineEdit.setEnabled(false);
				UI_socketUdpOwnPortLineEdit.setEnabled(false);
				
				UI_connectionTypeComboBox.setEnabled(false);
			 }
		}
		else
		{
			serialPort.close();
			isConnectedSecondInterface = false;
			scriptThread.messageBox("Critical", 'serial port open error', 'could not open serial port: ' + UI_serialPortInfoListBox.currentText())
		}
	}
	else
	{//Socket
	
		if(UI_socketsTypeComboBox.currentText() == "Tcp client")
		{
			connectionType = "TCP_CLIENT";
			isConnectedSecondInterface = false;
			tcpClient.connectToHost(UI_socketAdressLineEdit.text() , UI_socketPortLineEdit.text());
			appendToConsoleBuffer("<br>connecting to host<br>");
			
			UI_disconnectButton.setEnabled(true);
			UI_connectButton.setEnabled(false);
		}
		else if(UI_socketsTypeComboBox.currentText() == "Tcp server")
		{
			connectionType = "TCP_SERVER";
			isConnectedSecondInterface = false;
			
			if(tcpServer.listen(UI_socketPortLineEdit.text()))
			{
				UI_disconnectButton.setEnabled(true);
				UI_connectButton.setEnabled(false);
				appendToConsoleBuffer("<br>waiting for tcp client<br>");
			}
			else
			{
				appendToConsoleBuffer("<br>error while starting tcp server<br>");
			}
		}
		else if(UI_socketsTypeComboBox.currentText() == "Udp socket")
		{
			connectionType = "UDP_SOCKET";
			isConnectedSecondInterface = true;
			if(udpSocket.bind(UI_socketUdpOwnPortLineEdit.text()))
			{
				appendToConsoleBuffer("<br>udp socket: waiting data<br>");
				
				UI_disconnectButton.setEnabled(true);
				UI_connectButton.setEnabled(false);
			}
			else
			{
	
				udpSocket.close();
				isConnectedSecondInterface = false;
				scriptThread.messageBox("Critical", 'UDP socket open error', 'could not open UDP socket')
			}
		}
		else
		{
			appendToConsoleBuffer("<br>invalid connection type in dataReceivedSlot<br>");
			connectionType = "INVALID";
			isConnectedSecondInterface = false;
		}
	}
}
function textFromGuiElementChanged(text)
{
	if(UI_socketsTypeComboBox.currentText() == "Tcp client")
	{
		UI_soketAddressLabel.setText("partner adress");
		UI_socketPortLabel.setText("partner port");
		UI_socketOwnUdpPort.setText(" ");

	}
	else if(UI_socketsTypeComboBox.currentText() == "Tcp server")
	{
		UI_soketAddressLabel.setText(" ");
		UI_socketPortLabel.setText("own port");
		UI_socketOwnUdpPort.setText(" ");
	}
	else
	{//Udp socket
		UI_soketAddressLabel.setText("partner adress");
		UI_socketPortLabel.setText("partner port");
		UI_socketOwnUdpPort.setText("own port");
	}
	
	if(UI_connectionTypeComboBox.currentText() == "socket")
	{
		UI_baudRateBox.setEnabled(false);
		UI_dataBitsBox.setEnabled(false);
		UI_parityBox.setEnabled(false);
		UI_stopBitsBox.setEnabled(false);
		UI_flowControlBox.setEnabled(false);
		UI_serialPortInfoListBox.setEnabled(false);

		UI_socketsTypeComboBox.setEnabled(true);
		UI_socketPortLineEdit.setEnabled(true);

		if(UI_socketsTypeComboBox.currentText() == "Udp socket")
		{
			UI_socketUdpOwnPortLineEdit.setEnabled(true);
		}
		else
		{
			UI_socketUdpOwnPortLineEdit.setEnabled(false);
		}

		if(UI_socketsTypeComboBox.currentText() == "Tcp server")
		{
			UI_socketAdressLineEdit.setEnabled(false);
		}
		else
		{
			UI_socketAdressLineEdit.setEnabled(true);
		}
	}
	else
	{
		UI_baudRateBox.setEnabled(true);
		UI_dataBitsBox.setEnabled(true);
		UI_parityBox.setEnabled(true);
		UI_stopBitsBox.setEnabled(true);
		UI_flowControlBox.setEnabled(true);
		UI_serialPortInfoListBox.setEnabled(true);

		UI_socketsTypeComboBox.setEnabled(false);
		UI_socketAdressLineEdit.setEnabled(false);
		UI_socketPortLineEdit.setEnabled(false);
		UI_socketUdpOwnPortLineEdit.setEnabled(false);
	}
}


scriptThread.appendTextToConsole('script sniff_convert started');

//Add the available serial ports.
var availablePorts = scriptInf.availableSerialPorts();
for(var i = 0; i < availablePorts.length; i++)
{
	UI_serialPortInfoListBox.addItem(availablePorts[i]);
}

var consoleBuffer = ""
var consoleTimer = scriptThread.createTimer()
consoleTimer.timeoutSignal.connect(updateConsole);
consoleTimer.start(200);

var serialPort = scriptInf.createSerialPort();
serialPort.readyReadSignal.connect(readyReadSlot);

var tcpClient = scriptInf.createTcpClient();
tcpClient.readyReadSignal.connect(readyReadSlot);
tcpClient.disconnectedSignal.connect(secondInterfaceDisconnectedSlot);
tcpClient.connectedSignal.connect(secondInterfaceConnectedSlot);
tcpClient.errorSignal.connect(tcpClientErrorSlot);

var tcpServer = scriptInf.createTcpServer()
var tcpServerClient  = undefined;
tcpServer.newConnectionSignal.connect(secondInterfaceConnectedSlot);

var udpSocket = scriptInf.createUdpSocket()
udpSocket.readyReadSignal.connect(readyReadSlot);

var isConnectedSecondInterface = false;
var connectionType = "INVALID";
scriptInf.dataReceivedSignal.connect(dataReceivedSlot);

UI_SettingsDialog.finishedSignal.connect(dialogFinished);
UI_disconnectButton.clickedSignal.connect(secondInterfaceDisconnectedSlot)
UI_connectButton.clickedSignal.connect(connectButtonPressed)

UI_connectionTypeComboBox.currentTextChangedSignal.connect(textFromGuiElementChanged)
UI_socketsTypeComboBox.currentTextChangedSignal.connect(textFromGuiElementChanged)

UI_clearConsoleButton.clickedSignal.connect(clearConsoleButtonClicked)

UI_socketPortLineEdit.addIntValidator(0, 65536);
UI_socketUdpOwnPortLineEdit.addIntValidator(0, 65536);


loadUiSettings();

