/********************************************************************************************************
* The script demonstrates how to receive and show cyclic data in a GUI. To receive data following
* must be done:
* run CyclicSend.js in a second ScriptCommunicator instance
* connect the two ScriptCommunicator instances
*
**********************************************************************************************/

/*
 * This function is call if a data frame has been received.
 *
 * @param dataFrame
 *	The received data frame
 */
function frameReceived(dataFrame)
{
	if(dataFrame[0] == 1)
	{
		UI_Val1ComboBox.setCurrentText(String.fromCharCode(dataFrame[1]));
	}
	else if(dataFrame[0] == 2)
	{
		UI_Value2CheckBox.setChecked(dataFrame[1]);
	}
	else if(dataFrame[0] == 3)
	{
		UI_Value3SpinBox.setValue(dataFrame[1]);	
	}
	else if(dataFrame[0] == 4)
	{
		var intValue = dataFrame[1] << 24;
		intValue += dataFrame[2] << 16;
		intValue += dataFrame[3] << 8;
		intValue += dataFrame[4];
		var floatValue = parseFloat(intValue)
		
		UI_Value4DoubleSpinBox.setValue(floatValue / 100 );	
	}
	else if(dataFrame[0] == 5)
	{
		UI_Value5Dial.setValue(dataFrame[1]);	
		plotWidget.addDataToGraph(plotWidgetGraph1Index,  plotXCounter,  dataFrame[1]);
		plotWidget.addDataToGraph(plotWidgetGraph2Index,  plotXCounter,  dataFrame[1] + 10);
		
		plotWindow.addDataToGraph(plotWindowGraph1Index,  plotXCounter,  dataFrame[1]);
		plotWindow.addDataToGraph(plotWindowGraph2Index,  plotXCounter,  dataFrame[1] + 10);
		
		plotXCounter++;
	}
	else if(dataFrame[0] == 6)
	{
		var stringValue = "";
		for(var i = 0; i < 8; i++)
		{
			stringValue += String.fromCharCode(dataFrame[i]);
		}
		UI_Value6LineEdit.setText(stringValue);	
	}
	else if(dataFrame[0] == 7)
	{
		UI_Value7HorizontalSlider.setValue(dataFrame[1]);	
	}
	else if(dataFrame[0] == 8)
	{
		UI_tableWidget.setText(0, 0, dataFrame[1].toString())		
	}
	else if(dataFrame[0] == 9)
	{
		UI_tableWidget.setText(0, 1, dataFrame[1].toString())		
	}
	else if(dataFrame[0] == 10)
	{
		UI_tableWidget.setText(1, 0, dataFrame[1].toString())		
	}
	else if(dataFrame[0] == 11)
	{
		UI_tableWidget.setText(1, 1, dataFrame[1].toString())		
	}
	else if(dataFrame[0] == 12)
	{
		UI_tableWidget.setText(2, 0, dataFrame[1].toString())		
	}
	else if(dataFrame[0] == 13)
	{
		UI_tableWidget.setText(2, 1, dataFrame[1].toString())		
	}


}

/*
 * Is call if data has been received with the main interface.
 *
 * @param data
 * The received data.
 */
function dataReceivedSlot(data)
{
	appendByteArrayAtByteArray(receivedData, data, data.length);
	
	do
	{
		var startIndex = receivedData.indexOf(0xff);
		var frameFound = false;
		
		if(startIndex != -1)
		{
			var endIndex = receivedData.indexOf(0xfe, startIndex);
			
			if(endIndex != -1)
			{
				//Remove all data in front of the start byte
				receivedData.splice(0, startIndex + 1);
				endIndex = receivedData.indexOf(0xfe);
				
				var frameData = receivedData.splice(0, endIndex);
				frameFound = true;
				frameReceived(frameData);
				
				//Remove the end char (0xfe).
				receivedData.splice(0, 1);
			}
		}
	}while(frameFound);
}

/*
 * Is call if a can or several message has been received with the main interface.
 *
 * @param types
 *	The can message type.
 * @param ids
 *	The can ids.
 * @param timeStamps
 *	 Timestamps for the received can messages (milliseconds since the first message has been received). 
 * @param data	
 *	The received data.
 */
function canMessagesReceived(types, ids, timeStamps, data)
{
	for(var index = 0; index < types.length; index++)
	{
		var frameData = Array();
		frameData[0] = ids[index] & 0xff;
		appendByteArrayAtByteArray(frameData, data[index], data[index].length);
		
		//Call frameReceived for every received can message.
		frameReceived(frameData);
	}
}
//Is called if the plot window has been closed.
function plotWindowClosedSlot()
{
	scriptThread.appendTextToConsole("plot window has been closed");
	scriptThread.stopScript()
}

//Is called if the user clicks the button.
function clearButtonPressed()
{
	plotXCounter = 0;
	plotWindow.clearGraphs();
	plotWidget.clearGraphs();
}
function plotWindowMousePress(xValue, yValue, button)
{
	scriptThread.appendTextToConsole('plotWindowMousePress: ' + xValue + ", " + yValue + ", " + button);
}
function plotWidgetMousePress(xValue, yValue, button)
{
	scriptThread.appendTextToConsole('plotWidgetMousePress: ' + xValue + ", " + yValue + ", " + button);
}

scriptThread.loadScript("Helper.js")
var receivedData = Array();
scriptInf.dataReceivedSignal.connect(dataReceivedSlot)

//connect the dataReceivedSlot function to the dataReceivedSignal signal
scriptInf.canMessagesReceivedSignal.connect(canMessagesReceived)

//Create the plot widget und the corresponding graphs.
var plotWidget = UI_PlotGroupBox.addPlotWidget();
plotWidget.setAxisLabels("x axis plot 1", "y axis plot 1");
plotWidget.showLegend(true);
plotWidget.setInitialAxisRanges(1000, -10, 150);
var plotWidgetGraph1Index = plotWidget.addGraph("blue", "solid", "value 5");
var plotWidgetGraph2Index = plotWidget.addGraph("red", "solid", "value 5+10");
plotWidget.clearButtonPressedSignal.connect(clearButtonPressed)
plotWidget.showHelperElements(true, true, true, true, true, true, true, 100, true);
plotWidget.plotMousePressSignal.connect(plotWidgetMousePress)
plotWidget.setUpdateInterval(100);
var plotXCounter = 0;

//Create the plot window und the corresponding graphs.
var plotWindow = scriptThread.createPlotWindow();
plotWindow.show();
plotWindow.setWindowTitle("plot window created/used by script");
plotWindow.setAxisLabels("x axis plot 1", "y axis plot 1");
plotWindow.showLegend(true);
plotWindow.setInitialAxisRanges(1000, -10, 150);
var plotWindowGraph1Index = plotWindow.addGraph("blue", "solid", "value 5");
var plotWindowGraph2Index = plotWindow.addGraph("red", "solid", "value 5+10");
plotWindow.showHelperElements(true, true, true, true, true, true, true, 100, true);
plotWindow.closedSignal.connect(plotWindowClosedSlot)
plotWindow.clearButtonPressedSignal.connect(clearButtonPressed)
plotWindow.plotMousePressSignal.connect(plotWindowMousePress)
plotWindow.setUpdateInterval(100);
UI_tableWidget.setColumnWidth(0, 70)









