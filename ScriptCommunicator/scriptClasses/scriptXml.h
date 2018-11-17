#ifndef SCRIPTXML_H
#define SCRIPTXML_H

#include <QObject>
#include <QDomDocument>
#include <QDomNode>
#include <QScriptable>
#include <QScriptEngine>
#include <QBuffer>
#include <QXmlStreamWriter>
#include "scriptObject.h"

class ScriptFile;

///This class provides functions for creating/writing XML files.
///Note: All function are working on an internal XML buffer.
///The function writteBufferToFile must be used to write the content of the internal XML buffer
///to a file.
class ScriptXmlWriter : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:

    ScriptXmlWriter(ScriptFile* scriptFileObject, QObject *parent = 0) : QObject(parent),
        m_scriptFileObject(scriptFileObject)
    {
        m_xmlWriter.setAutoFormatting(true);
        m_xmlWriter.setAutoFormattingIndent(2);
        m_xmlBuffer.open(QIODevice::WriteOnly);
        m_xmlWriter.setDevice(&m_xmlBuffer);
    }

    ///Registers all (for this class) necessary meta types.
    static void registerScriptMetaTypes(QScriptEngine* scriptEngine)
    {
        (void)scriptEngine;
        qRegisterMetaType<ScriptXmlWriter*>("ScriptXmlWriter*");
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptXmlWriter.api");
    }

    ///Writes the internal XML Buffer to a file.
    Q_INVOKABLE bool writeBufferToFile(QString fileName, bool isRelativePath=true);

    ///Returns the content of the internal buffer.
    Q_INVOKABLE QString getInternalBuffer(void){return m_xmlBuffer.data();}

    ///Clears the internal buffer.
    Q_INVOKABLE void clearInternalBuffer(void){m_xmlBuffer.close();m_xmlBuffer.open(QIODevice::WriteOnly);}

    ///Sets the codec for this object to codec. The codec is used for encoding any data that is written.
    ///By default, ScriptXmlWriter uses UTF-8.
    ///The encoding information is stored in the initial xml tag which gets written when you call writeStartDocument().
    ///Call this function before calling writeStartDocument().
    Q_INVOKABLE void setCodec(QString codecName){m_xmlWriter.setCodec(codecName.toLocal8Bit().constData());}

    ///Sets the autoFormatting property. This property controls whether or not the stream writer automatically formats
    ///the generated XML data. If enabled, the writer automatically adds line-breaks and indentation to empty sections between elements
    ///(ignorable whitespace). The main purpose of auto-formatting is to split the data into several lines, and to
    ///increase readability for a human reader. The indentation depth can be controlled through the autoFormattingIndent property.
    Q_INVOKABLE void setAutoFormatting(bool autoFormatting){m_xmlWriter.setAutoFormatting(autoFormatting);}

    ///Returns the value of the autoFormating property.
    Q_INVOKABLE bool autoFormatting(void){return m_xmlWriter.autoFormatting();}

    ///Set the autoFormatingIndent property. This property holds the number of spaces or tabs used for indentation
    ///when auto-formatting is enabled. Positive numbers indicate spaces, negative numbers tabs.
    Q_INVOKABLE void setAutoFormattingIndent(int spacesOrTabs){m_xmlWriter.setAutoFormattingIndent(spacesOrTabs);}

    ///Returns the autoFormatingIndent property.
    Q_INVOKABLE int autoFormattingIndent(void){return m_xmlWriter.autoFormattingIndent();}

    ///Writes a document start with the attribute version.
    Q_INVOKABLE void writeStartDocument(QString version="1.0"){m_xmlWriter.writeStartDocument(version);}

    ///Writes a document start with the attributes version and standalone.
    Q_INVOKABLE void writeStartDocument(bool standalone, QString version="1.0"){m_xmlWriter.writeStartDocument(version, standalone);}

    ///Closes all remaining open start elements and writes a newline.
    Q_INVOKABLE void writeEndDocument(void){m_xmlWriter.writeEndDocument();}

    ///Writes a namespace declaration for namespaceUri with prefix. If prefix is empty, ScriptXmlWriter assigns a
    ///unique prefix consisting of the letter 'n' followed by a number.
    ///If writeStartElement() or writeEmptyElement() was called, the declaration applies to the current element;
    ///otherwise it applies to the next child element.
    ///Note that the prefix xml is both predefined and reserved for http://www.w3.org/XML/1998/namespace, which in turn
    ///cannot be bound to any other prefix. The prefix xmlns and its URI http://www.w3.org/2000/xmlns/ are used for the
    ///namespace mechanism itself and thus completely forbidden in declarations.
    Q_INVOKABLE void writeNamespace(QString namespaceUri, QString prefix = ""){m_xmlWriter.writeNamespace(namespaceUri, prefix);}

    ///Writes a default namespace declaration for namespaceUri.
    ///If writeStartElement() or writeEmptyElement() was called, the declaration applies to the current element;
    ///otherwise it applies to the next child element.
    ///Note that the namespaces http://www.w3.org/XML/1998/namespace (bound to xmlns) and
    ///http://www.w3.org/2000/xmlns/ (bound to xml) by definition cannot be declared as default.
    Q_INVOKABLE void writeDefaultNamespace(const QString namespaceUri){m_xmlWriter.writeDefaultNamespace(namespaceUri);}

    ///Writes a start element with name, prefixed for the specified namespaceUri. If the namespace has not been declared yet,
    ///ScriptXmlWriter will generate a namespace declaration for it. Subsequent calls to writeAttribute() will add attributes to this element.
    Q_INVOKABLE void writeStartElement(QString name, QString namespaceUri=""){m_xmlWriter.writeStartElement(namespaceUri, name);}

    ///Writes an empty element with name, prefixed for the specified namespaceUri. If the namespace has not been declared,
    ///ScriptXmlWriter will generate a namespace declaration for it. Subsequent calls to writeAttribute() will add attributes to this element.
    Q_INVOKABLE void writeEmptyElement(QString name, QString namespaceUri=""){m_xmlWriter.writeEmptyElement(namespaceUri,name);}

    ///Writes a text element with name, prefixed for the specified namespaceUri, and text.
    ///If the namespace has not been declared, ScriptXmlWriter will generate a namespace declaration for it.
    ///This is a convenience function equivalent to:
    ///     writeStartElement(name, namespaceUri);
    ///     writeCharacters(text);
    ///     writeEndElement();
    Q_INVOKABLE void writeTextElement(QString name, QString text, QString namespaceUri=""){m_xmlWriter.writeTextElement(namespaceUri, name, text);}

    ///Closes the previous start element.
    Q_INVOKABLE void writeEndElement(void){m_xmlWriter.writeEndElement();}

    ///Writes an attribute with name and value, prefixed for the specified namespaceUri. If the namespace has not been declared yet, ScriptXmlWriter will
    ///generate a namespace declaration for it. This function can only be called after writeStartElement() or writeEmptyElement() have been called.
    Q_INVOKABLE void writeAttribute(QString name, QString value, QString namespaceUri=""){m_xmlWriter.writeAttribute(namespaceUri, name, value);}

    ///Writes text as CDATA section. If text contains the forbidden character sequence "]]>", it is split into different
    ///CDATA sections.This function mainly exists for completeness. Normally you should not need use it, because
    ///writeCharacters() automatically escapes all non-content characters.
    Q_INVOKABLE void writeCDATA(QString text){m_xmlWriter.writeCDATA(text);}

    ///Writes text. The characters "<", "&", and """ are escaped as entity references "&lt;", "&amp;, and "&quot;".
    ///To avoid the forbidden sequence "]]>", ">" is also escaped as "&gt;".
    Q_INVOKABLE void writeCharacters(QString text){m_xmlWriter.writeCharacters(text);}

    ///Writes text as XML comment, where text must not contain the forbidden sequence "--" or end with "-".
    ///Note that XML does not provide any way to escape "-" in a comment.
    Q_INVOKABLE void writeComment(QString text){m_xmlWriter.writeComment(text);}

    ///Writes a DTD section. The dtd represents the entire doctypedecl production from the XML 1.0 specification.
    Q_INVOKABLE void writeDTD(QString dtd){m_xmlWriter.writeDTD(dtd);}

    ///Writes the entity reference name to the internal buffer, as "name".
    Q_INVOKABLE void writeEntityReference(QString name){m_xmlWriter.writeEntityReference(name);}

    ///Writes an XML processing instruction with target and data, where data must not contain the sequence "?>".
    Q_INVOKABLE void writeProcessingInstruction(QString target, QString data = ""){m_xmlWriter.writeProcessingInstruction(target, data);}


private:
    ///The internal data buffer.
    QBuffer m_xmlBuffer;

    ///The xml writer object.
    QXmlStreamWriter m_xmlWriter;

    ///Pointer to the script file object.
    ScriptFile* m_scriptFileObject;

};

///This class represents a xml attributte.
class ScriptXmlAttribute : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    ScriptXmlAttribute(QString name, QString value, QObject *parent = 0) : QObject(parent),
        m_name(name), m_value(value)
    {

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptXmlAttribute.api");
    }

    ///Returns the value of the attribute.
    Q_INVOKABLE QString value(void){return m_value;}

    ///Returns the name of the attribute.
    Q_INVOKABLE QString name(void){return m_name;}
private:
    ///The name of the attribute.
    QString m_name;

    ///The value of the attribute.
    QString m_value;
};

///Represents a xml element.
class ScriptXmlElement : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    ScriptXmlElement(const QDomNode& node, QObject *parent = 0) : QObject(parent), m_node(node)
    {
    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptXmlElement.api");
    }

    ///Returns the name of this element.
    Q_INVOKABLE QString elementName(void){return m_node.nodeName();}

    ///Returns all child elements.
    Q_INVOKABLE QList<ScriptXmlElement*> childElements(void);

    ///Returns all child text elements (includes the CDATA elements).
    Q_INVOKABLE QStringList childTextElements(void);

    ///Returns all child CDATA elements.
    Q_INVOKABLE QStringList childCDataElements(void);

    ///Returns all child comment elements.
    Q_INVOKABLE QStringList childCommentElements(void);

    ///Returns all attributes of this element.
    Q_INVOKABLE QList<ScriptXmlAttribute*> attributes(void);

    ///Returns an attribute value. The attribute is identified by attrName.
    Q_INVOKABLE QString attributeValue(QString attrName){return m_node.attributes().namedItem(attrName).nodeValue();}

private:
    QDomNode m_node;

};

///Class for reading a xml file.
class ScriptXmlReader : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptXmlReader(ScriptFile* scriptFileObject, QObject *parent = 0);

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptXmlReader.api");
    }

    ///Reads and parses a xml file. The parsed xml file is stored internally.
    Q_INVOKABLE qint32 readFile(QString fileName, bool isRelativePath=true);

    ///Parses a xml string. The parsed xml string is stored internally.
    Q_INVOKABLE bool parseString(QString xmlString);

    ///Returns a list containing all xml elements with the name 'name'.
    ///Note: The xml root element is not included.
    Q_INVOKABLE QList<ScriptXmlElement*> elementsByTagName(QString name);

    ///Returns the root xml element.
    Q_INVOKABLE ScriptXmlElement *getRootElement(void);

    ///Registers all (for this class) necessary meta types.
    static void registerScriptMetaTypes(QScriptEngine* scriptEngine)
    {
        qRegisterMetaType<ScriptXmlReader*>("ScriptXmlReader*");
        qRegisterMetaType<ScriptXmlElement*>("ScriptXmlElement*");
        qRegisterMetaType<ScriptXmlAttribute*>("ScriptXmlAttribute*");
        qRegisterMetaType<QList<ScriptXmlElement*>>("QList<ScriptXmlElement*>");
        qRegisterMetaType<QList<ScriptXmlAttribute*>>("QList<ScriptXmlAttribute*>");

        qScriptRegisterSequenceMetaType<QList<ScriptXmlElement*>>(scriptEngine);
        qScriptRegisterSequenceMetaType<QList<ScriptXmlAttribute*>>(scriptEngine);
    }

private:

    ///Pointer to the script file object.
    ScriptFile* m_scriptFileObject;

    ///The content of the parsed xml file.
    QDomDocument m_xmlDocument;

    ///The xml root element.
    QDomElement m_rootElement;
};

#endif // SCRIPTXML_H
