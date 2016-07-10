#ifndef SINGLEDOCUMENT_H
#define SINGLEDOCUMENT_H

#include <QObject>
#include <Qsci/qsciscintilla.h>
#include <QDomDocument>

class MainWindow;

///This class holds a single document.
class SingleDocument : public QsciScintilla
{
public:
    SingleDocument(MainWindow* mainWindow, QWidget *parent = 0);

    ///Initializes the lexer.
    void initLexer(QString script);

    ///Sets the document name/path.
    void setDocumentName(QString name);

    ///Returns the document name/path.
    QString getDocumentName(void){return m_documentName;}

    ///Initializes the autocompletion.
    void initAutoCompletion(QStringList additionalElements, QString currentText);

private:

    ///Checks if in the current document user interface files are loaded.
    ///If user interface are loaded then they will be parsed and added to the auto-completion
    ///list (g_autoCompletionEntries).
    void checkDocumentForUiFiles(QString currentText);

    void checkDocumentForDynamicObjects(QString currentText);


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
