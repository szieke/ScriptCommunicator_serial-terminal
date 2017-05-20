#include "parseThread.h"
#include <QFile>
#include <QFileInfo>
#include "mainwindow.h"
#include <QDirIterator>
#include <QTextStream>
#include <QCoreApplication>
#include "esprima/esprima.h"

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
    m_creatorObjects(), m_stringList(), m_unknownTypeObjects(), m_arrayList(), m_tableWidgets(),
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
            m_creatorObjects[autoCompletionName] = className;
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
 */
void ParseThread::checkDocumentForUiFiles(QString& currentText, QString activeDocument)
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


static void parseEsprimaObjectExpression(esprima::ObjectExpression* objExp, ParsedEntry* parent, int tabIndex,
                                         QMap<QString, ParsedEntry>* objects = NULL);

static void getTypeFromNode(esprima::Node* node, ParsedEntry& subEntry);
static void parseEsprimaFunctionDeclaration(esprima::FunctionDeclaration* function, ParsedEntry *parent, ParsedEntry* entry, int tabIndex,
                                            QMap<QString, ParsedEntry>* objects = NULL);

static void parseEsprimaIfStatement(esprima::IfStatement* ifStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL);

static void parseEsprimaSwitchStatement(esprima::SwitchStatement* switchStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaForStatement(esprima::ForStatement* forStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaWhileStatement(esprima::WhileStatement* whileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaDoWhileStatement(esprima::DoWhileStatement* doWhileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaForInStatement(esprima::ForInStatement* forInStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaFunctionExpression(esprima::FunctionExpression* funcExp, ParsedEntry* parent, int tabIndex,
                                           QMap<QString, ParsedEntry>* objects = NULL)


{
    for(int j = 0; j < funcExp->body->body.size(); j++)
    {
        ParsedEntry subEntry;

        esprima::VariableDeclaration* subVarDecl = dynamic_cast<esprima::VariableDeclaration*>(funcExp->body->body[j]);
        if(subVarDecl)
        {
            subEntry.line = subVarDecl->declarations[0]->loc->start->line - 1;
            subEntry.column = subVarDecl->declarations[0]->loc->start->column;
            subEntry.endLine = subVarDecl->declarations[0]->loc->end->line - 1;
            subEntry.name = subVarDecl->declarations[0]->id->name.c_str();
            subEntry.params = QStringList();
            subEntry.tabIndex = tabIndex;
            subEntry.isArrayIndex = false;
            subEntry.completeName = parent->name.isEmpty() ? subEntry.name : parent->completeName + "." + subEntry.name;

            esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(subVarDecl->declarations[0]->init);
            esprima::ObjectExpression* objExp = dynamic_cast<esprima::ObjectExpression*>(subVarDecl->declarations[0]->init);
            if(funcExp)
            {//Function

                subEntry.type = PARSED_ENTRY_TYPE_CLASS_FUNCTION;
                for(int j = 0; j < funcExp->params.size(); j++)
                {
                    subEntry.params.append(funcExp->params[j]->name.c_str());
                }

                parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
            }
            else if(objExp)
            {//Map/Array

                subEntry.type = PARSED_ENTRY_TYPE_MAP;
                parseEsprimaObjectExpression(objExp, &subEntry, tabIndex,objects);
                if(objects)
                {
                    (*objects)[subEntry.completeName] = subEntry;
                }
            }
            else
            {//Variable

               subEntry.type = PARSED_ENTRY_TYPE_CLASS_VAR;

               getTypeFromNode(subVarDecl->declarations[0]->init, subEntry);
            }

            parent->subElements.append(subEntry);
        }
        else
        {
            esprima::IfStatement* ifStatement = dynamic_cast<esprima::IfStatement*>(funcExp->body->body[j]);
            if(ifStatement)
            {
                parseEsprimaIfStatement(ifStatement, *parent, tabIndex, objects);
            }

            esprima::SwitchStatement* switchStatement = dynamic_cast<esprima::SwitchStatement*>(funcExp->body->body[j]);
            if(switchStatement)
            {
               parseEsprimaSwitchStatement(switchStatement, *parent, tabIndex, objects);
            }

            esprima::ForStatement* forStatement = dynamic_cast<esprima::ForStatement*>(funcExp->body->body[j]);
            if(forStatement)
            {
               parseEsprimaForStatement(forStatement, *parent, tabIndex, objects);
            }

            esprima::WhileStatement* whileStatement = dynamic_cast<esprima::WhileStatement*>(funcExp->body->body[j]);
            if(whileStatement)
            {
               parseEsprimaWhileStatement(whileStatement, *parent, tabIndex, objects);
            }

            esprima::DoWhileStatement* doWhileStatement = dynamic_cast<esprima::DoWhileStatement*>(funcExp->body->body[j]);
            if(doWhileStatement)
            {
               parseEsprimaDoWhileStatement(doWhileStatement, *parent, tabIndex, objects);
            }

            esprima::ExpressionStatement* expStatement = dynamic_cast<esprima::ExpressionStatement*>(funcExp->body->body[j]);
            if(expStatement)
            {
                esprima::AssignmentExpression* assignmentStatement = dynamic_cast<esprima::AssignmentExpression*>(expStatement->expression);
                if(assignmentStatement)
                {
                    subEntry.line = assignmentStatement->loc->start->line - 1;
                    subEntry.column = assignmentStatement->loc->start->column;
                    subEntry.endLine = assignmentStatement->loc->end->line - 1;
                    subEntry.params = QStringList();
                    subEntry.tabIndex = tabIndex;
                    subEntry.isArrayIndex = false;

                    esprima::MemberExpression* memExp = dynamic_cast<esprima::MemberExpression*>(assignmentStatement->left);
                    if(memExp)
                    {
                        esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(assignmentStatement->right);
                        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExp->property);
                        if(funcExp && memExp && id)
                        {
                            subEntry.name = id->name.c_str();
                            subEntry.completeName = parent->name.isEmpty() ? subEntry.name : parent->completeName + "." + subEntry.name;
                            subEntry.type = PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION;

                            for(int j = 0; j < funcExp->params.size(); j++)
                            {
                                subEntry.params.append(funcExp->params[j]->name.c_str());
                            }

                            parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
                            parent->subElements.append(subEntry);
                        }
                    }
                }
            }

            esprima::FunctionDeclaration* funcDecl = dynamic_cast<esprima::FunctionDeclaration*>(funcExp->body->body[j]);
            if(funcDecl)
            {
                parseEsprimaFunctionDeclaration(funcDecl, parent ,&subEntry, tabIndex, objects);

                parent->subElements.append(subEntry);
            }
        }

    }
}


static void getTypeFromCallExpression(esprima::CallExpression* callExpression, ParsedEntry& subEntry);

static void getTypeFromMemberExpression(esprima::MemberExpression* memExpression, ParsedEntry& subEntry)
{
    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->object);
    if(id)
    {
        subEntry.valueType += id->name.c_str();

        if(memExpression->computed)
        {
            subEntry.isArrayIndex = true;
        }
        else
        {
            id = dynamic_cast<esprima::Identifier*>(memExpression->property);
            if(id)
            {
                subEntry.valueType += QString(".") + id->name.c_str();
            }
        }
    }

    esprima::CallExpression* callExp = dynamic_cast<esprima::CallExpression*>(memExpression->object);
    if(callExp)
    {
        getTypeFromCallExpression(callExp, subEntry);

        id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            subEntry.valueType += QString(".") + id->name.c_str();

        }
    }

    esprima::MemberExpression* mem = dynamic_cast<esprima::MemberExpression*>(memExpression->object);
    if(mem)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            getTypeFromMemberExpression(mem, subEntry);

             subEntry.valueType += QString(".") + id->name.c_str();
        }
    }

    esprima::ThisExpression* thisExpr = dynamic_cast<esprima::ThisExpression*>(memExpression->object);
    if(thisExpr)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            int index;
            QString tmpCompleteName = subEntry.completeName;

            index = tmpCompleteName.lastIndexOf(".");
            if(index != -1)
            {
                tmpCompleteName.remove(index, tmpCompleteName.length() - index);
            }


            index = tmpCompleteName.lastIndexOf(".");
            if(index != -1)
            {
                subEntry.valueType = tmpCompleteName;
                subEntry.valueType.remove(index, subEntry.valueType.length() - index);
                subEntry.valueType += ".";
                tmpCompleteName = "";
            }


            if(!tmpCompleteName.isEmpty())
            {
                subEntry.valueType = tmpCompleteName + ".";
            }
            subEntry.valueType += id->name.c_str();
        }
    }
}

static void getTypeFromCallExpression(esprima::CallExpression* callExpression, ParsedEntry& subEntry)
{

    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(callExpression->callee);
    if(id)
    {
        subEntry.valueType = id->name.c_str();

        if(subEntry.valueType == "Array")
        {
            if(callExpression->arguments.size() > 0)
            {
                esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(callExpression->arguments[0]);
                if(strLiteral)
                {
                    subEntry.valueType = "Array<String>";
                }

                esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(callExpression->arguments[0]);
                if(numLiteral)
                {
                    subEntry.valueType = "Array<Number>";
                }
            }
        }
        else if((subEntry.valueType == "Date") || (subEntry.valueType == "String") ||
                (subEntry.valueType == "Number"))
        {

        }
        else
        {
            if(!subEntry.valueType.endsWith("()"))
            {
                subEntry.valueType+= "()";
            }
        }
    }

    esprima::MemberExpression* memExpression = dynamic_cast<esprima::MemberExpression*>(callExpression->callee);
    if(memExpression)
    {
        getTypeFromMemberExpression(memExpression, subEntry);
        if(!subEntry.valueType.endsWith("()"))
        {
            subEntry.valueType+= "()";
        }

        if(subEntry.valueType.endsWith("getWidget()"))
        {//Possibly ScriptTable.getWidget.

            for(auto el : callExpression->arguments)
            {
                esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(el);
                if(numLiteral)
                {
                    subEntry.additionalInformation.append(QString("%1").arg(numLiteral->value));
                }
            }
        }

    }
}

static void getTypeFromNode(esprima::Node* node, ParsedEntry& subEntry)
{
    esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(node);
    if(strLiteral)
    {
        subEntry.valueType = "String";
    }

    esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(node);
    if(numLiteral)
    {
        subEntry.valueType = "Number";
    }

    esprima::BooleanLiteral* boolLiteral = dynamic_cast<esprima::BooleanLiteral*>(node);
    if(boolLiteral)
    {
        subEntry.valueType = "bool";
    }
    esprima::NewExpression* newExp = dynamic_cast<esprima::NewExpression*>(node);
    if(newExp)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(newExp->callee);
        if(id)
        {
            subEntry.valueType = id->name.c_str();
        }
    }

    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(node);
    if(id)
    {
        subEntry.valueType = id->name.c_str();
    }

    esprima::MemberExpression* memExpression = dynamic_cast<esprima::MemberExpression*>(node);
    if(memExpression)
    {
        getTypeFromMemberExpression(memExpression, subEntry);

    }

    esprima::BinaryExpression* binExp = dynamic_cast<esprima::BinaryExpression*>(node);
    if(binExp)
    {
        getTypeFromNode(binExp->left, subEntry);
    }

    esprima::ArrayExpression* arrayExp = dynamic_cast<esprima::ArrayExpression*>(node);
    if(arrayExp)
    {
        subEntry.valueType = "Array";

        if(arrayExp->elements.size() > 0)
        {
            esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(arrayExp->elements[0]);
            if(strLiteral)
            {
                subEntry.valueType = "Array<String>";
            }

            esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(arrayExp->elements[0]);
            if(numLiteral)
            {
                subEntry.valueType = "Array<Number>";
            }
        }

    }


    if(node == 0)
    {
        subEntry.valueType = "";
    }

    esprima::CallExpression* callExpression = dynamic_cast<esprima::CallExpression*>(node);
    if(callExpression)
    {
        subEntry.valueType = "";
        getTypeFromCallExpression(callExpression, subEntry);
    }

}

/**
 * Parses an (esprima) object expression.
 * @param objExp
 *      The object expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
 */
static void parseEsprimaObjectExpression(esprima::ObjectExpression* objExp, ParsedEntry* parent, int tabIndex,
                                         QMap<QString, ParsedEntry>* objects)
{
    for(int j = 0; j < objExp->properties.size(); j++)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(objExp->properties[j]->key);
        esprima::StringLiteral* literal = dynamic_cast<esprima::StringLiteral*>(objExp->properties[j]->key);

        ParsedEntry subEntry;
        subEntry.line = objExp->properties[j]->loc->start->line - 1;
        subEntry.column = objExp->properties[j]->loc->start->column;
        subEntry.endLine = objExp->properties[j]->loc->end->line - 1;
        subEntry.isArrayIndex = false;
        if(id)
        {
            subEntry.name = id->name.c_str();
        }
        else if(literal)
        {
            subEntry.name = literal->value.c_str();
        }
        else
        {
        }

        subEntry.completeName = parent->name.isEmpty() ? subEntry.name : parent->completeName + "." + subEntry.name;

        subEntry.type = PARSED_ENTRY_TYPE_MAP_VAR;
        subEntry.params = QStringList();
        subEntry.tabIndex = tabIndex;

        esprima::ObjectExpression* subObjExp = dynamic_cast<esprima::ObjectExpression*>(objExp->properties[j]->value);
        if(subObjExp)
        {

            parseEsprimaObjectExpression(subObjExp, &subEntry, tabIndex, objects);

            if(objects)
            {
                (*objects)[subEntry.completeName] = subEntry;
            }
        }

        esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(objExp->properties[j]->value);
        if(funcExp)
        {//The current expression is a function.

            subEntry.type = PARSED_ENTRY_TYPE_MAP_FUNC;

            //Parse all arguments.
            for(int j = 0; j < funcExp->params.size(); j++)
            {
                subEntry.params.append(funcExp->params[j]->name.c_str());
            }

            parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
        }
        else
        {
            getTypeFromNode(objExp->properties[j]->value, subEntry);
        }

        parent->subElements.append(subEntry);
    }
}


static void parseEsprimaVariableDeclaration(esprima::VariableDeclaration* varDecl, ParsedEntry &parent, ParsedEntry& entry, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL);



/**
 * Parses an (esprima) function declaration.
 * @param function
 *      The function declaration.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the declaration belongs to.
 */
static void parseEsprimaFunctionDeclaration(esprima::FunctionDeclaration* function, ParsedEntry* parent, ParsedEntry* entry, int tabIndex,
                                            QMap<QString, ParsedEntry>* objects)
{

    entry->line = function->id->loc->start->line - 1;
    entry->column = function->id->loc->start->column;
    entry->endLine = function->body->loc->end->line - 1;

    entry->name = function->id->name.c_str();
    entry->completeName = parent->completeName.isEmpty() ? entry->name : parent->completeName + "." + entry->name;
    entry->type = PARSED_ENTRY_TYPE_FUNCTION;
    entry->tabIndex = tabIndex;

    //Parse all arguments.
    for(int j = 0; j < function->params.size(); j++)
    {
        entry->params.append(function->params[j]->name.c_str());
    }

    //Parse the body of of the function.
    for(int j = 0; j < function->body->body.size(); j++)
    {
        ParsedEntry subEntry;
        esprima::VariableDeclaration* subVarDecl = dynamic_cast<esprima::VariableDeclaration*>(function->body->body[j]);
        if(subVarDecl)
        {
            parseEsprimaVariableDeclaration(subVarDecl, *entry, subEntry, tabIndex, objects);

            entry->subElements.append(subEntry);
        }
        else
        {
            esprima::IfStatement* ifStatement = dynamic_cast<esprima::IfStatement*>(function->body->body[j]);
            if(ifStatement)
            {
                parseEsprimaIfStatement(ifStatement, *entry, tabIndex, objects);
            }

            esprima::SwitchStatement* switchStatement = dynamic_cast<esprima::SwitchStatement*>(function->body->body[j]);
            if(switchStatement)
            {
               parseEsprimaSwitchStatement(switchStatement, *entry, tabIndex, objects);
            }

            esprima::ForStatement* forStatement = dynamic_cast<esprima::ForStatement*>(function->body->body[j]);
            if(forStatement)
            {
               parseEsprimaForStatement(forStatement, *entry, tabIndex, objects);
            }

            esprima::WhileStatement* whileStatement = dynamic_cast<esprima::WhileStatement*>(function->body->body[j]);
            if(whileStatement)
            {
               parseEsprimaWhileStatement(whileStatement, *entry, tabIndex, objects);
            }

            esprima::DoWhileStatement* doWhileStatement = dynamic_cast<esprima::DoWhileStatement*>(function->body->body[j]);
            if(doWhileStatement)
            {
               parseEsprimaDoWhileStatement(doWhileStatement, *entry, tabIndex, objects);
            }


            esprima::ExpressionStatement* expStatement = dynamic_cast<esprima::ExpressionStatement*>(function->body->body[j]);
            if(expStatement)
            {
                esprima::AssignmentExpression* assignmentStatement = dynamic_cast<esprima::AssignmentExpression*>(expStatement->expression);
                if(assignmentStatement)
                {
                    esprima::MemberExpression* memExp = dynamic_cast<esprima::MemberExpression*>(assignmentStatement->left);
                    esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(assignmentStatement->right);
                    if(funcExp && memExp)
                    {
                        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExp->property);
                        if(funcExp && memExp && id)
                        {
                            subEntry.line = assignmentStatement->loc->start->line - 1;
                            subEntry.column = assignmentStatement->loc->start->column;
                            subEntry.endLine = assignmentStatement->loc->end->line - 1;
                            subEntry.name = id->name.c_str();
                            subEntry.isArrayIndex = false;
                            subEntry.type = PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION;
                            subEntry.params = QStringList();
                            subEntry.tabIndex = tabIndex;
                            subEntry.completeName = entry->completeName.isEmpty() ? subEntry.name : entry->completeName + "." + subEntry.name;
                            for(int j = 0; j < funcExp->params.size(); j++)
                            {
                                subEntry.params.append(funcExp->params[j]->name.c_str());
                            }

                            parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
                            entry->subElements.append(subEntry);
                        }
                    }
                }
            }
        }

    }
}




/**
 * Parses an (esprima) new expression.
 * @param newExp
 *      The new expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
 */
static void parseEsprimaNewExpression(esprima::NewExpression* newExp, ParsedEntry* parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL)
{
    esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(newExp->callee);
    if(funcExp)
    {
        parseEsprimaFunctionExpression(funcExp, parent, tabIndex, objects);
    }
}


/**
 * Adds a parsed entry (and his subentries) to the auto completion list.
 * @param entry
 *      The parsed entry.
 * @param autoCompletionEntries
 *      The auto completion list.
 * @param parentString
 *      The string of the parent in the auto completion list.
 * @param rootObjectName
 *      The name of the root parsed entry.
 */
static void addParsedEntiresToAutoCompletionList(const ParsedEntry& entry, QMap<QString, QStringList>& autoCompletionEntries,
                                                 QString parentString, QString rootObjectName)
{
    QString value = (parentString.isEmpty()) ? entry.name : parentString + "::" + entry.name;

    autoCompletionEntries[rootObjectName] << value;

    for(auto el : entry.subElements)
    {
        addParsedEntiresToAutoCompletionList(el, autoCompletionEntries, value, rootObjectName);
    }
}


static getAllParsedTypes(QMap<QString, QString>& parsedTypes, ParsedEntry& entry, QString parentName)
{
    QString key = (parentName.isEmpty()) ? entry.name : parentName + "." + entry.name;

    parsedTypes[key] = entry.valueType;


    for(auto el : entry.subElements)
    {
        getAllParsedTypes(parsedTypes, el, key);
    }
}

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

bool ParseThread::replaceAllParsedTypes(QMap<QString, QString>& parsedTypes, ParsedEntry& entry,
                                        QString parentName)
{
    bool entryChanged = false;

    entry.completeName = (parentName.isEmpty()) ? entry.name : parentName + "::"  + entry.name;

    QString tmpType = entry.valueType;
    QString tmpCompleteName = entry.completeName;
    int index = entry.completeName.lastIndexOf("::");
    while(index != -1)
    {
        tmpCompleteName.remove(index, tmpCompleteName.length() - index);
        tmpType = tmpCompleteName + "." + entry.valueType;

        if(parsedTypes.contains(tmpType))
        {
            if(!parsedTypes[tmpType].isEmpty())
            {
                entry.valueType = parsedTypes[tmpType];
                tmpType = "";
                break;
            }
        }

        index = tmpCompleteName.lastIndexOf("::");
    }

    if(!tmpType.isEmpty())
    {
        if(parsedTypes.contains(entry.valueType))
        {
            if(!parsedTypes[entry.valueType].isEmpty())
            {
                entry.valueType = parsedTypes[entry.valueType];

                if(entry.isArrayIndex)
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


                        return true;
                    }
                }
            }
        }

        QString creatorName = entry.completeName;
        int tmpIndex = creatorName.lastIndexOf(entry.name);
        if(tmpIndex != -1)
        {
            creatorName.remove(tmpIndex, creatorName.length() - index);
            creatorName += split[0];
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

        if (m_creatorObjects.contains(creatorName))
        {
            split[0] = m_creatorObjects[creatorName];
            split[1] = split[split.size() - 1];
            bool isArrayIndex = false;
            if(entry.isArrayIndex)
            {
                isArrayIndex = true;
            }

            if(split[0].startsWith("Array<"))
            {
                if(isArrayIndex)
                {
                    split[0].replace("Array<", "");
                    split[0].replace(">", "");
                }
                else
                {
                    split[0] = "Array";
                }
            }
        }

        if(m_functionsWithResultObjects.contains(split[0]))
        {
            split[1].replace("()", "");

            for(int i = 0; i < m_functionsWithResultObjects[split[0]].size(); i++)
            {
                if((m_functionsWithResultObjects[split[0]][i].functionName == (split[1] + "(")) ||
                    (m_functionsWithResultObjects[split[0]][i].functionName == split[1]))
                {
                    bool isArray = m_functionsWithResultObjects[split[0]][i].isArray;
                    if(entry.isArrayIndex)
                    {
                        isArray = false;

                    }

                    addObjectToAutoCompletionList(entry.completeName, m_functionsWithResultObjects[split[0]][i].resultType, false,
                            isArray, true);

                    entry.valueType = "";

                    if(isArray)
                    {
                        entry.valueType = "Array<";
                    }
                    entry.valueType += m_functionsWithResultObjects[split[0]][i].resultType;

                    if(isArray)
                    {
                        entry.valueType += ">";
                    }

                    if(entry.valueType == "Array<Dummy>")
                    {
                        entry.valueType = "Array";
                    }

                    entryChanged = true;

                    m_creatorObjects[entry.completeName] = entry.valueType;

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

    for(int i = 0; i < entry.subElements.size(); i++)
    {
        if(replaceAllParsedTypes(parsedTypes, entry.subElements[i], entry.completeName))
        {
            entryChanged = true;
        }
    }

    return entryChanged;
}


static void parseEsprimaVariableDeclaration(esprima::VariableDeclaration* varDecl, ParsedEntry& parent, ParsedEntry& entry, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects )
{
    entry.line = varDecl->declarations[0]->loc->start->line - 1;
    entry.column = varDecl->declarations[0]->loc->start->column;
    entry.name = varDecl->declarations[0]->id->name.c_str();
    entry.isArrayIndex = false;
    entry.endLine = varDecl->declarations[0]->loc->end->line - 1;
    entry.completeName = parent.completeName.isEmpty() ? entry.name : parent.completeName + "." + entry.name;
    entry.params = QStringList();
    entry.tabIndex = tabIndex;
    entry.isArrayIndex = false;


    esprima::NewExpression* newExp = dynamic_cast<esprima::NewExpression*>(varDecl->declarations[0]->init);
    esprima::ObjectExpression* objExp = dynamic_cast<esprima::ObjectExpression*>(varDecl->declarations[0]->init);

    if(newExp)
    {//Class.

        esprima::FunctionExpression* funcExpr = dynamic_cast<esprima::FunctionExpression*>(newExp->callee);
        if(funcExpr)
        {//Class

            entry.type = PARSED_ENTRY_TYPE_CLASS;
            parseEsprimaNewExpression(newExp, &entry, tabIndex, objects);
            (*objects)[entry.name] = entry;
        }
        else
        {//Variable
            entry.type = PARSED_ENTRY_TYPE_VAR;
            esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(newExp->callee);
            if(id)
            {
                entry.params.append(id->name.c_str());
            }
            getTypeFromNode(newExp->callee, entry);
        }

    }
    else if(objExp)
    {//Map/Array

        entry.type = PARSED_ENTRY_TYPE_MAP;
        parseEsprimaObjectExpression(objExp, &entry, tabIndex, objects);
        (*objects)[entry.name] = entry;

    }
    else
    {//Function, Const or variable.

        esprima::VariableDeclarator* decl = dynamic_cast<esprima::VariableDeclarator*>(varDecl->declarations[0]);
        if(decl)
        {
            esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(decl->init);
            if(funcExp)
            {
                entry.type = PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION;
                parseEsprimaFunctionExpression(funcExp, &entry, tabIndex, objects);
            }
            else
            {
                entry.type = (varDecl->kind == "const") ? PARSED_ENTRY_TYPE_CONST : PARSED_ENTRY_TYPE_VAR;
                esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(decl->init);
                if(id)
                {//The the declaration of the current variable has an assignement of another variable/object
                 //e.g. var map2 = map1;
                    entry.params.append(id->name.c_str());
                }

                getTypeFromNode(decl->init, entry);

                if(dynamic_cast<esprima::MemberExpression*>(decl->init))
                {
                    entry.params.append(entry.valueType);

                    if(dynamic_cast<esprima::MemberExpression*>(decl->init)->computed)
                    {
                        entry.isArrayIndex = true;
                    }
                }
            }

        }
    }
}

static void parseEsprimaBlockStatement(esprima::BlockStatement* blockStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL)
{
    for(int i = 0; i < blockStatement->body.size(); i++)
    {
        esprima::VariableDeclaration* varDecl = dynamic_cast<esprima::VariableDeclaration*>(blockStatement->body[i]);
        if(varDecl)
        {
            ParsedEntry entry;
            parseEsprimaVariableDeclaration(varDecl, parent, entry, tabIndex, objects);

            //Append the current entry to the file result list.
            parent.subElements.append(entry);

        }

        esprima::IfStatement* ifStatement = dynamic_cast<esprima::IfStatement*>(blockStatement->body[i]);
        if(ifStatement)
        {
            parseEsprimaIfStatement(ifStatement, parent, tabIndex, objects);
        }

        esprima::SwitchStatement* switchStatement = dynamic_cast<esprima::SwitchStatement*>(blockStatement->body[i]);
        if(switchStatement)
        {
           parseEsprimaSwitchStatement(switchStatement, parent, tabIndex, objects);
        }

        esprima::ForStatement* forStatement = dynamic_cast<esprima::ForStatement*>(blockStatement->body[i]);
        if(forStatement)
        {
           parseEsprimaForStatement(forStatement, parent, tabIndex, objects);
        }

        esprima::WhileStatement* whileStatement = dynamic_cast<esprima::WhileStatement*>(blockStatement->body[i]);
        if(whileStatement)
        {
           parseEsprimaWhileStatement(whileStatement, parent, tabIndex, objects);
        }

        esprima::DoWhileStatement* doWhileStatement = dynamic_cast<esprima::DoWhileStatement*>(blockStatement->body[i]);
        if(doWhileStatement)
        {
           parseEsprimaDoWhileStatement(doWhileStatement, parent, tabIndex, objects);
        }
        esprima::ForInStatement* forInStatement = dynamic_cast<esprima::ForInStatement*>(blockStatement->body[i]);
        if(forInStatement)
        {
           parseEsprimaForInStatement(forInStatement, parent, tabIndex, objects);
        }
    }
}

static void parseEsprimaIfStatement(esprima::IfStatement* ifStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( ifStatement->alternate);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

    }

    blockStatement = dynamic_cast<esprima::BlockStatement*>( ifStatement->consequent);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

    }


}

static void parseEsprimaSwitchCase(esprima::SwitchCase* switchCase, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{

    for(auto el : switchCase->consequent)
    {
        esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( el);
        if(blockStatement)
        {
            parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

        }
    }
}

static void parseEsprimaSwitchStatement(esprima::SwitchStatement* switchStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    for(auto el : switchStatement->cases)
    {
        parseEsprimaSwitchCase(el, parent, tabIndex, objects);
    }
}

static void parseEsprimaForStatement(esprima::ForStatement* forStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::VariableDeclaration* varDecl = dynamic_cast<esprima::VariableDeclaration*>(forStatement->init);
    if(varDecl)
    {
        ParsedEntry entry;
        parseEsprimaVariableDeclaration( varDecl, parent, entry, tabIndex, objects);

        parent.subElements.append(entry);

    }

    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( forStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

    }
}

static void parseEsprimaWhileStatement(esprima::WhileStatement* whileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( whileStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}

static void parseEsprimaDoWhileStatement(esprima::DoWhileStatement* doWhileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( doWhileStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}

static void parseEsprimaForInStatement(esprima::ForInStatement* forInStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::VariableDeclaration* varDecl = dynamic_cast<esprima::VariableDeclaration*>(forInStatement->left);
    if(varDecl)
    {
        ParsedEntry entry;
        parseEsprimaVariableDeclaration( varDecl, parent, entry, tabIndex, objects);

        parent.subElements.append(entry);

    }

    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( forInStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}


/**
 * Returns all functions and gloabl variables in the loaded script files.
 * @param loadedScripts
 *      The loaded scripts.
 * @return
 *      All parsed entries.
 */
QMap<int,QVector<ParsedEntry>> ParseThread::getAllFunctionsAndGlobalVariables(QMap<int, QString> loadedScripts)
{
    QMap<int,QVector<ParsedEntry>> result;

    QMap<int, QString>::const_iterator iter = loadedScripts.constBegin();
    QMap<QString, QVector<ParsedEntry>> prototypeFunctionsSingleFile;
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
            entry.isArrayIndex = false;
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
                esprima::IfStatement* ifStatement = dynamic_cast<esprima::IfStatement*>(program->body[i]);
                if(ifStatement)
                {
                    parseEsprimaIfStatement(ifStatement, entry, iter.key(), &objects);

                    for(auto el : entry.subElements)
                    {
                        fileResult.append(el);
                    }
                }

                esprima::SwitchStatement* switchStatement = dynamic_cast<esprima::SwitchStatement*>(program->body[i]);
                if(switchStatement)
                {
                    parseEsprimaSwitchStatement(switchStatement, entry, iter.key(), &objects);

                    for(auto el : entry.subElements)
                    {
                        fileResult.append(el);
                    }
                }

                esprima::ForStatement* forStatement = dynamic_cast<esprima::ForStatement*>(program->body[i]);
                if(forStatement)
                {
                    parseEsprimaForStatement(forStatement, entry, iter.key(), &objects);

                    for(auto el : entry.subElements)
                    {
                        fileResult.append(el);
                    }
                }

                esprima::WhileStatement* whileStatement = dynamic_cast<esprima::WhileStatement*>(program->body[i]);
                if(whileStatement)
                {
                    parseEsprimaWhileStatement(whileStatement, entry, iter.key(), &objects);

                    for(auto el : entry.subElements)
                    {
                        fileResult.append(el);
                    }
                }

                esprima::DoWhileStatement* doWhileStatement = dynamic_cast<esprima::DoWhileStatement*>(program->body[i]);
                if(doWhileStatement)
                {
                    parseEsprimaDoWhileStatement(doWhileStatement, entry, iter.key(), &objects);

                    for(auto el : entry.subElements)
                    {
                        fileResult.append(el);
                    }
                }

                esprima::ForInStatement* forInStatement = dynamic_cast<esprima::ForInStatement*>(program->body[i]);
                if(forInStatement)
                {
                    parseEsprimaForInStatement(forInStatement, entry, iter.key(), &objects);

                    for(auto el : entry.subElements)
                    {
                        fileResult.append(el);
                    }
                }

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
                    esprima::ExpressionStatement* expStatement = dynamic_cast<esprima::ExpressionStatement*>(program->body[i]);
                    if(expStatement)
                    {
                        esprima::AssignmentExpression* assExp = dynamic_cast<esprima::AssignmentExpression*>(expStatement->expression);
                        if(assExp)
                        {
                            esprima::MemberExpression* left = dynamic_cast<esprima::MemberExpression*>(assExp->left);
                            esprima::FunctionExpression* right = dynamic_cast<esprima::FunctionExpression*>(assExp->right);
                            if(left && right)
                            {
                                esprima::MemberExpression* object = dynamic_cast<esprima::MemberExpression*>(left->object);
                                esprima::Identifier* property = dynamic_cast<esprima::Identifier*>(left->property);
                                if(object && property)
                                {
                                    esprima::Identifier* subObject = dynamic_cast<esprima::Identifier*>(object->object);
                                    if(subObject)
                                    {//Prototyp function

                                        ParsedEntry prot;
                                        prot.line = assExp->loc->start->line -1;
                                        prot.column = assExp->loc->start->column;
                                        prot.endLine = assExp->loc->end->line -1;
                                        prot.name = property->name.c_str();
                                        prot.params = QStringList();
                                        prot.tabIndex = iter.key();
                                        prot.type = PARSED_ENTRY_TYPE_PROTOTYPE_FUNC;

                                        for(int j = 0; j < right->params.size(); j++)
                                        {
                                            prot.params.append(right->params[j]->name.c_str());
                                        }

                                         parseEsprimaFunctionExpression(right, &prot, iter.key(), &objects);

                                        if(!prototypeFunctionsSingleFile.contains(subObject->name.c_str()))
                                        {
                                          prototypeFunctionsSingleFile[subObject->name.c_str()] = QVector<ParsedEntry>();
                                        }
                                        prototypeFunctionsSingleFile[subObject->name.c_str()].append(prot);
                                    }
                                }
                            }
                        }

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

            if(prototypeFunctionsSingleFile.contains(result[i][j].name))
            {//The current object/class has prototype functions.

                //Add the prototyp function to the current objects/classes
                for (auto member : prototypeFunctionsSingleFile[result[i][j].name])
                {
                    result[i][j].subElements.append(member);
                }
            }

            //Remove the prototype functions.
            prototypeFunctionsSingleFile.remove(result[i][j].name);

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

    return result;
}
/**
 * Parses the current text. Emits parsingFinishedSignal if the parsing is finished.
 * @param loadedUiFiles
 *      All loaded ui files.
 * @param loadedScripts
 *      All loaded scripts.
 */
void ParseThread::parseSlot(QMap<QString, QString> loadedUiFiles, QMap<int, QString> loadedScripts, QMap<int, QString> loadedScriptsIndex, bool loadedFileChanged, bool parseOnlyUIFiles)
{

    //Clear all parsed objects (all but m_autoCompletionApiFiles).
    m_autoCompletionEntries.clear();
    m_creatorObjects.clear();
    m_tableWidgetObjects.clear();
    m_parsedUiObjects.clear();
    m_parsedUiFiles.clear();
    m_stringList.clear();
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

