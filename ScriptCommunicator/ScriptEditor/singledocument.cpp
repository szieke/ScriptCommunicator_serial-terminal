#include "singledocument.h"
#include "Qsci/qscilexerjavascript.h"
#include "mainwindow.h"
#include <Qsci/qsciapis.h>
#include <QTextStream>
#include <QCoreApplication>
#include <QDirIterator>
#include <QThread>
#include "parseThread.h"




/**
 * Constructor.
 * @param mainWindow
 *      Pointer to the main window.
 * @param parent
 *      The parent.
 */
SingleDocument::SingleDocument(MainWindow *mainWindow, QWidget *parent) :
    QsciScintilla(parent), m_mainWindow(mainWindow), m_documentName(""), m_fileLastModified(QDateTime::currentDateTime()),
    m_fileMustBeParsed(true), m_clickIndicatorStart(-1), m_clickIndicatorEnd(-1), m_clickIndicatorIdentifier(1),
    m_functions()
{

    connect(this, SIGNAL(textChanged()), m_mainWindow, SLOT(documentWasModified()));

    m_clickIndicatorIdentifier = indicatorDefine(PlainIndicator, -1);
    setIndicatorForegroundColor(QColor(0,0,255), m_clickIndicatorIdentifier);

    setUtf8(true);
}

void SingleDocument::keyReleaseEvent(QKeyEvent *event)
{
    if((event->modifiers() & Qt::ControlModifier) == 0)
    {
        m_mainWindow->m_ctrlIsPressed = false;
        removeUndlineFromWordWhichCanBeClicked();
    }
}
void SingleDocument::keyPressEventChild(QKeyEvent *event)
{

    if((event->modifiers() & Qt::ControlModifier) != 0)
    {
        removeUndlineFromWordWhichCanBeClicked();
        m_mainWindow->m_ctrlIsPressed = true;
        m_mainWindow->m_mouseEventTimer.start(100);
    }
}

/**
 * Returns the current context string.
 * @param line
 *      The context line.
 * @return
 *      The context string.
 */
QString SingleDocument::getContextString(int line)
{
    QString result;
    for(auto el : m_functions)
    {
        if((el.endLine >= line) && (el.line <= line))
        {//Context found.

            result = el.completeName;
        }
    }

    return result;
}

///Adds a function the current document.
void SingleDocument::addFunction(ParsedEntry& function)
{
    m_functions.append(function);
}

void SingleDocument::mouseMoveEventChild(QMouseEvent *event)
{
    if(!m_mainWindow->m_ctrlIsPressed)
    {
        removeUndlineFromWordWhichCanBeClicked();
    }
    m_mainWindow->m_lastMouseMoveEvent = *event;
    m_mainWindow->m_mouseEventTimer.start(100);
}

/**
 * Underlines a word which can be clicked (funktion or variable in the outline window).
 * @param pos
 *      The position of one character of the word.
 */
void SingleDocument::underlineWordWhichCanBeClicked(int pos)
{
    if(m_clickIndicatorStart != -1)
    {
        removeUndlineFromWordWhichCanBeClicked();
    }

    m_clickIndicatorStart = SendScintilla(SCI_WORDSTARTPOSITION, pos, true);
    m_clickIndicatorEnd = SendScintilla(SCI_WORDENDPOSITION, pos, true);

    fillIndicatorRangeWithPosition(m_clickIndicatorStart, m_clickIndicatorEnd, m_clickIndicatorIdentifier);
}

/**
 * Clears the current underline (clickable word).
 */
void SingleDocument::removeUndlineFromWordWhichCanBeClicked(void)
{
    if(m_clickIndicatorStart != -1)
    {
        clearIndicatorRangeWithPosition(m_clickIndicatorStart, m_clickIndicatorEnd, m_clickIndicatorIdentifier);
        m_clickIndicatorStart = -1;
        m_clickIndicatorEnd = -1;
    }
}

/**
 * Returns the last modified time stamp.
 */
QDateTime SingleDocument::getLastModified(void)
{
    return m_fileLastModified;
}

/**
 * Updates the last modified time stamp.
 */
void SingleDocument::updateLastModified(void)
{
    QFileInfo fileInfo(m_documentName);
    m_fileLastModified = fileInfo.lastModified();
}

/**
 * Sets the font of the line number margin.
 * @param pointSize
 *      The font size.
 */
void SingleDocument::setLineNumberMarginFont(QFont font)
{
    setMarginsFont(font);
    setMarginWidth(0, QString("00%1").arg(lines()));
}

/**
 * Sets the document name/path.
 * @param name
 *      The document name.
 */
void SingleDocument::setDocumentName(QString name, QFont font)
{
    m_documentName = name;
    initLexer(name);
    lexer()->setFont(font, -1);
    setLineNumberMarginFont(font);
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
        setAutoCompletionCaseSensitivity(false);
        setAutoCompletionThreshold(3);
        setAutoIndent(true);
        setIndentationWidth(4);
        setTabWidth(4);
        setMarginLineNumbers(0,true);
        setMarginType(0, QsciScintilla::NumberMargin);
        setFolding(QsciScintilla::CircledTreeFoldStyle);
        setMarginsForegroundColor(QColor(128, 128, 128));

        dynamic_cast<QsciLexerJavaScript*>(lexer())->setFoldComments(true);
        dynamic_cast<QsciLexerJavaScript*>(lexer())->setFoldCompact(false);
       }


}


/**
 * Initializes the autocompletion.
 * @param additionalElements
 *      Additional elements for the autocompletion api.
 */
void SingleDocument::initAutoCompletion(QMap<QString, QStringList>& autoCompletionEntries,
                                        QMap<QString, QStringList>& autoCompletionApiFiles)
{


    //Initialize the api.
    if(lexer() && lexer()->apis())
    {
        QsciAPIs* apis = static_cast<QsciAPIs*>(lexer()->apis());
        apis->clear();
        QMap<QString, QStringList>::iterator i;
        for (i = autoCompletionEntries.begin(); i != autoCompletionEntries.end(); ++i)
        {
            for(auto el : i.value())
            {
                apis->add(el);
            }
        }

        for (i = autoCompletionApiFiles.begin(); i != autoCompletionApiFiles.end(); ++i)
        {
            for(auto el : i.value())
            {
                apis->add(el);
            }
        }
        apis->prepare();

    }

}
