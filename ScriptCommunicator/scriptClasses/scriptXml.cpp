#include "scriptXml.h"
#include "scriptFile.h"
#include <QFile>
#include <QTextStream>


/**
 * Writes the internal XML Buffer to a file.
 * @param fileName
 *      The name/path of the file.
 * @param isRelativePath
 *      True if the file path is relative to the main script.
 */
bool ScriptXmlWriter::writeBufferToFile(QString fileName, bool isRelativePath)
{
    fileName = isRelativePath ? m_scriptFileObject->createAbsolutePath(fileName) : fileName;
    bool success = false;

    QFile file(fileName);
    file.remove();
    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream data( &file );
        data.setCodec("UTF-8");

        data << m_xmlBuffer.data();
        file.close();
        success = true;
    }

    return success;
}

/**
 * Returns all child CDATA elements.
 * @return
 *      The elements.
 */
QStringList ScriptXmlElement::childCDataElements()
{
    QDomNodeList nodeList = m_node.childNodes();
    QStringList result;

    for(qint32 i = 0; i < nodeList.length(); i++)
    {
        if(nodeList.at(i).isCDATASection())
        {
            result.append(nodeList.at(i).toCDATASection().data());
        }
    }
    return result;
}

/**
 * Returns all child comment elements.
 * @return
 *      The elements.
 */
QStringList ScriptXmlElement::childCommentElements()
{
    QDomNodeList nodeList = m_node.childNodes();
    QStringList result;

    for(qint32 i = 0; i < nodeList.length(); i++)
    {
        if(nodeList.at(i).isComment())
        {
            result.append(nodeList.at(i).toComment().data());
        }
    }
    return result;
}

/**
 * Returns all child text elements (includes the CDATA elements).
 * @return
 *      The elements.
 */
QStringList ScriptXmlElement::childTextElements()
{
    QDomNodeList nodeList = m_node.childNodes();
    QStringList result;

    for(qint32 i = 0; i < nodeList.length(); i++)
    {
        if(nodeList.at(i).isText())
        {
            result.append(nodeList.at(i).toText().data());
        }
    }
    return result;
}

/**
 * Returns all child elements.
 * @return
 *      The elements.
 */
QList<ScriptXmlElement*> ScriptXmlElement::childElements()
{
    QDomNodeList nodeList = m_node.childNodes();
    QList<ScriptXmlElement*> result;

    for(qint32 i = 0; i < nodeList.length(); i++)
    {
        if(nodeList.at(i).isElement())
        {
            ScriptXmlElement* newNode = new ScriptXmlElement(nodeList.at(i), engine());
            result.append(newNode);
        }
    }

    return result;
}

/**
 * Returns all attributes of this element.
 * @return
 *      The attributes.
 */
QList<ScriptXmlAttribute*> ScriptXmlElement::attributes()
{
    QDomNamedNodeMap attributes = m_node.attributes();
    QList<ScriptXmlAttribute*> result;

    for(qint32 i = 0; i < attributes.length(); i++)
    {
        QDomNode node = attributes.item(i);
        ScriptXmlAttribute* newAttr = new ScriptXmlAttribute(node.nodeName(), node.nodeValue(), engine());
        result.append(newAttr);
    }

    return result;
}


/**
 * Constructor.
 * @param scriptFileObject
 *      Pointer to the file object.
 * @param parent
 *      Pointer to the parent.
 */
ScriptXmlReader::ScriptXmlReader(ScriptFile* scriptFileObject, QObject *parent) : QObject(parent),
    m_scriptFileObject(scriptFileObject), m_xmlDocument(), m_rootElement()
{

}


/**
 * Reads a xml file.
 * @param name
 *      The name of the file.
 * @param isRelativePath
 *      True if the file path is relative to the main script.
 * @return
 *      0: success
 *      1: file could not be opened
 *      2: parse error
 */
qint32 ScriptXmlReader::readFile(QString fileName, bool isRelativePath)
{
    qint32 result = 0;
    fileName = isRelativePath ? m_scriptFileObject->createAbsolutePath(fileName) : fileName;

    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly))
    {
        QByteArray content = file.readAll();
        file.close();
        if (m_xmlDocument.setContent(content))
        {
            result = 0;
            m_rootElement = m_xmlDocument.documentElement();
        }
        else
        {
            result= 2;
        }
    }
    else
    {
        result = 1;
    }

    return result;
}


/**
 * Parses a xml string. The parsed xml string is stored internally.
 * @param xmlString
 *      The xml string.
 * @return
 *      True on success.
 */
bool ScriptXmlReader::parseString(QString xmlString)
{
    bool result = false;

    if (m_xmlDocument.setContent(xmlString))
    {
        result = true;
        m_rootElement = m_xmlDocument.documentElement();
    }

    return result;
}

/**
 * Returns the root xml element.
 * @return
 *      The root element.
 */
ScriptXmlElement* ScriptXmlReader::getRootElement(void)
{
    ScriptXmlElement* newNode = new ScriptXmlElement(m_rootElement);
    engine()->newQObject(newNode, QScriptEngine::ScriptOwnership);
    return newNode;
}

/**
 * Returns the root xml element.
 * @param name
 *      The name of the element.
 * @return
 *      The element.
 */
QList<ScriptXmlElement*> ScriptXmlReader::elementsByTagName(QString name)
{
    QDomNodeList nodeList = m_rootElement.elementsByTagName(name);
    QList<ScriptXmlElement*> result;

    for(qint32 i = 0; i < nodeList.length(); i++)
    {
        ScriptXmlElement* newNode = new ScriptXmlElement(nodeList.at(i), engine());
        result.append(newNode);
    }

    return result;
}
