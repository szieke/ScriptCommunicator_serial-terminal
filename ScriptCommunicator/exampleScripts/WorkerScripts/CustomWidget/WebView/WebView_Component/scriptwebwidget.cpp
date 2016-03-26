#include "scriptwebwidget.h"
#include "qwebviewplugin.h"


///Returns the class name of the custom widget.
const char* GetScriptCommunicatorWidgetName(void)
{
    return "QWebView";
}

/**
* Creates the wrapper classe
* @param scriptThread
*      Pointer to the script thread.
* @param customWidget
*      The custom widget.
* @param scriptRunsInDebugger
*       True if the script thread runs in a script debugger.
*/
QObject *CreateScriptCommunicatorWidget(QObject *scriptThread, QWidget *customWidget, bool scriptRunsInDebugger)
{
    return new ScriptWebWidget(scriptThread, customWidget, scriptRunsInDebugger);
}

