

function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}
function UI_DialogFinished(e)
{
	scriptThread.stopScript()
}

function dataReceived(data)
{
	if(!g_stopSending)
	{
		g_receivedData = g_receivedData.concat(data);
		
		if(g_receivedData.length == (g_counter + "___" + g_compareString).length)
		{	
			if(scriptThread.byteArrayToString(g_receivedData) == (g_counter + "___" + g_compareString))
			{
				g_counter++;
				UI_Counter1.setText("counter: " + g_counter);
				scriptInf.sendString(g_counter + "___" + g_compareString)
			}
			else
			{
				scriptThread.appendTextToConsole("received data is not correct");
			}
			g_receivedData = Array();
		}
	}
}

function start()
{
	g_receivedData = Array();
	UI_StartButton.setEnabled(false);
	UI_StopButton.setEnabled(true);
	g_stopSending = false;
	scriptInf.sendString(g_counter + "___" + g_compareString)
	UI_Counter1.setText("counter: " + g_counter);
}

function stop()
{
	g_stopSending = true;
	scriptThread.sleepFromScript(500);
	
	UI_StartButton.setEnabled(true);
	UI_StopButton.setEnabled(false);
}


var g_counter = 0;
var g_receivedData = Array();
var g_compareString = "01kjhu476uH9abcdefghijklmnopqrstuvwxyz0123456789A(<EFGHI(((gzft!!2ghhQRST)jgtfg(jhXYZ" +
									"01234OIIHU()()zhefghijklm3h7stuvwxyz01234()&hjhu89h5uziokFGHIddde4jknjhuRSaTUVWXYZ" +
									"0434OdI3dghijklm3h7stuvwxy54de)&hjhftrHIddde4jknj4rYZjzhgTTZZhQWWWfd867s54HH9988JJ" +
									"7zhztfuiUZZikj87654JKJIHuz654efOKiozjh87565R5gjokjhFZ_ljuhuj9rd3fHUJTFi96ghdu=jkhu" +
									"86huzzgikikjZGZOKKJHZGHlpihuhkik986756zh0KHUtzuguzTThij9787ZGUHI9eqqseeGUx787utgzq" +
                                    "jh87865tZTTUIJUGrdujoj987644wsuiu89756tzhUZTFUj9086rfHHIJK98866rfik9o()hut6rfzsz89" +
									"01238767zhu9abcdefghijklmnopqrstuvwxyz0123456789A(<EFGHI(((gzft!!2ghhQRST)tfg(jhXZ" +
									"01234OIIHU()()zhefghijklm87Ujktuvwxyz01234()&hjhu89h5uziokFGHIddde4jknjhuRSTUVWXYZ" +
									"0434OdI3dghijklm3h7stuvwxy54de)&hjhftrr45r4rgzhjh6YZjzhgTTZZher458ud86754HH9988jJJ" +
									"7zhztfuiUZZikj87654JKJIHuz654efOKiozjh87565R5gjokjhFZ_ljuhuj9rd3fHUJT878z6ghu=jkhu" +
									"86huzzgik/&/Z889KK5rfrtf98Zkik986756zh0KHUtzuguerrer5r4j9787ZGUHI9eqqseeGU787utgzq" +
                                    "jh87865tZTTUIJUGrdujoj987644w878Z/(&64r567kTFUj9086rfHHIJK98866rfik9o()hut6rfzz489" +
									"96zi456987JI)8uz8%h((fg8uZT&/ioj9iujhu)(UZT%zgzUH123456789ABC8/899LMNOPQRdddVWXYZ\n"


scriptThread.appendTextToConsole('script has started');

//Hide the dialog (the tab will be removed from the dialog therefore the dialog is not needed).
UI_Dialog.hide();

//Remove the pages from the dialog and add it to the main window.
if(scriptThread.addToolBoxPagesToMainWindow(UI_ToolBox))
{
	var g_stopSending = true;

	UI_Dialog.finishedSignal.connect(UI_DialogFinished);
	scriptThread.dataReceivedSignal.connect(dataReceived);
	UI_StartButton.clickedSignal.connect(start);
	UI_StopButton.clickedSignal.connect(stop);

}
else
{
	scriptThread.messageBox("Critical", "Error", "scriptThread::addToolBoxPagesToMainWindow cannot be called in command-line mode");
	scriptThread.stopScript();
}


