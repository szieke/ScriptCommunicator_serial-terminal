#include "singledocument.h"
#include "Qsci/qscilexerjavascript.h"
#include "mainwindow.h"
#include <Qsci/qsciapis.h>
#include <QTextStream>
#include <QCoreApplication>
#include <QDirIterator>
#include <QThread>


//Contains all autocompletion entries.
QMap<QString, QStringList> g_autoCompletionEntries;

QStringList g_parsedUiFiles;

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


SingleDocument::SingleDocument(MainWindow *mainWindow, QWidget *parent) :
    QsciScintilla(parent), m_mainWindow(mainWindow), m_documentName("")
{

    if(g_autoCompletionEntries.isEmpty())
    {
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
                    g_autoCompletionEntries[fileName] << QString(singleEntry).replace("\\n", "\n");
                    singleEntry = in.readLine();
                }
            }
            file.close();
        }

    }

    connect(this, SIGNAL(textChanged()), m_mainWindow, SLOT(documentWasModified()));
}


void addObjectToAutoCompletionList(QString& objectName, QString& className, bool isGuiElement)
{
    if(!objectName.isEmpty() && !g_autoCompletionEntries.contains(objectName))
    {//The object is not in g_autoCompletionEntries.

        QString autoCompletionName = isGuiElement ? "UI_" + objectName : objectName;

        //Open the corresponding api file.
        QFile file(getScriptEditorFilesFolder()+ "/apiFiles/" + className + ".api");
        if (file.open(QFile::ReadOnly))
        {
            QTextStream in(&file);
            QString singleEntry = in.readLine();

            while(!singleEntry.isEmpty())
            {
                singleEntry.replace(className + "::", autoCompletionName + "::");

                //Add the current line to the api map.
                g_autoCompletionEntries[objectName] << QString(singleEntry).replace("\\n", "\n");
                singleEntry = in.readLine();
            }
            file.close();
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
    if(!g_parsedUiFiles.contains(uiFileName))
    {
        QFile uiFile(uiFileName);
        QDomDocument doc("ui");

        if (uiFile.open(QFile::ReadOnly))
        {
            if (doc.setContent(&uiFile))
            {
                QDomElement docElem = doc.documentElement();
                parseWidgetList(docElem, false);
                parseWidgetList(docElem, true);
            }

            uiFile.close();
        }
        g_parsedUiFiles.append(uiFileName);
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
 * Initializes the autocompletion.
 * @param additionalElements
 *      Additional elements for the autocompletion api.
 */
void SingleDocument::initAutoCompletion(QStringList additionalElements)
{
    QString currentText = text();

    //Remove all '/**/' comments.
    QRegExp comment("/\\*(.|[\r\n])*\\*/");
    comment.setMinimal(true);
    comment.setPatternSyntax(QRegExp::RegExp);
    currentText.replace(comment, " ");

    //Remove all '//' comments.
    comment.setMinimal(false);
    comment.setPattern("//[^\n]*");
    currentText.remove(comment);


    checkDocumentForUiFiles(currentText);
    checkDocumentForDynamicObjects(currentText);

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

        for(auto el : additionalElements)
        {
            apis->add(el);
        }
        apis->prepare();
    }

}


static inline void removeAllLeft(QString& text, QString character)
{
    int index = text.indexOf(character);
    if(index != -1)
    {
        text = text.right((text.length() - index) - 1);
    }
}

static inline void searchSingleType(QString className, QString searchString, QStringList& lines)
{
    int index;
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
            addObjectToAutoCompletionList(objectName, className, false);
        }
    }
}

void SingleDocument::checkDocumentForDynamicObjects(QString currentText)
{
    currentText.replace("var ", "");
    currentText.replace(" ", "");

    QRegExp regexp("[\n;]");
    QStringList lines = currentText.split(regexp);

    searchSingleType("ScriptUdpSocket", "=scriptThread.createUdpSocket", lines);
    searchSingleType("ScriptTcpServer", "=scriptThread.createTcpServer", lines);
    searchSingleType("ScriptTcpClient", "=scriptThread.createTcpClient", lines);
    searchSingleType("ScriptTimer", "=scriptThread.createTimer", lines);
    searchSingleType("ScriptSerialPort", "=scriptThread.createSerialPort", lines);
    searchSingleType("ScriptCheetahSpi", "=scriptThread.createCheetahSpiInterface", lines);
    searchSingleType("ScriptPcanInterface", "=scriptThread.createPcanInterface", lines);
    searchSingleType("ScriptPlotWindow", "=scriptThread.createPlotWindow", lines);

    searchSingleType("ScriptPlotWidget", ".addPlotWidget", lines);
    searchSingleType("ScriptCanvas2DWidget", ".addCanvas2DWidget", lines);

    searchSingleType("ScriptXmlReader", "=scriptThread.createXmlReader", lines);
    searchSingleType("ScriptXmlWriter", "=scriptThread.createXmlWriter", lines);
    searchSingleType("ScriptXmlReader", "=cust.createXmlReader", lines);
    searchSingleType("ScriptXmlWriter", "=cust.createXmlWriter", lines);
    searchSingleType("ScriptSqlDatabase", "=scriptSql.addDatabase", lines);
    searchSingleType("ScriptSqlDatabase", "=scriptSql.cloneDatabase", lines);
    searchSingleType("ScriptSqlDatabase", "=scriptSql.database", lines);
    searchSingleType("ScriptSqlQuery", "=scriptSql.createQuery", lines);
    searchSingleType("ScriptSqlField", "=scriptSql.createField", lines);
    searchSingleType("ScriptSqlRecord", "=scriptSql.createRecord", lines);
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



