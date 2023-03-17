/*************************************************************************
This script file contains function for script settings data loading and saving

  We need to find ESP project build path (ideally by selscting project's output .elf) 
  from where we can parse  other necessary info, like project name and exact 
  xtensa tool path which should be used (from CmakeCache.txt).
  This way it may be OS independent.
  
Requres ScriptCommunicator v6+ (getUserGenericConfigFolder())
***************************************************************************/

/* ====    Load additional scripts and UI    ==== */
scriptThread.loadUserInterfaceFile("./backtraceSettings.ui", true, false);

/* ====    Global variables    ==== */
var g_settingsFolder = scriptThread.getUserGenericConfigFolder() + "/SCScripts/";
var g_settingsFileName = g_settingsFolder + "ESPConsole.ini";

/* ====   Connect signals   ==== */
// Buttons
UI_pb_openProjectElf.clickedSignal.connect(clickedOpenElfFile);
UI_pbOk.clickedSignal.connect(UI_pbOk, closeSettingsDialog);
UI_pbCancel.clickedSignal.connect(UI_pbCancel, closeSettingsDialog);
UI_pb_selectAddr2Line.clickedSignal.connect(UI_pb_selectAddr2Line, clickedOpenToolsFile);
UI_pb_selectReadElf.clickedSignal.connect(UI_pb_selectReadElf, clickedOpenToolsFile);
// Checkbox
UI_chkBox_backtraceDecode.clickedSignal.connect(clickedDecode);
UI_chkBox_autodetectTools.clickedSignal.connect(clickedAutodetect);
// Combo box
UI_comBox_projecElfFile.currentTextChangedSignal.connect(parseFwFileInfo);


/* ====    Functions     ==== */

/* Empty firmware info if requirements not satisfied */
function cleanFwInfo() 
{
	UI_lnEd_FwName.setText("");
	UI_lnEd_target.setText("");
	UI_lnEd_FwIDFVer.setText("");
	UI_lnEd_FwAppVer.setText("");
	UI_lnEd_FwBuildDate.setText("");
	UI_lnEd_FwBuildTime.setText("");
	UI_lnEd_projectLoaded.setText("");
}

/* Empty all info if requirements not satisfied */
function cleanAll() 
{
	// Only if autodetect, otherwise keep what is set
	if(UI_chkBox_autodetectTools.isChecked() ) {
		UI_lnEd_pathAddr2Line.setText("");
		UI_label_verAddr2Line.setText("");
		UI_label_verAddr2Line.setWindowTextColor("darkGray");	// Reasonable default that works on dark and light themes
		UI_lnEd_pathReadElf.setText("");
		UI_label_verReadelf.setText("");
		UI_label_verReadelf.setWindowTextColor("darkGray");		// Reasonable default that works on dark and light themes
	}
	cleanFwInfo();
}

// Get addr2line tool version
function addr2lineVersionUpdate() 
{
	var programPath = UI_lnEd_pathAddr2Line.text();
	if(scriptThread.checkFileExists(programPath, false)) 
	{
		var ret = runProcessAsync(programPath, Array("-v"), -1, 1000, "");
		if(ret.exitCode != 0) {		// Error
			UI_label_verAddr2Line.setText( (ret.stdErr).split("\n")[0] );	// First Line
			UI_label_backtrace.setWindowTextColor("red");
		}
		else {		// OK
			UI_label_verAddr2Line.setText( (ret.stdOut).split("\n")[0] );	// First Line
			UI_label_verAddr2Line.setWindowTextColor("darkGray");	// Reasonable default that works on dark and light themes
		}
	}
}

// Get readelf tool version
function readelfVersionUpdate() 
{
	var programPath = UI_lnEd_pathReadElf.text();
	if(scriptThread.checkFileExists(programPath, false)) 
	{
		var ret = runProcessAsync(programPath, Array("-v"), -1, 1000, "");
		if(ret.exitCode != 0) {		// Error
			UI_label_verReadelf.setText( (ret.stdErr).split("\n")[0] );	// First Line
			UI_label_verReadelf.setWindowTextColor("red");			
		}
		else {		// OK
			UI_label_verReadelf.setText( (ret.stdOut).split("\n")[0] );		// First Line
			UI_label_verReadelf.setWindowTextColor("darkGray");	// Reasonable default that works on dark and light themes
		}
	}
}

/* Check if addr2line executable present and its version */
function checkAddr2line()
{
	if( scriptThread.checkFileExists(UI_lnEd_pathAddr2Line.text(), false) ) {
		UI_chkBox_backtraceDecode.setChecked(true);
		clickedDecode();	// Simulate checkbox clicked signal
		addr2lineVersionUpdate();
	}
	else {
		UI_label_verAddr2Line.setText("Addr2Line not found!");
		UI_label_verAddr2Line.setWindowTextColor("red");
		UI_chkBox_backtraceDecode.setChecked(false);
		clickedDecode();	// Simulate checkbox clicked signal
	}
}

/* Check if readelf executable present and its version 
 * 	Also update fwinfo if present (elf should be checked before)
*/
function checkReadelf(elfFile)
{
	if( scriptThread.checkFileExists(UI_lnEd_pathReadElf.text(), false) ) 
	{
		readelfVersionUpdate();
		
		// Get String dump of section '.flash.appdesc'. Use: xtensa-esp32-elf-readelf -p .flash.appdesc <project.elf>
		var program = UI_lnEd_pathReadElf.text();
		var arguments = Array("-p", ".flash.appdesc", elfFile);	
		var ret = runProcessAsync(program, arguments, -1, 1000, "");
		var stringArray = (ret.stdOut).split("\n");
		
		if((ret.exitCode == 0) && (stringArray.length > 7)) 
		{
			// NOTE: Nothing too crazy here, positions are hardcoded.
			// 	In case the tool output format changes it will only break firmware info, not decoding ability
			UI_lnEd_FwAppVer.setText(	stringArray[3].slice(12));	// Project version example: '  [    10]  0.1.0.77'
			UI_lnEd_FwName.setText(		stringArray[4].slice(12));	// Project name example: 	'  [    30]  my_project'
			UI_lnEd_FwBuildTime.setText(stringArray[5].slice(12));	// Build time example: 		'  [    50]  12:23:58'
			UI_lnEd_FwBuildDate.setText(stringArray[6].slice(12));	// Build date example: 		'  [    60]  Mar 11 2023'
			UI_lnEd_FwIDFVer.setText(	stringArray[7].slice(12));	// IDF version example: 	'  [    70]  v4.4.1'			
			// Main window tab info label:
			UI_lnEd_projectLoaded.setText(UI_lnEd_FwName.text() + " v" + UI_lnEd_FwAppVer.text() + 
						" / ESP-IDF " + UI_lnEd_FwIDFVer.text() + " on " + UI_lnEd_target.text());
		}
		//else just dont fill FW info - not required for backtrace
	}
	else {
		// addr2line not found
		cleanFwInfo();
		UI_label_verReadelf.setText("Readelf not found!");
		UI_label_verReadelf.setWindowTextColor("red");
	}
}

/* Manual Tools Executable selection */
function clickedOpenToolsFile()
{
	var sender = this;
	var filePath = "";
	//scriptThread.appendTextToConsole(sender.getObjectName());
	if(sender == UI_pb_selectAddr2Line) {
		filePath = scriptThread.showFileDialog(false, "Select xtensa -elf-addr2line executable", UI_lnEd_pathAddr2Line.text(), "Executable (*-elf-addr2line*);;Any (*)");
		if(filePath != "") {
			UI_lnEd_pathAddr2Line.setText(filePath);
			addr2lineVersionUpdate();
		}
	}
	else if(sender == UI_pb_selectReadElf) {
		filePath = scriptThread.showFileDialog(false, "Select xtensa -elf-readelf executable", UI_lnEd_pathReadElf.text(), ("Executable (*-elf-readelf*);;Any (*)") );
		if(filePath != "") {
			UI_lnEd_pathReadElf.setText(filePath);
			readelfVersionUpdate();
			//parseFwFileInfo();	// Refresh FW info too
		}
	}
	UI_settingsDialog.raise();
}


/* Frimware ELF file selection */
function clickedOpenElfFile()
{
	var filePath = "";
	filePath = scriptThread.showFileDialog(false, "Select ESP-IDF Project Build ELF File", UI_comBox_projecElfFile.currentText(), "Exec/Link Format (*.elf)");
	
	if(filePath != "") {
		UI_comBox_projecElfFile.addItem(filePath);
		UI_comBox_projecElfFile.setCurrentText(filePath);
	}
	else {	// No file, clean additional info
		cleanAll();
	}

	UI_settingsDialog.raise();
}


/* Fead basic info from file and fill it to UI */
function parseFwFileInfo ()
{
	var elfFile = UI_comBox_projecElfFile.currentText();
	
	// First check if elf file is present, otherwise we can't do anything
	if(scriptFile.checkFileExists(elfFile, false))
	{
		var autodetect = UI_chkBox_autodetectTools.isChecked();
		var idx = elfFile.lastIndexOf('/');				// index of last forward slash. Works on windows too
		var projBuildPath = elfFile.substring(0, idx+1);	// including last slash
		
		// Check for CmakeCache.txt in same folder as .elf file:
		var fileCmakeCache = projBuildPath + "CMakeCache.txt";
		var cmakeCachePresent = scriptFile.checkFileExists(fileCmakeCache, false)
		if(cmakeCachePresent)
		{
			var idx = -1;
			var cmakeFile = scriptThread.readFile(fileCmakeCache, false, 0, -1);
			
			if(autodetect)
			{
				// Find Addr2Line executable. Use: xtensa-esp32-elf-addr2line -pfiaC -e build/PROJECT.elf ADDRESS
				// Example to search: CMAKE_ADDR2LINE:FILEPATH=/home/user/.espressif/tools/xtensa-esp32-elf/esp-2021r2-patch3-8.4.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-addr2line
				idx = cmakeFile.indexOf("CMAKE_ADDR2LINE:FILEPATH");	// Match string (first occurence)
				idx = cmakeFile.indexOf('=', idx);						// Match following '=' (first occurence starting at previous index)
				UI_lnEd_pathAddr2Line.setText( cmakeFile.substring(idx+1, cmakeFile.indexOf('\n', idx)).trim() );
		
				// Find ReadElf executable. Use: xtensa-esp32-elf-readelf -p .flash.appdesc build//my_project.elf
				// Example to search: CMAKE_READELF:FILEPATH=/home/user/.espressif/tools/xtensa-esp32-elf/esp-2021r2-patch3-8.4.0/xtensa-esp32-elf/bin/xtensa-esp32-elf-readelf		
				idx = cmakeFile.indexOf("CMAKE_READELF:FILEPATH");		// Match string (first occurence)
				idx = cmakeFile.indexOf('=', idx);						// Match following '=' (first occurence starting at previous index)
				UI_lnEd_pathReadElf.setText( cmakeFile.substring(idx+1, cmakeFile.indexOf('\n', idx)).trim() );
			}
			
			checkAddr2line();		// Check if addr2line executable present and its version
			checkReadelf(elfFile);	// Check if readelf executable present and its version 

			// Find target chip
			// Example to search: IDF_TARGET:STRING=esp32
			idx = cmakeFile.indexOf("IDF_TARGET:STRING");	// Match string (first occurence)
			idx = cmakeFile.indexOf('=', idx);	// Match following '=' (first occurence starting at index)
			UI_lnEd_target.setText( cmakeFile.substring(idx+1, cmakeFile.indexOf('\n', idx)).trim() );
		}
		else {	// CmakeCache.txt not found
			// Cleanup invalid info
			cleanAll();
			if(autodetect) {
				UI_lnEd_pathAddr2Line.setText("");
				UI_label_verAddr2Line.setText("CmakeCache.txt not found!");
				UI_label_verAddr2Line.setWindowTextColor("red");
				UI_lnEd_pathReadElf.setText("");
				UI_label_verReadelf.setText("CmakeCache.txt not found!");
				UI_label_verReadelf.setWindowTextColor("red");
				UI_chkBox_backtraceDecode.setChecked(false);
				clickedDecode();	// Simulate checkbox clicked signal
				// Also show warning message if backtrace is enabled so user knows what's up
				if(UI_chkBox_backtraceDecode.isChecked()) {
					scriptThread.messageBox("Warning", "CmakeCache.txt not found", "File <b>CmakeCache.txt</b> was not found. "
						+ "It is expected to be in same folder as the selected .elf file." 
						+ "<br><br>Alternativelly you can disable '<b>Autodetect Tools</b>' function and select the tools manually."
						+ "Backtrace can only be decoded if correct <b>Addr2Line</b> tool is used for selected .elf file."
					);
				}
			}
			else {	// Elf found, CmakeCache.txt not, but autodetect off
				// try parsing with selected tools
				checkAddr2line();		// Check if addr2line executable present and its version
				checkReadelf(elfFile);	// Check if readelf executable present and its version
			}
		}
	}
	else {
		// Elf file not present, cant do anything -> disable backtrace
		cleanAll();
		UI_label_verAddr2Line.setText("ELF File not found!");
		UI_label_verAddr2Line.setWindowTextColor("red");
		UI_label_verReadelf.setText("ELF File not found!");
		UI_label_verReadelf.setWindowTextColor("red");
		UI_chkBox_backtraceDecode.setChecked(false);
		clickedDecode();	// Simulate checkbox clicked signal
		scriptThread.messageBox("Critical", ".ELF file not found", "ELF File was not found, backtrace won't be decoded.");
	}
}


// Show Settings Window
function showSettingsDialog() 
{
	UI_settingsDialog.show();
}

// Show Settings Window
function closeSettingsDialog() 
{
	var sender = this;
	//scriptThread.appendTextToConsole(sender.getObjectName());
	if(sender == UI_pbOk) {
		saveUiSettings();	// Save settings
	}
	else if(sender == UI_pbCancel) {
		loadUiSettings();	// Reload previous settings
	}
	UI_settingsDialog.hide();
}

/* Sets correct state of other elements in relation 
	to UI_chkBox_backtraceDecode.isChecked() state */
function clickedDecode ()
{
	if(UI_chkBox_backtraceDecode.isChecked()) {
		UI_lnEd_projectLoaded.setEnabled(true);	// Main window tab
		UI_label_backtrace.setEnabled(true);	// Main window tab
		UI_grpBox_FWInfo.setEnabled(true);
		UI_lnEd_pathAddr2Line.setEnabled(true);
		UI_lnEd_pathReadElf.setEnabled(true);
		UI_chkBox_autodetectTools.setEnabled(true);
	}
	else {
		UI_lnEd_projectLoaded.setEnabled(false);// Main window tab
		UI_label_backtrace.setEnabled(false);	// Main window tab
		UI_grpBox_FWInfo.setEnabled(false);
		UI_lnEd_pathAddr2Line.setEnabled(false);
		UI_lnEd_pathReadElf.setEnabled(false);
		UI_chkBox_autodetectTools.setEnabled(false);
	}
}


/* Sets correct state of other elements in relation 
	to UI_chkBox_autodetectTools.isChecked() state */
function clickedAutodetect ()
{
	if(UI_chkBox_autodetectTools.isChecked()) {
		UI_pb_selectAddr2Line.setEnabled(false);
		UI_pb_selectReadElf.setEnabled(false);
		UI_lnEd_pathAddr2Line.setReadOnly(true);
		UI_lnEd_pathReadElf.setReadOnly(true);
	}
	else {
		UI_pb_selectAddr2Line.setEnabled(true);
		UI_pb_selectReadElf.setEnabled(true);
		UI_lnEd_pathAddr2Line.setReadOnly(false);
		UI_lnEd_pathReadElf.setReadOnly(false);
	}
}

//Returns a value from stringArray (values are stored in a key/value string,
//eg. 'UI_dataBitsBox=8')
function getValOfStrArr(stringArray, key)
{
	for (var i=0; i < stringArray.length; i++)
	{
		var subStringArray = stringArray[i].split("=");
		if(subStringArray[0] == key)
		{
			return subStringArray[1];
		}
	}
}

// Loads the saved user interface settings.
function loadUiSettings()
{
	UI_comBox_projecElfFile.clear();	// Clear list in case of reload (Cancel button for example)
	
	if(scriptFile.checkFileExists(g_settingsFileName, false))
	{
		var settings = scriptFile.readFile(g_settingsFileName, false);
		var stringArray = settings.split("\r\n");
		// Search for indexes of specific parts:
		var settingsIdx = stringArray.indexOf("[Settings]");
		var elfListIdx = stringArray.indexOf("[ELF List]");
		
		// Parse values:
		UI_chkBox_backtraceDecode.setChecked(((getValOfStrArr(stringArray, "DecodeBacktrace") == "true") ? true : false));		
		clickedDecode();  // Sets correct state of other elements in relation checkbox
		
		var elfCurIndex = getValOfStrArr(stringArray, "ElfCurrentIndex");
		var elfCount = getValOfStrArr(stringArray, "ElfCount");
		
		var autodetectTools = ((getValOfStrArr(stringArray, "AutodetectTools") == "false") ? false : true);
		UI_chkBox_autodetectTools.setChecked( autodetectTools );
		clickedAutodetect();  // Sets correct state of other elements in relation checkbox
		if(!autodetectTools) {
			UI_lnEd_pathAddr2Line.setText(getValOfStrArr(stringArray, "ManualAddr2LinePath"));
			UI_lnEd_pathReadElf.setText(getValOfStrArr(stringArray, "ManualReadElfPath"));
		}
		
		UI_comBox_projecElfFile.blockSignals(true);		// Temporarily disable index/text changed signal	
		for(var i=0; i < elfCount; i++) {
			//scriptThread.appendTextToConsole( stringArray[elfListIdx+1+i] );
			UI_comBox_projecElfFile.addItem( stringArray[elfListIdx+1+i] );
		}
		UI_comBox_projecElfFile.setCurrentIndex(elfCurIndex);		
		parseFwFileInfo();
		UI_comBox_projecElfFile.blockSignals(false);	// Unblock index/text changed signal
	}
}

// Saves the user interface settings.
function saveUiSettings()
{
	var settings = "";
	
	settings += "[Settings]" + "\r\n";
	settings += "DecodeBacktrace=" + (UI_chkBox_backtraceDecode.isChecked() ? "true" : "false") + "\r\n";
	settings += "ElfCurrentIndex=" + UI_comBox_projecElfFile.currentIndex() + "\r\n";
	settings += "ElfCount=" + UI_comBox_projecElfFile.count() + "\r\n";
	settings += "AutodetectTools=" + (UI_chkBox_autodetectTools.isChecked() ? "true" : "false") + "\r\n";
	settings += "ManualAddr2LinePath=" + UI_lnEd_pathAddr2Line.text() + "\r\n";
	settings += "ManualReadElfPath=" + UI_lnEd_pathReadElf.text() + "\r\n";
	
	settings += "[ELF List]" + "\r\n";
	for(var i=0; i < UI_comBox_projecElfFile.count(); i++) {
		settings += UI_comBox_projecElfFile.itemText(i) + "\r\n";
	}
	
	if( !scriptFile.checkDirectoryExists(g_settingsFolder, false) ) {
		scriptFile.createDirectory(g_settingsFolder, false);
	}
	try {
		scriptFile.writeFile(g_settingsFileName, false, settings, true);
	}
	catch(e) {
		scriptThread.messageBox("Critical", "exception in saveUiSettings", e.toString());
	}
}
