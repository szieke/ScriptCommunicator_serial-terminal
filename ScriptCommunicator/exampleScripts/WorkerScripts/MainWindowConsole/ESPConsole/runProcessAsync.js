/* Helper function for those cases when you just want to execute external program 
 * and only parse its stdOut, stdErr and/or exitCode without further interaction.
 * No overloading here, all inputs has to be provided. 
 * Returns object with stdOut and stdErr as strings and exitCode as number.
 */
function runProcessAsync(program, arguments, startWaitTime, execWaitTime, workingDirectory) 
{
	var stdOut = "";
	var stdErr = "";
	var exitCode = -1;

	var process = scriptThread.createProcessAsynchronous(program, arguments, startWaitTime, workingDirectory);	

	if (typeof process == 'undefined') {
		stdErr = "Process could not start: '" + program+ "'";
		scriptThread.appendTextToConsole(stdErr);
	}
	else {
		var finished = scriptThread.waitForFinishedProcess(process, execWaitTime);
		if(!finished) {
			stdErr = "Killing process: '" + program + "'";
			scriptThread.appendTextToConsole(stdErr);
			scriptThread.killProcess(process);
		}
		else {
			// Executed as expected
			stdErr = conv.byteArrayToString(scriptThread.readAllStandardErrorFromProcess(process));	
			exitCode = scriptThread.getProcessExitCode(process);
		}
		stdOut = conv.byteArrayToString(scriptThread.readAllStandardOutputFromProcess(process));
	}

	return {
		stdOut: stdOut,
		stdErr: stdErr,
		exitCode: exitCode
	};
}