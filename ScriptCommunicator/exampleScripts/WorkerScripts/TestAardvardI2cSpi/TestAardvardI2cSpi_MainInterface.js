/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
Aardvard I2C/SPI varerface (used as main varerface).
***************************************************************************/

const AARDVARD_I2C_SPI_GPIO_COUNT = 6;

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script test Aardvard I2C/SPI (MainInterface) stopped");
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

     if(g_varerfaceSettingsCanBeChanged)
     {
         UI_AardvardI2cSpiPort.setEnabled(true);
         UI_AardvardI2cSpiMode.setEnabled(true);
         UI_AardvardI2cSpiScan.setEnabled(true);
         UI_AardvardI2cSpi5V.setEnabled(true);

         if(AardvardI2cSpiMode.currentText() == "I2C Master")
         {
             UI_AardvardI2cBaudrate.setEnabled(true);
             UI_AardvardI2cPullUp.setEnabled(true);
         }
         else if(AardvardI2cSpiMode.currentText() == "SPI Master")
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
     }

     var startIndex = 0;
     var endIndex = 0;
     if(AardvardI2cSpiMode.currentText() == "I2C Master")
     {
         UI_AardvardI2cFreeBus.setEnabled(true);
         startIndex = 2;
         endIndex = AARDVARD_I2C_SPI_GPIO_COUNT - 1;
     }
     else if(AardvardI2cSpiMode.currentText() == "SPI Master")
     {
         startIndex = 0;
         endIndex = 1;
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

scriptThread.appendTextToConsole('script test Aardvard I2C/SPI (MainInterface) started');
scriptThread.loadUserInterfaceFile("TestAardvardI2cSpi.ui");

g_varerfaceSettingsCanBeChanged = true;
g_aardvardI2cGpioGuiElements = Array();
g_aardvardI2cGpioGuiElements[0] = Array();
g_aardvardI2cGpioGuiElements[0].mode = UI_AardvardGpioMode0;
g_aardvardI2cGpioGuiElements[0].outValue = UI_AardvardGpioOutValue0;
g_aardvardI2cGpioGuiElements[0].inValue = UI_AardvardGpioInValue0;
g_aardvardI2cGpioGuiElements[1] = Array();
g_aardvardI2cGpioGuiElements[1].mode = UI_AardvardGpioMode1;
g_aardvardI2cGpioGuiElements[1].outValue = UI_AardvardGpioOutValue1;
g_aardvardI2cGpioGuiElements[1].inValue = UI_AardvardGpioInValue1;
g_aardvardI2cGpioGuiElements[2] = Array();
g_aardvardI2cGpioGuiElements[2].mode = UI_AardvardGpioMode2;
g_aardvardI2cGpioGuiElements[2].outValue = UI_AardvardGpioOutValue2;
g_aardvardI2cGpioGuiElements[2].inValue = UI_AardvardGpioInValue2;
g_aardvardI2cGpioGuiElements[3] = Array();
g_aardvardI2cGpioGuiElements[3].mode = UI_AardvardGpioMode3;
g_aardvardI2cGpioGuiElements[3].outValue = UI_AardvardGpioOutValue3;
g_aardvardI2cGpioGuiElements[3].inValue = UI_AardvardGpioInValue3;
g_aardvardI2cGpioGuiElements[4] = Array();
g_aardvardI2cGpioGuiElements[4].mode = UI_AardvardGpioMode4;
g_aardvardI2cGpioGuiElements[4].outValue = UI_AardvardGpioOutValue4;
g_aardvardI2cGpioGuiElements[4].inValue = UI_AardvardGpioInValue4;
g_aardvardI2cGpioGuiElements[5] = Array();
g_aardvardI2cGpioGuiElements[5].mode = UI_AardvardGpioMode5;
g_aardvardI2cGpioGuiElements[5].outValue = UI_AardvardGpioOutValue5;
g_aardvardI2cGpioGuiElements[5].inValue = UI_AardvardGpioInValue5;
