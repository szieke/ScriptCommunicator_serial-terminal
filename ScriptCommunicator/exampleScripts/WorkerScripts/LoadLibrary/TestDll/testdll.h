#ifndef TESTDLL_H
#define TESTDLL_H

#include "testdll_global.h"
#include <QScriptEngine>
#include <QObject>
#include <QVector>

extern "C" Q_DECL_EXPORT void init(QScriptEngine* engine);

class TestDll : public QObject
{
    Q_OBJECT

public:
    TestDll();

    Q_INVOKABLE QVector<unsigned char> appendChecksum(QVector<unsigned char> data)
    {
        unsigned char checksum = 0;
        for(auto el : data )
        {
            checksum += el;
        }

        data.push_back(checksum);
        return data;
    }

    Q_INVOKABLE bool appendChecksumAndSend(QVector<unsigned char> data)
    {
        bool hasSucceeded = true;

        QScriptValue scriptArray = convertByteVectorToScriptArray(appendChecksum(data));

        if (!m_sendDataArrayFunction.isError())
        {
            QScriptValue result = m_sendDataArrayFunction.call(QScriptValue(), QScriptValueList() << scriptArray);
            hasSucceeded = result.toBool();
        }
        else
        {
            hasSucceeded = false;
        }

        return hasSucceeded;
    }

    void setSendDataArrayFunction(QScriptValue sendDataArrayFunction)
    {
        m_sendDataArrayFunction = sendDataArrayFunction;
    }

    void setScriptEngine(QScriptEngine* scriptEngine)
    {
        m_scriptEngine = scriptEngine;
    }

private:

    QScriptValue convertByteVectorToScriptArray(QVector<unsigned char> vector)
    {
        QScriptValue scriptArray = m_scriptEngine->newArray(vector.size());

        for(int i = 0; i < vector.size(); i++)
        {
            scriptArray.setProperty(i, QScriptValue(m_scriptEngine, vector[i]));
        }

        return scriptArray;
    }

    QScriptValue m_sendDataArrayFunction;
    QScriptEngine* m_scriptEngine;

};

#endif // TESTDLL_H
