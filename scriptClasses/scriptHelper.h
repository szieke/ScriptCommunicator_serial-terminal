#ifndef SCRIPTHELPER_H
#define SCRIPTHELPER_H

#include <QScriptValueIterator>
#include <QScriptEngine>
#include <QObject>

namespace ScriptHelper
{



///Converts a byte array to a variant list.
static inline QList<QVariant> byteArrayToVariantList(QByteArray byteArray)
{
    QList<QVariant> list;
    for(auto el : byteArray){list.push_back((unsigned char) el);}
    return list;
}

///Converts a variant list to a byte array.
static inline QByteArray variantListToByteArray(QList<QVariant> list)
{
    QByteArray bytesArray;
    for(auto el : list){bytesArray.append(el.toChar());}
    return bytesArray;
}

/**
 * Converts a byte array into a hex string.
 * @param data
 *      The data.
 * @return
 *      The created string.
 */
static inline QString byteArrayToHexString(QVector<unsigned char> data)
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
static inline QString byteArrayToString(QVector<unsigned char> data)
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
static inline QVector<unsigned char> stringToArray(QString str)
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
static inline QVector<unsigned char> addStringToArray(QVector<unsigned char> array , QString str)
{
    for(auto val : str.toLocal8Bit())
    {
        array.append(val);
    }
    return array;
}
}
///Script map class.
class ScriptMap : public QMap<QString, QVariant>
{
public:

    ScriptMap(){}
    ScriptMap(QMap<QString, QVariant> map)
    {
        QMap<QString,QVariant>::const_iterator it(map.begin());
        for(; it != map.end(); ++it)
        {
            insert(it.key(), it.value());
        }
    }

    static QScriptValue ScriptMapToScriptValue(QScriptEngine* eng, const ScriptMap& map)
    {
        QScriptValue a = eng->newObject();
        ScriptMap::const_iterator it(map.begin());
        for(; it != map.end(); ++it)
        {
            QString key = it.key();
            QVariant val = it.value();
            if(val.type() == QVariant::ByteArray){val = ScriptHelper::byteArrayToVariantList(val.toByteArray());}
            a.setProperty(key, qScriptValueFromValue(eng, val));
        }
        return a;
    }

    static void ScriptMapFromScriptValue( const QScriptValue& value, ScriptMap& map)
    {
        QScriptValueIterator itr(value);
        while(itr.hasNext())
        {
            itr.next();
            map[itr.name()] = qscriptvalue_cast<ScriptMap::mapped_type>(itr.value());
            if(map[itr.name()].type() == QVariant::List){map[itr.name()] = ScriptHelper::variantListToByteArray(map[itr.name()].toList());}
        }
    }

    static void registerScriptMetaTypes(QScriptEngine* engine)
    {
        qScriptRegisterMetaType<ScriptMap>(engine, ScriptMapToScriptValue, ScriptMapFromScriptValue);
    }
}; // work around because typedefs do not register correctly.
Q_DECLARE_METATYPE(ScriptMap)


#endif // SCRIPTHELPER_H
