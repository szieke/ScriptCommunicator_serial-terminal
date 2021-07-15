//the user has pressed the get text button
function UI_testGetTextLineEditTextChangedSignal(text)
{
	UI_testTextEdit.append("UI_UI_testGetTextLineEditTextChangedSignal");
	UI_testSetTextLineEdit.setText(text);
	UI_testSetTextLineEdit.setReadOnly(true);
	UI_testSetTextLineEdit.setBackgroundColor("red");
}

//the user has pressed the check box
function radioButton1Clicked(isChecked)
{
	UI_testTextEdit.append("UI_radioButton1Clicked");
	UI_radioButton1.setAutoFillBackground(true);//must be called (outherwise setBackgroundColor
	//would have no effekt with radio buttons)
	UI_radioButton1.setBackgroundColor("red");
}

//the text of the combo box has been changed
function testSendComboBoxCurrentTextChanged(text)
{
	UI_testTextEdit.append("UI_testSendComboBoxCurrentTextChanged");
	UI_testReceiveComboBox.setCurrentText(text);
	UI_testReceiveComboBox.setPaletteColor("Text", "red");
}

//the text of the font combo box has been changed
function testSendFontComboBoxCurrentTextChanged(text)
{
	UI_testTextEdit.append("UI_testSendFontComboBoxCurrentTextChanged");
	UI_testReceiveFontComboBox.setCurrentText(text);
	UI_testReceiveFontComboBox.setPaletteColor("Text", "red");
}

//the user has pressed the check box
function testSendCheckBoxClicked(isChecked)
{
	UI_testTextEdit.append("UI_testSendCheckBoxClicked");
	UI_testReceiveCheckBox.setChecked(isChecked);
	UI_testReceiveCheckBox.setWindowTextColor("red");
}

function groupBoxCheckBoxClickedSlot(checked)
{
	UI_testTextEdit.append("groupBoxCheckBoxClickedSlot: " + checked)
}

UI_testGetTextLineEdit.setToolTip("tool tip text", -1);
UI_testGetTextLineEdit.textChangedSignal.connect(UI_testGetTextLineEditTextChangedSignal)

UI_testSendCheckBox.clickedSignal.connect(testSendCheckBoxClicked)
UI_testSendComboBox.addItem("myItem")
//UI_testSendComboBox.setCurrentText("myItem");
UI_testSendComboBox.currentTextChangedSignal.connect(testSendComboBoxCurrentTextChanged)
	
UI_testReceiveComboBox.addItem("myItem")
UI_testReceiveComboBox.setCurrentText(UI_testSendComboBox.currentText());

UI_testSendFontComboBox.currentTextChangedSignal.connect(testSendFontComboBoxCurrentTextChanged)

UI_radioButton1.clickedSignal.connect(radioButton1Clicked)


//Toggle the group box checkbox.
UI_checkableGroupBox.setChecked(!UI_checkableGroupBox.isChecked())
UI_checkableGroupBox.checkBoxClickedSignal.connect(groupBoxCheckBoxClickedSlot)