#include "singledocument.h"
#include "Qsci/qscilexerjavascript.h"
#include "mainwindow.h"
#include <Qsci/qsciapis.h>
#include <QTextStream>
#include <QCoreApplication>
#include <QDirIterator>
#include <QThread>


//Contains all autocompletion entries.
static QMap<QString, QStringList> g_autoCompletionEntries;
bool g_objectAddedToCompletionList = false;

//Conatins all parsed api files.
static QMap<QString, QStringList> g_autoCompletionApiFiles;

//Contains all Objects which can create other objects.
static QMap<QString, QString> g_creatorObjects;

//Conatins all String Objects.
QVector<QString> g_stringList;

//Conatins all Objects with a unknown type.
QVector<QString> g_unknownTypeObjects;

//Conatins all Array Objects.
QVector<QString> g_arrayList;

//Conatins all ScriptTableEidget Objects.
QVector<QString> g_tableWidgets;


//Object created by ScriptTable::InsertWidget.
typedef struct
{
    int row;
    int column;
    QString className;
}TableWidgetSubObject;

//Conatins all objects created by ScriptTable::InsertWidget.
static QMap<QString, QVector<TableWidgetSubObject>> g_tableWidgetObjects;

//Caintains all found ui files.
static QStringList g_foundUiFiles;

//Caintains all parsed ui files.
static QStringList g_parsedFiles;

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
 * Constructor.
 * @param mainWindow
 *      Pointer to the main window.
 * @param parent
 *      The parent.
 */
SingleDocument::SingleDocument(MainWindow *mainWindow, QWidget *parent) :
    QsciScintilla(parent), m_mainWindow(mainWindow), m_documentName("")
{

    if(g_autoCompletionApiFiles.isEmpty())
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
                    g_autoCompletionApiFiles[fileName] << QString(singleEntry).replace("\\n", "\n");
                    singleEntry = in.readLine();
                }
            }
            file.close();
        }
    }

    connect(this, SIGNAL(textChanged()), m_mainWindow, SLOT(documentWasModified()));
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
static void addObjectToAutoCompletionList(QString& objectName, QString& className, bool isGuiElement, bool isArray=false, bool replaceExistingEntry=false)
{
    QString autoCompletionName = isGuiElement ? "UI_" + objectName : objectName;


    if(!objectName.isEmpty())
    {

        if(g_autoCompletionEntries.contains(autoCompletionName))
        {
            if(replaceExistingEntry)
            {
                g_autoCompletionEntries.remove(autoCompletionName);
            }
            else
            {
                return;
            }
        }
        else
        {
            g_objectAddedToCompletionList = true;
        }

        if(!isArray)
        {
            if (g_autoCompletionApiFiles.contains(className + ".api"))
            {
                QStringList list = g_autoCompletionApiFiles[className + ".api"];

                for(auto singleEntry : list)
                {
                    singleEntry.replace(className + "::", autoCompletionName + "::");

                    //Add the current line to the api map.
                    g_autoCompletionEntries[autoCompletionName] << singleEntry;
                }
            }
            else
            {
                if(!g_unknownTypeObjects.contains(autoCompletionName))
                {
                    g_unknownTypeObjects.append(autoCompletionName);
                }
            }
        }
        else
        {

            if (g_autoCompletionApiFiles.contains(className + ".api"))
            {
                QStringList list = g_autoCompletionApiFiles[className + ".api"];

                for(auto singleEntry : list)
                {

                    singleEntry.replace(className + "::", autoCompletionName + "[" + "::");

                    //Add the current line to the api map.
                    g_autoCompletionEntries[autoCompletionName + "["] << singleEntry;
                }
            }
            else
            {
                if(!g_unknownTypeObjects.contains(autoCompletionName + "["))
                {
                    g_unknownTypeObjects.append(autoCompletionName + "[");
                }
            }

            QStringList arrayList = g_autoCompletionApiFiles["Array.api"];
            for(auto singleEntry : arrayList)
            {
                singleEntry.replace("Array::", autoCompletionName + "::");

                //Add the current line to the api map.
                g_autoCompletionEntries[autoCompletionName] << singleEntry;

                int index = g_arrayList.indexOf(autoCompletionName);
                if(index != -1)
                {
                    g_arrayList.remove(index);
                }
                g_arrayList.append(autoCompletionName);
            }
        }

        if(className == "String")
        {
            int index = g_stringList.indexOf(autoCompletionName);
            if(index != -1)
            {
                g_stringList.remove(index);
            }
            g_stringList.append(autoCompletionName);
        }
        else if(className == "ScriptTableWidget")
        {
            int index = g_tableWidgets.indexOf(autoCompletionName);
            if(index != -1)
            {
                g_tableWidgets.remove(index);
            }
            g_tableWidgets.append(autoCompletionName);
        }
        else
        {
            if((className != "Dummy"))
            {
                g_creatorObjects[autoCompletionName] = className;
            }
        }
    }
}

/**
 * Parse a widget list from a user interface file (auto-completion).
 * @param docElem
 *      The QDomElement from the user interface file.
 * @param parseActions
 *      If true then all actions will be parsed. If false then all widgets will be parsed.
 */
void SingleDocument::parseWidgetList(QDomElement& docElem, bool parseActions)
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
 * Parses an user interface file (auto-completion).
 * @param uiFileName
 *      The user interface file.
 */
void SingleDocument::parseUiFile(QString uiFileName)
{

    QFile uiFile(uiFileName);
    QDomDocument doc("ui");

    if(!g_parsedFiles.contains(uiFileName))
    {
        g_parsedFiles.append(uiFileName);

        if (uiFile.open(QFile::ReadOnly))
        {
            if (doc.setContent(&uiFile))
            {
                QDomElement docElem = doc.documentElement();
                parseWidgetList(docElem, false);
                parseWidgetList(docElem, true);
            }

            uiFile.close();
            if(!g_foundUiFiles.contains(uiFileName))
            {
                g_foundUiFiles.append(uiFileName);
            }
        }
    }

}

/**
 * Sets the document name/path.
 * @param name
 *      The document name.
 */
void SingleDocument::setDocumentName(QString name)
{
    m_documentName = name;
    initLexer(name);

    //Check if a ui file exist.
    QString uiFileName = MainWindow::getTheCorrespondingUiFile(m_documentName);
    if(!uiFileName.isEmpty())
    {
        parseUiFile(uiFileName);
    }
}

/**
 * Initializes the lexer.
 * @param script
 *      The script file.
 */
void SingleDocument::initLexer(QString script)
{
    (void)script;

    if(lexer() == 0)
    {
        setLexer(new QsciLexerJavaScript(this));
        QsciAPIs* apis = new QsciAPIs(lexer());
        (void)apis;


        setAutoCompletionSource(QsciScintilla::AcsAPIs);
        setAutoCompletionCaseSensitivity(true);
        setAutoCompletionThreshold(3);
        setAutoIndent(true);
        setIndentationWidth(4);
        setTabWidth(4);
        setMarginLineNumbers(0,true);
        setMarginType(0, QsciScintilla::NumberMargin);
        setMarginsForegroundColor(QColor(128, 128, 128));
       }

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
static inline void searchSingleType(QString className, QString searchString, QStringList& lines, bool isArray=false,
                                    bool withOutDotsAndBracked= false, bool replaceExistingEntry=false, bool matchExact = false)
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

            addObjectToAutoCompletionList(objectName, className, false, isArray, replaceExistingEntry);
        }
    }
}

/**
 * Initializes the autocompletion.
 * @param additionalElements
 *      Additional elements for the autocompletion api.
 */
void SingleDocument::initAutoCompletion(QStringList additionalElements, QString& currentText)
{

    g_autoCompletionEntries.clear();
    g_creatorObjects.clear();
    g_tableWidgetObjects.clear();
    g_parsedFiles.clear();
    g_stringList.clear();
    g_arrayList.clear();
    g_tableWidgets.clear();
    g_unknownTypeObjects.clear();
    QRegExp regexp("[\n;]");

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

     QStringList linesWithBrackets = currentText.split(regexp);

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

    QStringList lines = currentText.split(regexp);

    //Parse all found user interface files (check for changes).
    for(auto el : g_foundUiFiles)
    {
        parseUiFile(el);
    }

    //Find all includes user interface files.
    checkDocumentForUiFiles(currentText);

    int counter = 1;
    do
    {
        g_objectAddedToCompletionList = false;

        //Call several times to get all objects created by dynamic objects.
        checkDocumentForCustomDynamicObjects(lines, linesWithBrackets, currentText, counter);
        counter++;
    }while(g_objectAddedToCompletionList);


    counter = 1;
    do
    {
        g_objectAddedToCompletionList = false;

        //Call several times to get all objects created by dynamic objects.
        checkDocumentForStandardDynamicObjects(lines, linesWithBrackets, currentText, counter);
        counter++;
    }while(g_objectAddedToCompletionList);


    //Search all objects with an unknwon type.
    searchSingleType("Dummy", "=", lines);


    for(auto el : g_unknownTypeObjects)
    {
        if(!g_autoCompletionEntries.contains(el))
        {
            g_autoCompletionEntries[el] << el;
        }
    }


    //Initialize the api.
    if(lexer() && lexer()->apis())
    {
        QsciAPIs* apis = static_cast<QsciAPIs*>(lexer()->apis());
        apis->clear();
        QMap<QString, QStringList>::iterator i;
        for (i = g_autoCompletionEntries.begin(); i != g_autoCompletionEntries.end(); ++i)
        {
            for(auto el : i.value())
            {
                apis->add(el);
            }
        }

        for (i = g_autoCompletionApiFiles.begin(); i != g_autoCompletionApiFiles.end(); ++i)
        {
            for(auto el : i.value())
            {
                apis->add(el);
            }
        }

        for(auto el : additionalElements)
        {
            apis->add(el);
        }
        apis->prepare();
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
void searchSingleTableSubWidgets(QString objectName, QVector<TableWidgetSubObject> subObjects, QStringList lines)
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
void parseTableWidetInsert(const QString objectName, QStringList lines)
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
                    TableWidgetSubObject subObject = {list[0].toUInt(&isOk),list[1].toUInt(&isOk), list[2]};


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

                    if(g_tableWidgetObjects.contains(objectName))
                    {
                        g_tableWidgetObjects[objectName].append(subObject);
                    }
                    else
                    {
                        QVector<TableWidgetSubObject> vect;
                        vect.append(subObject);

                        g_tableWidgetObjects[objectName] = vect;
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
inline void searchForScriptWidgetCommonFunctions(const QString& objectName, QStringList& lines)
{
    searchSingleType("String", "=" + objectName + ".getAdditionalData(", lines);
    searchSingleType("String", "=" + objectName + ".getClassName(", lines);
    searchSingleType("String", "=" + objectName + ".getPublicScriptElements(", lines);
}

/**
 * Searches all dynamically created objects created by custom objects (like ScriptTimer).
 * @param currentText
 *      The text in which shall be searched.
 */
void SingleDocument::checkDocumentForCustomDynamicObjects(QStringList& lines, QStringList &linesWithBrackets , QString &currentText, int passNumber)
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
            searchSingleType("Dummy", "=scriptThread.stringToArray(", lines, true);
            searchSingleType("Dummy", "=scriptThread.addStringToArray(", lines, true);
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
            searchSingleType("String", "=scriptThread.getPublicScriptElements(", lines);
            searchSingleType("String", "=scriptThread.byteArrayToString(", lines);
            searchSingleType("String", "=scriptThread.byteArrayToHexString(", lines);
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
            searchSingleType("ScriptTcpClient", "=scriptThread.createTcpClient(", lines);
        }

        if(currentText.contains("=seq."))
        {
            searchSingleType("Dummy", "=seq.stringToArray(", lines, true);
            searchSingleType("Dummy", "=seq.addStringToArray(", lines, true);
            searchSingleType("Dummy", "=seq.getGlobalDataArray(", lines, true);
            searchSingleType("Dummy", "=seq.getGlobalUnsignedNumber(", lines, true);
            searchSingleType("Dummy", "=seq.getGlobalSignedNumber(", lines, true);
            searchSingleType("Dummy", "=seq.getGlobalRealNumber(", lines, true);
            searchSingleType("Dummy", "=seq.showGetIntDialog(", lines, true);
            searchSingleType("Dummy", "=seq.showGetDoubleDialog(", lines, true);
            searchSingleType("Dummy", "=seq.showColorDialog(", lines, true);
            searchSingleType("String", "=seq.getAllObjectPropertiesAndFunctions(", lines);
            searchSingleType("String", "=seq.byteArrayToString(", lines);
            searchSingleType("String", "=seq.byteArrayToHexString(", lines);
            searchSingleType("String", "=seq.getGlobalString(", lines);
            searchSingleType("String", "=seq.getCurrentVersion(", lines);
            searchSingleType("String", "=seq.showTextInputDialog(", lines);
            searchSingleType("String", "=seq.showMultiLineTextInputDialog(", lines);
            searchSingleType("String", "=seq.showGetItemDialog(", lines);
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
            searchSingleType("String", "=cust.byteArrayToString(", lines);
            searchSingleType("String", "=cust.byteArrayToHexString(", lines);
            searchSingleType("String", "=cust.getScriptFolder(", lines);
            searchSingleType("String", "=cust.readFile(", lines);
            searchSingleType("String", "=cust.createAbsolutePath(", lines);
            searchSingleType("String", "=cust.getCurrentVersion(", lines);
            searchSingleType("Dummy", "=cust.stringToArray(", lines, true);
            searchSingleType("Dummy", "=cust.addStringToArray(", lines, true);
            searchSingleType("Dummy", "=cust.readBinaryFile(", lines, true);
            searchSingleType("String", "=cust.readDirectory(", lines, true);
            searchSingleType("String", "=cust.getAllObjectPropertiesAndFunctions(", lines, true);
        }


        for(auto el : g_tableWidgets)
        {
            parseTableWidetInsert(el, lines);
            searchSingleType("Dummy", "=" + el + ".getAllSelectedCells(", lines, true);
            searchForScriptWidgetCommonFunctions(el, lines);
        }

        QStringList keys = g_tableWidgetObjects.keys();
        for(int i = 0; i < keys.length(); i++)
        {
            searchSingleTableSubWidgets(keys[i], g_tableWidgetObjects.value(keys[i]), lines);
        }

    }//if(passNumber == 1)



    QMap<QString, QString> tmpCreatorList = g_creatorObjects;
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
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            }
            else if(i.value() == "ScriptXmlWriter")
            {
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
                searchSingleType("String", "=" + i.key() + ".getInternalBuffer(", lines);
            }
            else if(i.value() == "ScriptSqlDatabase")
            {
                searchSingleType("ScriptSqlIndex", "=" + i.key() + ".primaryIndex(", lines);
                searchSingleType("ScriptSqlRecord", "=" + i.key() + ".record(", lines);
                searchSingleType("ScriptSqlQuery", "=" + i.key() + ".exec(", lines);
                searchSingleType("ScriptSqlError", "=" + i.key() + ".lastError(", lines);
                searchSingleType("String", "=" + i.key() + ".tables(", lines, true);
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
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
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            }
            else if(i.value() == "ScriptCheetahSpi")
            {
                searchSingleType("Dummy", "=" + i.key() + ".readAll(", lines, true);
                searchSingleType("String", "=" + i.key() + ".readAllLines(", lines, true);
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
                searchSingleType("String", "=" + i.key() + ".detectDevices(", lines);
            }
            else if(i.value() == "ScriptSerialPort")
            {
                searchSingleType("Dummy", "=" + i.key() + ".readAll(", lines, true);
                searchSingleType("String", "=" + i.key() + ".readAllLines(", lines, true);
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
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
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
                searchSingleType("String", "=" + i.key() + ".readLine(", lines);
            }
            else if(i.value() == "ScriptPcanInterface")
            {
                searchSingleType("Dummy", "=" + i.key() + ".getCanParameter(", lines, true);
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
                searchSingleType("String", "=" + i.key() + ".getStatusString(", lines);
            }
            else if(i.value() == "ScriptSplitter")
            {
                searchSingleType("Dummy", "=" + i.key() + ".sizes(", lines, true);
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            }
            else if(i.value() == "LED")
            {
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            }
            else if(i.value() == "QWebView")
            {
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
                searchSingleType("String", "=" + i.key() + ".url(", lines);
                searchSingleType("String", "=" + i.key() + ".selectedHtml(", lines);
                searchSingleType("String", "=" + i.key() + ".selectedText(", lines);
                searchSingleType("String", "=" + i.key() + ".title(", lines);
            }
            else if(i.value() == "ScriptAction")
            {
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
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
                searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
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
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".elementName(", lines);
            searchSingleType("String", "=" + i.key() + ".attributeValue(", lines);
        }
        else if(i.value() == "ScriptXmlAttribute")
        {
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".value(", lines);
            searchSingleType("String", "=" + i.key() + ".name(", lines);
        }
        else if(i.value() == "ScriptSqlQuery")
        {
            searchSingleType("ScriptSqlError", "=" + i.key() + ".lastError(", lines);
            searchSingleType("ScriptSqlRecord", "=" + i.key() + ".record(", lines);
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".lastQuery(", lines);
            searchSingleType("String", "=" + i.key() + ".executedQuery(", lines);
        }
        else if(i.value() == "ScriptSqlRecord")
        {
            searchSingleType("ScriptSqlField", "=" + i.key() + ".field(", lines);
            searchSingleType("ScriptSqlRecord", "=" + i.key() + ".keyValues(", lines);
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".fieldName(", lines);
        }
        else if(i.value() == "ScriptSqlError")
        {
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".driverText(", lines);
            searchSingleType("String", "=" + i.key() + ".databaseText(", lines);
            searchSingleType("String", "=" + i.key() + ".nativeErrorCode(", lines);
            searchSingleType("String", "=" + i.key() + ".text(", lines);
        }
        else if(i.value() == "ScriptSqlField")
        {
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".name(", lines);
        }
        else if(i.value() == "ScriptSqlIndex")
        {
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".cursorName(", lines);
            searchSingleType("String", "=" + i.key() + ".name(", lines);
        }
        else if(i.value() == "ScriptTcpClient")
        {
            searchSingleType("Dummy", "=" + i.key() + ".readAll(", lines, true);
            searchSingleType("String", "=" + i.key() + ".readAllLines(", lines, true);
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
            searchSingleType("String", "=" + i.key() + ".getErrorString(", lines);
            searchSingleType("String", "=" + i.key() + ".readLine(", lines);
        }
        else if(i.value() == "ScriptPlotWidget")
        {
            searchForScriptWidgetCommonFunctions(i.key(), lines);
        }
        else if(i.value() == "ScriptCanvas2DWidget")
        {
            searchSingleType("String", "=" + i.key() + ".getPublicScriptElements(", lines);
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

        searchSingleType(i.value(), "=" + i.key(), linesWithBrackets, g_arrayList.contains(i.key()), true, false, true);
        searchSingleType(i.value(), "=" + i.key() + "[", linesWithBrackets);
    }
}

/**
 * Searches all dynamically created objects created by standard objects (like String).
 * @param currentText
 *      The text in which shall be searched.
 */
void SingleDocument::checkDocumentForStandardDynamicObjects(QStringList& lines, QStringList &linesWithBrackets , QString &currentText, int passNumber)
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

    QVector<QString> tmpList = g_stringList;
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

        searchSingleType("String", "=" + el, linesWithBrackets, g_arrayList.contains(el), true, false, true);
        searchSingleType("String", "=" + el + "[", linesWithBrackets);


    }

    tmpList = g_arrayList;
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
 * Checks if in the current document user interface files are loaded.
 * If user interface are loaded then they will be parsed and added to the auto-completion
 * list (g_autoCompletionEntries).
 */
void SingleDocument::checkDocumentForUiFiles(QString currentText)
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
            uiFile = QFileInfo(getDocumentName()).absolutePath() + "/" + uiFile;
        }

        parseUiFile(uiFile);
        index = currentText.indexOf("scriptThread.loadUserInterfaceFile", index + 1);

    }
}



