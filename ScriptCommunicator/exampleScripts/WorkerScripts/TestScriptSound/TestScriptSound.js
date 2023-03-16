/********************************************************************************************************
* The script demonstrates the usage of the ScriptCommunicator ScriptSound object (play wav files).
*******************************************************************dd*************************************/


var soundObject = scriptThread.createSoundObject("yes-2.wav");

//Play the sound three times.
for(var i = 0; i < 3; i++)
{
	soundObject.play();
	while(!soundObject.isFinished())
	{
		scriptThread.sleep(1);
	}
}
scriptThread.stopScript();
