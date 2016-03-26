#include "testdll.h"



TestDll testDll;

TestDll::TestDll() : QObject()
{
}
void init(QScriptEngine* engine)
{
    qRegisterMetaType<QScriptEngine*>("TestDll*");

    engine->globalObject().setProperty("TestDll", engine->newQObject(&testDll));

    QScriptValue sendDataArrayFunction = engine->evaluate("sendDataArray");

    testDll.setSendDataArrayFunction(sendDataArrayFunction);
    testDll.setScriptEngine(engine);

}
