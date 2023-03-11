#ifndef TESTDLL_H
#define TESTDLL_H

#include <QJSEngine>
#include <QObject>
#include <QVector>

extern "C" Q_DECL_EXPORT void init(QJSEngine* engine);

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

        QJSValue scriptArray = convertByteVectorToScriptArray(appendChecksum(data));

        if (!m_sendDataArrayFunction.isError())
        {
            QJSValue result = m_sendDataArrayFunction.call(QJSValueList() << scriptArray);
            hasSucceeded = result.toBool();
        }
        else
        {
            hasSucceeded = false;
        }

        return hasSucceeded;
    }

    void setSendDataArrayFunction(QJSValue sendDataArrayFunction)
    {
        m_sendDataArrayFunction = sendDataArrayFunction;
    }

    void setScriptEngine(QJSEngine* scriptEngine)
    {
        m_scriptEngine = scriptEngine;
    }

private:

    QJSValue convertByteVectorToScriptArray(QVector<unsigned char> vector)
    {
        QJSValue scriptArray = m_scriptEngine->newArray(vector.size());

        for(int i = 0; i < vector.size(); i++)
        {
            scriptArray.setProperty(i, QJSValue(vector[i]));
        }

        return scriptArray;
    }

    QJSValue m_sendDataArrayFunction;
    QJSEngine* m_scriptEngine;

};

#endif // TESTDLL_H
