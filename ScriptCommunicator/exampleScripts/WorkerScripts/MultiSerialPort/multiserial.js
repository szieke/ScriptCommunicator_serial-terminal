/***************************************************************************************
This script can open several serial ports and prints their data in selected format to console and table. 
Data sizes are printed in serial plot window which acts as a timeline. If you press the timeline, this will select
a corresponding cell in the table. The script is useful when you need to relate data from two or more
serial ports (such as two Arduinos communicating with each other).

The script is based on AdditionalInterface script (author unknown to me). The changes were done by
Marko Mikkonen.
****************************************************************************************/

function setColumnLabels()
{
	UI_Table.setHorizontalHeaderLabel(0, "Time");
	UI_Table.setHorizontalHeaderLabel(1, "Data");
	UI_Table.setHorizontalHeaderLabel(2, "Received/Sent");
	UI_Table.setHorizontalHeaderLabel(3, "Port");
}

//Is called if the user has pressed the clear button.
function ClearButtonPressed()
{
	UI_TextEdit.clear();
	g_consoleData = [];
	for(var i = 0; i<g_bytesInConsoleData.length; i++) g_bytesInConsoleData[i] = 0;
	plotWindow.clearGraphs();
	UI_Table.clear();
	UI_Table.setRowCount(0);
	setColumnLabels();
	start = 0;
	
	if(serialPorts.length == 0) plotWindow.removeAllGraphs(); //it's not (yet) possible to remove just one graph
}

//Is called if this script shall be exited.
function stopScript() 
{
	saveUiSettings();
	scriptThread.appendTextToConsole("script stopped");
}


//Is called if the user closes the user interface.
function UI_DialogFinished()
{
	//Stop this script.
	scriptThread.stopScript()
}


//Returns a value from stringArray (values are stored in a key/value string,
//eg. 'UI_dataBitsBox=8')
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

//Checks the version of ScriptCommunicator.
function checkVersion()
{
	var versionIsOk = false;
	try
	{
		versionIsOk = scriptThread.checkScriptCommunicatorVersion("05.10");
	}
	catch(e)
	{
	}
	
	if(!versionIsOk)
	{
		scriptThread.messageBox("Critical", "version is too old",
								 "The current version of ScriptCommunicator is to old to execute this script.\n" +
									"The latest version of ScriptCommunicator can be found here:\n" +
									"http://sourceforge.net/projects/scriptcommunicator/");						
		scriptThread.stopScript();
	}
}

 // Next three functions generate vibrant, "evenly spaced" colours (i.e. no clustering). This is ideal for creating easily distinguishable vibrant markers in Google Maps and other apps.
//Based on code by Adam Cole, 2011-Sept-14
// HSV to RBG adapted from: http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript

var numOfSteps = 10;

function getRed(step)
{
    var r;
    var h = step / numOfSteps;
    var i = ~~(h * 6);
    var f = h * 6 - i;
    var q = 1 - f;
    switch(i % 6){
        case 0: r = 1; break;
        case 1: r = q; break;
        case 2: r = 0; break;
        case 3: r = 0; break;
        case 4: r = f; break;
        case 5: r = 1; break;
    }
    return (~ ~(r * 255));
}

function getGreen(step)
{
    var g;
    var h = step / numOfSteps;
    var i = ~~(h * 6);
    var f = h * 6 - i;
    var q = 1 - f;
    switch(i % 6){
        case 0: g = f; break;
        case 1: g = 1; break;
        case 2: g = 1; break;
        case 3: g = q; break;
        case 4: g = 0; break;
        case 5: g = 0; break;
    }
    return (~ ~(g * 255));
}

function getBlue(step)
{
    var b;
    var h = step / numOfSteps;
    var i = ~~(h * 6);
    var f = h * 6 - i;
    var q = 1 - f;
    switch(i % 6){
        case 0: b = 0; break;
        case 1: b = 0; break;
        case 2: b = f; break;
        case 3: b = 1; break;
        case 4: b = 1; break;
        case 5: b = q; break;
    }
    return (~ ~(b * 255));
}

var stepCoef = 2;

function getPortColorRed(serialPortsIndex)
{
	return getRed(serialPortsIndex*stepCoef);
}

function getPortColorGreen(serialPortsIndex)
{
	return getGreen(serialPortsIndex*stepCoef);
}

function getPortColorBlue(serialPortsIndex)
{
	return getBlue(serialPortsIndex*stepCoef);
}

var colorCoef = 0.8;

function getPortSentColorRed(serialPortsIndex)
{
	return getRed(serialPortsIndex)*colorCoef;
}

function getPortSentColorGreen(serialPortsIndex)
{
	return getGreen(serialPortsIndex)*colorCoef;
}

function getPortSentColorBlue(serialPortsIndex)
{
	return getBlue(serialPortsIndex)*colorCoef;
}

function getPortColorHTML(serialPortsIndex)
{
	var color = "#";
	color += ("00"+getPortColorRed(serialPortsIndex).toString(16)).slice(-2);
	color += ("00"+getPortColorGreen(serialPortsIndex).toString(16)).slice(-2);
	color += ("00"+getPortColorBlue(serialPortsIndex).toString(16)).slice(-2);
	return color;
}

function getPortSentColorHTML(serialPortsIndex)
{
	var color = "#";
	color += ("00"+getPortSentColorRed(serialPortsIndex).toString(16)).slice(-2);
	color += ("00"+getPortSentColorGreen(serialPortsIndex).toString(16)).slice(-2);
	color += ("00"+getPortSentColorBlue(serialPortsIndex).toString(16)).slice(-2);
	return color;
}

var g_consoleData  = [];
var g_bytesInConsoleData = [];

function addData(data, received, serialPortsIndex)
{
	g_consoleData[g_consoleData.length] = Array(data, received, serialPortsIndex, Date.now());
	
	var i = 0;
	while(g_bytesInConsoleData[serialPortsIndex] > 50000)
	{
		if(g_consoleData[i][2] == serialPortsIndex)
		{
			g_bytesInConsoleData[serialPortsIndex] -= g_consoleData[i][0].length;
			g_consoleData[i].splice(i, 1);
		}
		else
		{
			i++;
		}
	}
	
	addConsoleData();
	addTableData();
	addPlotData();
}

function addConsoleData()
{
	var consoleString;
	var serialPortsIndex = g_consoleData[g_consoleData.length-1][2];
	
	if(g_consoleData[g_consoleData.length-1][1] ) //received
	{
		consoleString = "<span style=\"color:" + getPortColorHTML(serialPortsIndex) + ";\">";
	}
	else
	{
		consoleString = "<span style=\"color:" + getPortSentColorHTML(serialPortsIndex) + ";\">" ;
	}

	if(UI_ShowAscii.isChecked())
	{
		consoleString += UI_TextEdit.replaceNonHtmlChars(scriptThread.byteArrayToString(g_consoleData[g_consoleData.length-1][0]).replace(/\r\n/g, "\n")) ;
	}
	else if(UI_ShowHex.isChecked())
	{
		consoleString += scriptThread.byteArrayToHexString(g_consoleData[g_consoleData.length-1][0]);
	}
	else
	{
		consoleString += byteArrayToDecimalString(g_consoleData[g_consoleData.length-1][0]);
	}
				
	if(g_consoleData[g_consoleData.length-1][1] )
	{
		consoleString = consoleString.replace(/<br>/g, "</span><br><span style=\"color:"+getPortColorHTML(serialPortsIndex)+";\">");
	}
	else
	{
		consoleString = consoleString.replace(/<br>/g, "</span><br><span style=\"color:"+getPortSentColorHTML(serialPortsIndex)+";\">");
	}
	consoleString += "</span>";
		
	var list = consoleString.split("<br>");
	
	UI_TextEdit.insertHtml(list[0]);
	
	for(var i = 1; i < list.length; i++)
	{
		UI_TextEdit.append(list[i]);
	}
}

function getTimeString(ms)
{
	var date = new Date(ms);
	return date.toLocaleTimeString('en-GB') + "." + ("000"+date.getMilliseconds().toString(10)).slice(-3);
}

function addTableData()
{
	var consoleString;
	var serialPortsIndex = g_consoleData[g_consoleData.length-1][2];
	var datestr = getTimeString(g_consoleData[g_consoleData.length-1][3]);
	
	if(UI_Table.columnCount() == 0)
	{
		for(var i=0; i<4; i++)
		{
			UI_Table.insertColumn(i);
		}
		setColumnLabels();
	}	
	
	if(UI_ShowAscii.isChecked())
	{
		consoleString = scriptThread.byteArrayToString(g_consoleData[g_consoleData.length-1][0]).replace(/^[\r\n]+|\.|[\r\n]+$/g, "").replace(/\r\n/g, "\n") ;
	}
	else if(UI_ShowHex.isChecked())
	{
		consoleString = scriptThread.byteArrayToHexString(g_consoleData[g_consoleData.length-1][0]);
	}
	else
	{
		consoleString = byteArrayToDecimalString(g_consoleData[g_consoleData.length-1][0]);
	}
				
	var list = consoleString.split("\n");
	var row;

	for(var i = 0; i < list.length; i++)
	{
		if(list[i].length)
		{
			UI_Table.insertRow(UI_Table.rowCount());
			row = UI_Table.rowCount()-1;

			UI_Table.setText(row, 0, datestr);
	
			if(g_consoleData[g_consoleData.length-1][1]) //received
			{
				for(var j=0; j<UI_Table.columnCount(); j++)
				{
					UI_Table.setCellBackgroundColorRgb(row, j, getPortColorRed(serialPortsIndex), getPortColorGreen(serialPortsIndex), getPortColorBlue(serialPortsIndex)); //(getPortColorHTML(serialPortsIndex), row, j);
				}
				UI_Table.setText(row, 2, "received");
			}
			else //sent
			{
				for(var j=0; j<UI_Table.columnCount(); j++)
				{
					UI_Table.setCellBackgroundColorRgb(row, j, getPortSentColorRed(serialPortsIndex), getPortSentColorGreen(serialPortsIndex), getPortColorBlue(serialPortsIndex)); //setCellBackgroundColor(getPortSentColorHTML(serialPortsIndex), row, j);
				}
				UI_Table.setText(row, 2, "sent");
			}

			UI_Table.setText(row, 1, list[i]);
			UI_Table.setText(row, 3, serialPorts[serialPortsIndex].portName());
		}
	}
	
	UI_Table.resizeColumnToContents(1);
	UI_Table.scrollToRow(UI_Table.rowCount() - 1);//Scroll to the end of the table.
}

var start = 0;

function addPlotData()
{
	if(start == 0)
	{
		start = g_consoleData[g_consoleData.length-1][3];
	}
	var serialPortsIndex = g_consoleData[g_consoleData.length-1][2];
	plotWindow.addDataToGraph(plotWindowGraphs[serialPortsIndex],  g_consoleData[g_consoleData.length-1][3] - start,  g_consoleData[g_consoleData.length-1][0].length);
}

function zeroPlots()
{
	if(start > 0)
	{
		for(var serialPortsIndex=0; serialPortsIndex<serialPorts.length; serialPortsIndex++)
		{
			for(var j=g_consoleData.length-1; j>=0; j--)
			{
				if(g_consoleData[j][2] == serialPortsIndex && (Date.now()-g_consoleData[j][3])>100)
				{
					plotWindow.addDataToGraph(plotWindowGraphs[serialPortsIndex], Date.now()-start,  0);
					break;
				}
			}
		}
	}
}

function plotWindowMousePress(xValue, yValue, mouseButton)
{
	var timestr;
	for(var i=0; i<g_consoleData.length-2; i++)
	{
		if(g_consoleData[i][3]<=(xValue+start) && g_consoleData[i+1][3]>=(xValue+start))
		{
			timestr = getTimeString(g_consoleData[i][3]);
			for(var j=0; j<UI_Table.rowCount(); j++)
			{
				if(timestr == UI_Table.getText(j,0))
				{
					UI_Table.selectCell(j,0);
					return;
				}
			}
		}
	}
	
	if(g_consoleData.length>0)
	{
		if(g_consoleData[g_consoleData.length-1][3]<=(xValue+start))
		{
			UI_Table.selectCell(UI_Table.rowCount()-1,0);
		}
	}
}

function hexStringToByteArray(str) { 
    var result = [];
	str.replace(/\s/g,'');
    while (str.length >= 2) { 
        result.push(parseInt(str.substring(0, 2), 16));

        str = str.substring(2, str.length);
    }

    return result;
}

function decimalStringToByteArray(str) { 
    var result = [];
	str.replace(/\s/g, " ") ;
	var decimals = str.split(" ");
	var dec;
	for(var i=0; i<decimals.length; i++)
    { 
		dec = parseInt(decimals[i]);
        if(dec >= 0 && dec <= 255) result.push(dec);
    }

    return result;
}

function byteArrayToDecimalString(data)
{
	var str = "";
	for(var i=0; i<data.length; i++)
	{
		str += data[i].toString(10);
		str += " ";
	}
	return str;
}

function DisconnectButtonPressed()
{
	var serialPortsIndex = UI_OpenPortsList.currentSelectedRow();
	if(serialPortsIndex >= 0)
	{
		serialPorts[serialPortsIndex].close();
		serialPorts.splice(serialPortsIndex, 1);
		g_bytesInConsoleData.splice(serialPortsIndex,1);

		plotWindowGraphs.splice(serialPortsIndex, 1);

		UI_OpenPortsList.removeItem(serialPortsIndex);
		
		if(serialPorts.length == 0)
		{
			UI_ShowAscii.setEnabled(true);
			UI_ShowDecimal.setEnabled(true);
			UI_ShowHex.setEnabled(true);
			UI_sendAscii.setEnabled(true);
			UI_sendDecimal.setEnabled(true);
			UI_sendHex.setEnabled(true);
		}
	}
}

//Is called if the user has pressed the connect button.
function ConnectButtonPressed()
{
	serialPorts[serialPorts.length] = scriptThread.createSerialPort();
	var serialPortsIndex = serialPorts.length-1;
	serialPorts[serialPortsIndex].readyReadSignal.connect(dataReceived);
	g_bytesInConsoleData[serialPortsIndex] = 0;

	serialPorts[serialPortsIndex].setDTR(true);
	serialPorts[serialPortsIndex].setPortName(UI_SerialPortInfoListBox.currentText());
	if (serialPorts[serialPortsIndex].open())
	{
		if (serialPorts[serialPortsIndex].setBaudRate(Number(UI_BaudRateBox.currentText()))
		 && serialPorts[serialPortsIndex].setDataBits(Number(UI_DataBitsBox.currentText()))
		 && serialPorts[serialPortsIndex].setParity(UI_ParityBox.currentText())
		 && serialPorts[serialPortsIndex].setStopBits(UI_StopBitsBox.currentText())
		 && serialPorts[serialPortsIndex].setFlowControl(UI_FlowControlBox.currentText()))
		 {
			serialPorts[serialPortsIndex].setRTS(true);
		 }
			 
		 UI_OpenPortsList.insertNewItem(UI_OpenPortsList.rowCount(), serialPorts[serialPortsIndex].portName(), "");
		 
		 plotWindowGraphs[serialPortsIndex] = plotWindow.addGraph(getPortColorHTML(serialPortsIndex), "solid", serialPorts[serialPortsIndex].portName());
		 
		 UI_OpenPortsList.setCurrentRow(UI_OpenPortsList.rowCount()-1);
		 
		 UI_ShowAscii.setEnabled(false);
		 UI_ShowDecimal.setEnabled(false);
		 UI_ShowHex.setEnabled(false);
		 UI_sendAscii.setEnabled(false);
		 UI_sendDecimal.setEnabled(false);
		 UI_sendHex.setEnabled(false);
	}
	else
	{
		serialPorts[serialPortsIndex].close();
		serialPorts.splice(serialPortsIndex,1);
		scriptThread.messageBox("Critical", 'serial port open error', 'could not open serial port: ' + UI_SerialPortInfoListBox.currentText())
	}
}

//Is called if the additional interface has received data.
function dataReceived()
{
	var data;
	for(var i = 0; i < serialPorts.length; i++)
	{
		data  = serialPorts[i].readAll();
	
		if(data.length > 0)
		{
			addData(data, true,i);
		}
	}
}

function SendButtonPressed()
{
	var serialPortsIndex = UI_OpenPortsList.currentSelectedRow();
	var data = [];
	if(serialPorts.length > 0 && serialPortsIndex >= 0)
	{
		if(UI_sendAscii.isChecked())
		{
			data = scriptThread.stringToArray(UI_sendLineEdit.text());
		}
		else if(UI_sendHex.isChecked())
		{
			data = hexStringToByteArray(UI_sendLineEdit.text());
		}
		else
		{
			data = decimalStringToByteArray(UI_sendLineEdit.text());
		}
		
		if(serialPorts[serialPortsIndex].write(data) != data.length)
		{
			DisconnectButtonPressed();
			scriptThread.messageBox("Error", "Error writing to port.", "");
		}
		else
		{
			addData(data, false, serialPortsIndex);
		}
	}
	else
	{
		scriptThread.messageBox("Error", "Sending failed: no ports are open", "");
	}
}

//Loads the saved user interface settings.
function loadUiSettings()
{
	if(scriptThread.checkFileExists(g_settingsFileName))
	{
		var settings = scriptThread.readFile(g_settingsFileName);
		var stringArray = settings.split("\r\n");
		
		UI_SerialPortInfoListBox.setCurrentText(getValueOfStringArray(stringArray, "UI_SerialPortInfoListBox"));
		UI_BaudRateBox.setCurrentText(getValueOfStringArray(stringArray, "UI_BaudRateBox"));
		UI_DataBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_DataBitsBox"));
		UI_StopBitsBox.setCurrentText(getValueOfStringArray(stringArray, "UI_StopBitsBox"));
		UI_ParityBox.setCurrentText(getValueOfStringArray(stringArray, "UI_ParityBox"));
		UI_FlowControlBox.setCurrentText(getValueOfStringArray(stringArray, "UI_FlowControlBox"));
	}	
}

//Saves the user interface settings.
function saveUiSettings()
{
	var settings = "";
	
	try
	{
		settings += "UI_SerialPortInfoListBox=" + UI_SerialPortInfoListBox.currentText() + "\r\n";
		settings += "UI_BaudRateBox=" + UI_BaudRateBox.currentText() + "\r\n";
		settings += "UI_DataBitsBox=" + UI_DataBitsBox.currentText() + "\r\n";
		settings += "UI_StopBitsBox=" + UI_StopBitsBox.currentText() + "\r\n";
		settings += "UI_ParityBox=" + UI_ParityBox.currentText() + "\r\n";
		settings += "UI_FlowControlBox=" + UI_FlowControlBox.currentText() + "\r\n";
		
		scriptThread.writeFile(g_settingsFileName, true, settings, true);
	}
	catch(e)
	{
		scriptThread.messageBox("Critical", "exception in saveUiSettings", e.toString());
	}
}

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

scriptThread.appendTextToConsole('script has started');

//The instance number of the current script.
var instanceNumber = 1;
var g_prefix = "Serial";

var g_consoleData = [];
var g_instanceName = "MultiSerial";
var g_settingsFileName = "uiSettings_MultiSerial.txt";

//Add the available serial ports.
var availablePorts = scriptThread.availableSerialPorts();
for(var i = 0; i < availablePorts.length; i++)
{
	UI_SerialPortInfoListBox.addItem(availablePorts[i]);
}


//Load the saved settings.
loadUiSettings();

//Check the version of ScriptCommunicator.
checkVersion();

UI_Dialog.finishedSignal.connect(UI_DialogFinished);


scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox);


UI_TabWidget.setTabText(0, g_instanceName);
scriptThread.addTabsToMainWindow(UI_TabWidget)

var serialPorts = [];

var isConnected = [];


var plotWindow = scriptThread.createPlotWindow();
plotWindow.setWindowTitle("Serial plot");
plotWindow.setAxisLabels("time (ms)", "bytes");
plotWindow.showLegend(true);
var plotWindowGraphs = [];
plotWindow.showHelperElements(true, true, true, true, true, true, true, 80, true);
plotWindow.show();
plotWindow.setMaxDataPointsPerGraph(1000000);
plotWindow.setUpdateInterval(100);
var plotTimer = scriptThread.createTimer();
plotWindow.clearButtonPressedSignal.connect(ClearButtonPressed);
plotWindow.setInitialAxisRanges(10000,-10,500);
plotWindow.plotMousePressSignal.connect(plotWindowMousePress);

plotTimer.interval = 100;
plotTimer.timeoutSignal.connect(zeroPlots);
plotTimer.start();

plotWindow.closedSignal.connect(UI_DialogFinished);


UI_ConnectButton.clickedSignal.connect(ConnectButtonPressed);
UI_DisconnectButton.clickedSignal.connect(DisconnectButtonPressed);
UI_ClearButton.clickedSignal.connect(ClearButtonPressed);
UI_sendButton.clickedSignal.connect(SendButtonPressed);

