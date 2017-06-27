/*************************************************************************
This ScriptCommunicator worker script routes:

- serial port (RS232, USB to serial)<->TCP (Client/Server)
- serial port (RS232, USB to serial)<->UDP
- TCP (Client/Server)<->UDP

All received data from the main interface is sent with the second interface 
(can be selected in the script gui) and vice versa.
***************************************************************************/
//Is called if the second interface has been disconnected.
function secondInterfaceDisconnectedSlot()
{	
	isConnectedSecondInterface = false;
	appendDataToConsole('<br>second interface is disconnected');
	
    //Disable the main interface routing function.
	udpSocket.disableMainInterfaceRouting();
	serialPort.disableMainInterfaceRouting();
	
	serialPort.close();
	tcpServer.close();
	udpSocket.close();
	
	if (typeof (tcpServerClient)  != 'undefined') 
	{
		tcpServerClient.disconnectedSignal.disconnect(secondInterfaceDisconnectedSlot);
		
		//Disable the main interface routing function.
		tcpServerClient.disableMainInterfaceRouting();
		tcpServerClient.close();
		
		//Delete the TCP server client.
		tcpServerClient  = undefined;
	}
	
	if (typeof(tcpClient)  != 'undefined') 
	{
		lastTcpClientErrorString = tcpClient.getErrorString();
		tcpClient.disconnectedSignal.disconnect(secondInterfaceDisconnectedSlot);
		tcpClient.errorSignal.disconnect(tcpClientErrorSlot);
		
		//Disable the main interface routing function.
		tcpClient.disableMainInterfaceRouting();
	
		tcpClient.close();
		
		//Delete the TCP client.
		tcpClient  = undefined;
	}
	
		
	UI_disconnectButton.setEnabled(false);
	UI_connectButton.setEnabled(true);
	textFromGuiElementChanged("");
	UI_connectionTypeComboBox.setEnabled(true);
	
	if(restartTcpServer && ("TCP_SERVER" == connectionType))
	{
		UI_connectButtonPressed()
	}
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
								 lastTcpClientErrorString);
	}
    
}

//Is called if the second interface has been connected.
function secondInterfaceConnectedSlot()
{
	isConnectedSecondInterface = true;
	appendDataToConsole('<br>second interface is connected');
	
	if("SERIAL_PORT" == connectionType)
	{	//Print the serial port signals (eg. DTR).
		appendDataToConsole('<br>serial port signals: ' + serialPort.getSerialPortSignals().toString(16));
		
		//Enable the main interface routing function (all data received by the main interface 
		//is sent with this interface and all data received by this interface is sent with the 
		//main interface).
		serialPort.enableMainInterfaceRouting();
	}
	
	if("TCP_SERVER" == connectionType)
	{
		tcpServerClient  = tcpServer.nextPendingConnection();
		tcpServer.close();
		
		tcpServerClient .disconnectedSignal.connect(secondInterfaceDisconnectedSlot);
		
		//Enable the main interface routing function (all data received by the main interface 
		//is sent with this interface and all data received by this interface is sent with the 
		//main interface).
		tcpServerClient.enableMainInterfaceRouting();
		UI_disconnectButton.setText("disconnect");
	}
	
	if("TCP_CLIENT" == connectionType)
	{
		//Enable the main interface routing function (all data received by the main interface 
		//is sent with this interface and all data received by this interface is sent with the 
		//main interface).
		tcpClient.enableMainInterfaceRouting();
	}
}

//Is called if the user has pressed the disconnect button.
function UI_disconnectButtonPressed()
{
	restartTcpServer = false;
	secondInterfaceDisconnectedSlot();
	restartTcpServer = true;
}

//Is called if the user has pressed the connect button.
function UI_connectButtonPressed()
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
				UI_socketDestinationPort.setEnabled(false);
				UI_socketDestinationAddress.setEnabled(false);
				UI_socketOwnPort.setEnabled(false);
				
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
	
		UI_socketsTypeComboBox.setEnabled(false);
		UI_socketDestinationPort.setEnabled(false);
		UI_socketDestinationAddress.setEnabled(false);
		UI_socketOwnPort.setEnabled(false);
		UI_noProxyRadioButton.setEnabled(false);
		UI_useSystemProxyRadioButton.setEnabled(false);
		UI_useSpecificProxyRadioButton.setEnabled(false);
		UI_proxyAddressLineEdit.setEnabled(false);
		UI_proxyPortLineEdit.setEnabled(false);
		UI_proxyUserNameLineEdit.setEnabled(false);
		UI_proxyPasswordLineEdit.setEnabled(false);
		
		if(UI_socketsTypeComboBox.currentText() == "TCP client")
		{
			connectionType = "TCP_CLIENT";
			isConnectedSecondInterface = false;
	
			tcpClient = scriptThread.createTcpClient();
			tcpClient.disconnectedSignal.connect(secondInterfaceDisconnectedSlot);
			tcpClient.connectedSignal.connect(secondInterfaceConnectedSlot);
			tcpClient.errorSignal.connect(tcpClientErrorSlot);
			
			if(UI_useSystemProxyRadioButton.isChecked())
			{
				tcpClient.setProxy("SYSTEM_PROXY", UI_proxyUserNameLineEdit.text(), UI_proxyPasswordLineEdit.text());
			}
			else if(UI_useSpecificProxyRadioButton.isChecked())
			{
				tcpClient.setProxy("CUSTOM_PROXY", UI_proxyUserNameLineEdit.text(), UI_proxyPasswordLineEdit.text(),
				UI_proxyAddressLineEdit.text(), UI_proxyPortLineEdit.text());
			}
			else
			{
				tcpClient.setProxy("NO_PROXY");
			}
			appendDataToConsole("<br>connecting to host");
			UI_disconnectButton.setEnabled(true);
			UI_connectButton.setEnabled(false);
			
			tcpClient.connectToHost(UI_socketDestinationAddress.text() , UI_socketDestinationPort.text());

		}
		else if(UI_socketsTypeComboBox.currentText() == "TCP server")
		{
			connectionType = "TCP_SERVER";
			isConnectedSecondInterface = false;
			UI_disconnectButton.setText("stop waiting");
			
			if(tcpServer.listen(UI_socketOwnPort.text()))
			{
				UI_disconnectButton.setEnabled(true);
				UI_connectButton.setEnabled(false);
				appendDataToConsole("<br>waiting for tcp client");
			}
			else
			{
				appendDataToConsole("<br>error while starting tcp server");
			}
		}
		else if(UI_socketsTypeComboBox.currentText() == "UDP socket")
		{
			connectionType = "UDP_SOCKET";
			isConnectedSecondInterface = true;
			udpSocket.bind(UI_socketOwnPort.text());
			
			//Enable the main interface routing function (all data received by the main interface 
			//is sent with this interface and all data received by this interface is sent with the 
			//main interface).
			udpSocket.enableMainInterfaceRouting(UI_socketDestinationAddress.text() , UI_socketDestinationPort.text());
			appendDataToConsole("<br>udp socket: waiting for data");
			
			UI_disconnectButton.setEnabled(true);
			UI_connectButton.setEnabled(false);
		}
		else
		{
			appendDataToConsole("<br>invalid connection type in UI_connectButtonPressed");
			connectionType = "INVALID";
			isConnectedSecondInterface = false;
		}
	}
}


try
{   //Check if the user interface has been loaded (if not this statement
    //will generate an exception).
	if(UI_SettingsDialog) 
	{
	}
}
catch(e)
{
	//Load the user interface.
	scriptThread.loadUserInterfaceFile("routeSerialTcpUdp.ui");
}

scriptThread.appendTextToConsole('script started');

//Load the script with the helper functions.
scriptThread.loadScript("helper.js");

//Check the version of ScriptCommunicator.
checkVersion();

//Add the available serial ports.
var availablePorts = scriptThread.availableSerialPorts();
for(var i = 0; i < availablePorts.length; i++)
{
	UI_serialPortInfoListBox.addItem(availablePorts[i]);
}

//Load the GUI settings.
loadUiSettings();

var restartTcpServer = true;
var serialPort = scriptThread.createSerialPort();
var tcpClient = undefined
var lastTcpClientErrorString = "";
var tcpServer = scriptThread.createTcpServer()
var tcpServerClient  = undefined;
tcpServer.newConnectionSignal.connect(secondInterfaceConnectedSlot);

var udpSocket = scriptThread.createUdpSocket()

var isConnectedSecondInterface = false;
var connectionType = "INVALID";

UI_SettingsDialog.finishedSignal.connect(UI_DialogFinished);
UI_disconnectButton.clickedSignal.connect(UI_disconnectButtonPressed)
UI_connectButton.clickedSignal.connect(UI_connectButtonPressed)

UI_connectionTypeComboBox.currentTextChangedSignal.connect(textFromGuiElementChanged)
UI_socketsTypeComboBox.currentTextChangedSignal.connect(textFromGuiElementChanged)

UI_noProxyRadioButton.clickedSignal.connect(checkBoxClicked);
UI_useSystemProxyRadioButton.clickedSignal.connect(checkBoxClicked);
UI_useSpecificProxyRadioButton.clickedSignal.connect(checkBoxClicked);

UI_clearConsoleButton.clickedSignal.connect(UI_clearConsoleButtonClicked)


