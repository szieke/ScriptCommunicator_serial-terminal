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

        if(!g_autoCompletionEntries.contains(objectName))
        {//The object is not in g_autoCompletionEntries.

            //Open the corresponding api file.
            QFile file(getScriptEditorFilesFolder()+ "/apiFiles/" + className + ".api");
            if (file.open(QFile::ReadOnly))
            {
                QTextStream in(&file);
                QString singleEntry = in.readLine();

                while(!singleEntry.isEmpty())
                {
                    singleEntry.replace(className + "::", "UI_" + objectName + "::");

                    //Add the current line to the api map.
                    g_autoCompletionEntries[objectName] << QString(singleEntry).replace("\\n", "\n");
                    singleEntry = in.readLine();
                }
                file.close();
            }
        }
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

/**
 * Checks if in the current document user interface files are loaded.
 * If user interface are loaded then they will be parsed and added to the auto-completion
 * list (g_autoCompletionEntries).
 */
void SingleDocument::checkDocumentForUiFiles(void)
{
    QString currentText = text();
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



