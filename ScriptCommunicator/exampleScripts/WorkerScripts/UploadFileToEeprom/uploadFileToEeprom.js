/*************************************************************************
This script uploads a file to an I2C eeprom (connected to a AardvarkI2cSpi interface).
***************************************************************************/


const AARDVARD_I2C_SPI_GPIO_COUNT = 6;
const SETTINGS_FILE_NAME = scriptThread.getUserDocumentsFolder() + "/ScriptCommunicator/uploadEepromSettings.txt"

//Is called if this script shall be exited.
function stopScript() 
{
	saveUiSettings(SETTINGS_FILE_NAME);
	g_interface.disconnect();
	scriptThread.appendTextToConsole("script has been stopped");
}

function read(slaveAddress, eepromAdress, bytesToRead, straceData, pageSize)
{
	var readBytesArray = Array();
	var readBytes = 0;
	
	while(readBytes < bytesToRead)
	{
		var tmpEepromAdress = eepromAdress + readBytes;
		var bytesInPageLeft = pageSize - (tmpEepromAdress % pageSize);
		
		if((readBytes + bytesInPageLeft) > bytesToRead)
		{
			bytesInPageLeft = bytesToRead - readBytes;
		}
		
		var tmpReadBytes  = readPage(slaveAddress, tmpEepromAdress, bytesInPageLeft, straceData);
		
		readBytesArray = readBytesArray.concat(tmpReadBytes);
		readBytes += tmpReadBytes.length;
		incrementProgress();
		
		if(tmpReadBytes.length != bytesInPageLeft)
		{
			break;
		}
	}
	
	return readBytesArray;
}

function readPage(slaveAddress, eepromAdress, bytesToRead, straceData)
{
	var readBytes = Array();
	
	if(i2cMasterReadWrite(0, slaveAddress,  eepromAdress, bytesToRead,  Array()))
	{
		readBytes = g_interface.i2cMasterReadLastReceivedData();
		if(readBytes.length != 0)
		{
			if(straceData)
			{
				UI_Log.append("I2C data received: address=0x" + slaveAddress.toString(16) + " data=" + conv.byteArrayToHexString(readBytes));
			}
		}
	}
	else
	{
		UI_Log.append("execute I2C failed (read page): address=0x" + slaveAddress.toString(16));
	}
	
	return readBytes;
}

function write(slaveAddress, eepromAdress, dataToSend, straceData, pageSize)
{
	var writtenBytes = 0;
	var bytesToWrite = dataToSend.length;
	var hasSucceeded = false;
	
	while(writtenBytes < bytesToWrite)
	{
		var tmpEepromAdress = eepromAdress + writtenBytes;
		var bytesInPageLeft = pageSize - (tmpEepromAdress % pageSize);
		var tmpData = dataToSend.slice(writtenBytes,  writtenBytes + bytesInPageLeft);
		
		if(writePage(slaveAddress, tmpEepromAdress, tmpData, straceData))
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
function writePage(slaveAddress, eepromAdress, dataToSend, straceData)
{
	var hasSucceeded = true;
	
	if(i2cMasterReadWrite(0, slaveAddress,  eepromAdress, 0,  dataToSend))
	{
		hasSucceeded = true;
		if(straceData)
		{
			UI_Log.append("execute I2C: address=0x" + slaveAddress.toString(16) + " data= " + conv.byteArrayToHexString(dataToSend));
		}
	}
	else
	{
		hasSucceeded = false;
		UI_Log.append("execute I2C failed (write page): address=0x" + slaveAddress.toString(16));
	}
	
	return hasSucceeded;
}

function connect()
{
	var hasSucceeded = true;
	
	/************************Create the AardvarkI2cSpiSettings structure**********************/
	settings = Array();
	settings.devicePort =  parseInt(UI_AardvarkPort.text());
	settings.deviceMode =  0;//I2C master
	settings.device5VIsOn =  true;
	
	settings.i2cBaudrate =  400;
	settings.i2cSlaveAddress =  0;
	settings.i2cPullupsOn = true;
	
	settings.spiPolarity = 0;
	settings.spiSSPolarity =  0;
	settings.spiBitorder =  0;
	settings.spiPhase =  0;
	settings.spiBaudrate =  1000;

	settings.pinConfigs =  Array();
	for(var i = 0; i < AARDVARD_I2C_SPI_GPIO_COUNT; i++)
	{
		settings.pinConfigs[i] = Array();	
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
function UI_DialogFinished(e)
{
	scriptThread.stopScript()
}

function setProgress(progress)
{
	currentProgressValue = progress;
	UI_Progress.setValue(progress);
}

function incrementProgress()
{
	currentProgressValue++;
	UI_Progress.setValue(currentProgressValue);
}

function fillFileList()
{
	var files = scriptFile.readDirectory(UI_Folder.text(), false, false, true, false);
	UI_File.clear();
	for(var i = 0; i < files.length; i++)
	{
		UI_File.addItem(files[i]);
	}
}

function folderSlot()
{
	var folder = scriptThread.showDirectoryDialog("select folder", UI_Folder.text(), UI_Dialog.getWidgetPointer());
	
	if(folder.length > 0)
	{
		UI_Folder.setText(folder);
		fillFileList();
	}
}
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


function uploadSlot()
{
	var straceData = false;
	
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
				
				if(write(i2cAddress, eepromAddress, writtenData, straceData, pageSize))
				{
				
					UI_Progress.setValue(pagesToWrite);

					var readData = read(i2cAddress, eepromAddress, writtenData.length, straceData, pageSize);
					g_interface.disconnect();
					var dataIsOk = true;

					for(var i = 0; i < writtenData.length; i++)
					{
						if(writtenData[i] != readData[i])
						{
							UI_Log.append("read data is unequal to the written data, index:" + i);
							
							if(straceData)
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
					
				}//if(writtenData.length > 0)
				else
				{
					UI_Log.append("could not read: " + UI_File.currentText());
				}
			}//if(writtenData.length > 0)
			else
			{
				UI_Log.append("reading " + fileName + " failed");
			}
			g_interface.disconnect();
			
		}//if(connect())
		
		setGuiState(true);

	}//if((UI_File.currentText().length > 0) && (pageSize != 0))

}

//Save the GUI settings.
function saveUiSettings(fileName)
{
	settings = "UI_Folder=" + UI_Folder.text() + "\r\n";
	settings += "UI_I2CAddress=" + UI_I2CAddress.text() + "\r\n";
	settings += "UI_EepromType=" + UI_EepromType.currentText() + "\r\n";
	settings += "UI_EepromAddress=" + UI_EepromAddress.text() + "\r\n";
	settings += "UI_AardvarkPort=" + UI_AardvarkPort.text() + "\r\n";

	scriptFile.writeFile(fileName, false, settings, true);
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
	if(scriptFile.checkFileExists(fileName, false))
	{
		var settings = scriptFile.readFile(fileName, false);
		var stringArray = settings.split("\r\n");
		
		UI_Folder.setText(getValueOfStringArray(stringArray, "UI_Folder"));
		UI_I2CAddress.setText(getValueOfStringArray(stringArray, "UI_I2CAddress"));
		UI_EepromAddress.setText(getValueOfStringArray(stringArray, "UI_EepromAddress"));
		UI_EepromType.setCurrentText(getValueOfStringArray(stringArray, "UI_EepromType"));
		UI_AardvarkPort.setText(getValueOfStringArray(stringArray, "UI_AardvarkPort"));
	}
	
}

function detectAardvarkI2cSpiDevicesSlot()
{
	UI_Log.append("scanning for aardvark devices")
	UI_Log.append(g_interface.detectDevices());
}

function currentFileChangedSlot(newFile)
{
	UI_File.setToolTip(newFile, -1);
}

function currentFolderChangedSlot(newFolder)
{
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

UI_I2CAddress.addIntValidator(0, 127);
UI_EepromAddress.addIntValidator(0, 0xffffff);
UI_AardvarkPort.addIntValidator(0, 127);
fillFileList();
scriptThread.addTabsToMainWindow(UI_TabWidget);
detectAardvarkI2cSpiDevicesSlot();

currentFileChangedSlot(UI_File.currentText());
currentFolderChangedSlot(UI_Folder.text());


