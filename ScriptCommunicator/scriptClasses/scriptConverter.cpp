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
 * Converts a byte array which contains ascii characters into a ascii string (QString).
 * @param data
 *      The data.
 * @return
 *      The created string.
 */
QString ScriptConverter::byteArrayToString(QVector<unsigned char> data)
{
    QString result;
    for(auto val : data)
    {
        result.append(static_cast<char>(val));
    }
    return result;
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

uint16_t ScriptConverter::byteArrayToUint16(QVector<unsigned char> data, bool lsb)
{
    uint16_t value = 0;
    if(data.length() >= 2)
    {
        if(lsb)
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

uint32_t ScriptConverter::byteArrayToUint32(QVector<unsigned char> data, bool lsb)
{
    uint32_t value = 0;
    if(data.length() >= 2)
    {
        if(lsb)
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

uint64_t ScriptConverter::byteArrayToUint64(QVector<unsigned char> data, bool lsb)
{
    uint64_t value = 0;

    if(data.length() >= 8)
    {
        if(lsb)
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

int16_t ScriptConverter::byteArrayToInt16(QVector<unsigned char> data, bool lsb)
{
    uint16_t value = ScriptConverter::byteArrayToUint16(data, lsb);
    int16_t* result = (int16_t*)&value;
    return *result;
}

int32_t ScriptConverter::byteArrayToInt32(QVector<unsigned char> data, bool lsb)
{
    uint32_t value = ScriptConverter::byteArrayToUint32(data, lsb);
    int32_t* result = (int32_t*)&value;
    return *result;
}

int64_t ScriptConverter::byteArrayToInt64(QVector<unsigned char> data, bool lsb)
{
    uint64_t value = ScriptConverter::byteArrayToUint64(data, lsb);
    int64_t* result = (int64_t*)&value;
    return *result;
}

float ScriptConverter::byteArrayToFloat32(QVector<unsigned char> data, bool lsb)
{
    uint32_t value = ScriptConverter::byteArrayToUint32(data, lsb);
    float* result = (float*)&value;
    return *result;
}

double ScriptConverter::byteArrayToFloat64(QVector<unsigned char> data, bool lsb)
{
    uint64_t value = ScriptConverter::byteArrayToUint64(data, lsb);
    float* result = (float*)&value;
    return *result;
}

#if 0
/**
 * Adds an uint16 string to a byte array.
 * @param array
 *      The byte array.
 * @param str
 *      The string
 * @return
 *      The byte array.
 */
QVector<unsigned char> ScriptConverter::addUint16ToArray(QVector<unsigned char> array , QString str)
{
     array.append(val);
    }
    return array;
}
#endif

