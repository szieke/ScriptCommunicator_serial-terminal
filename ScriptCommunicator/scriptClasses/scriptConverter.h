#ifndef SCRIPTCONVERTER_H
#define SCRIPTCONVERTER_H

#include <QObject>
#include <QScriptable>
#include "scriptObject.h"

class ScriptConverter : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptConverter();

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return "QString byteArrayToString(QVector<unsigned char> data);QString byteArrayToHexString(QVector<unsigned char> data);"
               "QVector<unsigned char> stringToArray(QString str);QVector<unsigned char> addStringToArray(QVector<unsigned char> array, QString str);";
    }

    ///Registers all (for this class) necessary meta types.
    void registerScriptMetaTypes(QScriptEngine* scriptEngine);

    ///Converts a byte array into a hex string.
    Q_INVOKABLE static QString byteArrayToHexString(QVector<unsigned char> data);

    ///Converts a byte array which contains ascii characters into a ascii string (QString).
    Q_INVOKABLE static QString byteArrayToString(QVector<unsigned char> data);

    ///Converts an ascii string into a byte array.
    Q_INVOKABLE static QVector<unsigned char> stringToArray(QString str);

    ///Adds an ascii string to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addStringToArray(QVector<unsigned char> array , QString str);

    Q_INVOKABLE static uint16_t byteArrayToUint16(QVector<unsigned char> data, bool lsb);

    Q_INVOKABLE static uint32_t byteArrayToUint32(QVector<unsigned char> data, bool lsb);

    Q_INVOKABLE static uint64_t byteArrayToUint64(QVector<unsigned char> data, bool lsb);

    Q_INVOKABLE static int16_t byteArrayToInt16(QVector<unsigned char> data, bool lsb);

    Q_INVOKABLE static int32_t byteArrayToInt32(QVector<unsigned char> data, bool lsb);

    Q_INVOKABLE static int64_t byteArrayToInt64(QVector<unsigned char> data, bool lsb);

    Q_INVOKABLE static float byteArrayToFloat32(QVector<unsigned char> data, bool lsb);

    Q_INVOKABLE static double byteArrayToFloat64(QVector<unsigned char> data, bool lsb);
};

#endif // SCRIPTCONVERTER_H
