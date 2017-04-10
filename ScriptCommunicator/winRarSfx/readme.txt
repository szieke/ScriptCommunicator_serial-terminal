With create.bat and sfx.bat a self extracting (sfx) zip (exe) file can be created (only on windows) which:
- contains all necessary ScriptCommunictor files
- a script (and it's resources) which shall be executed

If the created executable is started all files are extracted in a temporary folder und the script is executed.
After the script is finished the temporary is deleted.

To create a sfx file:
- create a sce project (see manual for details) and generate the sce file/folder
- copy the created sce folder to ScriptCommunicator\winRarSfx
- open ScriptCommunicator\winRarSfx\create.bat
- replace the path to winrar (1. argument)
- replace name the sfx file name (2. argument)
- replace the sce folder name (3. argument)
- replace the name of the sce file within the sce folder (4. argument)
- replace or remove (the argument is optional) the name of the icon file (5. argument, this file must be 
  within the sce folder)
- execute create.bat


Note: You can find an example sce project in ScriptCommunicator\winRarSfx\exampleSfx (to build this sfx 
example open create.bat, replace the path to winrar and execute create.bat.