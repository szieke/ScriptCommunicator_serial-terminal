#ifndef SCRIPTCONVERTER_H
#define SCRIPTCONVERTER_H

#include <QObject>
#include <QScriptable>
#include "scriptObject.h"

class ScriptConverter : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptConverter();

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("conv.api");
    }

    ///Registers all (for this class) necessary meta types.
    void registerScriptMetaTypes(QScriptEngine* scriptEngine);

    ///Converts a byte array into a hex string.
    Q_INVOKABLE static QString byteArrayToHexString(QVector<unsigned char> data);

    ///Converts a byte array which contains ascii characters into an ascii string (QString).
    Q_INVOKABLE static QString byteArrayToString(QVector<unsigned char> data);

    ///Converts a byte array which contains utf8 characters into an utf8 string (QString).
    Q_INVOKABLE static QString byteArrayToUtf8String(QVector<unsigned char> data);

    ///Converts an ascii string into a byte array.
    Q_INVOKABLE static QVector<unsigned char> stringToArray(QString str);

    ///Converts a string into an utf8 byte array.
    Q_INVOKABLE static QVector<unsigned char> stringToUtf8Array(QString str);

    ///Converts the first Bytes of a byte array to an uint16_t.
    ///Note: This functions works only if the return value is uint32_t (and not uint16_t).
    Q_INVOKABLE static uint32_t byteArrayToUint16(QVector<unsigned char> data, bool littleEndian);

    ///Converts the first Bytes of a byte array to an uint32_t.
    ///Note: This functions works only if the return value is uint64_t (and not uint32_t).
    Q_INVOKABLE static uint64_t byteArrayToUint32(QVector<unsigned char> data, bool littleEndian);

    ///Converts the first Bytes of a byte array to an uint64_t.
    Q_INVOKABLE static uint64_t byteArrayToUint64(QVector<unsigned char> data, bool littleEndian);

    ///Converts an unsigned char to a signed char (int8_t).
    ///Note: This functions works only if the return value is int16_t (nd not int8_t).
    Q_INVOKABLE static int16_t unsignedCharToSignedChar(unsigned char number);

    ///Converts the first Bytes of a byte array to an int16_t.
    Q_INVOKABLE static int16_t byteArrayToInt16(QVector<unsigned char> data, bool littleEndian);

    ///Converts the first Bytes of a byte array to an int32_t.
    Q_INVOKABLE static int32_t byteArrayToInt32(QVector<unsigned char> data, bool littleEndian);

    ///Converts the first Bytes of a byte array to an int64_t.
    Q_INVOKABLE static int64_t byteArrayToInt64(QVector<unsigned char> data, bool littleEndian);

    ///Converts the first Bytes of a byte array to a float32.
    Q_INVOKABLE static float byteArrayToFloat32(QVector<unsigned char> data, bool littleEndian);

    ///Converts the first Bytes of a byte array to a float64.
    Q_INVOKABLE static double byteArrayToFloat64(QVector<unsigned char> data, bool littleEndian);

    ///Adds an ascii string to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addStringToArray(QVector<unsigned char> array , QString str);

    ///Adds an uint16 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addUint16ToArray(QVector<unsigned char> array, uint16_t value, bool littleEndian);

    ///Adds an uint32 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addUint32ToArray(QVector<unsigned char> array, uint32_t value, bool littleEndian);

    ///Adds an uint64 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addUint64ToArray(QVector<unsigned char> array, uint64_t value, bool littleEndian);

    ///Adds an int16 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addInt16ToArray(QVector<unsigned char> array, int16_t value, bool littleEndian);

    ///Adds an int32 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addInt32ToArray(QVector<unsigned char> array, int32_t value, bool littleEndian);

    ///Adds an int64 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addInt64ToArray(QVector<unsigned char> array, int64_t value, bool littleEndian);

    ///Adds a float32 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addFloat32ToArray(QVector<unsigned char> array, float value, bool littleEndian);

    ///Adds a float64 to a byte array.
    Q_INVOKABLE static QVector<unsigned char> addFloat64ToArray(QVector<unsigned char> array, double value, bool littleEndian);
};

#endif // SCRIPTCONVERTER_H
