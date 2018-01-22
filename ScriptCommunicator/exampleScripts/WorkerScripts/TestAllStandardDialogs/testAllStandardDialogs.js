/**********************************************************************
This script tests all WorkerScript standard dialogs.
***********************************************************************/

var data = Array();	
var input = scriptThread.showTextInputDialog("Title", "label", "initial text");
if(input != "")
{//OK button pressed.
	data = scriptThread.addStringToArray(data, input + '\n')
}

input = scriptThread.showMultiLineTextInputDialog("Enter name", "item name", "newItem");
if(input != "")
{//OK button pressed.
	data = scriptThread.addStringToArray(data, input + '\n')
}

input = scriptThread.showGetItemDialog("Enter name", "item name", Array("item1", "item2"), 1, true);
if(input != "")
{//OK button pressed.
	data = scriptThread.addStringToArray(data, input + '\n')
}

var resultArray = scriptThread.showGetIntDialog("Enter value","value:",0, 0, 100, 1);
if(resultArray[0] == 1)
{//OK button pressed.	
	data = scriptThread.addStringToArray(data, "int:" + parseInt(resultArray[1]) + '\n');
}

resultArray = scriptThread.showGetDoubleDialog("Enter value", "value", 10, 0, 20, 1);
if(resultArray[0] >= 1.0)
{//OK button pressed.
	data = scriptThread.addStringToArray(data, "double:" + parseFloat(resultArray[1]) + '\n');
}

scriptThread.messageBox("Information", "Title", "test text");

if(scriptThread.showYesNoDialog("Information", "Title", "yes or no?"))
{//Yes clicked.
	data = scriptThread.addStringToArray(data, "yes/no:yes" + '\n');
}
else
{
	data = scriptThread.addStringToArray(data, " yes/no:no" + '\n');
}


resultArray = scriptThread.showColorDialog(1,2,3,4,true);
if(resultArray[0])
{//OK clicked.
	data = scriptThread.addStringToArray(data, "color:" + parseInt(resultArray[1]) + "," + parseInt(resultArray[2]) + "," + parseInt(resultArray[3]) + "," + parseInt(resultArray[4]) + '\n');
}

input = scriptThread.showFileDialog(false, "select a file", "", "");
if(input != "")
{//One file selected.
	data = scriptThread.addStringToArray(data, input + '\n')
}

input = scriptThread.showOpenFileNamesDialog("select one or more files", "", "");
if(input.length != 0)
{//One ore more files selected.
	
	for(var i = 0; i < input.length; i++)
	{
		data = scriptThread.addStringToArray(data, input[i] + '\n');
	}
}


if(!scriptInf.sendDataArray(data))
{
	scriptThread.messageBox("Critical", "Error", "sending failed");
}

scriptThread.stopScript();

