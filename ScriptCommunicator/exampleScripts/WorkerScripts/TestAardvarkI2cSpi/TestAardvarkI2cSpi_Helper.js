

function initializeGUI(isConnected)
{
	UI_AardvarkI2cBaudrate.setEnabled(false);
	UI_AardvarkI2cSlaveAddress.setEnabled(false);
    UI_AardvarkI2cPullUp.setEnabled(false);
    UI_AardvarkI2cFreeBus.setEnabled(false);

    UI_AardvarkSpiPolarity.setEnabled(false);
    UI_AardvarkSpiSSPolarity.setEnabled(false);
    UI_AardvarkSpiBitorder.setEnabled(false);
    UI_AardvarkSpiPhase.setEnabled(false);
    UI_AardvarkSpiBaudrate.setEnabled(false);

    for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
    {
        g_aardvardI2cGpioGuiElements[i].mode.setEnabled(false);
        g_aardvardI2cGpioGuiElements[i].outValue.setEnabled(false);
    }
	
	UI_I2cAddress.setEnabled(false);
	UI_I2cFlags.setEnabled(false);
	UI_I2cBytesToSend.setEnabled(false);
	UI_I2cNumberOfBytesToRead.setEnabled(false);
	UI_I2cExecute.setEnabled(false);
	
	UI_SpiBytesToSend.setEnabled(false);
	UI_SpiExecute.setEnabled(false);

	var isConnected = false;
	if(typeof g_interface != 'undefined')
	{
		isConnected = g_interface.isConnected();
	}
	else
	{
		isConnected = scriptInf.isConnected();
	}

     if(!isConnected)
     {
         UI_AardvarkI2cSpiPort.setEnabled(true);
         UI_AardvarkI2cSpiMode.setEnabled(true);
         UI_AardvarkI2cSpiScan.setEnabled(true);
         UI_AardvarkI2cSpi5V.setEnabled(true);

         if((UI_AardvarkI2cSpiMode.currentText() == "I2C Master") ||
			 (UI_AardvarkI2cSpiMode.currentText() == "I2C Slave"))
         {
             UI_AardvarkI2cBaudrate.setEnabled(true);
             UI_AardvarkI2cPullUp.setEnabled(true);
			 
			 if(UI_AardvarkI2cSpiMode.currentText() == "I2C Slave")
			 {
				 UI_AardvarkI2cSlaveAddress.setEnabled(true);
			 }
         }
         else if((UI_AardvarkI2cSpiMode.currentText() == "SPI Master") ||
			 (UI_AardvarkI2cSpiMode.currentText() == "SPI Slave"))
         {
             UI_AardvarkSpiPolarity.setEnabled(true);
             UI_AardvarkSpiSSPolarity.setEnabled(true);
             UI_AardvarkSpiBitorder.setEnabled(true);
             UI_AardvarkSpiPhase.setEnabled(true);
             UI_AardvarkSpiBaudrate.setEnabled(true);
         }

     }
     else
     {
         UI_AardvarkI2cSpiPort.setEnabled(false);
         UI_AardvarkI2cSpiMode.setEnabled(false);
         UI_AardvarkI2cSpiScan.setEnabled(false);
         UI_AardvarkI2cSpi5V.setEnabled(false);
		 
		 if((UI_AardvarkI2cSpiMode.currentText() == "I2C Master") ||
			 (UI_AardvarkI2cSpiMode.currentText() == "I2C Slave"))
         {			 
			 UI_I2cExecute.setEnabled(true);
			 
			 if(UI_AardvarkI2cSpiMode.currentText() == "I2C Master")
			 {
				  UI_AardvarkI2cFreeBus.setEnabled(true);
			 }
         }
         else if((UI_AardvarkI2cSpiMode.currentText() == "SPI Master") ||
			 (UI_AardvarkI2cSpiMode.currentText() == "SPI Slave"))
         {
			 UI_SpiExecute.setEnabled(true);
         }
     }

     var startIndex = 0;
     var endIndex = 0;
      if((UI_AardvarkI2cSpiMode.currentText() == "I2C Master") ||
			 (UI_AardvarkI2cSpiMode.currentText() == "I2C Slave"))
     {
         startIndex = 2;
         endIndex = AARDVARD_I2C_SPI_GPIO_COUNT - 1;
		 
		 UI_I2cAddress.setEnabled(true);
		 UI_I2cFlags.setEnabled(true);
		 UI_I2cBytesToSend.setEnabled(true);
		 UI_I2cNumberOfBytesToRead.setEnabled(true);
     }
     else if((UI_AardvarkI2cSpiMode.currentText() == "SPI Master") ||
			 (UI_AardvarkI2cSpiMode.currentText() == "SPI Slave"))
     {
         startIndex = 0;
         endIndex = 1;
		 
		 UI_SpiBytesToSend.setEnabled(true);
     }
     else
     {
         startIndex = 0;
         endIndex = AARDVARD_I2C_SPI_GPIO_COUNT - 1;
     }

     for(var i = startIndex; i <= endIndex; i++)
     {
         g_aardvardI2cGpioGuiElements[i].mode.setEnabled(true);
         g_aardvardI2cGpioGuiElements[i].outValue.setEnabled((g_aardvardI2cGpioGuiElements[i].mode.currentText() == "out") ? true : false);
     }
}

function convertHexStringToByteArray(str) 
{ 
    var result = [];
	
	str = str.replace(/ /gm, "")//remove all spaces;
	
    while (str.length >= 2) 
	{ 
        result.push(parseInt(str.substring(0, 2), 16));
        str = str.substring(2, str.length);
    }
	
	if(str.length == 1)
	{
		result.push(parseInt(str, 16));
	}

    return result;
}
function limitAndFormatHexString(text, withSpaces)
{
	text = text.replace(RegExp("[^a-fA-F\\d\\s]"), "");
	
	var split = text.split(" ");
	text = "";
	
	for(var index = 0; index <  split.length; index++)
	{
		if(split[index].length > 2)
		{
			while(split[index].length > 2)
			{
				text += split[index].slice(0, 2) 
				if(withSpaces)
				{
					text += " ";
				}
				split[index] = split[index].slice(2);
			}
		}
		
		if(split[index].length > 0)
		{
			text += split[index];
			if(withSpaces)
			{
				text += " ";
			}
		}
	}
	
	if(text.indexOf(" ", text.length - 1) != -1)
	{
		text = text.slice(0, text.length - 1)
	}
	
	return text;
}

function hexTextLineTextChangedSlot()
{
	var text = this.text();
	text = text.replace(RegExp("[^a-fA-F\\d\\s]"), "");
	text = text.replace(/ /gm, "")//remove all spaces;

	if(this.text() != text)
	{
		this.blockSignals(true);
		this.setText(text);
		this.blockSignals(false);
	}
}

function hexTextEditTextChangedSlot()
{
	var text = this.toPlainText();
	text = text.replace(RegExp("[^a-fA-F\\d\\s]"), "");
	
	if(this.toPlainText() != text)
	{
		this.blockSignals(true);
		this.setPlainText(text);
		this.blockSignals(false);
	}
}
function initializeGuiElements()
{
	
 UI_Dialog.hide();
 scriptThread.addTabsToMainWindow(UI_TabWidget);
	
	g_aardvardI2cGpioGuiElements = Array();
	g_aardvardI2cGpioGuiElements[0] = Array();
	g_aardvardI2cGpioGuiElements[0].mode = UI_AardvarkGpioMode0;
	g_aardvardI2cGpioGuiElements[0].mode.setAdditionalData(0, "Pin1/SCL");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[0].outValue = UI_AardvarkGpioOutValue0;
	g_aardvardI2cGpioGuiElements[0].outValue.setAdditionalData(0, "Pin1/SCL");//Store the pin name.
	g_aardvardI2cGpioGuiElements[0].inValue = UI_AardvarkGpioInValue0;

	g_aardvardI2cGpioGuiElements[1] = Array();
	g_aardvardI2cGpioGuiElements[1].mode = UI_AardvarkGpioMode1;
	g_aardvardI2cGpioGuiElements[1].mode.setAdditionalData(0, "Pin3/SDA");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[1].outValue = UI_AardvarkGpioOutValue1;
	g_aardvardI2cGpioGuiElements[1].outValue.setAdditionalData(0, "Pin3/SDA");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[1].inValue = UI_AardvarkGpioInValue1;

	g_aardvardI2cGpioGuiElements[2] = Array();
	g_aardvardI2cGpioGuiElements[2].mode = UI_AardvarkGpioMode2;
	g_aardvardI2cGpioGuiElements[2].mode.setAdditionalData(0, "Pin5/MISO");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[2].outValue = UI_AardvarkGpioOutValue2;
	g_aardvardI2cGpioGuiElements[2].outValue.setAdditionalData(0, "Pin5/MISO");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[2].inValue = UI_AardvarkGpioInValue2;

	g_aardvardI2cGpioGuiElements[3] = Array();
	g_aardvardI2cGpioGuiElements[3].mode = UI_AardvarkGpioMode3;
	g_aardvardI2cGpioGuiElements[3].mode.setAdditionalData(0, "Pin7/SCK");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[3].outValue = UI_AardvarkGpioOutValue3;
	g_aardvardI2cGpioGuiElements[3].outValue.setAdditionalData(0, "Pin7/SCK");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[3].inValue = UI_AardvarkGpioInValue3;

	g_aardvardI2cGpioGuiElements[4] = Array();
	g_aardvardI2cGpioGuiElements[4].mode = UI_AardvarkGpioMode4;
	g_aardvardI2cGpioGuiElements[4].mode.setAdditionalData(0, "Pin8/MOSI");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[4].outValue = UI_AardvarkGpioOutValue4;
	g_aardvardI2cGpioGuiElements[4].outValue.setAdditionalData(0, "Pin8/MOSI");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[4].inValue = UI_AardvarkGpioInValue4;

	g_aardvardI2cGpioGuiElements[5] = Array();
	g_aardvardI2cGpioGuiElements[5].mode = UI_AardvarkGpioMode5;
	g_aardvardI2cGpioGuiElements[5].mode.setAdditionalData(0, "Pin9/SS0");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[5].outValue = UI_AardvarkGpioOutValue5;
	g_aardvardI2cGpioGuiElements[5].outValue.setAdditionalData(0, "Pin9/SS0");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[5].inValue = UI_AardvarkGpioInValue5;

	UI_AardvarkI2cSpiMode.currentIndexChangedSignal.connect(initializeGUI);

	for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
	 {
		 g_aardvardI2cGpioGuiElements[i].mode.setAdditionalData(1, i);//Store the pin number.
		 g_aardvardI2cGpioGuiElements[i].outValue.setAdditionalData(1, i);//Store the pin number.
		 
		 g_aardvardI2cGpioGuiElements[i].mode.currentIndexChangedSignal.connect(g_aardvardI2cGpioGuiElements[i].mode, pinModeChangedSlot);
		 g_aardvardI2cGpioGuiElements[i].outValue.currentIndexChangedSignal.connect(g_aardvardI2cGpioGuiElements[i].outValue, outValueChangedSlot);
	 }
	 
	 UI_AardvarkI2cSpiScan.clickedSignal.connect(detectAardvarkI2cSpiDevicesSlot);
	 
	 UI_I2cBytesToSend.textChangedSignal.connect(UI_I2cBytesToSend, hexTextEditTextChangedSlot);
	 UI_SpiBytesToSend.textChangedSignal.connect(UI_SpiBytesToSend, hexTextEditTextChangedSlot);
	 
	 UI_I2cExecute.clickedSignal.connect(executeI2cSlot);
	 UI_SpiExecute.clickedSignal.connect(executeSpiSlot);
	 UI_I2cAddress.textChangedSignal.connect(UI_I2cAddress, hexTextLineTextChangedSlot);
	 UI_I2cFlags.textChangedSignal.connect(UI_I2cFlags, hexTextLineTextChangedSlot);
	 UI_AardvarkI2cFreeBus.clickedSignal.connect(freeI2cBusSlot);

}

function settingsStructToString(readSettings)
{
	var result = "devicePort=" + readSettings.devicePort + " deviceMode=" + readSettings.deviceMode + " device5VIsOn=" + readSettings.device5VIsOn;
	result += " i2cBaudrate=" + readSettings.i2cBaudrate + " i2cSlaveAddress=" + readSettings.i2cSlaveAddress + " i2cPullupsOn=" + readSettings.i2cPullupsOn + " spiSSPolarity=" + readSettings.spiSSPolarity;
	result += " spiBitorder=" + readSettings.spiBitorder + " spiPhase=" + readSettings.spiPhase + " spiBaudrate=" + readSettings.spiBaudrate;
	for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
	{
		var mode = "out";
		if(readSettings.pinConfigs[i].isInput)
		{
			if(readSettings.pinConfigs[i].withPullups)
			{
				mode = "in pullups";
			}
			else
			{
				mode = "in";
			}
		}
		result += "<br>"+ g_aardvardI2cGpioGuiElements[i].mode.getAdditionalData(0) + " mode=" + mode;
		result += " "+ g_aardvardI2cGpioGuiElements[i].mode.getAdditionalData(0) + " outValue=" + readSettings.pinConfigs[i].outValue;
	}
	
	return result;
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
function loadUiSettings(fileName)
{
	if(scriptFile.checkFileExists(fileName))
	{
		var settings = scriptFile.readFile(fileName);
		var stringArray = settings.split("\r\n");
		
		UI_AardvarkI2cSpiPort.setText(getValueOfStringArray(stringArray, "UI_AardvarkI2cSpiPort"));
		UI_AardvarkI2cSpiMode.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkI2cSpiMode"));
		UI_AardvarkI2cSpi5V.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkI2cSpi5V"));
		
		UI_AardvarkI2cBaudrate.setText(getValueOfStringArray(stringArray, "UI_AardvarkI2cBaudrate"));
		UI_AardvarkI2cSlaveAddress.setText(getValueOfStringArray(stringArray, "UI_AardvarkI2cSlaveAddress"));
		UI_AardvarkI2cPullUp.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkI2cPullUp"));
		
		UI_AardvarkSpiPolarity.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkSpiPolarity"));
		UI_AardvarkSpiPhase.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkSpiPhase"));
		UI_AardvarkSpiBaudrate.setText(getValueOfStringArray(stringArray, "UI_AardvarkSpiBaudrate"));
		UI_AardvarkSpiSSPolarity.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkSpiSSPolarity"));
		UI_AardvarkSpiBitorder.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkSpiBitorder"));
		
		for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
		{
			g_aardvardI2cGpioGuiElements[i].mode.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkGpioMode" + i ));
			g_aardvardI2cGpioGuiElements[i].outValue.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvarkGpioOutValue" + i));
		}
		
		UI_I2cAddress.setText(getValueOfStringArray(stringArray, "UI_I2cAddress"));
		UI_I2cFlags.setText(getValueOfStringArray(stringArray, "UI_I2cFlags"));
		UI_I2cNumberOfBytesToRead.setValue(getValueOfStringArray(stringArray, "UI_I2cNumberOfBytesToRead"));
		UI_I2cBytesToSend.setPlainText(getValueOfStringArray(stringArray, "UI_I2cBytesToSend"));
		
		UI_SpiBytesToSend.setPlainText(getValueOfStringArray(stringArray, "UI_SpiBytesToSend"));
	}
	
}


//Save the GUI settings.
function saveUiSettings(fileName)
{
	settings = "UI_AardvarkI2cSpiPort=" + UI_AardvarkI2cSpiPort.text() + "\r\n";
	settings += "UI_AardvarkI2cSpiMode=" + UI_AardvarkI2cSpiMode.currentText() + "\r\n";
	settings += "UI_AardvarkI2cSpi5V=" + UI_AardvarkI2cSpi5V.currentText() + "\r\n";
	
	settings += "UI_AardvarkI2cBaudrate=" + UI_AardvarkI2cBaudrate.text() + "\r\n";
	settings += "UI_AardvarkI2cSlaveAddress=" + UI_AardvarkI2cSlaveAddress.text() + "\r\n";
	settings += "UI_AardvarkI2cPullUp=" + UI_AardvarkI2cPullUp.currentText() + "\r\n";
	
	settings += "UI_AardvarkSpiPolarity=" + UI_AardvarkSpiPolarity.currentText() + "\r\n";
	settings += "UI_AardvarkSpiPhase=" + UI_AardvarkSpiPhase.currentText() + "\r\n";
	settings += "UI_AardvarkSpiBaudrate=" + UI_AardvarkSpiBaudrate.text() + "\r\n";
	settings += "UI_AardvarkSpiSSPolarity=" + UI_AardvarkSpiSSPolarity.currentText() + "\r\n";
	settings += "UI_AardvarkSpiBitorder=" + UI_AardvarkSpiBitorder.currentText() + "\r\n";
	
	for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
    {
		settings += "UI_AardvarkGpioMode" + i + "=" + g_aardvardI2cGpioGuiElements[i].mode.currentText() + "\r\n";
		settings += "UI_AardvarkGpioOutValue" + i + "=" + g_aardvardI2cGpioGuiElements[i].outValue.currentText() + "\r\n";
    }
	
	settings += "UI_I2cAddress=" + UI_I2cAddress.text() + "\r\n";
	settings += "UI_I2cFlags=" + UI_I2cFlags.text() + "\r\n";
	settings += "UI_I2cNumberOfBytesToRead=" + UI_I2cNumberOfBytesToRead.value() + "\r\n";
	settings += "UI_I2cBytesToSend=" + UI_I2cBytesToSend.toPlainText() + "\r\n";
	
	settings += "UI_SpiBytesToSend=" + UI_SpiBytesToSend.toPlainText() + "\r\n";
	
	scriptFile.writeFile(fileName, true, settings, true);
}



