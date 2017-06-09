/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
Aardvard I2C/SPI interface (used as main interface and as separate interface).
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script test Aardvard I2C/SPI stopped");
}

scriptThread.appendTextToConsole('script test Aardvard I2C/SPI started');
