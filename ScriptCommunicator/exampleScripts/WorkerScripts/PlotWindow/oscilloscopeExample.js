/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
script plot window class.
***************************************************************************/
//The plot window is closed.
function plotWindowClosedSlot()
{
	scriptThread.stopScript()
}

var x = 0;
var y = 0;
var i = 0;
function timeout() 
{
	//add points to the graph

	x++;
	y++;
	
	if(y > 20)
	{
		y = 0;
	}
	
	if(x >g_xRange)
	{
		x = 0;
	}
	
	plotWindow.removeDataRangeFromGraph(plotWindowGraph1Index, x - 0.5, x + 0.5);
	plotWindow.addDataToGraph(plotWindowGraph1Index,  x,  y);
	
}

//Is called if the user clicks the clear button.
function clearButtonPressed()
{
	//Clear all data with clearGraphs.
	plotWindow.clearGraphs();
	
	x = 0;
	y = 0;
	
	//Add a zero line.
	for(var i = 0; i <= g_xRange; i++)
	{
		plotWindow.addDataToGraph(plotWindowGraph1Index,  i,  0);
	}
}

//Is called if the user clicks on the plot.
function plotWindowMousePress(xValue, yValue, button)
{
	scriptThread.appendTextToConsole('plotWindowMousePress: ' + xValue + ", " + yValue + ", " + button);
}

//Is called if the user changes the x-range textedit.
function xRangeChangedSlot(newValue)
{
	scriptThread.appendTextToConsole('xRangeChangedSlot: ' + newValue);
	
	if(newValue > g_xRange)
	{
		//Add a zero line.
		for(var i = g_xRange; i <= newValue; i++)
		{
			plotWindow.addDataToGraph(plotWindowGraph1Index,  i,  0);
		}
	}
	else
	{
		plotWindow.removeDataRangeFromGraph(plotWindowGraph1Index, newValue + 0.5, g_xRange + 0.5);
	}
	g_xRange = newValue;
}

scriptThread.appendTextToConsole('script plot window started ');

var g_xRange = 100;

//create and configure plot window 1
var plotWindow = scriptThread.createPlotWindow();
plotWindow.setWindowTitle("plot window created/used by script");
plotWindow.setAxisLabels("x axis plot 1", "y axis plot 1");
plotWindow.showLegend(true);
plotWindow.setInitialAxisRanges(g_xRange, -5, 25, false);
var plotWindowGraph1Index = plotWindow.addGraph("blue", "solid", "channel 1");


plotWindow.showHelperElements(true, true, true, true, true, true, true, 80, true);
plotWindow.show();
plotWindow.clearButtonPressedSignal.connect(clearButtonPressed)
plotWindow.closedSignal.connect(plotWindowClosedSlot)
plotWindow.setMaxDataPointsPerGraph(1000000);
plotWindow.setUpdateInterval(100);
plotWindow.plotMousePressSignal.connect(plotWindowMousePress)
plotWindow.xRangeChangedSignal.connect(xRangeChangedSlot)



//get the window positon and size of the two plot windows.
var plot1PositionAndSizelist = plotWindow.windowPositionAndSize().split(",");

//Add a zero line.
for(var i = 0; i <= g_xRange; i++)
{
	plotWindow.addDataToGraph(plotWindowGraph1Index,  i,  0);
}

//start the periodically timer which calls the function timeout
var timer = scriptThread.createTimer()
timer.timeoutSignal.connect(timeout);
timer.start(10);



