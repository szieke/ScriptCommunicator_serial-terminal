echo off

rem make the batch file directory the working directory
pushd %~dp0

rem set the path to winrar
set winRarExe=%1

rem set the name of the resulting file
set sfxName=%2

rem set the sce file folder (scripts and the sce file)
set sceFolder=%3

rem set the name of the sce file path
set sceFile=%sceFolder%\%4

rem set icon file name

IF "%5"=="" (
	set iconFile=""
) else (
	set iconFile=%sceFolder%\%5
)

rem set the file which shall be added to the executable
set filesForSfx=..\*.dll ..\plugins ..\qt.conf ..\ScriptCommunicator.exe %sceFolder%

rem set the winrar sfx config file name
set sfxConfigFileName=.\xfs.conf

rem set the winrar arguments
set winRarArguments=a -ep1 -sfx -z%sfxConfigFileName%

rem create the winrar config file
@echo Setup=ScriptCommunicator.exe %sceFile%> %sfxConfigFileName%
@echo TempMode>> %sfxConfigFileName%
@echo Silent=1 >> %sfxConfigFileName%


rem check is the icon file exists
if exist %iconFile% (
	rem create the executable with an icon
	%winRarExe% %winRarArguments% -iicon%iconFile% %sfxName% %filesForSfx%
) else (
	rem create the executable with no icon
	%winRarExe% %winRarArguments% %sfxName% %filesForSfx%
)

rem delete the winrar config file
del %sfxConfigFileName%
