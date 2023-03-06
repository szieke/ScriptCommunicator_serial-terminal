#ifndef SCRIPTHELPER_H
#define SCRIPTHELPER_H

#include <QJSValueIterator>
#include <QJSEngine>
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
        for(auto el : list){bytesArray.append(el.toUInt());}
        return bytesArray;
    }

}


#endif // SCRIPTHELPER_H
