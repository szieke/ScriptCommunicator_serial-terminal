/**********************************************************************
This is an example sequence send script which can be added to a sequence (send window), It demonstrates
the possibilities of sequence send scripts.

This script can not be executed by the "normal" script interface (script window).
***********************************************************************/


//This function appends a user input and a checksum at data.
function sendData(data)
{	
	var resultArray = seq.showGetIntDialog("Enter value","value:",0, 0, 100, 1);
	
	if(resultArray[0] == 1)
	{//OK button pressed.
		//Append the input value.
		data = conv.addStringToArray(data, "  input: " + resultArray[1] + "\n")	
	}
	else
	{//No OK button pressed.
		
		//The sequence will not be sent if an empty array is returned.
		data = Array();
	}
	
	seq.appendTextToConsole("sequenceSendScriptWithDialog.js send: " + data, true, false);
	
	return data;
}