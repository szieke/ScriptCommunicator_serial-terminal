//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script test Aardvard I2C/SPI (MainInterface) stopped");
}

//The dialog is closed.
function dialogFinishedSlot(e)
{	
	scriptThread.stopScript()
}

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
	UI_I2cReadBytes.setEnabled(false);
	UI_I2cExecute.setEnabled(false);
	
	UI_SpiBytesToSend.setEnabled(false);
	UI_SpiReadBytes.setEnabled(false);
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
		 UI_I2cReadBytes.setEnabled(true);
     }
     else if(UI_AardvardI2cSpiMode.currentText() == "SPI Master")
     {
         startIndex = 0;
         endIndex = 1;
		 
		 UI_SpiBytesToSend.setEnabled(true);
	     UI_SpiReadBytes.setEnabled(true);
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

function initializeGuiElements()
{
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

}