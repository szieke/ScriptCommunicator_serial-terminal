/********************************************************************************************************
* The script demonstrates how to send data from GUI elements cyclically. To receive this data following
* must be done:
* run CyclicReceive.js in a second ScriptCommunicator instance
* connect the two ScriptCommunicator instances
*
**********************************************************************************************/
/*
 * This function send data from a GUI elements.
 *
 * @param id
 *	The id of the GUI element.
 * @param data
 *	The data which shall be sent.
 */
function sendValue(id, data)
{
	if(scriptInf.isConnectedWithCan())
	{
		scriptInf.sendCanMessage(0, id, data);
	}
	else
	{
		var sendArray = Array(0xff, id);
		appendByteArrayAtByteArray(sendArray, data, data.length);
		appendNumberAtByteArray(sendArray, 0xfe, 1);
		scriptInf.sendDataArray(sendArray)
	}
}
//Timer function which send the GUI elements.
function timerSlot()
{
	for(var i = 0; i < elementMap.length; i++)
	{
		var lastSend = elementMap[i][3];
		var interval = elementMap[i][2];
		var name = elementMap[i][0];
		var el = eval(name);
		var id = elementMap[i][1];
		
		if((Date.now() - lastSend) >= interval)
		{
			if(name == "UI_Val1ComboBox")
			{
				sendValue(id, appendStringToByteArray(Array(), el.currentText()));
			}
			else if(name == "UI_Value2CheckBox")
			{
				sendValue(id, appendNumberAtByteArray(Array(), el.isChecked(), 1));
			}
			else if(name == "UI_Value3SpinBox")
			{
				sendValue(id, appendNumberAtByteArray(Array(), el.value(), 1));
			}
			else if(name == "UI_Value4DoubleSpinBox")
			{
				var intvalue = Math.round( el.value() * 100);
				sendValue(id, appendNumberAtByteArray(Array(), intvalue, 4));
			}
			else if(name == "UI_Value5Dial")
			{
				sendValue(id, appendNumberAtByteArray(Array(), el.value(), 1));
			}
			else if(name == "UI_Value6LineEdit")
			{
				sendValue(id, appendStringToByteArray(Array(), el.text()));
			}
			
			else if(name == "UI_Value7HorizontalSlider")
			{
				sendValue(id, appendNumberAtByteArray(Array(), el.value(), 1));
			}
			
			else if((name == "UI_tableLineEdit1") 
			|| (name == "UI_tableLineEdit2")
			|| (name == "UI_tableLineEdit3")
			|| (name == "UI_tableLineEdit4")
			|| (name == "UI_tableLineEdit5")
			|| (name == "UI_tableLineEdit6"))
			{
				sendValue(id, appendNumberAtByteArray(Array(), parseInt(el.text()), 1));
			}
			
			elementMap[i][3] = Date.now();
		}
	}
}

//Load/include the helper script.
scriptThread.loadScript("Helper.js")


//Create line edits in every table cell.
UI_tableWidget.insertWidget(0, 0, "LineEdit");
var UI_tableLineEdit1 = UI_tableWidget.getWidget(0, 0);
UI_tableLineEdit1.setText(0);
UI_tableLineEdit1.addIntValidator(0, 99)

UI_tableWidget.insertWidget(0, 1, "LineEdit");
var UI_tableLineEdit2 = UI_tableWidget.getWidget(0, 1);
UI_tableLineEdit2.setText(0);
UI_tableLineEdit2.addIntValidator(0, 99)

UI_tableWidget.insertWidget(1, 0, "LineEdit");
var UI_tableLineEdit3 = UI_tableWidget.getWidget(1, 0);
UI_tableLineEdit3.setText(0);
UI_tableLineEdit3.addIntValidator(0, 99)

UI_tableWidget.insertWidget(1, 1, "LineEdit");
var UI_tableLineEdit4 = UI_tableWidget.getWidget(1, 1);
UI_tableLineEdit4.setText(0);
UI_tableLineEdit4.addIntValidator(0, 99)

UI_tableWidget.insertWidget(2, 0, "LineEdit");
var UI_tableLineEdit5 = UI_tableWidget.getWidget(2, 0);
UI_tableLineEdit5.setText(0);
UI_tableLineEdit5.addIntValidator(0, 99)

UI_tableWidget.insertWidget(2, 1, "LineEdit");
var UI_tableLineEdit6 = UI_tableWidget.getWidget(2, 1);
UI_tableLineEdit6.setText(0);
UI_tableLineEdit6.addIntValidator(0, 99)


//Create, start and connect the send timer.
var timer = scriptThread.createTimer();
timer.timeoutSignal.connect(eval("timerSlot"));
timer.start(1);//timer interval= 1ms

//Create the element map (contains the meta data from all GUI elements).
var elementMap = Array();
//Name,id, interval, last send
elementMap[0] = Array("UI_Val1ComboBox", 1, 10, Date.now())
elementMap[1] = Array("UI_Value2CheckBox", 2, 10, Date.now())
elementMap[2] = Array("UI_Value3SpinBox", 3, 10, Date.now())
elementMap[3] = Array("UI_Value4DoubleSpinBox", 4, 10, Date.now())
elementMap[4] = Array("UI_Value5Dial", 5, 10, Date.now())
elementMap[5] = Array("UI_Value6LineEdit", 6, 10, Date.now())
elementMap[6] = Array("UI_Value7HorizontalSlider", 7, 10, Date.now())
elementMap[7] = Array("UI_tableLineEdit1", 8, 10, Date.now())
elementMap[8] = Array("UI_tableLineEdit2", 9, 10, Date.now())
elementMap[9] = Array("UI_tableLineEdit3", 10, 10, Date.now())
elementMap[10] = Array("UI_tableLineEdit4", 11, 10, Date.now())
elementMap[11] = Array("UI_tableLineEdit5", 12, 10, Date.now())
elementMap[12] = Array("UI_tableLineEdit6", 13, 10, Date.now())


