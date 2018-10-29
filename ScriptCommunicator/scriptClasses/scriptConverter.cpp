#include "scriptConverter.h"
#include <QVector>
#include <QScriptEngine>

ScriptConverter::ScriptConverter()
{

}

/**
 * Registers all (for this class) necessary meta types.
 * @param engine
 *      The script engine.
 */
void ScriptConverter::registerScriptMetaTypes(QScriptEngine* engine)
{
    qRegisterMetaType<QScriptEngine*>("ConverterDll*");
    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<int32_t>("int32_t");
    qRegisterMetaType<int16_t>("int16_t");
    qRegisterMetaType<int16_t>("int8_t");
    qRegisterMetaType<int64_t>("uint64_t");
    qRegisterMetaType<int32_t>("uint32_t");
    qRegisterMetaType<int16_t>("uint16_t");
    engine->globalObject().setProperty("conv", engine->newQObject(this));
}

/**
 * Converts a byte array into a hex string.
 * @param data
 *      The data.
 * @return
 *      The created string.
 */
QString ScriptConverter::byteArrayToHexString(QVector<unsigned char> data)
{
    QString result;
    QString tmp;
    for(auto val : data)
    {
        tmp = QString::number(val, 16);

        if(tmp.size() == 1)
        {
            tmp = "0" + tmp;
        }
        result += tmp + " ";
    }
    return result;
}
/**
 * Converts a byte array which contains ascii characters into an ascii string (QString).
 * @param data
 *      The data.
 * @return
 *      The created string.
 */
QString ScriptConverter::byteArrayToString(QVector<unsigned char> data)
{
    return QString::fromLocal8Bit((const char*)data.constData(), data.length());
}

/**
 * Converts a byte array which contains utf8 characters into an utf8 string (QString).
 * @param data
 *      The data.
 * @return
 *      The created string.
 */
QString ScriptConverter::byteArrayToUtf8String(QVector<unsigned char> data)
{
    return QString::fromUtf8((const char*)data.constData(), data.length());
}

/**
 * Converts an ascii string into a byte array.
 * @param str
 *      The string
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::stringToArray(QString str)
{
    QVector<unsigned char> result;
    for(auto val : str.toLocal8Bit())
    {
        result.append(val);
    }
    return result;
}

/**
 * Converts a string into an utf8 byte array.
 * @param str
 *      The string
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::stringToUtf8Array(QString str)
{
    QVector<unsigned char> result;
    for(auto val : str.toUtf8())
    {
        result.append(val);
    }
    return result;
}


/**
 * Adds an ascii string to a byte array.
 * @param array
 *      The byte array.
 * @param str
 *      The string
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addStringToArray(QVector<unsigned char> array , QString str)
{
    for(auto val : str.toLocal8Bit())
    {
        array.append(val);
    }
    return array;
}

/**
 * Converts the first Bytes of a byte array to an uint16_t.
 *
 * Note: This functions works only if the return value is uint32_t (and not uint16_t).
 *
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
uint32_t ScriptConverter::byteArrayToUint16(QVector<unsigned char> data, bool littleEndian)
{
    uint16_t value = 0;
    if(data.length() >= 2)
    {
        if(littleEndian)
        {
            value = data[0] + ((uint16_t)data[1] << 8);
        }
        else
        {
            value = data[1] + ((uint16_t)data[0] << 8);
        }
    }
    return value;
}

/**
 * Converts the first Bytes of a byte array to an uint32_t.
 *
 * Note: This functions works only if the return value is uint64_t (and not uint32_t).
 *
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
uint64_t ScriptConverter::byteArrayToUint32(QVector<unsigned char> data, bool littleEndian)
{
    uint32_t value = 0;
    if(data.length() >= 4)
    {
        if(littleEndian)
        {
            value = data[0] + ((uint32_t)data[1] << 8)  + ((uint32_t)data[2] << 16)  + ((uint32_t)data[3] << 24);
        }
        else
        {
            value = data[3] + ((uint32_t)data[2] << 8)  + ((uint32_t)data[1] << 16)  + ((uint32_t)data[0] << 24);
        }
    }
    return value;
}

/**
 * Converts the first Bytes of a byte array to an uint64_t.
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
uint64_t ScriptConverter::byteArrayToUint64(QVector<unsigned char> data, bool littleEndian)
{
    uint64_t value = 0;

    if(data.length() >= 8)
    {
        if(littleEndian)
        {
            value = data[0] + ((uint64_t)data[1] << 8) + ((uint64_t)data[2] << 16) + ((uint64_t)data[3] << 24)  + ((uint64_t)data[4] << 32)
                     + ((uint64_t)data[5] << 40)  + ((uint64_t)data[6] << 48)  + ((uint64_t)data[7] << 56);
        }
        else
        {
            value = data[7] + ((uint64_t)data[6] << 8) + ((uint64_t)data[5] << 16) + ((uint64_t)data[4] << 24)  + ((uint64_t)data[3] << 32)
                     + ((uint64_t)data[2] << 40)  + ((uint64_t)data[1] << 48)  + ((uint64_t)data[0] << 56);
        }
    }
    return value;
}

/**
 * Converts an unsigned char to a signed char (int8_t).
 *
 * Note: This functions works only if the return value is int16_t (nd not int8_t).
 *
 * @param number
 *      The number.
 * @return
 *      The converted number.
 */
int16_t ScriptConverter::unsignedCharToSignedChar(unsigned char number)
{
    int16_t result = (int8_t)number;
    return result;
}


/**
 * Converts the first Bytes of a byte array to an int16_t.
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
int16_t ScriptConverter::byteArrayToInt16(QVector<unsigned char> data, bool littleEndian)
{
    uint16_t value = ScriptConverter::byteArrayToUint16(data, littleEndian);
    int16_t* result = (int16_t*)&value;
    return *result;
}

/**
 * Converts the first Bytes of a byte array to an int32_t.
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
int32_t ScriptConverter::byteArrayToInt32(QVector<unsigned char> data, bool littleEndian)
{
    uint32_t value = ScriptConverter::byteArrayToUint32(data, littleEndian);
    int32_t* result = (int32_t*)&value;
    return *result;
}

/**
 * Converts the first Bytes of a byte array to an int64_t.
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
int64_t ScriptConverter::byteArrayToInt64(QVector<unsigned char> data, bool littleEndian)
{
    uint64_t value = ScriptConverter::byteArrayToUint64(data, littleEndian);
    int64_t* result = (int64_t*)&value;
    return *result;
}

/**
 * Converts the first Bytes of a byte array to a float32.
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
float ScriptConverter::byteArrayToFloat32(QVector<unsigned char> data, bool littleEndian)
{
    uint32_t value = ScriptConverter::byteArrayToUint32(data, littleEndian);
    float* result = (float*)&value;
    return *result;
}

/**
 * Converts the first Bytes of a byte array to a float64.
 * @param data
 *      The byte array.
 * @param littleEndian
 *      True if the byte order is little endian (least significant byte first).
 * @return
 *      The value.
 */
double ScriptConverter::byteArrayToFloat64(QVector<unsigned char> data, bool littleEndian)
{
    uint64_t value = ScriptConverter::byteArrayToUint64(data, littleEndian);
    double* result = (double*)&value;
    return *result;
}


/**
 * Adds an uint16 to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addUint16ToArray(QVector<unsigned char> array, uint16_t value, bool littleEndian)
{
    if(littleEndian)
    {
        array.append(value & 0xff);
        array.append((value >> 8) & 0xff);
    }
    else
    {
        array.append((value >> 8) & 0xff);
        array.append(value & 0xff);
    }
    return array;
}

/**
 * Adds an uint32 to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addUint32ToArray(QVector<unsigned char> array, uint32_t value, bool littleEndian)
{
    if(littleEndian)
    {
        array.append(value & 0xff);
        array.append((value >> 8) & 0xff);
        array.append((value >> 16) & 0xff);
        array.append((value >> 24) & 0xff);
    }
    else
    {
        array.append((value >> 24) & 0xff);
        array.append((value >> 16) & 0xff);
        array.append((value >> 8) & 0xff);
        array.append(value & 0xff);
    }
    return array;
}

/**
 * Adds an uint64 to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addUint64ToArray(QVector<unsigned char> array, uint64_t value, bool littleEndian)
{
    if(littleEndian)
    {
        array.append(value & 0xff);
        array.append((value >> 8) & 0xff);
        array.append((value >> 16) & 0xff);
        array.append((value >> 24) & 0xff);
        array.append((value >> 32) & 0xff);
        array.append((value >> 40) & 0xff);
        array.append((value >> 48) & 0xff);
        array.append((value >> 56) & 0xff);
    }
    else
    {
        array.append((value >> 56) & 0xff);
        array.append((value >> 48) & 0xff);
        array.append((value >> 40) & 0xff);
        array.append((value >> 32) & 0xff);
        array.append((value >> 24) & 0xff);
        array.append((value >> 16) & 0xff);
        array.append((value >> 8) & 0xff);
        array.append(value & 0xff);
    }
    return array;
}

/**
 * Adds an int16 to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addInt16ToArray(QVector<unsigned char> array, int16_t value, bool littleEndian)
{
    return ScriptConverter::addUint16ToArray(array, (uint16_t) value, littleEndian);
}

/**
 * Adds an int32 to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addInt32ToArray(QVector<unsigned char> array, int32_t value, bool littleEndian)
{
    return ScriptConverter::addUint32ToArray(array, (uint32_t) value, littleEndian);
}

/**
 * Adds an int64 to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addInt64ToArray(QVector<unsigned char> array, int64_t value, bool littleEndian)
{
    return ScriptConverter::addUint64ToArray(array, (uint64_t) value, littleEndian);
}

/**
 * Adds a float to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addFloat32ToArray(QVector<unsigned char> array, float value, bool littleEndian)
{
    uint32_t* tmp = (uint32_t*)&value;
    return ScriptConverter::addUint32ToArray(array, *tmp, littleEndian);
}

/**
 * Adds a float64 to a byte array.
 * @param array
 *      The byte array.
 * @param value
 *      The Value
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addFloat64ToArray(QVector<unsigned char> array, double value, bool littleEndian)
{
    uint64_t* tmp = (uint64_t*)&value;
    return ScriptConverter::addUint64ToArray(array, *tmp, littleEndian);
}
