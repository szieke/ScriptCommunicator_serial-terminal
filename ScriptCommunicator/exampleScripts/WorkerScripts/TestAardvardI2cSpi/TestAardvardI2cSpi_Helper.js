

function initializeGUI()
{
	UI_AardvardI2cBaudrate.setEnabled(false);
    UI_AardvardI2cPullUp.setEnabled(false);
    UI_AardvardI2cFreeBus.setEnabled(false);

    UI_AardvardSpiPolarity.setEnabled(false);
    UI_AardvardSpiSSPolarity.setEnabled(false);
    UI_AardvardSpiBitorder.setEnabled(false);
    UI_AardvardSpiPhase.setEnabled(false);
    UI_AardvardSpiBaudrate.setEnabled(false);

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


     if(!g_isConnected)
     {
         UI_AardvardI2cSpiPort.setEnabled(true);
         UI_AardvardI2cSpiMode.setEnabled(true);
         UI_AardvardI2cSpiScan.setEnabled(true);
         UI_AardvardI2cSpi5V.setEnabled(true);

         if(UI_AardvardI2cSpiMode.currentText() == "I2C Master")
         {
             UI_AardvardI2cBaudrate.setEnabled(true);
             UI_AardvardI2cPullUp.setEnabled(true);
         }
         else if(UI_AardvardI2cSpiMode.currentText() == "SPI Master")
         {
             UI_AardvardSpiPolarity.setEnabled(true);
             UI_AardvardSpiSSPolarity.setEnabled(true);
             UI_AardvardSpiBitorder.setEnabled(true);
             UI_AardvardSpiPhase.setEnabled(true);
             UI_AardvardSpiBaudrate.setEnabled(true);
         }

     }
     else
     {
         UI_AardvardI2cSpiPort.setEnabled(false);
         UI_AardvardI2cSpiMode.setEnabled(false);
         UI_AardvardI2cSpiScan.setEnabled(false);
         UI_AardvardI2cSpi5V.setEnabled(false);
		 
		 if(UI_AardvardI2cSpiMode.currentText() == "I2C Master")
         {			 
			 UI_I2cExecute.setEnabled(true);
			 UI_AardvardI2cFreeBus.setEnabled(true);
         }
         else if(UI_AardvardI2cSpiMode.currentText() == "SPI Master")
         {
			 UI_SpiExecute.setEnabled(true);
         }
     }

     var startIndex = 0;
     var endIndex = 0;
     if(UI_AardvardI2cSpiMode.currentText() == "I2C Master")
     {
         startIndex = 2;
         endIndex = AARDVARD_I2C_SPI_GPIO_COUNT - 1;
		 
		 UI_I2cAddress.setEnabled(true);
		 UI_I2cFlags.setEnabled(true);
		 UI_I2cBytesToSend.setEnabled(true);
		 UI_I2cNumberOfBytesToRead.setEnabled(true);
     }
     else if(UI_AardvardI2cSpiMode.currentText() == "SPI Master")
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
	g_aardvardI2cGpioGuiElements[0].mode = UI_AardvardGpioMode0;
	g_aardvardI2cGpioGuiElements[0].mode.setAdditionalData(0, "Pin1/SCL");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[0].outValue = UI_AardvardGpioOutValue0;
	g_aardvardI2cGpioGuiElements[0].outValue.setAdditionalData(0, "Pin1/SCL");//Store the pin name.
	g_aardvardI2cGpioGuiElements[0].inValue = UI_AardvardGpioInValue0;

	g_aardvardI2cGpioGuiElements[1] = Array();
	g_aardvardI2cGpioGuiElements[1].mode = UI_AardvardGpioMode1;
	g_aardvardI2cGpioGuiElements[1].mode.setAdditionalData(0, "Pin3/SDA");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[1].outValue = UI_AardvardGpioOutValue1;
	g_aardvardI2cGpioGuiElements[1].outValue.setAdditionalData(0, "Pin3/SDA");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[1].inValue = UI_AardvardGpioInValue1;

	g_aardvardI2cGpioGuiElements[2] = Array();
	g_aardvardI2cGpioGuiElements[2].mode = UI_AardvardGpioMode2;
	g_aardvardI2cGpioGuiElements[2].mode.setAdditionalData(0, "Pin5/MISO");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[2].outValue = UI_AardvardGpioOutValue2;
	g_aardvardI2cGpioGuiElements[2].outValue.setAdditionalData(0, "Pin5/MISO");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[2].inValue = UI_AardvardGpioInValue2;

	g_aardvardI2cGpioGuiElements[3] = Array();
	g_aardvardI2cGpioGuiElements[3].mode = UI_AardvardGpioMode3;
	g_aardvardI2cGpioGuiElements[3].mode.setAdditionalData(0, "Pin7/SCK");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[3].outValue = UI_AardvardGpioOutValue3;
	g_aardvardI2cGpioGuiElements[3].outValue.setAdditionalData(0, "Pin7/SCK");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[3].inValue = UI_AardvardGpioInValue3;

	g_aardvardI2cGpioGuiElements[4] = Array();
	g_aardvardI2cGpioGuiElements[4].mode = UI_AardvardGpioMode4;
	g_aardvardI2cGpioGuiElements[4].mode.setAdditionalData(0, "Pin8/MOSI");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[4].outValue = UI_AardvardGpioOutValue4;
	g_aardvardI2cGpioGuiElements[4].outValue.setAdditionalData(0, "Pin8/MOSI");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[4].inValue = UI_AardvardGpioInValue4;

	g_aardvardI2cGpioGuiElements[5] = Array();
	g_aardvardI2cGpioGuiElements[5].mode = UI_AardvardGpioMode5;
	g_aardvardI2cGpioGuiElements[5].mode.setAdditionalData(0, "Pin9/SS0");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[5].outValue = UI_AardvardGpioOutValue5;
	g_aardvardI2cGpioGuiElements[5].outValue.setAdditionalData(0, "Pin9/SS0");//Store the pin name. 
	g_aardvardI2cGpioGuiElements[5].inValue = UI_AardvardGpioInValue5;

	UI_AardvardI2cSpiMode.currentIndexChangedSignal.connect(initializeGUI);

	for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
	 {
		 g_aardvardI2cGpioGuiElements[i].mode.setAdditionalData(1, i);//Store the pin number.
		 g_aardvardI2cGpioGuiElements[i].outValue.setAdditionalData(1, i);//Store the pin number.
		 
		 g_aardvardI2cGpioGuiElements[i].mode.currentIndexChangedSignal.connect(g_aardvardI2cGpioGuiElements[i].mode, pinModeChangedSlot);
		 g_aardvardI2cGpioGuiElements[i].outValue.currentIndexChangedSignal.connect(g_aardvardI2cGpioGuiElements[i].outValue, outValueChangedSlot);
	 }
	 
	 UI_AardvardI2cSpiScan.clickedSignal.connect(detectAardvardI2cSpiDevicesSlot);
	 
	 UI_I2cBytesToSend.textChangedSignal.connect(UI_I2cBytesToSend, hexTextEditTextChangedSlot);
	 UI_SpiBytesToSend.textChangedSignal.connect(UI_SpiBytesToSend, hexTextEditTextChangedSlot);
	 
	 UI_I2cExecute.clickedSignal.connect(executeI2cSlot);
	 UI_SpiExecute.clickedSignal.connect(executeSpiSlot);
	 UI_I2cAddress.textChangedSignal.connect(UI_I2cAddress, hexTextLineTextChangedSlot);
	 UI_I2cFlags.textChangedSignal.connect(UI_I2cFlags, hexTextLineTextChangedSlot);
	 UI_AardvardI2cFreeBus.clickedSignal.connect(freeI2cBusSlot);

}

function settingsStructToString(readSettings)
{
	var result = "devicePort=" + readSettings.devicePort + " deviceMode=" + readSettings.deviceMode + " device5VIsOn=" + readSettings.device5VIsOn;
	result += " i2cBaudrate=" + readSettings.i2cBaudrate + " i2cPullupsOn=" + readSettings.i2cPullupsOn + " spiSSPolarity=" + readSettings.spiSSPolarity;
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
function loadUiSettings()
{
	if(scriptFile.checkFileExists("aardvardI2cSpi_settings.txt"))
	{
		var settings = scriptFile.readFile("aardvardI2cSpi_settings.txt");
		var stringArray = settings.split("\r\n");
		
		UI_AardvardI2cSpiPort.setText(getValueOfStringArray(stringArray, "UI_AardvardI2cSpiPort"));
		UI_AardvardI2cSpiMode.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardI2cSpiMode"));
		UI_AardvardI2cSpi5V.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardI2cSpi5V"));
		
		UI_AardvardI2cBaudrate.setText(getValueOfStringArray(stringArray, "UI_AardvardI2cBaudrate"));
		UI_AardvardI2cPullUp.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardI2cPullUp"));
		
		UI_AardvardSpiPolarity.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardSpiPolarity"));
		UI_AardvardSpiPhase.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardSpiPhase"));
		UI_AardvardSpiBaudrate.setText(getValueOfStringArray(stringArray, "UI_AardvardSpiBaudrate"));
		UI_AardvardSpiSSPolarity.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardSpiSSPolarity"));
		UI_AardvardSpiBitorder.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardSpiBitorder"));
		
		for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
		{
			g_aardvardI2cGpioGuiElements[i].mode.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardGpioMode" + i ));
			g_aardvardI2cGpioGuiElements[i].outValue.setCurrentText(getValueOfStringArray(stringArray, "UI_AardvardGpioOutValue" + i));
		}
		
		UI_I2cAddress.setText(getValueOfStringArray(stringArray, "UI_I2cAddress"));
		UI_I2cFlags.setText(getValueOfStringArray(stringArray, "UI_I2cFlags"));
		UI_I2cNumberOfBytesToRead.setValue(getValueOfStringArray(stringArray, "UI_I2cNumberOfBytesToRead"));
		UI_I2cBytesToSend.setPlainText(getValueOfStringArray(stringArray, "UI_I2cBytesToSend"));
		
		UI_SpiBytesToSend.setPlainText(getValueOfStringArray(stringArray, "UI_SpiBytesToSend"));
	}
	
}


//Save the GUI settings.
function saveUiSettings()
{
	settings = "UI_AardvardI2cSpiPort=" + UI_AardvardI2cSpiPort.text() + "\r\n";
	settings += "UI_AardvardI2cSpiMode=" + UI_AardvardI2cSpiMode.currentText() + "\r\n";
	settings += "UI_AardvardI2cSpi5V=" + UI_AardvardI2cSpi5V.currentText() + "\r\n";
	
	settings += "UI_AardvardI2cBaudrate=" + UI_AardvardI2cBaudrate.text() + "\r\n";
	settings += "UI_AardvardI2cPullUp=" + UI_AardvardI2cPullUp.currentText() + "\r\n";
	
	settings += "UI_AardvardSpiPolarity=" + UI_AardvardSpiPolarity.currentText() + "\r\n";
	settings += "UI_AardvardSpiPhase=" + UI_AardvardSpiPhase.currentText() + "\r\n";
	settings += "UI_AardvardSpiBaudrate=" + UI_AardvardSpiBaudrate.text() + "\r\n";
	settings += "UI_AardvardSpiSSPolarity=" + UI_AardvardSpiSSPolarity.currentText() + "\r\n";
	settings += "UI_AardvardSpiBitorder=" + UI_AardvardSpiBitorder.currentText() + "\r\n";
	
	for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
    {
		settings += "UI_AardvardGpioMode" + i + "=" + g_aardvardI2cGpioGuiElements[i].mode.currentText() + "\r\n";
		settings += "UI_AardvardGpioOutValue" + i + "=" + g_aardvardI2cGpioGuiElements[i].outValue.currentText() + "\r\n";
    }
	
	settings += "UI_I2cAddress=" + UI_I2cAddress.text() + "\r\n";
	settings += "UI_I2cFlags=" + UI_I2cFlags.text() + "\r\n";
	settings += "UI_I2cNumberOfBytesToRead=" + UI_I2cNumberOfBytesToRead.value() + "\r\n";
	settings += "UI_I2cBytesToSend=" + UI_I2cBytesToSend.toPlainText() + "\r\n";
	
	settings += "UI_SpiBytesToSend=" + UI_SpiBytesToSend.toPlainText() + "\r\n";
	
	scriptFile.writeFile("aardvardI2cSpi_settings.txt", true, settings, true);
}



