/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
Aardvard I2C/SPI varerface (used as main varerface).
***************************************************************************/

const AARDVARD_I2C_SPI_GPIO_COUNT = 6;

function connectSlot()
{
	
	if(!g_isConnected)
	{
		/************************Create the AardvardI2cSpiSettings strucure**********************/
		settings = Array();
		settings.devicePort =  parseInt(UI_AardvardI2cSpiPort.text());
		settings.deviceMode =  UI_AardvardI2cSpiMode.currentIndex();
		settings.device5VIsOn =  (UI_AardvardI2cSpi5V.currentText() == "On") ? true : false
		
		settings.i2cBaudrate =  parseInt(UI_AardvardI2cBaudrate.text());
		settings.i2cPullupsOn = (UI_AardvardI2cPullUp.currentText() == "On") ? true : false
		
		settings.spiPolarity =  UI_AardvardSpiPolarity.currentIndex();
		settings.spiSSPolarity =  UI_AardvardSpiSSPolarity.currentIndex();
		settings.spiBitorder =  UI_AardvardSpiBitorder.currentIndex();
		settings.spiPhase =  UI_AardvardSpiPhase.currentIndex();
		settings.spiBaudrate =  parseInt(UI_AardvardSpiBaudrate.text());

		settings.pinConfigs =  Array();
		for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
		{
			settings.pinConfigs[i] = Array();	
			settings.pinConfigs[i].isInput = (g_aardvardI2cGpioGuiElements[i].mode.currentText().indexOf("in") != -1) ? true : false;
			settings.pinConfigs[i].withPullups = (g_aardvardI2cGpioGuiElements[i].mode.currentText().indexOf("in pullup") != -1) ? true : false;
			settings.pinConfigs[i].outValue = (g_aardvardI2cGpioGuiElements[i].outValue.currentText() == 1) ? true : false;
		}
		/*********************************************************************************************************/
		
		g_isConnected = scriptThread.connectAardvardI2cSpiDevice(settings);
	}
	else
	{
		scriptThread.disconnect();
		g_isConnected = false;
	}
		
	if(g_isConnected)
	{
		UI_AardvardI2cConnect.setText("disconnect");
	}
	else
	{
		UI_AardvardI2cConnect.setText("connect");
	}
	
	initializeGUI();
}

function outValueChangedSlot()
{
	//ToDo: Wert aendern.
	UI_Console.append("output value changed:  pin= " + this.getAdditionalData(0) + " value=" + this.currentText());
	initializeGUI();
}

function pinModeChangedSlot()
{
	//ToDo: Pinmode aendern.
	UI_Console.append("pin mode changed:  pin= " + this.getAdditionalData(0) + " value=" + this.currentText());
	initializeGUI();
}

function detectAardvardI2cSpiDevicesSlot()
{
	UI_Console.append(scriptThread.detectAardvardI2cSpiDevices());
}

scriptThread.appendTextToConsole('script test Aardvard I2C/SPI (MainInterface) started');
scriptThread.loadUserInterfaceFile("TestAardvardI2cSpi.ui");
scriptThread.loadScript("TestAardvardI2cSpi_Helper.js");

UI_Dialog.finishedSignal.connect(dialogFinishedSlot);
UI_AardvardI2cConnect.clickedSignal.connect(connectSlot);

scriptThread.disconnect();
g_isConnected = false;

initializeGuiElements();
initializeGUI();
detectAardvardI2cSpiDevicesSlot();

