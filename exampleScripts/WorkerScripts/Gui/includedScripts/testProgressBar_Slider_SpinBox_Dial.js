
//the user has changed the spin box value
function progressBarSpinBoxValueChanged(value)
{
	UI_progressBar.setValue(UI_progressBarSpinBox.value());
	UI_testTextEdit.append("progressBarSpinBoxValueChanged: " + value);
}

//the user has changed the double spin box value
function progressBarDoubleSpinBoxValueChanged(value)
{
	UI_progressBar.setValue(UI_progressBarDoubleSpinBox.value());
	UI_testTextEdit.append("progressBarDoubleSpinBoxValueChanged: " + value);
}

//the user has changed the slider value
function UI_horizontalSliderValueChanged(value)
{
	UI_progressBar.setValue((value));
	UI_verticalSlider.setValue((value));
	UI_dial.setValue(UI_verticalSlider.value());
	UI_testTextEdit.append("UI_horizontalSliderValueChanged");
}

//the user has changed the dial value
function UI_dialValueChanged(value)
{
	UI_progressBar.setValue((value));
}

//the user has changed the line edit text
function UI_progressBarLineEditTextChangedSlot(text)
{
	UI_testTextEdit.append("UI_progressBarLineEditTextChangedSlot");
	UI_progressBar.setValue(text);
}
		
UI_progressBar.setMinimum(0);
UI_progressBar.setMaximum(100);
UI_progressBar.setValue(10);
UI_progressBarLineEdit.addIntValidator(0, 100);
UI_progressBarLineEdit.textChangedSignal.connect(UI_progressBarLineEditTextChangedSlot)

UI_horizontalSlider.valueChangedSignal.connect(UI_horizontalSliderValueChanged);
UI_horizontalSlider.setRange(0,100);
UI_horizontalSlider.setValue(0);

UI_dial.valueChangedSignal.connect(UI_dialValueChanged);
UI_dial.setRange(0,100);
UI_dial.setValue(0);

UI_progressBarSpinBox.setRange(0,100);
UI_progressBarSpinBox.setSingleStep(UI_progressBarSpinBox.singleStep());
UI_progressBarSpinBox.valueChangedSignal.connect(progressBarSpinBoxValueChanged);

UI_progressBarDoubleSpinBox.setRange(0,100);
UI_progressBarDoubleSpinBox.setSingleStep(UI_progressBarDoubleSpinBox.singleStep());
UI_progressBarDoubleSpinBox.setDecimals(UI_progressBarDoubleSpinBox.decimals() + 1);
UI_progressBarDoubleSpinBox.valueChangedSignal.connect(progressBarDoubleSpinBoxValueChanged);
		