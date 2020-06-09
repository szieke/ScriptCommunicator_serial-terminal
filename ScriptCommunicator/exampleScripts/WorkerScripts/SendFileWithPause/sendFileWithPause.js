/*************************************************************************
This worker script (worker scripts can be added in the script window)  demonstrates how to send a file and wait for a response.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script send file stopped");
}

//The dialog is closed.
function dialogFinished(e)
{	
	scriptThread.stopScript()
}

function sendChunk()
{
	var fileSize = scriptThread.getFileSize(UI_FilePathLineEdit.text(), false);
	var errorOccured = false;

	g_timeoutTimer.stop();
	
	g_sendingInProgress = true;
	

	var array = scriptFile.readBinaryFile(UI_FilePathLineEdit.text(), false, g_bytesSend, 256);
	if(array.length > 0)
	{
		if(!scriptInf.sendDataArray(array))
		{
			errorOccured = true;
			g_sendingInProgress = false;
		}
				
		if(!errorOccured)
		{
			g_timeoutTimer.start(2000);
			g_bytesSend += array.length;
			UI_InformationLabel.setText("bytes send: " + g_bytesSend);
			UI_SendFileProgressBar.setValue((g_bytesSend * 100) / fileSize);
		}
	}
	else
	{
		errorOccured = true;
		g_sendingInProgress = false;
	}
	
	if(g_bytesSend >= fileSize)
	{
		g_sendingInProgress = false;
	}

	
	
	if(!g_sendingInProgress)
	{
		if(errorOccured)
		{
			UI_InformationLabel.setText("sending failed, bytes send: " + g_bytesSend);
			g_sendingInProgress = false;
		}
		else
		{
			UI_SendFileProgressBar.setValue(100);
			UI_InformationLabel.setText("sending finished, bytes send: " + g_bytesSend);
		}


		g_timeoutTimer.stop();
		UI_OpenFilePushButton.setEnabled(true);
		UI_SendFilePushButton.setEnabled(true);
	}
}

function timeoutSlot()
{
	UI_InformationLabel.setText("sending failed (timeout), bytes send: " + g_bytesSend);
	g_sendingInProgress = false;
	UI_OpenFilePushButton.setEnabled(true);
	UI_SendFilePushButton.setEnabled(true);
}

function dataRecievedSlot(data)
{
	if(g_sendingInProgress)
	{
		g_receivedData = g_receivedData.concat(data);	
		if(conv.byteArrayToString(g_receivedData).indexOf("Ready") != -1)
		{//Ready received
		
			//Delete the received data.
			g_receivedData = Array();
		
			sendChunk();
		}
	}
}

function sendFilePushButtonClickedSlot()
{
	if(scriptInf.isConnected())
	{
		if(UI_FilePathLineEdit.text() != "")
		{
			UI_OpenFilePushButton.setEnabled(false);
			UI_SendFilePushButton.setEnabled(false);
			UI_SendFileProgressBar.reset();
			UI_InformationLabel.setText("read file");
			
			UI_SendFileProgressBar.setValue(0);
			g_bytesSend = 0;
			g_receivedData = Array();
			
				
			
			var fileSize = scriptThread.getFileSize(UI_FilePathLineEdit.text(), false);
			if(fileSize > 0)
			{
				g_receivedData = Array();
				sendChunk();
			}
			else
			{
				UI_InformationLabel.setText("could not read file");
			}
		
		}
	}
	else
	{
		scriptThread.messageBox("Critical", "error while sending", 'main interface is not connected')
		UI_SendFileDialog.raise();
	}
}
	
function openFilePushButtonClickedSlot()
{
	var path = scriptThread.showFileDialog (false, "Open File", "","Files (*)")
	if(path != "")
	{
		UI_FilePathLineEdit.setText(path);
	}
	UI_SendFileDialog.raise();
}


scriptThread.appendTextToConsole('script send file started');

UI_SendFileDialog.finishedSignal.connect(dialogFinished);
UI_OpenFilePushButton.clickedSignal.connect(openFilePushButtonClickedSlot)
UI_SendFilePushButton.clickedSignal.connect(sendFilePushButtonClickedSlot)
UI_SendFileProgressBar.setRange(0, 100);
UI_SendFileProgressBar.setValue(0);

var g_bytesSend = 0;
var g_sendingInProgress = false;
var g_receivedData = Array();
var g_timeoutTimer = scriptThread.createTimer();
g_timeoutTimer.timeoutSignal.connect(timeoutSlot);

scriptInf.dataReceivedSignal.connect(dataRecievedSlot);

