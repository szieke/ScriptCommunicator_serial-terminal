/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of
a custom widget class (LED class).
****************************************************************************************/

//Is called if the dialog is closed.
function DialogFinished()
{
scriptThread.stopScript()
}

function stateChanged(state)
{
	scriptThread.appendTextToConsole("Flashing led: " + state);
}

function inputSignal1Changed(state)
{
	scriptThread.appendTextToConsole("Led 1: " + !state);
}

function inputSignal2Changed(state)
{
	scriptThread.appendTextToConsole("Led 2: " + !state);
}

function inputSignal3Changed(state)
{
	scriptThread.appendTextToConsole("Led 3: " + !state);
}

function inputSignal4Changed(state)
{
	scriptThread.appendTextToConsole("Led 4: " + !state);
}

function inputSignal5Changed(state)
{
	scriptThread.appendTextToConsole("Led 5: " + !state);
}

function inputSignal6Changed(state)
{
	scriptThread.appendTextToConsole("Led 6: " + !state);
}

function inputSignal7Changed(state)
{
	scriptThread.appendTextToConsole("Led 7: " + !state);
}
function inputSignal8Changed(state)
{
	scriptThread.appendTextToConsole("Led 8: " + !state);
}
function inputSignal9Changed(state)
{
	scriptThread.appendTextToConsole("Led 69: " + !state);
}
function inputSignal10Changed(state)
{
	scriptThread.appendTextToConsole("Led 10: " + !state);
}

function checkBoxSlot(checked)
{
		UI_led.setFlashing(checked);
}

function sliderSlot(value)
{
	//Deaktivate all LEDs (all but the flashing LED).
	for(var i = 1; i <= 10; i++)
	{
		eval("UI_l" + i).setState(false);
	}

	try
	{
		//Get the corresponding LED and switch it on.
		eval("UI_l" + Math.round((value / 10))).setState(true);
	}
	catch(e)
	{
	}

}

UI_Dialog.show(); 
UI_Dialog.finishedSignal.connect(DialogFinished);
UI_led.stateChangedSignal.connect(stateChanged);

UI_l1.stateChangedSignal.connect(inputSignal1Changed);
UI_l2.stateChangedSignal.connect(inputSignal2Changed);
UI_l3.stateChangedSignal.connect(inputSignal3Changed);
UI_l4.stateChangedSignal.connect(inputSignal4Changed);
UI_l5.stateChangedSignal.connect(inputSignal5Changed);
UI_l6.stateChangedSignal.connect(inputSignal6Changed);
UI_l7.stateChangedSignal.connect(inputSignal7Changed);
UI_l8.stateChangedSignal.connect(inputSignal8Changed);
UI_l9.stateChangedSignal.connect(inputSignal9Changed);
UI_l10.stateChangedSignal.connect(inputSignal10Changed);
UI_checkBox.clickedSignal.connect(checkBoxSlot);
UI_horizontalSlider.valueChangedSignal.connect(sliderSlot);

//Set the led color to blue.
UI_led.setColorRgb(0,0,255);

//Set the flahing rate.
UI_led.setFlashRate(500);
