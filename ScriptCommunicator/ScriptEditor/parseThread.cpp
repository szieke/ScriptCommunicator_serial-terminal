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

/**
 * Parses a single line from an api file and adds functions which return objects to m_functionsWithResultObjects.
 *
 * @param singleLine
 *      The current line
 * @return
 *      True if the current entry is a property.
 */
void ParseThread::parseSingleLineForFunctionsWithResultObjects(QString& singleLine)
{

    QStringList split = singleLine.split(":");
    if(split.length() >= 4)
    {
        FunctionWithResultObject element;
        element.isArray = false;
        element.isProperty = false;
        QString className = split[0];
        element.functionName = split[2];

        QStringList resultType = singleLine.split("):");
        element.resultType = (resultType.length() > 1) ? resultType[1] : split[3];

        int index = element.functionName.indexOf("(");
        if(index != -1)
        {
            element.functionName.remove(index + 1, element.functionName.length() - (index + 1));
        }
        else
        {
            element.isProperty = true;
        }

        index = element.resultType.indexOf("\\n");
        if(index != -1)
        {
            element.resultType.remove(index, element.resultType.length() - index);

            if(element.isProperty)
            {
                singleLine.replace(":" + element.resultType, "");
            }
            element.resultType.replace(" ", "");//Remove all spaces

            if(element.resultType.startsWith("Array"))
            {
                element.isArray = true;

                if(element.resultType == "Array")
                {
                    element.resultType = "Dummy";
                }
                else
                {
                    element.resultType.replace("Array<", "");
                    element.resultType.replace(">", "");
                }
            }
            else
            {

            }

        }
        else
        {//Invalid entry.
            return;
        }


        if(!m_functionsWithResultObjects.contains((className)))
        {
            m_functionsWithResultObjects[className] = QVector<FunctionWithResultObject>();
        }


        m_functionsWithResultObjects[className].append(element);
    }

}

ParseThread::ParseThread(QObject *parent) : QThread(parent), m_autoCompletionApiFiles(), m_autoCompletionEntries(), m_objectAddedToCompletionList(false),
    m_creatorObjects(), m_unknownTypeObjects(), m_arrayList(), m_tableWidgets(),
    m_tableWidgetObjects(), m_parsedUiFiles(), m_parsedUiFilesFromFile()
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
                    parseSingleLineForFunctionsWithResultObjects(singleEntry);

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
    QThread::setPriority(QThread::LowestPriority);
    exec();
}

/**
 * Removes in text all left of character.
 *
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
 * Removes all unnecessary characters (e.g. comments).
 *
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
 * Parses a widget list from a user interface file (auto-completion).
 *
 * @param uiFileName
 *      The user interface file.
 * @param docElem
 *      The QDomElement from the user interface file.
 * @param parseActions
 *      If true then all actions will be parsed. If false then all widgets will be parsed.
 */
void ParseThread::parseWidgetList(QString uiFileName, QDomElement& docElem, bool parseActions)
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
        else
        {
            if(!m_autoCompletionApiFiles.contains(className + ".api"))
            {//No API file for the current class.
                className = "";
            }
        }

        if(!m_parsedUiObjects.contains(uiFileName))
        {
            m_parsedUiObjects[uiFileName] = QStringList();
        }
        if(!m_parsedUiObjects[uiFileName].contains(objectName))
        {
            m_parsedUiObjects[uiFileName].append(objectName);
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
 * @param isArray
 *      True if the object is an array.
 * @param replaceExistingEntry
 *      True if the object shall be replaced if m_autoCompletionEntries contains it already.
 */
void ParseThread::addObjectToAutoCompletionList(QString objectName, QString className, bool isGuiElement,
                                                bool isArray, bool replaceExistingEntry)
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

        if(className == "ScriptTableWidget")
        {
            int index = m_tableWidgets.indexOf(autoCompletionName);
            if(index != -1)
            {
                m_tableWidgets.remove(index);
            }
            m_tableWidgets.append(autoCompletionName);

            if(isArray)
            {
                className = "Array<" + className + ">";
            }
            m_creatorObjects[autoCompletionName] = className;
        }
        else
        {
            if((className != "Dummy"))
            {
                if(isArray)
                {
                    className = "Array<" + className + ">";
                }

                m_creatorObjects[autoCompletionName] = className;
            }
        }
    }
}

/**
 * Parses an user interface file (auto-completion).
 * @param uiFileName
 *      The user interface file.
 * @param fileContent
 *      The file content.
 */
void ParseThread::parseUiFile(QString uiFileName, QString fileContent)
{

    QFile uiFile(uiFileName);
    QDomDocument doc("ui");

    if(!m_parsedUiFiles.contains(uiFileName))
    {
        m_parsedUiFiles.append(uiFileName);

        if(fileContent.isEmpty())
        {
            //Load the file content.
            if (uiFile.open(QFile::ReadOnly))
            {
                if (doc.setContent(&uiFile))
                {
                    QDomElement docElem = doc.documentElement();
                    parseWidgetList(uiFileName, docElem, false);
                    parseWidgetList(uiFileName, docElem, true);
                }

                uiFile.close();

                QFileInfo fileInfo(uiFileName);
                m_parsedUiFilesFromFile[uiFileName] = fileInfo.lastModified();
            }
        }
        else
        {
            if (doc.setContent(fileContent))
            {
                QDomElement docElem = doc.documentElement();
                parseWidgetList(uiFileName, docElem, false);
                parseWidgetList(uiFileName, docElem, true);
            }

        }
    }

}
/**
 * Checks if in the current document user interface files are loaded.
 * If user interface are loaded then they will be parsed and added to the auto-completion
 * list (m_autoCompletionEntries).
 *
 * @param currentText
 *      The test which shall be parsed.
 * @param activeDocumentPath
 *      The path of the current documents (is used to create absolut pathes for scriptThread.loadUserInterfaceFile).
 */
void ParseThread::checkDocumentForUiFiles(QString& currentText, QString currentDocumentPath)
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
            uiFile = QFileInfo(currentDocumentPath).absolutePath() + "/" + uiFile;
        }

        parseUiFile(uiFile, "");
        index = currentText.indexOf("scriptThread.loadUserInterfaceFile", index + 1);

    }
}


/**
 * Searches all ScriptTableWidget::insertWidgets calls of a specific ScriptTableWidget object.
 * @param objectName
 *      The name of the current ScriptTableWidget.
 * @param lines
 *      The lines in which shall be searched.
 */
void ParseThread::parseTableWidgetInsert(const QString objectName, QStringList lines)
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
                    else
                    {
                        subObject.className = "";
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
 * Replace the object in entries type (if objects contains it).
 *
 * @param objects
 *      The parsed objects.
 * @param entry
 *      The entry.
 */
void ParseThread::replaceAllParsedObject(QMap<QString, ParsedEntry>& objects, ParsedEntry& entry)
{
    QString tmpType = entry.valueType;
    QString tmpCompleteName = entry.completeName;
    int index = tmpCompleteName.lastIndexOf(".");
    while(index != -1)
    {
        tmpCompleteName.remove(index, tmpCompleteName.length() - index);
        tmpType = tmpCompleteName + "." + entry.valueType;

        if(objects.contains(tmpType))
        {
            entry.valueType = tmpType;
            break;
        }
        index = tmpCompleteName.lastIndexOf(".");
    }

    if(objects.contains(entry.valueType))
    {//The assigned object is in the objects list.

        //Add all subelements of the assigned variable/object to the current variable.
        entry.subElements = objects[entry.valueType].subElements;

        //Add the current variable to the autocompletion list.
        addParsedEntiresToAutoCompletionList(entry, m_autoCompletionEntries, "", entry.completeName);
    }

    for(int i = 0; i < entry.subElements.size(); i++)
    {
        replaceAllParsedObject(objects, entry.subElements[i]);
    }
}

/**
 * Replaces a type of an entry with the correspondig parsed type.
 *
 * @param parsedTypes
 *      The parsed types.
 * @param entry
 *      The entry.
 * @param parentName
 *      The complete name of entries parent.
 * @return
 */
bool ParseThread::replaceAllParsedTypes(QMap<QString, QString>& parsedTypes, ParsedEntry& entry,
                                        QString parentName)
{
    bool entryChanged = false;

    entry.completeName = (parentName.isEmpty()) ? entry.name : parentName + "::"  + entry.name;


    /******************************Look in the upper contexts for the type***************************/
    QString tmpType = entry.valueType;
    QString tmpCompleteName = entry.completeName;
    tmpCompleteName.replace("::", ".");
    int index = tmpCompleteName.lastIndexOf(".");
    while(index != -1)
    {
        tmpCompleteName.remove(index, tmpCompleteName.length() - index);
        tmpType = tmpCompleteName + "." + entry.valueType;

        if(parsedTypes.contains(tmpType))
        {
            if(!parsedTypes[tmpType].isEmpty())
            {
                entry.valueType = parsedTypes[tmpType];
                if(entry.isObjectArrayIndex)
                {//Array element.
                    entry.valueType.replace("Array<", "");
                    entry.valueType.replace(">", "");
                }
                tmpType = "";
                break;
            }
        }

        index = tmpCompleteName.lastIndexOf(".");
    }
    /********************************************************************************************************/

    if(!tmpType.isEmpty())
    {//The type was not found in an upper context.

        if(parsedTypes.contains(entry.valueType))
        {
            if(!parsedTypes[entry.valueType].isEmpty())
            {
                entry.valueType = parsedTypes[entry.valueType];

                if(entry.isObjectArrayIndex)
                {//Array element.
                    entry.valueType.replace("Array<", "");
                    entry.valueType.replace(">", "");
                }
            }
        }
    }


    if (entry.valueType.indexOf(".") != -1)
    {
        QStringList split = entry.valueType.split(".");

        //Look for UI elements created with a table widget.
        if((split.size() >= 2) && m_tableWidgets.contains(split[0]))
        {
            if(split[1] == "getWidget()")
            {
                bool isOk;
                int row = entry.additionalInformation[0].toInt(&isOk);
                int column = entry.additionalInformation[1].toInt(&isOk);
                QVector<TableWidgetSubObject> tableWidgetObjects = m_tableWidgetObjects[split[0]];
                for(auto el : tableWidgetObjects)
                {
                    if((row == el.row) && (column == el.column))
                    {
                        split.pop_front();
                        split.pop_front();
                        split.push_front(el.className);
                        entry.valueType = "";

                        for(auto splitEl : split)
                        {
                            entry.valueType += splitEl + ".";
                        }
                        //Remove the last '.'.
                        entry.valueType.remove(entry.valueType.size() - 1, 1);
                        m_creatorObjects[entry.completeName] = entry.valueType;


                        return true;
                    }
                }
            }
        }

        /**********************************************************************************************/
        //Check if the object in the type of entry is declared in the same contex as entry
        //(in this case m_creatorObjects contains this object).
        QString creatorName = entry.completeName;
        int tmpIndex = creatorName.lastIndexOf(entry.name);
        if(tmpIndex != -1)
        {
            creatorName.remove(tmpIndex, creatorName.length() - index);
            creatorName += split[0];
        }

        if (!m_creatorObjects.contains(creatorName))
        {
            creatorName = split[0];
        }

        if (!m_creatorObjects.contains(creatorName))
        {
            creatorName = "";

            for(int i = 0; i < (split.size() - 1); i++)
            {
                creatorName += split[i];

                if(i < (split.size() - 2))
                {
                    creatorName += "::";
                }
            }
        }
        /**********************************************************************************************/

        QString arrayType;
        //Check if m_creatorObjects contains the object in the type of entry.
        if (m_creatorObjects.contains(creatorName))
        {
            split[0] = m_creatorObjects[creatorName];

            if(split.size() <= 2)
            {
                split[1] = split[split.size() - 1];
                bool isArrayIndex = false;
                if(entry.isObjectArrayIndex)
                {
                    isArrayIndex = true;
                }

                if(split[0].startsWith("Array<"))
                {
                    split[0].replace("Array<", "");
                    split[0].replace(">", "");

                    if(!isArrayIndex)
                    {
                        arrayType = split[0];
                        split[0] = "Array";
                    }
                }
            }
        }

         //Check if m_functionsWithResultObjects contains the object in the type of entry.
        if(m_functionsWithResultObjects.contains(split[0]))
        {
            split[1].replace("()", "");

            for(int i = 0; i < m_functionsWithResultObjects[split[0]].size(); i++)
            {
                if((m_functionsWithResultObjects[split[0]][i].functionName == (split[1] + "(")) ||
                    (m_functionsWithResultObjects[split[0]][i].functionName == split[1]))
                {
                    bool isArray = m_functionsWithResultObjects[split[0]][i].isArray;
                    if(entry.isFunctionArrayIndex)
                    {
                        isArray = false;

                    }

                    QString resultType = m_functionsWithResultObjects[split[0]][i].resultType;
                    if((resultType == "Dummy") && !arrayType.isEmpty())
                    {
                        resultType = arrayType;
                    }

                    if(split.size() <= 2)
                    {

                        addObjectToAutoCompletionList(entry.completeName, resultType, false,
                                isArray, true);

                        entry.valueType = "";

                        if(isArray)
                        {
                            entry.valueType = "Array<";
                        }
                        entry.valueType += resultType;

                        if(isArray)
                        {
                            entry.valueType += ">";
                        }

                        if(entry.valueType == "Array<Dummy>")
                        {
                            entry.valueType = "Array";
                        }

                        m_creatorObjects[entry.completeName] = entry.valueType;
                    }
                    else
                    {
                        entry.valueType = resultType;
                        for(int splitIndex = 2; splitIndex < split.size(); splitIndex++)
                        {
                            entry.valueType += "." + split[splitIndex];
                        }
                    }

                    entryChanged = true;

                }
            }
        }

    }
    else
    {
        QString valueType = entry.valueType;
        bool isArray = false;

        if(entry.valueType.startsWith("Array"))
        {
            isArray = true;
            valueType = entry.valueType;

            if(valueType == "Array")
            {
                valueType = "Array";
                isArray = false;
            }
            else
            {
                valueType.replace("Array<", "");
                valueType.replace(">", "");
            }

        }

        if (m_autoCompletionApiFiles.contains(valueType + ".api"))
        {
            addObjectToAutoCompletionList(entry.completeName, valueType, false, isArray);
        }
        else if (m_creatorObjects.contains(entry.valueType))
        {
            addObjectToAutoCompletionList(entry.completeName, m_creatorObjects[valueType], false, isArray);
            entry.valueType = m_creatorObjects[valueType];
            entryChanged = true;
        }

    }

    //Call replaceAllParsedTypes for all sub elements.
    for(int i = 0; i < entry.subElements.size(); i++)
    {
        if(replaceAllParsedTypes(parsedTypes, entry.subElements[i], entry.completeName))
        {
            entryChanged = true;
        }
    }

    return entryChanged;
}

/**
 * Creates the complete name of an entry (includes all parents).
 *
 * @param entry
 *      The entry.
 * @param parent
 *      The parent entry.
 */
static void createCompleteName(ParsedEntry& entry, ParsedEntry& parent)
{
    entry.completeName = parent.completeName.isEmpty() ? entry.name : parent.completeName + "." + entry.name;

    for(int i = 0; i < entry.subElements.size(); i++)
    {
        createCompleteName(entry.subElements[i], entry);
    }
}

/**
 * Returns all functions and gloabl variables in the loaded script files.
 *
 * @param loadedScripts
 *      The loaded scripts.
 * @return
 *      All parsed entries.
 */
QMap<int,QVector<ParsedEntry>> ParseThread::getAllFunctionsAndGlobalVariables(QMap<int, QString> loadedScripts)
{
    QMap<int,QVector<ParsedEntry>> result;

    QMap<int, QString>::const_iterator iter = loadedScripts.constBegin();
    QMap<QString, QVector<ParsedEntry>> prototypeFunctions;
    QMap<QString, ParsedEntry> objects;

    //Parse all files.
    while (iter != loadedScripts.constEnd())
    {
        QVector<ParsedEntry> fileResult;
        esprima::Pool pool;
        esprima::Program *program = NULL;
        try
        {
            program = esprima::parse(pool, iter.value().toLocal8Bit().constData());
        }
        catch(esprima::ParseError e)
        {
            ParsedEntry entry;
            entry.line = e.lineNumber;
            entry.type = PARSED_ENTRY_TYPE_PARSE_ERROR;
            entry.tabIndex = iter.key();
            entry.name = e.description.c_str();
            entry.isFunctionArrayIndex = false;
            entry.isObjectArrayIndex = false;
            fileResult.append(entry);
            result[iter.key()] = fileResult;
            iter++;
            continue;
        }

        //Parse the complete File.
        for(int i = 0; i < program->body.size(); i++)
        {
            ParsedEntry entry;
            esprima::VariableDeclaration* varDecl = dynamic_cast<esprima::VariableDeclaration*>(program->body[i]);
            if(varDecl)
            {
                ParsedEntry dummyParent;
                parseEsprimaVariableDeclaration( varDecl, dummyParent, entry, iter.key(), &objects);

                //Append the current entry to the file result list.
                fileResult.append(entry);

            }//if(varDecl)
            else
            {
                if(checkForControlStatements(program->body[i], entry, iter.key(), &objects))
                {
                    for(auto el : entry.subElements)
                    {
                        fileResult.append(el);
                    }
                }
                else
                {
                    esprima::FunctionDeclaration* funcDecl = dynamic_cast<esprima::FunctionDeclaration*>(program->body[i]);
                    if(funcDecl)
                    {//Function

                        ParsedEntry dummyParent;
                        parseEsprimaFunctionDeclaration(funcDecl, &dummyParent, &entry, iter.key(), &objects);
                        fileResult.append(entry);
                        objects[entry.name] = entry;
                    }
                    else
                    {
                        checkForPrototypFunction(program->body[i], iter.key(), &objects,prototypeFunctions);
                    }
                }
            }
        }



        if(!fileResult.isEmpty())
        {
            //Add the result for the current file to the result list.
            result[iter.key()] = fileResult;
        }

        iter++;

    }//while (iter != loadedScripts.constEnd())


    QMap<QString, QString> parsedTypes;
    for(int i = 0; i < result.size(); i++)
    {
        //Add the prototyp function to their correspondig objects/classes and
        //add the parsed entries to the autocompletion list.
        for(int j = 0; j < result[i].size(); j++)
        {

            if(prototypeFunctions.contains(result[i][j].name))
            {//The current object/class has prototype functions.

                //Add the prototyp function to the current objects/classes
                for (int index = 0; index < prototypeFunctions[result[i][j].name].length(); index++)
                {
                    createCompleteName(prototypeFunctions[result[i][j].name][index], result[i][j]);
                    result[i][j].subElements.append(prototypeFunctions[result[i][j].name][index]);
                }
            }

            //Remove the prototype functions.
            prototypeFunctions.remove(result[i][j].name);

            if(result[i][j].type != PARSED_ENTRY_TYPE_VAR)
            {
                //Add the parsed entries to the autocompletion list.
                addParsedEntiresToAutoCompletionList(result[i][j], m_autoCompletionEntries, "", result[i][j].name);
            }
            else
            {
                if(!result[i][j].params.isEmpty())
                {//The the declaration of the current variable has an assignement of another variable/object
                 //e.g. var map2 = map1;

                    if(objects.contains(result[i][j].params[0]))
                    {//The assigned object is in the objects list.

                        //Add all subelements of the assigned variable/object to the current variable.
                        result[i][j].subElements = objects[result[i][j].params[0]].subElements;

                        //Add the current variable to the autocompletion list.
                        addParsedEntiresToAutoCompletionList(result[i][j], m_autoCompletionEntries, "", result[i][j].name);
                    }

                    if(m_tableWidgets.contains(result[i][j].valueType))
                    {

                        m_tableWidgets.append(result[i][j].name);
                        m_tableWidgetObjects[result[i][j].name] = m_tableWidgetObjects[result[i][j].valueType];
                        result[i][j].valueType = "ScriptTableWidget";
                    }

                    result[i][j].valueType = result[i][j].params[0];

                }
            }
        }

        for(auto el : result[i])
        {
            getAllParsedTypes(parsedTypes, el, "");
        }
    }

    for(int i = 0; i < result.size(); i++)
    {
        for(int j = 0; j < result[i].size(); j++)
        {
            replaceAllParsedObject(objects, result[i][j]);
        }
    }



    bool entryChanged = false;
    do
    {
        entryChanged = false;
        for(int i = 0; i < result.size(); i++)
        {
            for(int j = 0; j < result[i].size(); j++)
            {
                if(replaceAllParsedTypes(parsedTypes, result[i][j], ""))
                {
                    entryChanged = true;
                }
            }
        }
    }while(entryChanged);


    //Add the objects with an unknown type.
    for(int i = 0; i < result.size(); i++)
    {
        for(int j = 0; j < result[i].size(); j++)
        {
            if(!m_autoCompletionEntries.contains(result[i][j].completeName))
            {
                m_autoCompletionEntries[result[i][j].completeName] << result[i][j].completeName;
            }
        }
    }


    return result;
}
/**
 * Parses the current text. Emits parsingFinishedSignal if the parsing is finished.
 * @param loadedUiFiles
 *      All loaded ui files.
 * @param loadedScripts
 *      All loaded scripts.
 * @param loadedScriptsIndex
 *      The (tab) indexes of the loaded scripts.
 * @param loadedFileChanged
 *      True if the loaded files have been changed.
 * @param parseOnlyUIFiles
 *      True of only UI files shall be parsed.
 */
void ParseThread::parseSlot(QMap<QString, QString> loadedUiFiles, QMap<int, QString> loadedScripts,
                            QMap<int, QString> loadedScriptsIndex, bool loadedFileChanged, bool parseOnlyUIFiles)
{

    //Clear all parsed objects (all but m_autoCompletionApiFiles).
    m_autoCompletionEntries.clear();
    m_creatorObjects.clear();
    m_tableWidgetObjects.clear();
    m_parsedUiObjects.clear();
    m_parsedUiFiles.clear();
    m_arrayList.clear();
    m_tableWidgets.clear();
    m_unknownTypeObjects.clear();

    QRegExp splitRegexp("[\n;]");


    bool uiFileChanged = false;

    //Creates a string which contains the text of all loaded scripts.
    QMap<QString, QDateTime>::const_iterator timeIter = m_parsedUiFilesFromFile.constBegin();
    while (timeIter != m_parsedUiFilesFromFile.constEnd())
    {
        QFileInfo fileInfo(timeIter.key());
        if(fileInfo.lastModified() != timeIter.value())
        {
            uiFileChanged = true;
            parseOnlyUIFiles = false;
            break;
        }

        timeIter++;
    }

    if(parseOnlyUIFiles && !loadedFileChanged && !uiFileChanged)
    {//Nothing changed.
        emit parsingFinishedSignal(QMap<QString, QStringList>(), QMap<QString, QStringList>(), QMap<QString, QStringList>(),
                                   QMap<int,QVector<ParsedEntry>>(), false, parseOnlyUIFiles);
        return;
    }
    m_parsedUiFilesFromFile.clear();


    QString currentText;
    //Creates a string which contains the text of all loaded scripts.
    QMap<int, QString>::const_iterator iter = loadedScripts.constBegin();
    while (iter != loadedScripts.constEnd())
    {
        currentText += iter.value();
        iter++;
    }

    //Remove all unnecessary characters (e.g. comments).
    removeAllUnnecessaryCharacters(currentText);

    //Get all lines (without square brackets).
    QStringList lines = currentText.split(splitRegexp);

    //Parse all loaded ui files.
    QMap<QString, QString>::const_iterator iterUi  = loadedUiFiles.constBegin();
    while (iterUi != loadedUiFiles.constEnd())
    {
        parseUiFile(iterUi.key(), iterUi.value());
        iterUi++;
    }

    //Check if the loaded documents have a ui-file.
    iter = loadedScripts.constBegin();
    while (iter != loadedScripts.constEnd())
    {
        QString uiFileName = MainWindow::getTheCorrespondingUiFile(loadedScriptsIndex[iter.key()]);
        if(!uiFileName.isEmpty())
        {
            parseUiFile(uiFileName, "");
        }
        iter++;
    }

    //Find all included user interface files.
    iter = loadedScripts.constBegin();
    while (iter != loadedScripts.constEnd())
    {
        checkDocumentForUiFiles(currentText,loadedScriptsIndex[iter.key()]);
        iter++;
    }


    QMap<int,QVector<ParsedEntry>> parsedEntries;
    if(!parseOnlyUIFiles)
    {
        //Search for GUI elements created by a table widget.
        for(auto el : m_tableWidgets)
        {
            parseTableWidgetInsert(el, lines);
        }
        //Get all global function and variables (with the esprima parser).
        parsedEntries = getAllFunctionsAndGlobalVariables(loadedScripts);

        //Add all objects with an unknown type.
        for(auto el : m_unknownTypeObjects)
        {
            if(!m_autoCompletionEntries.contains(el))
            {
                m_autoCompletionEntries[el] << el;
            }
        }

    }

   emit parsingFinishedSignal(m_autoCompletionEntries, m_autoCompletionApiFiles, m_parsedUiObjects, parsedEntries, true, parseOnlyUIFiles);
}

