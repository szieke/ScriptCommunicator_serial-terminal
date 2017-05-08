/**********************************************************************
This script tests all SequenceSendScript dialogs.
***********************************************************************/

//This function tests all SendSequenceScript dialogs.
function sendData(data)
{	
	var input = seq.showTextInputDialog("Title", "label", "initial text");
	if(input != "")
	{//OK button pressed.
		data = conv.addStringToArray(data, input)
	}
	else
	{
		return Array();
	}
	
	input = seq.showMultiLineTextInputDialog("Enter name", "item name", "newItem");
	if(input != "")
	{//OK button pressed.
		data = conv.addStringToArray(data, " ");
		data = conv.addStringToArray(data, input)
	}
	else
	{
		return Array();
	}

	input = seq.showGetItemDialog("Enter name", "item name", Array("item1", "item2"), 1, true);
	if(input != "")
	{//OK button pressed.
		data = conv.addStringToArray(data, " ");
		data = conv.addStringToArray(data, input)
	}

	var resultArray = seq.showGetIntDialog("Enter value","value:",0, 0, 100, 1);
	if(resultArray[0] == 1)
	{//OK button pressed.	
		data = conv.addStringToArray(data, " int:" + parseInt(resultArray[1]));
	}
	else
	{
		return Array();
	}
		
	resultArray = seq.showGetDoubleDialog("Enter value", "value", 10, 0, 20, 1);
	if(resultArray[0] >= 1.0)
	{//OK button pressed.
		data = conv.addStringToArray(data, " double:" + parseFloat(resultArray[1]));
	}
	else
	{
		return Array();
	}
	
	seq.messageBox("Information", "Title", "test text");
	
	if(seq.showYesNoDialog("Information", "Title", "yes or no?"))
	{//Yes clicked.
		data = conv.addStringToArray(data, " yes/no:yes");
	}
	else
	{
		data = conv.addStringToArray(data, " yes/no:no");
	}
	
	resultArray = seq.showColorDialog(1,2,3,4,true);
	if(resultArray[0])
	{//OK clicked.
		data = conv.addStringToArray(data, " color:" + parseInt(resultArray[1]) + "," + parseInt(resultArray[2]) + "," + parseInt(resultArray[3]) + "," + parseInt(resultArray[4]));
	}
	
	seq.appendTextToConsole("sequenceSendTestAllDialogs.js send: " + data, true, false);
	
	return data;
}