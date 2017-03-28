/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
process functions.
****************************************************************************************/

function getExecutableName()
{
	var result = "TestProcess";
	if(scriptThread.productType() == "windows")
	{
		result += ".exe" 
	}
	else if(scriptThread.productType() == "osx")
	{
		result += "_Mac";
	}
	else
	{//Linux
		if(scriptThread.currentCpuArchitecture() == "x86_64")
		{
		  result += "_Linux64";
		}
		else
		{
			result += "_Linux32";
		}
	}
	
	return result;
}


var arguments = Array("arg1","arg2","arg3","arg4");
var executable = scriptFile.getScriptFolder()+"/" + getExecutableName();
scriptThread.appendTextToConsole("starting: " + executable);


var process = scriptThread.createProcessAsynchronous(executable, arguments);

if(typeof process == 'undefined')
{
	scriptThread.appendTextToConsole("could not start: " + executable);
	if(executable.indexOf("_Linux") != -1)
	{
		scriptThread.appendTextToConsole("has " + executable + " the correct permissions?");
	}
}
else
{
	var data = scriptThread.stringToArray("test standard in\n");
	scriptThread.writeToProcessStdin(process, data);


	var res = scriptThread.readAllStandardOutputFromProcess(process, true, String('\n').charCodeAt(0), 10000);
	scriptThread.appendTextToConsole("stdout data:  " + conv.byteArrayToString(res).replace("\n", ""));
	
	
	var err = scriptThread.readAllStandardErrorFromProcess(process, true, String('\n').charCodeAt(0), 10000);
	scriptThread.appendTextToConsole("stderr data: " + conv.byteArrayToString(err).replace("\n", ""));
	
	scriptThread.waitForFinishedProcess(process);
	var exitCode = scriptThread.getProcessExitCode(process)
	scriptThread.appendTextToConsole("exitCode: " + exitCode);
}
scriptThread.stopScript();
