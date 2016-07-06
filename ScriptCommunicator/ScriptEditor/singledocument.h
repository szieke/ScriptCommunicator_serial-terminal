#ifndef SINGLEDOCUMENT_H
#define SINGLEDOCUMENT_H

#include <QObject>
#include <Qsci/qsciscintilla.h>

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
    void initAutoCompletion(QStringList additionalElements);

private:
    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///The name of the document.
    QString m_documentName;
};

#endif // SINGLEDOCUMENT_H
