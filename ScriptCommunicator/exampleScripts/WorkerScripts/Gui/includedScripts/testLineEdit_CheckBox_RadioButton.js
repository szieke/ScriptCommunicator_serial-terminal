//the user has pressed the get text button
function UI_UI_testGetTextLineEditTextChangedSignal(text)
{
	UI_testTextEdit.append("UI_UI_testGetTextLineEditTextChangedSignal");
	UI_testSetTextLineEdit.setText(text);
	UI_testSetTextLineEdit.setReadOnly(true);
	UI_testSetTextLineEdit.setBackgroundColor("red");
}

//the user has pressed the check box
function UI_radioButton1Clicked(isChecked)
{
	UI_testTextEdit.append("UI_radioButton1Clicked");
	UI_radioButton1.setWindowTextColor("red");
}

//the text of the combo box has been changed
function UI_testSendComboBoxCurrentTextChanged(text)
{
	UI_testTextEdit.append("UI_testSendComboBoxCurrentTextChanged");
	UI_testReceiveComboBox.setCurrentText(text);
	UI_testReceiveComboBox.setPaletteColor("Text", "red");
}

//the text of the font combo box has been changed
function UI_testSendFontComboBoxCurrentTextChanged(text)
{
	UI_testTextEdit.append("UI_testSendFontComboBoxCurrentTextChanged");
	UI_testReceiveFontComboBox.setCurrentText(text);
	UI_testReceiveFontComboBox.setPaletteColor("Text", "red");
}

//the user has pressed the check box
function UI_testSendCheckBoxClicked(isChecked)
{
	UI_testTextEdit.append("UI_testSendCheckBoxClicked");
	UI_testReceiveCheckBox.setChecked(isChecked);
	UI_testReceiveCheckBox.setWindowTextColor("red");
}

UI_testGetTextLineEdit.setToolTip("tool tip text", -1);
UI_testGetTextLineEdit.textChangedSignal.connect(UI_UI_testGetTextLineEditTextChangedSignal)

UI_testSendCheckBox.clickedSignal.connect(UI_testSendCheckBoxClicked)
UI_testSendComboBox.addItem("myItem")
//UI_testSendComboBox.setCurrentText("myItem");
UI_testSendComboBox.currentTextChangedSignal.connect(UI_testSendComboBoxCurrentTextChanged)
	
UI_testReceiveComboBox.addItem("myItem")
UI_testReceiveComboBox.setCurrentText(UI_testSendComboBox.currentText());

UI_testSendFontComboBox.currentTextChangedSignal.connect(UI_testSendFontComboBoxCurrentTextChanged)

UI_radioButton1.clickedSignal.connect(UI_radioButton1Clicked)