/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
PCAN interface.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
	pcan2.close();
    scriptThread.appendTextToConsole("script has been stopped");
}

//The dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}


var sendData = 0;
function sendMessages()
{
	for(var i = 0xff; i < (0xff + 2); i++)
	{
		scriptInf.sendCanMessage(2, i, Array(0,0,0,0,(sendData >> 24) & 0xff,(sendData >> 16) & 0xff,(sendData >> 8) & 0xff,sendData & 0xff));
		sendData++;
	}
}
function canMessagesReceived(type, id, timeStamp, data)
{
	//scriptThread.appendTextToConsole(type);
	if(UI_showReceivedMessages.isChecked())
	{
		for(var index = 0; index < type.length; index++)
		{
			var idString = id[index].toString(16);
			for(var i = idString.length; i < 8; i++)
			{
				idString = "0" + idString;
			}
			
			var timeStampString = timeStamp[index].toString();
			for(var i = timeStampString.length; i < 10; i++)
			{
				timeStampString = "0" + timeStampString;
			}
			
	
			UI_textEdit.append("mInter type: " + type[index] + "  id: " + idString + "  timeStamp: " + timeStampString + 
					"  data: " + scriptThread.byteArrayToHexString(data[index]));
		}
				
				
		
	}

}

function pcan2MessagesReceived(type, id, timeStamp, data)
{
	for(var index = 0; index < type.length; index++)
	{
		var idString = id[index].toString(16);
		for(var i = idString.length; i < 8; i++)
		{
			idString = "0" + idString;
		}
		
		var timeStampString = timeStamp[index].toString();
		for(var i = timeStampString.length; i < 10; i++)
		{
			timeStampString = "0" + timeStampString;
		}
				
		if(UI_showReceivedMessages.isChecked())
		{
				UI_textEdit.append("pcan2  type: " + type[index] + "  id: " + idString + "  timeStamp: " + timeStampString + 
						"  data: " + scriptThread.byteArrayToHexString(data[index]));
		}
			
		pcan2.sendCanMessage(type[index], id[index], data[index]);
		
	}
		
}

function statusTimerSlot()
{
	UI_StatusLabel.setText(pcan2.getStatusString());
	UI_StatusRawLabel.setText("Raw status: " + pcan2.getCurrentStatus());
}


scriptThread.appendTextToConsole('script has started');

UI_Dialog.finishedSignal.connect(dialogFinished);

scriptInf.disconnect()
if(!scriptInf.connectPcan(1, 1000))
{
	scriptThread.messageBox("Critical", "error", 'error while connecting main interface')
}

scriptInf.canMessagesReceivedSignal.connect(canMessagesReceived);

var SendTimer = scriptThread.createTimer();
SendTimer.timeoutSignal.connect(sendMessages);
SendTimer.start(500);

var pcan2 = scriptInf.createPcanInterface();
pcan2.canMessagesReceivedSignal.connect(pcan2MessagesReceived);

if(!pcan2.open(2, 1000, true, false) || !pcan2.isConnected())
{
	scriptThread.messageBox("Critical", "error", 'could not open pcan interface');
}

if(!pcan2.setFilter(true, 0, 0x1fffffffff))
{
	scriptThread.messageBox("Critical", "error", 'setFilter failed');
}


var statusTimer = scriptThread.createTimer();
statusTimer.timeoutSignal.connect(statusTimerSlot);
statusTimer.start(500);

var value;
if(pcan2.setCanParameter(0x15, 1))
{
	var result = pcan2.getCanParameter(0x15, value);
	if((result[0] == 0) || (result[1] != 1))
	{
			scriptThread.messageBox("Critical", "error", 'get/setPcanParameterfailed')
	}
}
else
{
	scriptThread.messageBox("Critical", "error", 'setValue failed')
}	

