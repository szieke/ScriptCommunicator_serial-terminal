/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the script plot window class.
***************************************************************************/
//The plot window is closed.
function plotWindowClosedSlot()
{
	scriptThread.stopScript()
}

var x1 = 0;
var y1 = 0;
function timeout1() 
{
	//add points to the graphs

	y1++;
	x1++;
	
	plotWindow1.addDataToGraph(plotWindowGraph1Index,  x1,  y1);
	plotWindow1.addDataToGraph(plotWindowGraph2Index,  x1 + 10,  y1);
	
	if(y1 > 20)
	{
		y1 = 0;
	}
	
}


var y2 = 0;
function timeout2() 
{
	//add points to the graphs
	
	var tmp = new Date(Date.now());
	var x2 = tmp.getTime() / 1000;
	y2++;
	
	plotWindow2.addDataToGraph(plotWindow2Graph1Index,  x2,  y2);
	plotWindow2.addDataToGraph(plotWindow2Graph2Index,  x2 ,  y2 + 2);
	
	if(y2 > 20)
	{
		y2 = 0;
	}
	
}




//Is called if the user clicks the button.
function clearButtonPressed()
{
	//Remove data with removeDataRangeFromGraph (this line only shows the usage of removeDataRangeFromGraph).
	plotWindow1.removeDataRangeFromGraph(plotWindowGraph1Index, 0, x + 10);
	
	//Clear all data with clearGraphs.
	plotWindow1.clearGraphs();
	plotWindow2.clearGraphs();
	
	x1 = 0;
	y1 = 0;
	y2 = 0;
}

//Is called if the user clicks on the plot in window 1.
function plotWindowMousePress(xValue, yValue, button)
{
	scriptThread.appendTextToConsole('plotWindowMousePress: ' + xValue + ", " + yValue + ", " + button);
}

//Is called if the user clicks on the plot in window 2.
function plotWindow2MousePress(xValue, yValue, button)
{
	scriptThread.appendTextToConsole('plotWindow2MousePress: ' + xValue + ", " + yValue + ", " + button);
}

scriptThread.appendTextToConsole('script plot window started ');


//create and configure plot window 1
var plotWindow1 = scriptThread.createPlotWindow();
plotWindow1.setWindowTitle("plot window created/used by script");
plotWindow1.setAxisLabels("x axis plot 1", "y axis plot 1");
plotWindow1.showLegend(true);
plotWindow1.setInitialAxisRanges(50, 0, 30);
var plotWindowGraph1Index = plotWindow1.addGraph("blue", "solid", "graph 1");
plotWindow1.setScatterStyle(plotWindowGraph1Index, "Cross", 5);
plotWindow1.setLineStyle(plotWindowGraph1Index, "None");

var plotWindowGraph2Index = plotWindow1.addGraph("red", "solid", "graph 2");
plotWindow1.showHelperElements(true, true, true, true, true, true, true, 80, true);
plotWindow1.show();
plotWindow1.clearButtonPressedSignal.connect(clearButtonPressed)
plotWindow1.closedSignal.connect(plotWindowClosedSlot)
plotWindow1.setMaxDataPointsPerGraph(1000000);
plotWindow1.setUpdateInterval(100);
plotWindow1.plotMousePressSignal.connect(plotWindowMousePress)


//create and configure plot window 2
var plotWindow2 = scriptThread.createPlotWindow();
plotWindow2.setWindowTitle("plot window created/used by script");
plotWindow2.setAxisLabels("x axis plot 2", "y axis plot 2");
plotWindow2.showLegend(true);
plotWindow2.setInitialAxisRanges(60, 0, 30, false);
var plotWindow2Graph1Index = plotWindow2.addGraph("green", "solid", "graph 1");
plotWindow2.setScatterStyle(plotWindow2Graph1Index, "Circle", 5);
plotWindow2.setLineStyle(plotWindow2Graph1Index, "None");

var plotWindow2Graph2Index = plotWindow2.addGraph("black", "dot", "graph 2");
plotWindow2.showHelperElements(true, true, true, true, true, true, true, 80, true);
plotWindow2.show();
plotWindow2.closedSignal.connect(plotWindowClosedSlot)
plotWindow2.clearButtonPressedSignal.connect(clearButtonPressed)
plotWindow2.setMaxDataPointsPerGraph(1000000);
plotWindow2.setUpdateInterval(100);
plotWindow2.plotMousePressSignal.connect(plotWindow2MousePress)

//Show the date time at the x-axis.
plotWindow2.showDateTimeAtXAxis("hh:mm:ss\nddd.MMM.yy");

//QLocale::Language::English=31,  QLocale::Country::UnitedKingdom=224.
plotWindow2.setLocale(31, 224);


//get the window positon and size of the two plot windows.
var plot1PositionAndSizelist = plotWindow1.windowPositionAndSize().split(",");
var plot2PositionAndSizelist = plotWindow2.windowPositionAndSize().split(",");

//move plot window 2 to the left side of plot window 1
plot2PositionAndSizelist[0] = (plot1PositionAndSizelist[0] - plot1PositionAndSizelist[2] - 10).toString();
var string =  plot2PositionAndSizelist[0] + "," + plot2PositionAndSizelist[1] + "," + plot2PositionAndSizelist[2] + "," + plot2PositionAndSizelist[3];
plotWindow2.setWindowPositionAndSize(string);

//start the periodically timer which calls the function timeout
var timer1 = scriptThread.createTimer()
timer1.timeoutSignal.connect(timeout1);
timer1.start(100);

var timer2 = scriptThread.createTimer()
timer2.timeoutSignal.connect(timeout2);
timer2.start(1000);



