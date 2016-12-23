#include "parseThread.h"
#include <QFile>
#include <QFileInfo>
#include "mainwindow.h"
#include <QDirIterator>
#include <QTextStream>
#include <QCoreApplication>

/**
 * Returns the folder ich which the ScriptEditor files
 * are locared (api files).
 * @return
 *      The folder.
 */
QString getScriptEditorFilesFolder(void)
{
#ifdef Q_OS_MAC
    return QCoreApplication::applicationDirPath() + "/../../..";
#else
    return QCoreApplication::applicationDirPath();
#endif
}

ParseThread::ParseThread(QObject *parent) : QThread(parent), m_autoCompletionApiFiles(), m_autoCompletionEntries(), m_objectAddedToCompletionList(false),
    m_creatorObjects(), m_stringList(), m_unknownTypeObjects(), m_arrayList(), m_tableWidgets(),
    m_tableWidgetObjects(), m_foundUiFiles(), m_parsedFiles()
{
    if(m_autoCompletionApiFiles.isEmpty())
    {
        //Parse all api files.
        QDirIterator it(getScriptEditorFilesFolder()+ "/apiFiles", QStringList() << "*.api", QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext())
        {
            QFile file(it.next());
            if (file.open(QFile::ReadOnly))
            {
                QTextStream in(&file);
                QString singleEntry = in.readLine();
                QString fileName = QFileInfo(file.fileName()).fileName();

                while(!singleEntry.isEmpty())
                {
                    //Add the current line to the api map.
                    m_autoCompletionApiFiles[fileName] << QString(singleEntry).replace("\\n", "\n");
                    singleEntry = in.readLine();
                }
            }
            file.close();
        }
    }
}

/**
 * The thread main function
 */
void ParseThread::run()
{
    exec();
}

/**
 * Removes in text all left of character.
 * @param text
 *      The string.
 * @param character
 *      The character.
 */
static inline void removeAllLeft(QString& text, QString character)
{
    int index = text.indexOf(character);
    if(index != -1)
    {
        text = text.right((text.length() - index) - 1);
    }
}

/**
 * Searches a single object type.
 * @param className
 *      The class name.
 * @param searchString
 *      The search sring.
 * @param lines
 *      The lines in which shall be searched.
 */
void ParseThread::searchSingleType(QString className, QString searchString, QStringList& lines, bool isArray,
                                    bool withOutDotsAndBracked, bool replaceExistingEntry, bool matchExact)
{
    int index;
    for(int i = 0; i < lines.length();i++)
    {
        index = lines[i].indexOf(searchString);
        if(index != -1)
        {
            if(withOutDotsAndBracked)
            {
                if(lines[i].indexOf(".", index) != -1)
                {
                    continue;
                }
                if(lines[i].indexOf("[", index) != -1)
                {
                    continue;
                }
            }

            if(matchExact)
            {
                if(lines[i].length() != (index + searchString.length()))
                {
                    continue;
                }
            }
            QString objectName =  lines[i].left(index);

            index = lines[i].indexOf("=");
            if(index != -1)
            {
                objectName = lines[i].left(index);
            }

            removeAllLeft(objectName, "{");
            removeAllLeft(objectName, ")");

            bool containsInvalidChars = false;
            for(auto character : objectName)
            {
                if(!character.isLetterOrNumber() && (character != '_') && (character != '[')
                        && (character != ']'))
                {
                    containsInvalidChars = true;
                    break;
                }
            }

            if(containsInvalidChars)
            {
                continue;
            }

            addObjectToAutoCompletionList(objectName, className, false, isArray, replaceExistingEntry);
        }
    }
}

/**
 * Removes all unnecessary characters (e.g. comments).
 * @param currentText
 *      The text.
 */
void ParseThread::removeAllUnnecessaryCharacters(QString& currentText)
{


    //Remove all '/**/' comments.
    QRegExp comment("/\\*(.|[\r\n])*\\*/");
    comment.setMinimal(true);
    comment.setPatternSyntax(QRegExp::RegExp);
    currentText.replace(comment, " ");

    //Remove all '//' comments.
    comment.setMinimal(false);
    comment.setPattern("//[^\n]*");
    currentText.remove(comment);

    currentText.replace("var ", "");
    currentText.replace(" ", "");
    currentText.replace("\t", "");
    currentText.replace("\r", "");
}

/**
 * Removes all square brackets and all between them.
 * @param currentText
 *      The text.
 */
void ParseThread::removeAllBetweenSquareBrackets(QString& currentText)
{
    int index1 = 0;
    int index2 = 0;
    bool textRemoved = true;

    while(textRemoved)
    {
        index1 = currentText.indexOf("[");
        index2 = currentText.indexOf("]", index1);

        if((index1 != -1) && (index2 != -1) && (index2 > index1))
        {
            currentText.remove(index1, (index2 - index1) + 1);
        }
        else
        {
            textRemoved = false;
        }
    }
}
/**
 * Parses a widget list from a user interface file (auto-completion).
 * @param docElem
 *      The QDomElement from the user interface file.
 * @param parseActions
 *      If true then all actions will be parsed. If false then all widgets will be parsed.
 */
void ParseThread::parseWidgetList(QDomElement& docElem, bool parseActions)
{
    QDomNodeList nodeList = docElem.elementsByTagName((parseActions) ? "addaction" : "widget");

    //Parse all widgets or actions.
    for (int i = 0; i < nodeList.size(); i++)
    {
        QDomNode nodeItem = nodeList.at(i);
        QString className = (parseActions) ? "QAction" : nodeItem.attributes().namedItem("class").nodeValue();
        QString objectName = nodeItem.attributes().namedItem("name").nodeValue();

        if(className == QString("QComboBox"))
        {
            className = "ScriptComboBox";
        }
        else if(className == QString("QFontComboBox"))
        {
            className = "ScriptFontComboBox";
        }
        else if(className == QString("QLineEdit"))
        {
            className = "ScriptLineEdit";
        }
        else if(className == QString("QTableWidget"))
        {
            className = "ScriptTableWidget";
        }
        else if(className == QString("QTextEdit"))
        {
            className = "ScriptTextEdit";
        }
        else if(className == QString("QCheckBox"))
        {
            className = "ScriptCheckBox";
        }
        else if(className == QString("QPushButton"))
        {
            className = "ScriptButton";
        }
        else if(className == QString("QToolButton"))
        {
            className = "ScriptToolButton";
        }
        else if(className == QString("QWidget"))
        {
            className = "ScriptWidget";
        }
        else if(className == QString("QDialog"))
        {
            className = "ScriptDialog";
        }
        else if(className == QString("QProgressBar"))
        {
            className = "ScriptProgressBar";
        }
        else if(className == QString("QLabel"))
        {
            className = "ScriptLabel";
        }
        else if(className == QString("QSlider"))
        {
            className = "ScriptSlider";
        }
        else if(className == QString("QDial"))
        {
            className = "ScriptDial";
        }
        else if(className == QString("QMainWindow"))
        {
            className = "ScriptMainWindow";
        }
        else if(className == QString("QAction"))
        {
            className = "ScriptAction";
        }
        else if(className == QString("QStatusBar"))
        {
            className = "ScriptComboBox";
        }
        else if(className == QString("QTabWidget"))
        {
            className = "ScriptTabWidget";
        }
        else if(className == QString("QGroupBox"))
        {
            className = "ScriptGroupBox";
        }
        else if(className == QString("QRadioButton"))
        {
            className = "ScriptRadioButton";
        }
        else if(className == QString("QSpinBox"))
        {
            className = "ScriptSpinBox";
        }
        else if(className == QString("QDoubleSpinBox"))
        {
            className = "ScriptDoubleSpinBox";
        }
        else if(className == QString("QTimeEdit"))
        {
            className = "ScriptTimeEdit";
        }
        else if(className == QString("QDateEdit"))
        {
            className = "ScriptDateEdit";
        }
        else if(className == QString("QDateTimeEdit"))
        {
            className = "ScriptDateTimeEdit";
        }
        else if(className == QString("QListWidget"))
        {
            className = "ScriptListWidget";
        }
        else if(className == QString("QTreeWidget"))
        {
            className = "ScriptTreeWidget";
        }
        else if(className == QString("QSplitter"))
        {
            className = "ScriptSplitter";
        }
        else if(className == QString("QToolBox"))
        {
            className = "ScriptToolBox";
        }
        else if(className == QString("QCalendarWidget"))
        {
            className = "ScriptCalendarWidget";
        }
        addObjectToAutoCompletionList(objectName, className, true);
    }
}


/**
 * Adds on obect to the auto-completion list.
 * @param objectName
 *      The object name.
 * @param className
 *      The class name.
 * @param isGuiElement
 *      True if the object is a GUI element.
 */
void ParseThread::addObjectToAutoCompletionList(QString& objectName, QString& className, bool isGuiElement, bool isArray, bool replaceExistingEntry)
{
    QString autoCompletionName = isGuiElement ? "UI_" + objectName : objectName;


    if(!objectName.isEmpty())
    {

        if(m_autoCompletionEntries.contains(autoCompletionName))
        {
            if(replaceExistingEntry)
            {
                m_autoCompletionEntries.remove(autoCompletionName);
            }
            else
            {
                return;
            }
        }

        if(!isArray)
        {
            if (m_autoCompletionApiFiles.contains(className + ".api"))
            {
                QStringList list = m_autoCompletionApiFiles[className + ".api"];
                m_objectAddedToCompletionList = true;

                for(auto singleEntry : list)
                {
                    singleEntry.replace(className + "::", autoCompletionName + "::");

                    //Add the current line to the api map.
                    m_autoCompletionEntries[autoCompletionName] << singleEntry;
                }
            }
            else
            {
                if(!m_unknownTypeObjects.contains(autoCompletionName))
                {
                    m_unknownTypeObjects.append(autoCompletionName);
                }
            }
        }
        else
        {
            m_objectAddedToCompletionList = true;

            if (m_autoCompletionApiFiles.contains(className + ".api"))
            {
                QStringList list = m_autoCompletionApiFiles[className + ".api"];

                for(auto singleEntry : list)
                {

                    singleEntry.replace(className + "::", autoCompletionName + "[" + "::");

                    //Add the current line to the api map.
                    m_autoCompletionEntries[autoCompletionName + "["] << singleEntry;
                }
            }
            else
            {
                if(!m_unknownTypeObjects.contains(autoCompletionName + "["))
                {
                    m_unknownTypeObjects.append(autoCompletionName + "[");
                }
            }

            QStringList arrayList = m_autoCompletionApiFiles["Array.api"];
            for(auto singleEntry : arrayList)
            {
                singleEntry.replace("Array::", autoCompletionName + "::");

                //Add the current line to the api map.
                m_autoCompletionEntries[autoCompletionName] << singleEntry;

                int index = m_arrayList.indexOf(autoCompletionName);
                if(index != -1)
                {
                    m_arrayList.remove(index);
                }
                m_arrayList.append(autoCompletionName);
            }
        }

        if(className == "String")
        {
            int index = m_stringList.indexOf(autoCompletionName);
            if(index != -1)
            {
                m_stringList.remove(index);
            }
            m_stringList.append(autoCompletionName);
        }
        else if(className == "ScriptTableWidget")
        {
            int index = m_tableWidgets.indexOf(autoCompletionName);
            if(index != -1)
            {
                m_tableWidgets.remove(index);
            }
            m_tableWidgets.append(autoCompletionName);
        }
        else
        {
            if((className != "Dummy"))
            {
                m_creatorObjects[autoCompletionName] = className;
            }
        }
    }
}

/**
 * Parses an user interface file (auto-completion).
 * @param uiFileName
 *      The user interface file.
 */
void ParseThread::parseUiFile(QString uiFileName)
{

    QFile uiFile(uiFileName);
    QDomDocument doc("ui");

    if(!m_parsedFiles.contains(uiFileName))
    {
        m_parsedFiles.append(uiFileName);

        if (uiFile.open(QFile::ReadOnly))
        {
            if (doc.setContent(&uiFile))
            {
                QDomElement docElem = doc.documentElement();
                parseWidgetList(docElem, false);
                parseWidgetList(docElem, true);
            }

            uiFile.close();
            if(!m_foundUiFiles.contains(uiFileName))
            {
                m_foundUiFiles.append(uiFileName);
            }
        }
    }

}
/**
 * Checks if in the current document user interface files are loaded.
 * If user interface are loaded then they will be parsed and added to the auto-completion
 * list (m_autoCompletionEntries).
 */
void ParseThread::checkDocumentForUiFiles(QString& currentText, QString& activeDocument)
{
    int index;

    index = currentText.indexOf("scriptThread.loadUserInterfaceFile");

    while(index != -1)
    {

        QString tmpString =  currentText.right(currentText.length() - index);
        QStringList list = tmpString.split("(");
        QString uiFile;
        bool isRelativePath = true;

        if(list.length() == 1)
        {//( not found.
            break;
        }

        list = list[1].split(")");

        if(list.length() == 1)
        {//) not found.
            break;
        }

        list = list[0].split(",");
        uiFile = list[0];
        if(list.length() > 1)
        {
            isRelativePath = list[1].contains("true") ? true : false;
        }


        uiFile.replace("\"", "");
        uiFile.replace("'", "");

        if(isRelativePath)
        {
            uiFile = QFileInfo(activeDocument).absolutePath() + "/" + uiFile;
        }

        parseUiFile(uiFile);
        index = currentText.indexOf("scriptThread.loadUserInterfaceFile", index + 1);

    }
}

/**
 * Searches all dynamically created objects created by custom objects (like ScriptTimer).
 * @param currentText
 *      The text in which shall be searched.
 */
void ParseThread::checkDocumentForCustomDynamicObjects(QStringList& lines, QStringList &linesWithBrackets , QString &currentText, int passNumber)
{

    if(passNumber == 1)
    {
        if(currentText.contains("=scriptThread."))
        {
            searchSingleType("ScriptUdpSocket", "=scriptThread.createUdpSocket(", lines);
            searchSingleType("ScriptTcpServer", "=scriptThread.createTcpServer(", lines);
            searchSingleType("ScriptTimer", "=scriptThread.createTimer(", lines);
            searchSingleType("ScriptSerialPort", "=scriptThread.createSerialPort(", lines);
            searchSingleType("ScriptCheetahSpi", "=scriptThread.createCheetahSpiInterface(", lines);
            searchSingleType("ScriptPcanInterface", "=scriptThread.createPcanInterface(", lines);
            searchSingleType("ScriptPlotWindow", "=scriptThread.createPlotWindow(", lines);
            searchSingleType("ScriptXmlReader", "=scriptThread.createXmlReader(", lines);
            searchSingleType("ScriptXmlWriter", "=scriptThread.createXmlWriter(", lines);
            searchSingleType("Dummy", "=scriptThread.readBinaryFile(", lines, true);
            searchSingleType("String", "=scriptThread.readDirectory(", lines, true);
            searchSingleType("Dummy", "=scriptThread.readAllStandardOutputFromProcess(", lines, true);
            searchSingleType("Dummy", "=scriptThread.readAllStandardErrorFromProcess(", lines, true);
            searchSingleType("String", "=scriptThread.getLocalIpAdress(", lines, true);
            searchSingleType("Dummy", "=scriptThread.showGetIntDialog(", lines, true);
            searchSingleType("Dummy", "=scriptThread.showGetDoubleDialog(", lines, true);
            searchSingleType("Dummy", "=scriptThread.showColorDialog(", lines, true);
            searchSingleType("Dummy", "=scriptThread.getGlobalDataArray(", lines, true);
            searchSingleType("Dummy", "=scriptThread.getGlobalUnsignedNumber(", lines, true);
            searchSingleType("Dummy", "=scriptThread.getGlobalSignedNumber(", lines, true);
            searchSingleType("Dummy", "=scriptThread.getGlobalRealNumber(", lines, true);
            searchSingleType("String", "=scriptThread.availableSerialPorts(", lines, true);
            searchSingleType("String", "=scriptThread.getScriptArguments(", lines, true);
            searchSingleType("String", "=scriptThread.getAllObjectPropertiesAndFunctions(", lines, true);
            searchSingleType("String", "=scriptThread.readFile(", lines);
            searchSingleType("String", "=scriptThread.createAbsolutePath(", lines);
            searchSingleType("String", "=scriptThread.getScriptFolder(", lines);
            searchSingleType("String", "=scriptThread.showFileDialog(", lines);
            searchSingleType("String", "=scriptThread.showDirectoryDialog(", lines);
            searchSingleType("String", "=scriptThread.showTextInputDialog(", lines);
            searchSingleType("String", "=scriptThread.showMultiLineTextInputDialog(", lines);
            searchSingleType("String", "=scriptThread.showGetItemDialog(", lines);
            searchSingleType("String", "=scriptThread.getGlobalString(", lines);
            searchSingleType("String", "=scriptThread.getCurrentVersion(", lines);
            searchSingleType("String", "=scriptThread.getScriptTableName(", lines);
            searchSingleType("String", "=scriptThread.currentCpuArchitecture(", lines);
            searchSingleType("String", "=scriptThread.productType(", lines);
            searchSingleType("String", "=scriptThread.productVersion(", lines);
            searchSingleType("String", "=scriptThread.getScriptCommunicatorFolder(", lines);
            searchSingleType("String", "=scriptThread.getUserDocumentsFolder(", lines);
            searchSingleType("String", "=scriptThread.getMainWindowTitle(", lines);
            searchSingleType("String", "=scriptThread.getTimestamp(", lines);
            searchSingleType("ScriptTcpClient", "=scriptThread.createTcpClient(", lines);
        }

        if(currentText.contains("=conv."))
        {
            searchSingleType("String", "=conv.byteArrayToString(", lines);
            searchSingleType("String", "=conv.byteArrayToHexString(", lines);
            searchSingleType("Dummy", "=conv.stringToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addStringToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addUint16ToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addUint32ToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addUint64ToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addInt16ToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addInt32ToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addInt64ToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addFloat32ToArray(", lines, true);
            searchSingleType("Dummy", "=conv.addFloat64ToArray(", lines, true);
        }

        if(currentText.contains("=seq."))
        {
            searchSingleType("String", "=seq.getGlobalString(", lines);
            searchSingleType("Dummy", "=seq.getGlobalDataArray(", lines, true);
            searchSingleType("Dummy", "=seq.getGlobalUnsignedNumber(", lines, true);
            searchSingleType("Dummy", "=seq.getGlobalSignedNumber(", lines, true);
            searchSingleType("String", "=seq.getCurrentVersion(", lines);
            searchSingleType("String", "=seq.showTextInputDialog(", lines);
            searchSingleType("String", "=seq.showMultiLineTextInputDialog(", lines);
            searchSingleType("String", "=seq.showGetItemDialog(", lines);
            searchSingleType("Dummy", "=seq.showGetIntDialog(", lines, true);
            searchSingleType("Dummy", "=seq.showGetDoubleDialog(", lines, true);
            searchSingleType("Dummy", "=seq.showColorDialog(", lines, true);
            searchSingleType("String", "=seq.getAllObjectPropertiesAndFunctions(", lines, true);

        }

        if(currentText.contains("=scriptSql."))
        {
            searchSingleType("ScriptSqlDatabase", "=scriptSql.addDatabase(", lines);
            searchSingleType("ScriptSqlDatabase", "=scriptSql.cloneDatabase(", lines);
            searchSingleType("ScriptSqlDatabase", "=scriptSql.database(", lines);
            searchSingleType("String", "=scriptSql.connectionNames(", lines, true);
            searchSingleType("String", "=scriptSql.drivers(", lines, true);
            searchSingleType("ScriptSqlQuery", "=scriptSql.createQuery(", lines);
            searchSingleType("ScriptSqlField", "=scriptSql.createField(", lines);
            searchSingleType("ScriptSqlRecord", "=scriptSql.createRecord(", lines);
        }

        if(currentText.contains("=cust."))
        {
            searchSingleType("String", "=cust.getScriptFolder(", lines);
            searchSingleType("String", "=cust.readFile(", lines);
            searchSingleType("Dummy", "=cust.readBinaryFile(", lines, true);
            searchSingleType("String", "=cust.readDirectory(", lines, true);
            searchSingleType("String", "=cust.createAbsolutePath(", lines);
            searchSingleType("String", "=cust.getCurrentVersion(", lines);
            searchSingleType("String", "=cust.getAllObjectPropertiesAndFunctions(", lines, true);
            searchSingleType("ScriptXmlReader", "=cust.createXmlReader(", lines);
            searchSingleType("ScriptXmlWriter", "=cust.createXmlWriter(", lines);
        }


        for(auto el : m_tableWidgets)
        {
            parseTableWidetInsert(el, lines);
            searchSingleType("Dummy", "=" + el + ".getAllSelectedCells(", lines, true);
            searchForScriptWidgetCommonFunctions(el, lines);
        }

        QStringList keys = m_tableWidgetObjects.keys();
        for(int i = 0; i < keys.length(); i++)
        {
            searchSingleTableSubWidgets(keys[i], m_tableWidgetObjects.value(keys[i]), lines);
        }

    }//if(passNumber == 1)



    QMap<QString, QString> tmpCreatorList = m_creatorObjects;
    QMap<QString, QString>::iterator i;
    for (i = tmpCreatorList.begin(); i != tmpCreatorList.end(); ++i)
    {
        if(passNumber == 1)
        {
            if(i.value() == "ScriptTreeWidget")
            {
                searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".createScriptTreeWidgetItem(", lines);
                searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".invisibleRootItem(", lines);
                searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".itemAbove(", lines);
                searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".itemBelow(", lines);
                searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".takeTopLevelItem(", lines);
                searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".topLevelItem(", lines);
                searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".currentItem(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptXmlReader")
            {
                searchSingleType("ScriptXmlElement", "=" + i.key() + ".getRootElement(", lines);
                searchSingleType("ScriptXmlElement", "=" + i.key() + ".elementsByTagName(", lines, true);

            }
            else if(i.value() == "ScriptXmlWriter")
            {

                searchSingleType("String", "=" + i.key() + ".getInternalBuffer(", lines);
            }
            else if(i.value() == "ScriptSqlDatabase")
            {
                searchSingleType("ScriptSqlIndex", "=" + i.key() + ".primaryIndex(", lines);
                searchSingleType("ScriptSqlRecord", "=" + i.key() + ".record(", lines);
                searchSingleType("ScriptSqlQuery", "=" + i.key() + ".exec(", lines);
                searchSingleType("ScriptSqlError", "=" + i.key() + ".lastError(", lines);
                searchSingleType("String", "=" + i.key() + ".tables(", lines, true);

                searchSingleType("String", "=" + i.key() + ".databaseName(", lines);
                searchSingleType("String", "=" + i.key() + ".userName(", lines);
                searchSingleType("String", "=" + i.key() + ".password(", lines);
                searchSingleType("String", "=" + i.key() + ".hostName(", lines);
                searchSingleType("String", "=" + i.key() + ".driverName(", lines);
                searchSingleType("String", "=" + i.key() + ".connectOptions(", lines);
                searchSingleType("String", "=" + i.key() + ".connectionName(", lines);
            }
            else if(i.value() == "ScriptTcpServer")
            {
                searchSingleType("ScriptTcpClient", "=" + i.key() + ".nextPendingConnection(", lines);

            }
            else if(i.value() == "ScriptCheetahSpi")
            {
                searchSingleType("Dummy", "=" + i.key() + ".readAll(", lines, true);
                searchSingleType("String", "=" + i.key() + ".readAllLines(", lines, true);

                searchSingleType("String", "=" + i.key() + ".detectDevices(", lines);
            }
            else if(i.value() == "ScriptSerialPort")
            {
                searchSingleType("Dummy", "=" + i.key() + ".readAll(", lines, true);
                searchSingleType("String", "=" + i.key() + ".readAllLines(", lines, true);

                searchSingleType("String", "=" + i.key() + ".portName(", lines);
                searchSingleType("String", "=" + i.key() + ".parity(", lines);
                searchSingleType("String", "=" + i.key() + ".stopBits(", lines);
                searchSingleType("String", "=" + i.key() + ".flowControl(", lines);
                searchSingleType("String", "=" + i.key() + ".errorString(", lines);
                searchSingleType("String", "=" + i.key() + ".readLine(", lines);
            }
            else if(i.value() == "ScriptUdpSocket")
            {
                searchSingleType("Dummy", "=" + i.key() + ".readDatagram(", lines, true);
                searchSingleType("Dummy", "=" + i.key() + ".readAll(", lines, true);
                searchSingleType("String", "=" + i.key() + ".readAllLines(", lines, true);

                searchSingleType("String", "=" + i.key() + ".readLine(", lines);
            }
            else if(i.value() == "ScriptPcanInterface")
            {
                searchSingleType("Dummy", "=" + i.key() + ".getCanParameter(", lines, true);

                searchSingleType("String", "=" + i.key() + ".getStatusString(", lines);
            }
            else if(i.value() == "ScriptSplitter")
            {
                searchSingleType("Dummy", "=" + i.key() + ".sizes(", lines, true);

            }
            else if(i.value() == "QWebView")
            {

                searchSingleType("String", "=" + i.key() + ".url(", lines);
                searchSingleType("String", "=" + i.key() + ".selectedHtml(", lines);
                searchSingleType("String", "=" + i.key() + ".selectedText(", lines);
                searchSingleType("String", "=" + i.key() + ".title(", lines);
            }
            else if(i.value() == "ScriptAction")
            {

                searchSingleType("String", "=" + i.key() + ".text(", lines);
            }
            else if(i.value() == "ScriptButton")
            {
                searchSingleType("String", "=" + i.key() + ".text(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptCalendarWidget")
            {
                searchSingleType("String", "=" + i.key() + ".getSelectedDate(", lines);
                searchSingleType("String", "=" + i.key() + ".getDateFormat(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptCheckBox")
            {
                searchSingleType("String", "=" + i.key() + ".text(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptComboBox")
            {
                searchSingleType("String", "=" + i.key() + ".currentText(", lines);
                searchSingleType("String", "=" + i.key() + ".itemText(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptDateEdit")
            {
                searchSingleType("String", "=" + i.key() + ".getDate(", lines);
                searchSingleType("String", "=" + i.key() + ".getDisplayFormat(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptDateTimeEdit")
            {
                searchSingleType("String", "=" + i.key() + ".getDateTime(", lines);
                searchSingleType("String", "=" + i.key() + ".getDisplayFormat(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptDial")
            {
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptDialog")
            {
                searchSingleType("String", "=" + i.key() + ".windowPositionAndSize(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptDoubleSpinBox")
            {
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptFontComboBox")
            {
                searchSingleType("String", "=" + i.key() + ".currentText(", lines);
                searchSingleType("String", "=" + i.key() + ".itemText(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptGroupBox")
            {
                searchSingleType("String", "=" + i.key() + ".title(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
                searchSingleType("ScriptPlotWidget",  "=" + i.key() + ".addPlotWidget(", lines);
                searchSingleType("ScriptCanvas2DWidget",  "=" + i.key() + ".addCanvas2DWidget(", lines);
            }
            else if(i.value() == "ScriptLabel")
            {
                searchSingleType("String", "=" + i.key() + ".text(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptLineEdit")
            {
                searchSingleType("String", "=" + i.key() + ".text(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptListWidget")
            {
                searchSingleType("String", "=" + i.key() + ".getItemText(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptMainWindow")
            {
                searchSingleType("String", "=" + i.key() + ".windowPositionAndSize(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptPlotWindow")
            {
                searchSingleType("String", "=" + i.key() + ".windowPositionAndSize(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptProgressBar")
            {
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptRadioButton")
            {
                searchSingleType("String", "=" + i.key() + ".text(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptSlider")
            {
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptSpinBox")
            {
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptStatusBar")
            {
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptTabWidget")
            {
                searchSingleType("String", "=" + i.key() + ".tabText(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptTextEdit")
            {
                searchSingleType("String", "=" + i.key() + ".toPlainText(", lines);
                searchSingleType("String", "=" + i.key() + ".toHtml(", lines);
                searchSingleType("String", "=" + i.key() + ".replaceNonHtmlChars(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptTimeEdit")
            {
                searchSingleType("String", "=" + i.key() + ".getTime(", lines);
                searchSingleType("String", "=" + i.key() + ".getDisplayFormat(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptTimer")
            {

            }
            else if(i.value() == "ScriptToolBox")
            {
                searchSingleType("String", "=" + i.key() + ".itemText(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptToolButton")
            {
                searchSingleType("String", "=" + i.key() + ".text(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }
            else if(i.value() == "ScriptWidget")
            {
                searchSingleType("String", "=" + i.key() + ".windowPositionAndSize(", lines);
                searchForScriptWidgetCommonFunctions(i.key(), lines);
            }

        }//if(passNumber == 1)

        if(i.value() == "ScriptTreeWidgetItem")
        {
            searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".takeChild(", lines);
            searchSingleType("ScriptTreeWidgetItem", "=" + i.key() + ".parent(", lines);
            searchForScriptWidgetCommonFunctions(i.key(), lines);
            searchSingleType("String", "=" + i.key() + ".text(", lines);
            searchSingleType("String", "=" + i.key() + ".data(", lines);
        }
        else if(i.value() == "ScriptXmlElement")
        {
            searchSingleType("ScriptXmlElement", "=" + i.key() + ".childElements(", lines, true);
            searchSingleType("String", "=" + i.key() + ".childTextElements(", lines, true);
            searchSingleType("String", "=" + i.key() + ".childCDataElements(", lines, true);
            searchSingleType("String", "=" + i.key() + ".childCommentElements(", lines, true);
            searchSingleType("ScriptXmlAttribute", "=" + i.key() + ".attributes(", lines, true);

            searchSingleType("String", "=" + i.key() + ".elementName(", lines);
            searchSingleType("String", "=" + i.key() + ".attributeValue(", lines);
        }
        else if(i.value() == "ScriptXmlAttribute")
        {

            searchSingleType("String", "=" + i.key() + ".value(", lines);
            searchSingleType("String", "=" + i.key() + ".name(", lines);
        }
        else if(i.value() == "ScriptSqlQuery")
        {
            searchSingleType("ScriptSqlError", "=" + i.key() + ".lastError(", lines);
            searchSingleType("ScriptSqlRecord", "=" + i.key() + ".record(", lines);

            searchSingleType("String", "=" + i.key() + ".lastQuery(", lines);
            searchSingleType("String", "=" + i.key() + ".executedQuery(", lines);
        }
        else if(i.value() == "ScriptSqlRecord")
        {
            searchSingleType("ScriptSqlField", "=" + i.key() + ".field(", lines);
            searchSingleType("ScriptSqlRecord", "=" + i.key() + ".keyValues(", lines);

            searchSingleType("String", "=" + i.key() + ".fieldName(", lines);
        }
        else if(i.value() == "ScriptSqlError")
        {

            searchSingleType("String", "=" + i.key() + ".driverText(", lines);
            searchSingleType("String", "=" + i.key() + ".databaseText(", lines);
            searchSingleType("String", "=" + i.key() + ".nativeErrorCode(", lines);
            searchSingleType("String", "=" + i.key() + ".text(", lines);
        }
        else if(i.value() == "ScriptSqlField")
        {

            searchSingleType("String", "=" + i.key() + ".name(", lines);
        }
        else if(i.value() == "ScriptSqlIndex")
        {

            searchSingleType("String", "=" + i.key() + ".cursorName(", lines);
            searchSingleType("String", "=" + i.key() + ".name(", lines);
        }
        else if(i.value() == "ScriptTcpClient")
        {
            searchSingleType("Dummy", "=" + i.key() + ".readAll(", lines, true);
            searchSingleType("String", "=" + i.key() + ".readAllLines(", lines, true);

            searchSingleType("String", "=" + i.key() + ".getErrorString(", lines);
            searchSingleType("String", "=" + i.key() + ".readLine(", lines);
        }
        else if(i.value() == "ScriptPlotWidget")
        {
            searchForScriptWidgetCommonFunctions(i.key(), lines);
        }
        else if(i.value() == "Date")
        {
            searchSingleType("String", "=" + i.key() + ".toString(", lines);
            searchSingleType("String", "=" + i.key() + ".toDateString(", lines);
            searchSingleType("String", "=" + i.key() + ".toTimeString(", lines);
            searchSingleType("String", "=" + i.key() + ".toLocaleString(", lines);
            searchSingleType("String", "=" + i.key() + ".toLocaleDateString(", lines);
            searchSingleType("String", "=" + i.key() + ".toLocaleTimeString(", lines);
            searchSingleType("String", "=" + i.key() + ".toUTCString(", lines);
            searchSingleType("String", "=" + i.key() + ".toISOString(", lines);
            searchSingleType("String", "=" + i.key() + ".toJSON(", lines);
        }

        searchSingleType(i.value(), "=" + i.key(), linesWithBrackets, m_arrayList.contains(i.key()), true, false, true);
        searchSingleType(i.value(), "=" + i.key() + "[", linesWithBrackets);
    }
}

/**
 * Searches objects which are returned by a ScriptTableWidget.
 * @param objectName
 *      The object name of the ScriptTableWidget.
 * @param subObjects
 *      All objects which have been created by the current ScriptTableWidget (ScriptTableWidget::insertWidget).
 * @param lines
 *      The lines in which shall be searched.
 */
void ParseThread::searchSingleTableSubWidgets(QString objectName, QVector<TableWidgetSubObject> subObjects, QStringList& lines)
{
    int index;
    QString searchString = objectName+ ".getWidget";
    for(int i = 0; i < lines.length();i++)
    {

        index = lines[i].indexOf(searchString);
        if(index != -1)
        {
            QString objectName =  lines[i].left(index);

            index = lines[i].indexOf("=");
            if(index != -1)
            {
                objectName = lines[i].left(index);
            }

            removeAllLeft(objectName, "{");
            removeAllLeft(objectName, ")");

            index = lines[i].indexOf("(", index);
            int endIndex = lines[i].indexOf(")", index);
            if((index != -1) && (endIndex != -1))
            {
                QString args = lines[i].mid(index + 1, endIndex - (index + 1));
                QStringList list = args.split(",");
                if(list.length() >= 2)
                {
                    bool isOk;
                    int row = list[0].toUInt(&isOk);
                    int column = list[1].toUInt(&isOk);

                    //Search the corresponding widget in subObjects.
                    for(int j = 0; j < subObjects.length(); j++)
                    {
                        if((row == subObjects[j].row) && (column == subObjects[j].column))
                        {
                            addObjectToAutoCompletionList(objectName, subObjects[j].className, false);
                        }
                    }
                }
            }
        }
    }
}

/**
 * Searches all ScriptTableWidget::insertWidgets calls of a specific ScriptTableWidget object.
 * @param objectName
 *      The name of the current ScriptTableWidget.
 * @param lines
 *      The lines in which shall be searched.
 */
void ParseThread::parseTableWidetInsert(const QString objectName, QStringList lines)
{
    int index;
    for(int i = 0; i < lines.length();i++)
    {
        QString searchString = objectName + ".insertWidget";

        index = lines[i].indexOf(searchString);
        if(index != -1)
        {

            index = lines[i].indexOf("(", index);
            int endIndex = lines[i].indexOf(")", index);
            if((index != -1) && (endIndex != -1))
            {
                QString args = lines[i].mid(index + 1, endIndex - (index + 1));
                QStringList list = args.split(",");
                if(list.length() >= 3)
                {
                    bool isOk;
                    list[2].replace("\"","");
                    TableWidgetSubObject subObject = {(int)list[0].toUInt(&isOk), (int)list[1].toUInt(&isOk), list[2]};


                    if(subObject.className  == QString("LineEdit"))
                    {
                        subObject.className  = "ScriptLineEdit";
                    }
                    else if(subObject.className == QString("ComboBox"))
                    {
                        subObject.className  = "ScriptComboBox";
                    }
                    else if(subObject.className  == QString("Button"))
                    {
                        subObject.className  = "ScriptButton";
                    }
                    else if(subObject.className  == QString("CheckBox"))
                    {
                        subObject.className  = "ScriptCheckBox";
                    }
                    else if(subObject.className  == QString("SpinBox"))
                    {
                        subObject.className  = "ScriptSpinBox";
                    }
                    else if(subObject.className  == QString("DoubleSpinBox"))
                    {
                        subObject.className  = "ScriptDoubleSpinBox";
                    }
                    else if(subObject.className  == QString("VerticalSlider"))
                    {
                        subObject.className  = "ScriptSlider";
                    }
                    else if(subObject.className  == QString("HorizontalSlider"))
                    {
                        subObject.className  = "ScriptSlider";
                    }
                    else if(subObject.className  == QString("TimeEdit"))
                    {
                        subObject.className  = "ScriptTimeEdit";
                    }
                    else if(subObject.className  == QString("DateEdit"))
                    {
                        subObject.className  = "ScriptDateEdit";
                    }
                    else if(subObject.className  == QString("DateTimeEdit"))
                    {
                        subObject.className  = "ScriptDateTimeEdit";
                    }
                    else if(subObject.className  == QString("CalendarWidget"))
                    {
                        subObject.className  = "ScriptCalendarWidget";
                    }
                    else if(subObject.className  == QString("TextEdit"))
                    {
                        subObject.className  = "ScriptTextEdit";
                    }
                    else if(subObject.className  == QString("Dial"))
                    {
                        subObject.className  = "ScriptDial";
                    }

                    if(m_tableWidgetObjects.contains(objectName))
                    {
                        m_tableWidgetObjects[objectName].append(subObject);
                    }
                    else
                    {
                        QVector<TableWidgetSubObject> vect;
                        vect.append(subObject);

                        m_tableWidgetObjects[objectName] = vect;
                    }


                }
            }

        }
    }
}

/**
 * Searches for functions which habe all ScriptWidgets in common.
 * @param objectName
 *      The object name.
 * @param lines
 *      The script lines.
 */
void ParseThread::searchForScriptWidgetCommonFunctions(const QString& objectName, QStringList& lines)
{
    searchSingleType("String", "=" + objectName + ".getAdditionalData(", lines);
    searchSingleType("String", "=" + objectName + ".getClassName(", lines);
    searchSingleType("String", "=" + objectName + ".getObjectName(", lines);
}

/**
 * Searches all dynamically created objects created by standard objects (like String).
 * @param currentText
 *      The text in which shall be searched.
 */
void ParseThread::checkDocumentForStandardDynamicObjects(QStringList& lines, QStringList &linesWithBrackets, int passNumber)
{
    if(passNumber == 1)
    {
        searchSingleType("Dummy", "=Array(", linesWithBrackets, true);
        searchSingleType("Dummy", "=newArray(", linesWithBrackets, true);

        searchSingleType("Date", "=Date(", linesWithBrackets);
        searchSingleType("Date", "=newDate(", linesWithBrackets);

        searchSingleType("String", "=\"", linesWithBrackets);
        searchSingleType("String", "='", linesWithBrackets);
    }

    QVector<QString> tmpList = m_stringList;
    for(auto el : tmpList)
    {
        searchSingleType("String", "=" + el + ".toString(", lines);
        searchSingleType("String", "=" + el + ".valueOf(", lines);
        searchSingleType("String", "=" + el + ".charAt(", lines);
        searchSingleType("String", "=" + el + ".concat(", lines);
        searchSingleType("String", "=" + el + ".match(", lines, true);
        searchSingleType("String", "=" + el + ".replace(", lines);
        searchSingleType("String", "=" + el + ".slice(", lines);
        searchSingleType("String", "=" + el + ".split(", lines, true);
        searchSingleType("String", "=" + el + ".substring(", lines);
        searchSingleType("String", "=" + el + ".toLowerCase(", lines);
        searchSingleType("String", "=" + el + ".toLocaleLowerCase(", lines);
        searchSingleType("String", "=" + el + ".toUpperCase(", lines);
        searchSingleType("String", "=" + el + ".toLocaleUpperCase(", lines);
        searchSingleType("String", "=" + el + ".trim(", lines);
        searchSingleType("String", "=" + el + ".fromCharCode(", lines);

        searchSingleType("String", "=" + el, linesWithBrackets, m_arrayList.contains(el), true, false, true);
        searchSingleType("String", "=" + el + "[", linesWithBrackets);


    }

    tmpList = m_arrayList;
    for(auto el : tmpList)
    {
        searchSingleType("Dummy", "=" + el + ".concat(", lines, true);
        searchSingleType("Dummy", "=" + el + ".slice(", lines, true);
        searchSingleType("Dummy", "=" + el + ".splice(", lines, true);
        searchSingleType("Dummy", "=" + el + ".map(", lines, true);
        searchSingleType("Dummy", "=" + el + ".filter(", lines, true);
        searchSingleType("Dummy", "=" + el + ".filter(", lines, true);
        searchSingleType("Dummy", "=" + el, linesWithBrackets, true, true,false, true);

        searchSingleType("String", "=" + el + ".toString(", lines);
        searchSingleType("String", "=" + el + ".toLocaleString(", lines);
        searchSingleType("String", "=" + el + ".join(", lines);
    }
}

/**
 * Parses the current text. Emits parsingFinishedSignal if the parsing is finished.
 * @param currentText
 *      The text which shall be parsed.
 * @param activeDocument
 *      The name of the active document.
 */
void ParseThread::parseSlot(QString& currentText, QString activeDocument)
{
    //Clear all parsed objects (all but m_autoCompletionApiFiles).
    m_autoCompletionEntries.clear();
    m_creatorObjects.clear();
    m_tableWidgetObjects.clear();
    m_parsedFiles.clear();
    m_stringList.clear();
    m_arrayList.clear();
    m_tableWidgets.clear();
    m_unknownTypeObjects.clear();


    QRegExp splitRegexp("[\n;]");

    //Remove all unnecessary characters (e.g. comments).
    removeAllUnnecessaryCharacters(currentText);

    //Get all lines (with square brackets).
    QStringList linesWithBrackets = currentText.split(splitRegexp);

    //Remove all brackets and all between them.
    removeAllBetweenSquareBrackets(currentText);

    //Get all lines (without square brackets).
    QStringList lines = currentText.split(splitRegexp);

    //Parse all found user interface files (check for changes).
    for(auto el : m_foundUiFiles)
    {
        parseUiFile(el);
    }

    //Check if the active document has a ui-file.
    QString uiFileName = MainWindow::getTheCorrespondingUiFile(activeDocument);
    if(!uiFileName.isEmpty())
    {
        parseUiFile(uiFileName);
    }

    //Find all included user interface files.
    checkDocumentForUiFiles(currentText, activeDocument);

    int counter = 1;
    do
    {
        m_objectAddedToCompletionList = false;

        //Call several times to get all objects created by dynamic objects.
        checkDocumentForCustomDynamicObjects(lines, linesWithBrackets, currentText, counter);
        counter++;
    }while(m_objectAddedToCompletionList);


    counter = 1;
    do
    {
        m_objectAddedToCompletionList = false;

        //Call several times to get all objects created by dynamic objects.
        checkDocumentForStandardDynamicObjects(lines, linesWithBrackets, counter);
        counter++;
    }while(m_objectAddedToCompletionList);


    //Search all objects with an unknown type.
    searchSingleType("Dummy", "=", lines);


    //Add all objects with an unknown type.
    for(auto el : m_unknownTypeObjects)
    {
        if(!m_autoCompletionEntries.contains(el))
        {
            m_autoCompletionEntries[el] << el;
        }
    }

    emit parsingFinishedSignal(m_autoCompletionEntries, m_autoCompletionApiFiles);
}

