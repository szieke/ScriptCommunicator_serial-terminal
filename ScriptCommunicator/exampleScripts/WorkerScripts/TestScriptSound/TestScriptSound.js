/********************************************************************************************************
* The script demonstrates the usage of the ScriptCommunicator ScriptSound object (play wav files).
*******************************************************************dd*************************************/


var soundObject = scriptThread.createSoundObject("yes-2.wav");
soundObject.setLoops(5);
soundObject.play();

while(!soundObject.isFinished())
{
	scriptThread.sleepFromScript(100);
}

scriptThread.stopScript();