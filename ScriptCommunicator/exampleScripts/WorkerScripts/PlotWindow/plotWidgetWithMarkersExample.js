/*************************************************************************
This worker script (worker scripts can be added in the script window) 
demonstrates the usage of the plotwidget.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script plot widget with markers stopped");
}

//The dialog is closed.
function mainWindowFinished(e)
{
	scriptThread.appendTextToConsole('script plot widget with markers finished');
	scriptThread.stopScript()
}

//Is called if the user clicks on the run button.
function runClicked()
{
	timer.start(100);
	plotWidget.setAutoUpdateEnabled(true);
	UI_pushButtonRun.setEnabled(false);
	UI_pushButtonStop.setEnabled(true);
	
	UI_MainWindow.showMessage("Data acquisition is running.", 0);
}

//Is called if the user clicks on the stop button.
function stopClicked()
{
	timer.stop();
	plotWidget.setAutoUpdateEnabled(false);
	UI_pushButtonRun.setEnabled(true);
	UI_pushButtonStop.setEnabled(false);
	
	UI_MainWindow.showMessage("Data acquisition is stopped.", 2000);
}

//Helper function to retrive data point form graphValues.
function getProcessYValueFromGrap(graphIdx, xValue)
{
	var dataPoints = plotWidget.getDataFromGraph(graphIdx, xValue, 1);
	
	if (dataPoints.length == 0)
		return "na";
	
	return dataPoints[0].y.toFixed(2);
}

//Is called if the user clicks on the plot.
function plotWindowMousePress(xValue, yValue, button)
{
	if ((button != 1) && (button != 2))
		return;

	var column = button - 1;
	
	var graphValues = [];
	graphValues.push(xValue.toFixed(2));
	graphValues.push(getProcessYValueFromGrap(plotWidgetGraph1Index, xValue));
	graphValues.push(getProcessYValueFromGrap(plotWidgetGraph2Index, xValue));
	graphValues.push(getProcessYValueFromGrap(plotWidgetGraph3Index, xValue));
	
	for (var row=0; row<graphValues.length; row++)
	{
		var textOld = UI_tableWidgetMarkers.getText(row, column);
		var textNew = graphValues[row];
		
		var color = (textOld == textNew) ? "black" : "red";
		
		UI_tableWidgetMarkers.setText(row, column, textNew);
		UI_tableWidgetMarkers.setCellForegroundColor(color, row, column);
		
		// calculate difference beetween second and first point
		var first = parseFloat(UI_tableWidgetMarkers.getText(row, 0));
		var second = parseFloat(UI_tableWidgetMarkers.getText(row, 1));
		
		var textOld = UI_tableWidgetMarkers.getText(row, 2);
		var textNew = (second - first).toFixed(2);
		
		color = (textOld == textNew) ? "black" : "red";
		
		UI_tableWidgetMarkers.setText(row, 2, textNew);
		UI_tableWidgetMarkers.setCellForegroundColor(color, row, 2);
	}
	
	// show marker
	var marker = (column == 0) ? plotWidgetGraphMark1Index : plotWidgetGraphMark2Index;
	
	plotWidget.removeDataRangeFromGraph(marker, -1e100, 1e100, true);
	plotWidget.addDataToGraph(marker, xValue, -1e100, true);
	plotWidget.addDataToGraph(marker, xValue, 1e100, true);
	
	// force update if disabled to view marker
	if (plotWidget.isAutoUpdateEnabled() == false)
		plotWidget.updatePlot();
	
	scriptThread.appendTextToConsole('plotWidgetMousePress: ' + xValue + ", " + yValue + ", " + button);
}

//Called periodically to add some data to the plot widget
function timeout() 
{
	//add some points to the graphs
	for (var i=0; i<20; i++)
	{
		var sinValue = 100*Math.sin(plotXCounter/40);
		var cosValue = 50*Math.cos(plotXCounter/80) - 150;
		var sawValue = plotXCounter % 200;
		
		plotWidget.addDataToGraph(plotWidgetGraph1Index,  plotXCounter, sinValue);
		plotWidget.addDataToGraph(plotWidgetGraph2Index,  plotXCounter, cosValue);
		plotWidget.addDataToGraph(plotWidgetGraph3Index,  plotXCounter, sawValue);
		
		plotXCounter++;
	}
}

scriptThread.appendTextToConsole('script plot widget with markers started');

//Register ui signals.
UI_MainWindow.finishedSignal.connect(mainWindowFinished);
UI_pushButtonRun.clickedSignal.connect(runClicked);
UI_pushButtonStop.clickedSignal.connect(stopClicked);

//Setup ui.
UI_tableWidgetMarkers.rowsCanBeMovedByUser(false);

for (var row=0; row<UI_tableWidgetMarkers.rowCount(); row++)
	for (var column=0; column<UI_tableWidgetMarkers.columnCount(); column++)
		UI_tableWidgetMarkers.setCellEditable(row, column, false);

//Create a plot widget and setup them.
var plotWidget = UI_groupBoxPlotContainer.addPlotWidget();
plotWidget.setAxisLabels("x", "y");
plotWidget.showLegend(true);
plotWidget.setInitialAxisRanges(1000, -250, 250);
plotWidget.showHelperElements(true, true, true, true, false, false, true, 100, true);
plotWidget.plotMousePressSignal.connect(plotWindowMousePress);

//Add some example graphs.
var plotWidgetGraph1Index = plotWidget.addGraph("red", "solid", "sin");
var plotWidgetGraph2Index = plotWidget.addGraph("orange", "solid", "cos");
var plotWidgetGraph3Index = plotWidget.addGraph("dodgerblue", "solid", "saw");
var plotWidgetGraphMark1Index = plotWidget.addGraph("crimson", "dash", "marker 1");
var plotWidgetGraphMark2Index = plotWidget.addGraph("limegreen", "dash", "marker 2");
plotWidget.setLineWidth(plotWidgetGraph1Index, 2);
// see https://www.w3.org/TR/SVG/types.html#ColorKeywords for full subset of color key words

var plotXCounter = 0;

//Create periodically timer which calls the function timeout.
var timer = scriptThread.createTimer()
timer.timeoutSignal.connect(timeout);





