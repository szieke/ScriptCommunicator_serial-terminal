/*************************************************************************
This file contains all device/eeprom specific functions.
***************************************************************************/

function getPageSize()
{
	var pageSize = 0;
	if(UI_EepromType.currentText() == "STM M24M01")
	{
		pageSize = 256;
	}
	else
	{
		UI_Log.append("unknown eeprom type: " + UI_EepromType.currentText());
	}
	
	return pageSize;
}


function i2cMasterReadWrite(flags, slaveAddress, eepromAdress, bytesToRead,  dataToSend)
{
	var sendArray = Array();
	var waitTimeAfterWrite = 5;
	var hasSucceded = false;

	if(UI_EepromType.currentText() == "STM M24M01")
	{
		sendArray[0] = (eepromAdress >> 8) & 0xff;
		sendArray[1] = eepromAdress & 0xff;
		slaveAddress += (eepromAdress >> 16) & 0x1;
		waitTimeAfterWrite = 5;
		hasSucceded = true;
	}
	else
	{
		UI_Log.append("unknown eeprom type: " + UI_EepromType.currentText());
		var hasSucceded = false;
	}

	if(hasSucceded)
	{
		if(dataToSend.length > 0)
		{
			sendArray = sendArray.concat(dataToSend);
		}
		
		hasSucceded = g_interface.i2cMasterReadWrite(flags, slaveAddress, bytesToRead,  sendArray);
		
		if(dataToSend.length > 0)
		{
			//Wait until the Eeprom is writting.
			scriptThread.sleepFromScript(waitTimeAfterWrite);
		}
	}
	
	return hasSucceded;
}