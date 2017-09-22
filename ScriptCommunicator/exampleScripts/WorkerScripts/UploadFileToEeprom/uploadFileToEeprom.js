/*************************************************************************
This script uploads a file to an I2C eeprom (connected to a AardvarkI2cSpi interface). Supported EEPROMs:
- STM M24M01
***************************************************************************/

//The name of the settings file.
const SETTINGS_FILE_NAME = scriptThread.getUserDocumentsFolder() + "/ScriptCommunicator/uploadEepromSettings.txt"

//Is called if this script shall be exited.
function stopScript() 
{
	saveUiSettings(SETTINGS_FILE_NAME);
	g_interface.disconnect();
	scriptThread.appendTextToConsole("script has been stopped");
}


/*
 * Reads data from the EEPROM.
 * @param i2cAddress The I2C address of the EEPROM (only the upper 7 Bits).
 * @param memoryAddress The memory address.
 * @param numberOfBytesToRead The number of bytes which shall be read.
 * @param logData True of the read data  shall be shown in the log window.
 * @param pageSize The page size of the EEPROM.
 * @return The read bytes (byte array).
 */
function read(i2cAddress, memoryAddress, numberOfBytesToRead, logData, pageSize)
{
	var readBytes = Array();
	var numberOfReadBytes = 0;
	
	while(numberOfReadBytes < numberOfBytesToRead)
	{
		var tmpMemoryAddress = memoryAddress + numberOfReadBytes;
		var bytesInPageLeft = pageSize - (tmpMemoryAddress % pageSize);
		
		if((numberOfReadBytes + bytesInPageLeft) > numberOfBytesToRead)
		{
			bytesInPageLeft = numberOfBytesToRead - numberOfReadBytes;
		}
		
		var tmpReadBytes  = readPage(i2cAddress, tmpMemoryAddress, bytesInPageLeft, logData);
		
		readBytes = readBytes.concat(tmpReadBytes);
		numberOfReadBytes += tmpReadBytes.length;
		incrementProgress();
		
		if(tmpReadBytes.length != bytesInPageLeft)
		{
			break;
		}
	}
	
	return readBytes;
}

/*
 * Reads bytes from an EEPROM page.
 * @param i2cAddress The I2C address of the EEPROM (only the upper 7 Bits).
 * @param memoryAddress The memory address.
 * @param numberOfBytesToRead The number of bytes which shall be read.
 * @param logData True of the read data  shall be shown in the log window.
 * @return The read bytes (byte array).
 */
function readPage(i2cAddress, memoryAddress, numberOfBytesToRead, logData)
{
	var readBytes = Array();
	
	if(i2cMasterReadWrite(0, i2cAddress,  memoryAddress, numberOfBytesToRead,  Array()))
	{
		readBytes = g_interface.i2cMasterReadLastReceivedData();
		if(readBytes.length != 0)
		{
			if(logData)
			{
				UI_Log.append("I2C data received: address=0x" + i2cAddress.toString(16) + " data=" + conv.byteArrayToHexString(readBytes));
			}
		}
	}
	else
	{
		UI_Log.append("execute I2C failed (read page): address=0x" + i2cAddress.toString(16));
	}
	
	return readBytes;
}

/*
 * Writes data to the EEPROM.
 * @param i2cAddress The I2C address of the EEPROM (only the upper 7 Bits).
 * @param memoryAddress The memory address.
 * @param dataToSend The bytes which shall be written.
 * @param logData True of the read data  shall be shown in the log window.
 * @param pageSize The page size of the EEPROM.
 * @return True on success.
 */
function write(i2cAddress, memoryAddress, dataToSend, logData, pageSize)
{
	var writtenBytes = 0;
	var bytesToWrite = dataToSend.length;
	var hasSucceeded = false;
	var counter = 1;
	
	while(writtenBytes < bytesToWrite)
	{
		var tmpMemoryAddress = memoryAddress + writtenBytes;
		var bytesInPageLeft = pageSize - (tmpMemoryAddress % pageSize);
		var tmpData = dataToSend.slice(writtenBytes,  writtenBytes + bytesInPageLeft);
		
		if(UI_AddFillBytes.isChecked() )
		{
			//Add fill bytes.
			while(tmpData.length < pageSize)
			{
				tmpData.push(parseInt(UI_FillBytes.text()));
			}
		}
		
		if(writePage(i2cAddress, tmpMemoryAddress, tmpData, logData))
		{
			writtenBytes += tmpData.length;
			incrementProgress();
			hasSucceeded = true;
		}
		else
		{
			hasSucceeded = false;
			break;
		}
	}
	
	return hasSucceeded;
}

/*
 * Writes data to a page of the EEPROM.
 * @param i2cAddress The I2C address of the EEPROM (only the upper 7 Bits).
 * @param memoryAddress The memory address.
 * @param dataToSend The bytes which shall be written.
 * @param logData True of the read data  shall be shown in the log window.
 * @param pageSize The page size of the EEPROM.
 * @return True on success.
 */
function writePage(i2cAddress, memoryAddress, dataToSend, logData)
{
	var hasSucceeded = true;
	
	if(i2cMasterReadWrite(0, i2cAddress,  memoryAddress, 0,  dataToSend))
	{
		hasSucceeded = true;
		if(logData)
		{
			UI_Log.append("execute I2C: address=0x" + i2cAddress.toString(16) + " data= " + conv.byteArrayToHexString(dataToSend));
		}
	}
	else
	{
		hasSucceeded = false;
		UI_Log.append("execute I2C failed (write page): address=0x" + i2cAddress.toString(16));
	}
	
	return hasSucceeded;
}

//Connects to an Aardvark interface.
function connect()
{
	var hasSucceeded = true;
	
	/************************fill the AardvarkI2cSpiSettings structure**********************/
	var settings = g_interface.getInterfaceSettings();
	settings.devicePort =  parseInt(UI_AardvarkPort.text());
	settings.deviceMode =  0;//I2C master
	settings.device5VIsOn =  true;
	
	settings.i2cBaudrate =  400;//400 kBit.
	settings.i2cPullupsOn = true;
	
	for(var i = 0; i < 	settings.pinConfigs.length; i++)
	{
		settings.pinConfigs[i].isInput = true ;
		settings.pinConfigs[i].withPullups = false;
		settings.pinConfigs[i].outValue = false;
	}
	/*********************************************************************************************************/
	
	if(!g_interface.connectToDevice(settings))
	{
		UI_Log.append("connect error (wrong port?)");
		hasSucceeded = false;
	}
	
	return hasSucceeded;
}

//Is called if the dialog is closed.
function UI_DialogFinished()
{
	scriptThread.stopScript()
}

/*
 * Sets the value of the progress bar.
 * @param progress The new value
 */
function setProgress(progress)
{
	currentProgressValue = progress;
	UI_Progress.setValue(progress);
}

//Increments the value of the progress bar.
function incrementProgress()
{
	currentProgressValue++;
	UI_Progress.setValue(currentProgressValue);
}

//Fills the file list with the files from the selected folder.
function fillFileList()
{
	var files = scriptFile.readDirectory(UI_Folder.text(), false, false, true, false);
	
	var filesString = "";
	for(var i = 0; i < files.length; i++)
	{
		filesString += files[i];
	}
	
	if(g_savedFoundFiles != filesString)
	{//The content of the selected folder has been changed.
		
		UI_File.clear();
		g_savedFoundFiles = filesString;
		for(var i = 0; i < files.length; i++)
		{
			UI_File.addItem(files[i]);
		}
	}
}

//Is called if the user clicks the select button.
function folderSlot()
{
	var folder = scriptThread.showDirectoryDialog("select folder", UI_Folder.text(), UI_Dialog.getWidgetPointer());
	if(folder.length > 0)
	{
		UI_Folder.setText(folder);
		fillFileList();
	}
}

/*
 * Sets the GUI state.
 * @param enable True for enable.
 */
function setGuiState(enable)
{
	UI_SelectFolder.setEnabled(enable);
	UI_Upload.setEnabled(enable);
	UI_File.setEnabled(enable);
	UI_I2CAddress.setEnabled(enable);
	UI_EepromAddress.setEnabled(enable);
	UI_EepromType.setEnabled(enable);
	UI_DetectedDevices.setEnabled(enable);
	UI_AardvarkPort.setEnabled(enable);
}

//Is called if the user clicks the upload button.
function uploadSlot()
{
	var logData = false;
	
	var eepromAddress = parseInt(UI_EepromAddress.text()) 
	var i2cAddress = parseInt(UI_I2CAddress.text()) 
	var pageSize = getPageSize();
	
	if((UI_File.currentText().length > 0) && (pageSize != 0))
	{
		setGuiState(false);
		UI_Log.append("connecting (aardvark interface port: " + parseInt(UI_AardvarkPort.text()) + ")");
		
		if(connect())
		{
			var writtenData = scriptFile.readBinaryFile(UI_File.currentText(), false);
			if(writtenData.length > 0)
			{
				var pagesToWrite = (writtenData.length / pageSize) + 1;
				setProgress(0);
				UI_Progress.setMaximum(pagesToWrite * 2);
				
				UI_Log.append("starting upload");
				
				if(write(i2cAddress, eepromAddress, writtenData, logData, pageSize))
				{
				
					UI_Progress.setValue(pagesToWrite);
					var readData = read(i2cAddress, eepromAddress, writtenData.length, logData, pageSize);
					g_interface.disconnect();
					var dataIsOk = true;

					for(var i = 0; i < writtenData.length; i++)
					{
						if(writtenData[i] != readData[i])
						{
							UI_Log.append("read data is unequal to the written data, index:" + i);
							
							if(logData)
							{
								UI_Log.append("writtenData:" + conv.byteArrayToHexString(writtenData));
								UI_Log.append("readData:" + conv.byteArrayToHexString(readData));
							}
							dataIsOk = false;
							break;
						}
					}
					
					setProgress(pagesToWrite * 2);

					if(dataIsOk)
					{
						UI_Log.append(writtenData.length + " bytes successfully written");
					}
					
				}//if(write(i2cAddress, eepromAddress, writtenData, logData, pageSize))

			}//if(writtenData.length > 0)
			else
			{
				UI_Log.append("could not read: " + UI_File.currentText());
			}
			g_interface.disconnect();
			
		}//if(connect())
		
		setGuiState(true);

	}//if((UI_File.currentText().length > 0) && (pageSize != 0))

}

/*
 * Saves the GUI settings.
 * @param fileName The file name.
 */
function saveUiSettings(fileName)
{
	settings = "UI_Folder=" + UI_Folder.text() + "\r\n";
	settings += "UI_I2CAddress=" + UI_I2CAddress.text() + "\r\n";
	settings += "UI_EepromType=" + UI_EepromType.currentText() + "\r\n";
	settings += "UI_EepromAddress=" + UI_EepromAddress.text() + "\r\n";
	settings += "UI_AardvarkPort=" + UI_AardvarkPort.text() + "\r\n";
	settings += "UI_FillBytes=" + UI_FillBytes.text() + "\r\n";
	settings += "UI_AddFillBytes=" + (UI_AddFillBytes.isChecked() ? 1 : 0 )+ "\r\n";

	scriptFile.writeFile(fileName, false, settings, true);
}

/*
 * Returns a value of a sting array.
 * @param stringArray The string array.
 * @param key The key.
 * @return The value.
 */
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

/*
 * Loads the GUI settings.
 * @param fileName The file name.
 */
function loadUiSettings(fileName)
{
	if(scriptFile.checkFileExists(fileName, false))
	{
		var settings = scriptFile.readFile(fileName, false);
		var stringArray = settings.split("\r\n");
		
		UI_Folder.setText(getValueOfStringArray(stringArray, "UI_Folder"));
		UI_I2CAddress.setText(getValueOfStringArray(stringArray, "UI_I2CAddress"));
		UI_EepromAddress.setText(getValueOfStringArray(stringArray, "UI_EepromAddress"));
		UI_EepromType.setCurrentText(getValueOfStringArray(stringArray, "UI_EepromType"));
		UI_AardvarkPort.setText(getValueOfStringArray(stringArray, "UI_AardvarkPort"));
		UI_FillBytes.setText(getValueOfStringArray(stringArray, "UI_FillBytes"));
		UI_AddFillBytes.setChecked((getValueOfStringArray(stringArray, "UI_AddFillBytes") == 1) ? true : false)
	}
	
}

//Is called if the user clicks the detects button.
function detectAardvarkI2cSpiDevicesSlot()
{
	UI_Log.append("scanning for aardvark devices")
	UI_Log.append(g_interface.detectDevices());
}

/*
 * Is called if the file in the file list has been changed.
 * @param newFile The new file name.
 */
function currentFileChangedSlot(newFile)
{
	//Set the tool tip to the new file name.
	UI_File.setToolTip(newFile, -1);
}

/*
 * Is called if the selected folder has been changed.
 * @param newFolder The new folder name.
 */
function currentFolderChangedSlot(newFolder)
{
	//Set the tool tip to the new folder name.
	UI_Folder.setToolTip(newFolder, -1);
}

UI_Dialog.hide();
scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(UI_DialogFinished);
UI_Upload.clickedSignal.connect(uploadSlot);
UI_SelectFolder.clickedSignal.connect(folderSlot);
UI_DetectedDevices.clickedSignal.connect(detectAardvarkI2cSpiDevicesSlot);
UI_File.currentTextChangedSignal.connect(currentFileChangedSlot);
UI_Folder.textChangedSignal.connect(currentFolderChangedSlot);

UI_Folder.setText(scriptThread.getScriptFolder());
loadUiSettings(SETTINGS_FILE_NAME);
scriptThread.loadScript("./helper/devices.js");

var g_interface = scriptInf.aardvarkI2cSpiCreateInterface();
var currentProgressValue = 0;

var g_savedFoundFiles = "";

UI_I2CAddress.addIntValidator(0, 127);
UI_EepromAddress.addIntValidator(0, 0xffffff);
UI_AardvarkPort.addIntValidator(0, 127);
UI_FillBytes.addIntValidator(0, 255);
fillFileList();
scriptThread.addTabsToMainWindow(UI_TabWidget);
detectAardvarkI2cSpiDevicesSlot();

currentFileChangedSlot(UI_File.currentText());
currentFolderChangedSlot(UI_Folder.text());

var g_folderChangeTimer = scriptThread.createTimer();
g_folderChangeTimer.timeoutSignal.connect(fillFileList);
g_folderChangeTimer.start(500);


