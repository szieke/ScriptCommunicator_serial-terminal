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
