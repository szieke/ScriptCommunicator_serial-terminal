/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
Aardvark I2C/SPI varerface (used as main varerface).
***************************************************************************/

const AARDVARD_I2C_SPI_GPIO_COUNT = 6;

//Is called if this script shall be exited.
function stopScript() 
{
	saveUiSettings("aardvarkI2cSpi_mainInf_settings.txt");
    scriptThread.appendTextToConsole("script test Aardvark I2C/SPI (MainInterface) stopped");
	scriptInf.disconnect();
}

//The dialog is closed.
function dialogFinishedSlot(e)
{	
	scriptThread.stopScript()
}

function freeI2cBusSlot()
{
	scriptInf.i2cMasterFreeBus();
	UI_Console.append("free I2C bus");
}
function executeI2cSlot()
{
	
	var flags;
	var slaveAddress;
	var numberOfBytesToRead;
	var dataToSend = Array();
	
	var tmpText = UI_I2cAddress.text();
	for(var i = UI_I2cAddress.text().length; i < 3; i++)
	{
		tmpText = "0" + tmpText;
	}
	UI_I2cAddress.setText(tmpText);
	
	var tmpText = UI_I2cFlags.text();
	for(var i = UI_I2cFlags.text().length; i < 2; i++)
	{
		tmpText = "0" + tmpText;
	}
	UI_I2cFlags.setText(tmpText);
	
	slaveAddress = (parseInt(UI_I2cAddress.text().slice(0,1), 16) & 0xff) << 8;
	slaveAddress += parseInt(UI_I2cAddress.text().slice(1,3), 16) & 0xff;
	
	flags = parseInt(UI_I2cFlags.text().slice(0, 2), 16) & 0xff;
	numberOfBytesToRead = parseInt(UI_I2cNumberOfBytesToRead.value());
	
	UI_I2cBytesToSend.setPlainText(limitAndFormatHexString(UI_I2cBytesToSend.toPlainText(), true));
	
	if(UI_I2cBytesToSend.toPlainText().length > 0)
	{
		dataToSend = convertHexStringToByteArray(UI_I2cBytesToSend.toPlainText());
	}
	
	if(UI_AardvarkI2cSpiMode.currentText() == "I2C Master")
	{
		if(scriptInf.i2cMasterReadWrite(flags, slaveAddress, numberOfBytesToRead,  dataToSend))
		{
			UI_Console.append("execute I2C: flags=0x" + flags.toString(16) + " address=0x" + slaveAddress.toString(16) + " data= " + conv.byteArrayToHexString(dataToSend));
		}
		else
		{
			UI_Console.append("execute I2C failed");
		}
	}
	else
	{//I2C Slave
		
		if(scriptInf.sendDataArray(dataToSend))
		{
			UI_Console.append("execute I2C (set slave response): data= " + conv.byteArrayToHexString(dataToSend));
		}
		else
		{
			UI_Console.append("execute I2C failed (set slave response)");
		}
	}
	
}

function slaveDataSentSlot(data)
{
	UI_Console.append("slave data sent: data=" + conv.byteArrayToHexString(data));
}

function executeSpiSlot()
{
	UI_SpiBytesToSend.setPlainText(limitAndFormatHexString(UI_SpiBytesToSend.toPlainText(), true));
	
	if(UI_SpiBytesToSend.toPlainText().length > 0)
	{
		var dataToSend = convertHexStringToByteArray(UI_SpiBytesToSend.toPlainText());
		if(scriptInf.sendDataArray(dataToSend))
		{
			if(UI_AardvarkI2cSpiMode.currentText() == "SPI Master")
			{
				UI_Console.append("execute SPI: data=" + conv.byteArrayToHexString(dataToSend));
			}
			else
			{
				UI_Console.append("set SPI slave response: data=" + conv.byteArrayToHexString(dataToSend));
			}
		}
		else
		{
			UI_Console.append("execute SPI failed");
		}
	}
	else
	{
		UI_Console.append("execute SPI failed (no data)");
	}
}

function connectSlot()
{
	
	if(!scriptInf.isConnected())
	{
		/************************fill the AardvarkI2cSpiSettings structure**********************/
		var settings = scriptInf.aardvarkI2cSpiGetMainInterfaceSettings();
		settings.devicePort =  parseInt(UI_AardvarkI2cSpiPort.text());
		settings.deviceMode =  UI_AardvarkI2cSpiMode.currentIndex();
		settings.device5VIsOn =  (UI_AardvarkI2cSpi5V.currentText() == "On") ? true : false
		
		settings.i2cBaudrate =  parseInt(UI_AardvarkI2cBaudrate.text());
		settings.i2cSlaveAddress =  parseInt(UI_AardvarkI2cSlaveAddress.text());
		settings.i2cPullupsOn = (UI_AardvarkI2cPullUp.currentText() == "On") ? true : false
		
		settings.spiPolarity =  UI_AardvarkSpiPolarity.currentIndex();
		settings.spiSSPolarity =  UI_AardvarkSpiSSPolarity.currentIndex();
		settings.spiBitorder =  UI_AardvarkSpiBitorder.currentIndex();
		settings.spiPhase =  UI_AardvarkSpiPhase.currentIndex();
		settings.spiBaudrate =  parseInt(UI_AardvarkSpiBaudrate.text());

		for(var i = 0; i < settings.pinConfigs.length; i++)
		{
			settings.pinConfigs[i].isInput = (g_aardvardI2cGpioGuiElements[i].mode.currentText().indexOf("in") != -1) ? true : false;
			settings.pinConfigs[i].withPullups = (g_aardvardI2cGpioGuiElements[i].mode.currentText().indexOf("in pullup") != -1) ? true : false;
			settings.pinConfigs[i].outValue = (g_aardvardI2cGpioGuiElements[i].outValue.currentText() == 1) ? true : false;
		}
		
		/*********************************************************************************************************/
		
		if(!scriptInf.aardvarkI2cSpiConnect(settings))
		{
			UI_Console.append("connect error");
		}
			
	}
	else
	{
		scriptInf.disconnect();
		
		for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
		{
			g_aardvardI2cGpioGuiElements[i].inValue.setText("0");
		}
	}
		
	if(scriptInf.isConnected())
	{
		UI_AardvarkI2cConnect.setText("disconnect");
		
		var readSettings = scriptInf.aardvarkI2cSpiGetMainInterfaceSettings();
		var consoleString = "connected: <br>" + settingsStructToString(readSettings);
		UI_Console.append(consoleString);
	}
	else
	{
		UI_AardvarkI2cConnect.setText("connect");
	}
	
	initializeGUI();
}

function i2cMasterDataReceivedSlot(flags, address, data)
{
	UI_Console.append("I2C master data received: flags=0x" + flags.toString(16) + " address=0x" + address.toString(16) + " data=" + conv.byteArrayToHexString(data));
}

function dataReceivedSlot(data)
{
	if(UI_AardvarkI2cSpiMode.currentText().indexOf("I2C") != -1)
	{
		UI_Console.append("I2C data received: data=" + conv.byteArrayToHexString(data));
	}
	else
	{
		UI_Console.append("SPI data received: data=" + conv.byteArrayToHexString(data));
	}
}

function aardvarkI2cSpiInputStatesChangedSlot(states)
{
	for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
		{
			g_aardvardI2cGpioGuiElements[i].inValue.setText(states[i] ? "1" : "0");
		}
}
function outValueChangedSlot()
{
	scriptInf.aardvarkI2cSpiSetOutput(this.getAdditionalData(1), (this.currentText()== 1) ? true : false);
	UI_Console.append("output value changed:  pin= " + this.getAdditionalData(0) + " value=" + this.currentText());
	initializeGUI();
}

function pinModeChangedSlot()
{
	var isInput = (this.currentText().indexOf("in") != -1) ? true : false;
	var withPullups = (this.currentText().indexOf("in pullup") != -1) ? true : false;
	
	if(isInput)
	{
		g_aardvardI2cGpioGuiElements[this.getAdditionalData(1)].outValue.setCurrentText("0");
	}
	
	scriptInf.aardvarkI2cSpiChangePinConfiguration(this.getAdditionalData(1), isInput, withPullups);
	UI_Console.append("pin mode changed:  pin= " + this.getAdditionalData(0) + " value=" + this.currentText());
	initializeGUI();
}

function detectAardvarkI2cSpiDevicesSlot()
{
	UI_Console.append(scriptInf.aardvarkI2cSpiDetectDevices());
}

scriptThread.appendTextToConsole('script test Aardvark I2C/SPI (MainInterface) started');
scriptThread.loadUserInterfaceFile("TestAardvarkI2cSpi.ui");
scriptThread.loadScript("TestAardvarkI2cSpi_Helper.js");

UI_Dialog.finishedSignal.connect(dialogFinishedSlot);
UI_AardvarkI2cConnect.clickedSignal.connect(connectSlot);
scriptInf.aardvarkI2cSpiInputStatesChangedSignal.connect(aardvarkI2cSpiInputStatesChangedSlot);
scriptInf.i2cMasterDataReceivedSignal.connect(i2cMasterDataReceivedSlot);
scriptInf.dataReceivedSignal.connect(dataReceivedSlot);
scriptInf.slaveDataSentSignal.connect(slaveDataSentSlot);

scriptInf.disconnect();

initializeGuiElements();
initializeGUI();
loadUiSettings("aardvarkI2cSpi_mainInf_settings.txt");
detectAardvarkI2cSpiDevicesSlot();

