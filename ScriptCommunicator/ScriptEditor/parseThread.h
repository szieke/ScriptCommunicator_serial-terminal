#ifndef PARSETHREAD_H
#define PARSETHREAD_H

#include <QObject>
#include <QThread>
#include <QMap>
#include <QVector>
#include <QStringList>
#include <QDomDocument>

///Object created by ScriptTable::InsertWidget.
typedef struct
{
    int row;
    int column;
    QString className;
}TableWidgetSubObject;

class ParseThread : public QThread
{
    Q_OBJECT
public:
    explicit ParseThread(QObject *parent = 0);

protected:
    ///The thread main function.
    void run();

signals:

    ///Is emitted if the parsing is finished.
    ///Note: autoCompletionApiFiles contains the auto-completion entries for all parsed files.
    void parsingFinishedSignal(QMap<QString, QStringList> autoCompletionEntries, QMap<QString, QStringList> autoCompletionApiFiles);

public slots:

    ///Parses the current text. Emits parsingFinishedSignal if the parsing is finished.
    void parseSlot(QString currentText, QString activeDocument);

private:

    ///Searches objects which are returned by a ScriptTableWidget.
    void searchSingleTableSubWidgets(QString objectName, QVector<TableWidgetSubObject> subObjects, QStringList& lines);

    ///Searches all ScriptTableWidget::insertWidgets calls of a specific ScriptTableWidget object.
    void parseTableWidetInsert(const QString objectName, QStringList lines);

    ///Searches for functions which habe all ScriptWidgets in common.
    void searchForScriptWidgetCommonFunctions(const QString& objectName, QStringList& lines);

    ///Searches a single object type.
    void searchSingleType(QString className, QString searchString, QStringList& lines, bool isArray=false,
                                        bool withOutDotsAndBracked= false, bool replaceExistingEntry=false, bool matchExact = false);

    ///Searches all dynamically created objects created by custom objects (like ScriptTimer).
    void checkDocumentForCustomDynamicObjects(QStringList& lines, QStringList &linesWithBrackets , QString &currentText, int passNumber);

    ///Searches all dynamically created objects created by standard objects (like String).
    void checkDocumentForStandardDynamicObjects(QStringList& lines, QStringList &linesWithBrackets, int passNumber);

    ///Checks if in the current document user interface files are loaded.
    void checkDocumentForUiFiles(QString& currentText, QString &activeDocument);

    ///Adds on obect to the auto-completion list./
    void addObjectToAutoCompletionList(QString& objectName, QString& className, bool isGuiElement, bool isArray=false, bool replaceExistingEntry=false);

    ///Parses a widget list from a user interface file (auto-completion).
    void parseWidgetList(QDomElement& docElem, bool parseActions);

    ///Parses an user interface file (auto-completion).
    void parseUiFile(QString uiFileName);

    ///Removes all unnecessary characters (e.g. comments).
    void removeAllUnnecessaryCharacters(QString& currentText);

    ///Removes all square brackets and all between them.
    void removeAllBetweenSquareBrackets(QString& currentText);

    ///Contains the auto-completion entries for all parsed api files.
    QMap<QString, QStringList> m_autoCompletionApiFiles;

    ///Contains all auto-completion entries (all but for the parsed api files).
    QMap<QString, QStringList> m_autoCompletionEntries;

    ///True if an entry hass been added to m_autoCompletionApiFiles.
    bool m_objectAddedToCompletionList;

    ///Contains all Objects which can create other objects.
    QMap<QString, QString> m_creatorObjects;

    ///Contains all String Objects.
    QVector<QString> m_stringList;

    ///Contains all Objects with a unknown type.
    QVector<QString> m_unknownTypeObjects;

    ///Contains all Array Objects.
    QVector<QString> m_arrayList;

    ///Contains all ScriptTableEidget Objects.
    QVector<QString> m_tableWidgets;

    ///Contains all objects created by ScriptTable::InsertWidget.
    QMap<QString, QVector<TableWidgetSubObject>> m_tableWidgetObjects;

    ///Contains all found ui files.
    QStringList m_foundUiFiles;

    ///Contains all parsed ui files.
    QStringList m_parsedFiles;
};

#endif // PARSETHREAD_H
