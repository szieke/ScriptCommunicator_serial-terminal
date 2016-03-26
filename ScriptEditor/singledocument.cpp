#include "singledocument.h"
#include "Qsci/qscilexerjavascript.h"
#include "mainwindow.h"
#include <Qsci/qsciapis.h>
#include <QTextStream>
#include <QCoreApplication>
#include <QDirIterator>
#include <QThread>

QMap<QString, QStringList> g_apiFiles;

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

    if(g_apiFiles.isEmpty())
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
                    g_apiFiles[fileName] << QString(singleEntry).replace("\\n", "\n");
                    singleEntry = in.readLine();
                }
            }
            file.close();
        }

    }
    connect(this, SIGNAL(textChanged()), m_mainWindow, SLOT(documentWasModified()));
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
        for (i = g_apiFiles.begin(); i != g_apiFiles.end(); ++i)
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

