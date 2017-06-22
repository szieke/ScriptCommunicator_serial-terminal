/*************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the loading of a dynamic link library. The libray must export following function
extern "C" Q_DECL_EXPORT void init(QScriptEngine* engine);
For details the the testDll Project.
***************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script load libraray stopped");
}

//The dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

function sendDataArray(data) 
{
	var success = true;
	scriptThread.appendTextToConsole("sendDataArray called: " + data);
	
	if(!scriptInf.sendDataArray(data, 0, 0))
	{
		success = scriptThread.appendTextToConsole("sendig failed (check connection status)");
	}
	return success;
}

//the user has pressed the button
function pushButtonClicked()
{
	var text = UI_lineEdit.text();
	var array = text.split(" ");
	var byteArray = Array();
	
	for(var i = 0; i < array.length; i++)
	{	
		byteArray[i] = Number(array[i]);
	}
	
	scriptThread.appendTextToConsole("");
	
	
	var result = TestDll.appendChecksum(byteArray);
	scriptThread.appendTextToConsole("appendChecksum result: " + result);
	
	
	var result = TestDll.appendChecksumAndSend(byteArray);
	scriptThread.appendTextToConsole("appendChecksumAndSend result: " + result);
}
function main(fileName)
{

	scriptThread.appendTextToConsole('script load libraray started');

	var uiFileLoaded = true;
	try
	{
		//check if the user interface file has been loaded
		//if the user interface file is not loaded an exeption will be generated
		if(UI_lineEdit)
		{
		}
	}
	catch(e)
	{
		uiFileLoaded = false;
	}

	if(!uiFileLoaded)
	{//No ui file given.
		uiFileLoaded = scriptThread.loadUserInterfaceFile("loadLibrary.ui");
	}

	if(uiFileLoaded)
	{
		scriptThread.loadLibrary(fileName);
		UI_lineEdit.setText("1 2 3 4 5 6 7 8 9");
		
		UI_Dialog.finishedSignal.connect(dialogFinished);
				UI_pushButton.clickedSignal.connect(pushButtonClicked)
	}
	else
	{
		scriptThread.messageBox("Critical", "no correct ui file", 'the nessesary ui file has not been loaded or is not correct')
		scriptThread.stopScript()
	}

}

if(scriptThread.productType() == "windows")
{
	main("TestDll.dll");
}
else if(scriptThread.productType() == "osx")
{
	main("libTestDll.dylib");
}
else
{//Linux
	if(scriptThread.currentCpuArchitecture() == "x86_64")
	{
		main("libTestDll.so");
	}
	else
	{
		main("libTestDll_32_bit.so");
	}
}




