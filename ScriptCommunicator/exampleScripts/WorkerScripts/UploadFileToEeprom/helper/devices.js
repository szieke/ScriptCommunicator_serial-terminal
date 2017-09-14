/*************************************************************************
This file contains all device/eeprom specific functions.
***************************************************************************/

/*
 * Returns athe page size of the selected EEPROM..
 * @return The page size.
 */
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

/*
 * Calls g_interface.i2cMasterReadWrite with the device/EEPROM specific 
 * data (memory address bytes and I2C address)
 * @param flags The flags for g_interface.i2cMasterReadWrite.
 * @param i2cAddress The I2C address of the EEPROM (only the upper 7 Bits).
 * @param memoryAddress The memory address.
 * @param numberOfBytesToRead The number of bytes which shall be read.
 * @param dataToSend The bytes which shall be written.
 * @return True on success.
 */
function i2cMasterReadWrite(flags, i2cAddress, memoryAddress, numberOfBytesToRead,  dataToSend)
{
	var sendArray = Array();
	var waitTimeAfterWrite = 5;
	var hasSucceded = false;

	if(UI_EepromType.currentText() == "STM M24M01")
	{
		sendArray[0] = (memoryAddress >> 8) & 0xff;
		sendArray[1] = memoryAddress & 0xff;
		i2cAddress += (memoryAddress >> 16) & 0x1;
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
		
		hasSucceded = g_interface.i2cMasterReadWrite(flags, i2cAddress, numberOfBytesToRead,  sendArray);
		
		if(dataToSend.length > 0)
		{
			//Wait until the Eeprom is writting.
			scriptThread.sleepFromScript(waitTimeAfterWrite);
		}
	}
	
	return hasSucceded;
}