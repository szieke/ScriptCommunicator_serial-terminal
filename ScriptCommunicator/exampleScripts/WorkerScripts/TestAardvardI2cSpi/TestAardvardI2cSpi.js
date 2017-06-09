/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
aardvard I2C/SPI interface (used as main interface and as separate interface).
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script test aardvard I2C/SPI stopped");
}

scriptThread.appendTextToConsole('script test aardvard I2C/SPI started');



