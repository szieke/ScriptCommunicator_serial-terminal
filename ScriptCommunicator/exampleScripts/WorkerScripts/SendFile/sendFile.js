/*************************************************************************
This worker script (worker scripts can be added in the script window)  demonstrates how to send a file.
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
			var succeeded = false;
				
			
			var fileSize = scriptThread.getFileSize(UI_FilePathLineEdit.text(), false);
			if(fileSize > 0)
			{
				var send = 0;
				while(send < fileSize)
				{
					var array = scriptFile.readBinaryFile(UI_FilePathLineEdit.text(), false, send, 20000);
					if(array.length > 0)
					{
						if(scriptInf.isConnectedWithCan())
						{
							if(!scriptInf.sendCanMessage(2, 0x1, array))
							{
								succeeded =  false;
								break;
							}
						}
						else
						{
							if(!scriptInf.sendDataArray(array))
							{
								succeeded = false;
								break;
							}
						}
						send += array.length;
						UI_InformationLabel.setText("bytes send: " + send);
						UI_SendFileProgressBar.setValue((send * 100) / fileSize);
						succeeded = true;
						scriptThread.sleepFromScript(10);
					}
					else
					{
						break;
					}
				}
				
				if(succeeded)
				{
					UI_SendFileProgressBar.setValue(100);
					UI_InformationLabel.setText("sending finished, bytes send: " + send);
				}
				else
				{
					UI_InformationLabel.setText("sending failed, bytes send: " + send);
				}
			}
			else
			{
				UI_InformationLabel.setText("could not read file");
			}
			
			UI_OpenFilePushButton.setEnabled(true);
			UI_SendFilePushButton.setEnabled(true);
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


