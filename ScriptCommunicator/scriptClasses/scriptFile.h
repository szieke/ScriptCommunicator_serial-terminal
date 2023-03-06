#ifndef SCRIPTFILE_H
#define SCRIPTFILE_H

#include <QObject>
#include <QVector>
#include <QMessageBox>
#include <QJSEngine>
#include <QProgressBar>
#include <QFileInfo>
#include "scriptObject.h"

class ScriptWindow;


class ScriptFile : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptFile(QObject *parent, QString scriptFileName, bool isWorkerScript);

    ///Connects all signals.
    void intSignals(ScriptWindow* scriptWindow, bool runsInDebugger, bool useBlockingSignals=true);

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void){return MainWindow::parseApiFile("scriptFile.api");}

    ///Reads a text file and returns the content.
    Q_INVOKABLE QString readFile(QString path, bool isRelativePath=true, quint64 startPosition=0, qint64 numberOfBytes=-1);

    ///Reads a binary file and returns the content.
    Q_INVOKABLE QVector<unsigned char> readBinaryFile(QString path, bool isRelativePath=true, quint64 startPosition=0, qint64 numberOfBytes=-1);

    ///Returns the size of a file.
    Q_INVOKABLE qint64 getFileSize(QString path, bool isRelativePath=true);

    ///Checks if a file exists.
    Q_INVOKABLE bool checkFileExists(QString path, bool isRelativePath=true);

    ///Checks if a directory exists.
    Q_INVOKABLE bool checkDirectoryExists(QString directory, bool isRelativePath=true);

    ///Creates a directory.
    Q_INVOKABLE bool createDirectory(QString directory, bool isRelativePath=true);

    ///Renames a directory.
    Q_INVOKABLE bool renameDirectory(QString directory, QString newName);

    ///Renames a file.
    Q_INVOKABLE bool renameFile(QString path, QString newName);

    ///Deletes a file.
    Q_INVOKABLE bool deleteFile(QString path, bool isRelativePath=true);

    ///Deletes a directory (must be empty).
    Q_INVOKABLE bool deleteDirectory(QString directory, bool isRelativePath=true);

    ///Removes the directory, including all its contents.
    ///If a file or directory cannot be removed, deleteDirectoryRecursively() keeps going and attempts
    ///to delete as many files and sub-directories as possible, then returns false.
    ///If the directory was already removed, the method returns true (expected result already reached).
    Q_INVOKABLE bool deleteDirectoryRecursively(QString directory, bool isRelativePath=true);

    ///Reads the content of a directory and his sub directories.
    Q_INVOKABLE QStringList readDirectory(QString directory, bool isRelativePath=true, bool recursive=true, bool returnFiles=true, bool returnDirectories=true);

    ///Writes a text file (if replaceFile is true, the existing file is overwritten, else the content is appended).
    Q_INVOKABLE bool writeFile(QString path, bool isRelativePath, QString content, bool replaceFile, qint64 startPosition=-1);

    ///Writes a binary file (if replaceFile is true, the existing file is overwritten, else the content is appended).
    Q_INVOKABLE bool writeBinaryFile(QString path, bool isRelativePath, QVector<unsigned char> content, bool replaceFile, qint64 startPosition=-1);

    ///Converts a relative path into an absolute path.
    Q_INVOKABLE QString createAbsolutePath(QString fileName);

    ///Returns the folder in which the script resides.
    Q_INVOKABLE QString getScriptFolder(void)
    {
        QFileInfo fi(m_scriptFileName);
        return fi.absolutePath();
    }

    ///Extracts a zip file.
    Q_INVOKABLE static bool extractZipFile(const QString& fileName, const QString& destinationDirectory);

    ///Zips a directory.
    Q_INVOKABLE static bool zipDirectory(const QString& fileName, const QString sourceDirName, QProgressBar *progress=0, const QString& comment=QString(""));

    ///Adds files to a zip file.
    Q_INVOKABLE bool zipFiles(QString fileName, QVariantList fileList, QString comment="");

    ///Adds files to a zip file.
    static bool zipFiles(const QString& fileName, const QList<QStringList> fileList,
                             QProgressBar* progress=0, const QString& comment=QString(""));

    ///Shows a script exception (message box) to the user.
    void showExceptionInMessageBox(QJSValue exception, QString scriptPath, QJSEngine* scriptEngine, QWidget *parent,
                                   ScriptWindow* scriptWindow);

    ///Loads/includes one script (QtScript has no built in include mechanism).
    bool loadScript(QString scriptPath, bool isRelativePath, QJSEngine* scriptEngine, QWidget* parent, ScriptWindow* scriptWindow, bool checkForUnsavedData, bool *scriptShallBeStopped);

    ///Sets the script file name (path).
    void setScriptFileName(QString scriptFileName){m_scriptFileName = scriptFileName;}


signals:
    ///Is connected with MainWindow::showMessageBoxSlot (shows a message box).
    ///This function must not be used from script.
    void showMessageBoxSignal(QMessageBox::Icon icon, QString title, QString text, QMessageBox::StandardButtons buttons, QWidget *parent);

    ///Is connected with MainWindow::showYesNoDialogSlot (shows a yes/no dialog).
    ///This function must not be used from script.
    void showYesNoDialogSignal(QMessageBox::Icon icon, QString title, QString text,  QWidget *parent, bool* yesButtonPressed);

    ///Disables all mouse events for all windows.
    ///This function must not be used from script.
    void disableMouseEventsSignal(void);

    ///Enables all mouse events for all windows.
    ///This function must not be used from script.
    void enableMouseEventsSignal(void);

    ///Appends text to the console.
    void appendTextToConsoleSignal(QString text, bool newLine,bool bringToForeground);

private:
    ///The path of the script which is executed by the thread.
    QString m_scriptFileName;

    ///The max. number of  bytes which are read at once from one file.
    static const quint32 MAX_READ_FROM_FILE = 100000000;

    ///True if this objects belongs to a worker script.
    bool m_isWorkerScript;
};

#endif // SCRIPTFILE_H
