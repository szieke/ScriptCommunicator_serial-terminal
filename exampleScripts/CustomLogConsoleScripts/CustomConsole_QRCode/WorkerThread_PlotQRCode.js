/*************************************************************************
This worker script (worker scripts can be added in the script window) 
triggers in creation of  QR code in CustomConsole_QRCoder.js.

Pier Andrea Serra, University of Sassari, Italy. 2015
***************************************************************************/

function stopScript() 
{
    scriptThread.appendTextToConsole("Printed");
}

//start the script
scriptThread.appendTextToConsole('QRCode');
scriptThread.addMessageToLogAndConsoles("QR",true);
scriptThread.stopScript();



