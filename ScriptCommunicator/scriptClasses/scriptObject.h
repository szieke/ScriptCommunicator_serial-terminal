#ifndef SCRIPTOBJECT_H
#define SCRIPTOBJECT_H

#include <QObject>
#include <mainwindow.h>

class ScriptObject
{
public:

    virtual ~ScriptObject(void)
    {

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void) = 0;
};

#endif // SCRIPTOBJECT_H
