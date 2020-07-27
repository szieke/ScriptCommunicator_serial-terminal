#ifndef SINGLEDOCUMENT_H
#define SINGLEDOCUMENT_H

#include <QObject>
#include <Qsci/qsciscintilla.h>
#include <QDomDocument>
#include <QDateTime>
#include "parseThread.h"



//A parsed ui object entry.
typedef struct
{
    QString objectName;
    QString uiFile;
}ParsedUiObject;

class MainWindow;


///This class holds a single document.
class SingleDocument : public QsciScintilla
{
public:
    SingleDocument(MainWindow* mainWindow, bool useDarkStyle, QWidget *parent = 0);

    ///Initializes the lexer.
    void initLexer(QString script);

    ///Sets the background color.
    void setUpBackgroundColor(void);

    ///Updates the last modified time stamp.
    void updateLastModified(void);

    ///Underlines a word which can be clicked (funktion or variable in the outline window).
    void underlineWordWhichCanBeClicked(int pos);

    ///Clears the current underline (clickable word).
    void removeUndlineFromWordWhichCanBeClicked(void);

    ///Returns the last modified time stamp.
    QDateTime getLastModified(void);

    ///Sets the document name/path.
    void setDocumentName(QString name, QFont font);

    ///Sets the font of the line number margin.
    void setLineNumberMarginFont(QFont font);

    ///Returns the document name/path.
    QString getDocumentName(void){return m_documentName;}

    ///Initializes the autocompletion.
    void initAutoCompletion(QMap<QString, QStringList> &autoCompletionEntries, QMap<QString, QStringList>& autoCompletionApiFiles);

    ///Sets m_fileMustBeParsed.
    void setFileMustBeParsed(bool fileMustBeParsed){m_fileMustBeParsed = fileMustBeParsed;}

    ///Returns m_fileMustBeParsed.
    bool getFileMustBeParsed(void){return m_fileMustBeParsed;}

    ///Go to line.
    void goToLine(int line)
    {
        SendScintilla(SCI_GOTOLINE, line);
    }

    ///Clears the vector which contains all function.
    void clearAllFunctions(void){m_functions.clear();}

    ///Adds a function the current document.
    void addFunction(ParsedEntry& function);

    ///Returns the current context string.
    QString getContextString(int line);

    ///Sets the style to 'dark style' if useDarkStyle is true otherwise the default style is used.
    void setUseDarkStyle(bool useDarkStyle);

protected:

    ///Handle mouse moves
   void mouseMoveEventChild(QMouseEvent *event);

   ///Handle key presses
   void keyPressEventChild(QKeyEvent *event);

   void keyReleaseEvent(QKeyEvent *event);


private:

   void setTextColors(QColor defaultColor);

    ///Checks if in the current document user interface files are loaded.
    ///If user interface are loaded then they will be parsed and added to the auto-completion
    ///list (g_autoCompletionEntries).
    void checkDocumentForUiFiles(QString currentText);

    ///Searches all dynamically created objects created by custom objects (like ScriptTimer).
    void checkDocumentForCustomDynamicObjects(QStringList &lines, QStringList& linesWithBrackets, QString& currentText, int passNumber);

    ///Searches all dynamically created objects created by standard objects (like String).
    void checkDocumentForStandardDynamicObjects(QStringList &lines, QStringList& linesWithBrackets, int passNumber);

    ///Parses an user interface file (auto-completion).
    void parseUiFile(QString uiFileName);

    ///Parse a widget list from a user interface file (auto-completion).
    void parseWidgetList(QDomElement& docElem, bool parseActions);

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///The name of the document.
    QString m_documentName;

    ///Time at which the corresponding file has been modified-
    QDateTime m_fileLastModified;

    ///True if the file must be parst (file content changed).
    bool m_fileMustBeParsed;

    ///The start of the current click indicator (-1 is invalid) .
    int m_clickIndicatorStart;

    ///The start of the current click indicator (-1 is invalid) .
    int m_clickIndicatorEnd;

    ///The id of the indicator which is used to underline clickable words.
    int m_clickIndicatorIdentifier;

    ///All function which belongs to the current document.
    QVector<ParsedEntry> m_functions;

    ///True if the dark style shall be used.
    bool m_useDarkStyle;
};

#endif // SINGLEDOCUMENT_H
