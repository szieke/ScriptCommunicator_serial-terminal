/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates how to receive a file.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script receive file stopped");
}

//The dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}
	
function ClearFilePushButtonClickedSlot()
{
	if(UI_FilePathLineEdit.text() != "")
	{
		if(scriptThread.deleteFile(UI_FilePathLineEdit.text(), false))
		{
			bytesWritten = 0;
			receivedData.length = 0;
			UI_InformationLabel.setText("bytes written: " + bytesWritten);
			
			//Create empty file.
			scriptFile.writeBinaryFile(UI_FilePathLineEdit.text(), false, Array(), false);
		}
		else
		{
			scriptThread.messageBox("Critical", "error", 'could not clear (delete) file')
			UI_ReceiveFileDialog.raise();
		}
	}
	else
	{
		scriptThread.messageBox("Critical", "error", 'file path is empty')
		UI_ReceiveFileDialog.raise();
	}

}
function openFilePushButtonClickedSlot()
{
	var path = scriptThread.showFileDialog (true, "Open File", "","Files (*)")
	if(path != "")
	{
		UI_FilePathLineEdit.setText(path);
		bytesWritten = 0;
		receivedData.length = 0;
		UI_InformationLabel.setText("bytes written: " + bytesWritten);
		

		if(scriptFile.checkFileExists(path, false))
		{
			if(!scriptFile.deleteFile(path, false))
			{
				scriptThread.messageBox("Information", "information", "could not access (delete) file: " + path);
			}
			//Create empty file.
			scriptFile.writeBinaryFile(path, false, Array(), false);
		}
	}
	UI_ReceiveFileDialog.raise();

}

function canMessagesReceived(type, id, timeStamp, data)
{
	for(var index = 0; index < type.length; index++)
	{
		dataReceivedSlot(data[index]);
	}
}

function dataReceivedSlot(data)
{
	if(UI_FilePathLineEdit.text() != "")
	{
		for(var i = 0; i < data.length; i++)
		{
			receivedData.push(data[i]);
		}
		
		if(receivedData.length > 100000)
		{
			writteData();
		}	
	}

}
function writteData()
{
	timer.stop();
	if(scriptFile.writeBinaryFile(UI_FilePathLineEdit.text(), false, receivedData, false))
	{
		bytesWritten += receivedData.length;
		UI_InformationLabel.setText("bytes written: " + bytesWritten);
		receivedData.length = 0;
	}
	timer.start(200);
}
function timerSlot()
{
	if(receivedData.length != 0)
	{
		writteData();
	}
}


scriptThread.appendTextToConsole('script receive file started');


var bytesWritten = 0;
var receivedData = Array();

UI_ReceiveFileDialog.finishedSignal.connect(dialogFinished);
UI_OpenFilePushButton.clickedSignal.connect(openFilePushButtonClickedSlot)
UI_ClearFilePushButton.clickedSignal.connect(ClearFilePushButtonClickedSlot)
scriptInf.dataReceivedSignal.connect(dataReceivedSlot);
scriptInf.canMessagesReceivedSignal.connect(canMessagesReceived);
UI_InformationLabel.setText("bytes written: " + bytesWritten);
var timer = scriptThread.createTimer();
timer.timeoutSignal.connect(timerSlot);
timer.start(200);


