#ifndef SINGLEDOCUMENT_H
#define SINGLEDOCUMENT_H

#include <QObject>
#include <Qsci/qsciscintilla.h>
#include <QDomDocument>

//A parsed entry.
typedef struct
{
    int line;
    int column;
    QString name;
    bool isFunction;
    QStringList params;
    int tabIndex;
}ParsedEntry;

class MainWindow;


///This class holds a single document.
class SingleDocument : public QsciScintilla
{
public:
    SingleDocument(MainWindow* mainWindow, QWidget *parent = 0);

    ///Initializes the lexer.
    void initLexer(QString script);

    ///Sets the document name/path.
    void setDocumentName(QString name, QFont font);

    ///Returns the document name/path.
    QString getDocumentName(void){return m_documentName;}

    ///Initializes the autocompletion.
    void initAutoCompletion(QStringList &additionalElements, QMap<QString, QStringList> &autoCompletionEntries, QMap<QString, QStringList>& autoCompletionApiFiles);

private:

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
};

#endif // SINGLEDOCUMENT_H
