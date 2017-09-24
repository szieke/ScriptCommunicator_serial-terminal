#include "scriptFile.h"
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include "scriptwindow.h"
#include <quazip.h>
#include <quazipdir.h>
#include <quazipfile.h>

ScriptFile::ScriptFile(QObject *parent, QString scriptFileName, bool isWorkerScript) : QObject(parent), m_scriptFileName(scriptFileName),
    m_isWorkerScript(isWorkerScript)
{
}


/**
 * Connects all signals.
 * @param scriptWindow
 *      Pointer to the script window.
 */
void ScriptFile::intSignals(ScriptWindow *scriptWindow, bool runsInDebugger, bool useBlockingSignals)
{
    Qt::ConnectionType directConnectionType = runsInDebugger ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

    connect(this, SIGNAL(showMessageBoxSignal(QMessageBox::Icon, QString, QString, QMessageBox::StandardButtons, QWidget* )),
            scriptWindow->getMainWindow(), SLOT(showMessageBoxSlot(QMessageBox::Icon, QString, QString, QMessageBox::StandardButtons, QWidget*)),
            useBlockingSignals ? directConnectionType : Qt::QueuedConnection);

    connect(this, SIGNAL(showYesNoDialogSignal(QMessageBox::Icon,QString,QString,QWidget*,bool*)),
            scriptWindow->getMainWindow(), SLOT(showYesNoDialogSlot(QMessageBox::Icon,QString,QString,QWidget*,bool*)),
            directConnectionType);

    connect(this, SIGNAL(disableMouseEventsSignal()),
            scriptWindow->getMainWindow(), SLOT(disableMouseEventsSlot()), useBlockingSignals ? directConnectionType : Qt::QueuedConnection);

    connect(this, SIGNAL(enableMouseEventsSignal()),
            scriptWindow->getMainWindow(), SLOT(enableMouseEventsSlot()), useBlockingSignals ? directConnectionType : Qt::QueuedConnection);

    connect(this, SIGNAL(appendTextToConsoleSignal(QString,bool,bool)),
            scriptWindow, SLOT(appendTextToConsoleSlot(QString,bool,bool)), Qt::QueuedConnection);
}

/**
 * Reads a file and return the content.
 * @param path
 *      The file path.
 * @param isRelativePath
 *      True if the file path is relative to the executable.
 * @param startPosition
 *      Start position (the file is read from this position).
 * @param numberOfBytes
 *      The number of bytes which shall be read. If numberOfBytes is < 0 then all bytes from
 *      startPosition are read.
 * @return
 *      The file content.
 */
QString ScriptFile::readFile(QString path, bool isRelativePath, quint64 startPosition, qint64 numberOfBytes)
{
    QString fileContent;

    path = isRelativePath ? createAbsolutePath(path) : path;

    QFile file(path);
    if(file.open(QIODevice::ReadOnly) && file.seek(startPosition) && file.size() > (qint64)startPosition)
    {
        QTextStream inStream(&file);
        fileContent = inStream.read((numberOfBytes < 0) ? file.size() : numberOfBytes);
        file.close();
    }
    return fileContent;
}

/**
* Reads a binary file and returns the content.
* @param path
*      The file path.
* @param isRelativePath
*      True if the file path is relative to the executable.
* @param startPosition
*      Start position (the file is read from this position).
* @param numberOfBytes
*      The number of bytes which shall be read. If numberOfBytes is < 0 then all bytes from
*      startPosition are read.
* @return
*      The file content.
*/
QVector<unsigned char> ScriptFile::readBinaryFile(QString path, bool isRelativePath, quint64 startPosition, qint64 numberOfBytes)
{
    QVector<unsigned char> fileContent;

    path = isRelativePath ? createAbsolutePath(path) : path;

    QFile file(path);
    if(file.open(QIODevice::ReadOnly) && file.seek(startPosition) && file.size() > (qint64)startPosition)
    {

        QDataStream inStream(&file);
        char buffer[4096];
        int currentReadBytes = 0;
        quint32 bytesToRead = (numberOfBytes > 0) ? numberOfBytes : file.size();
        quint64 numberOfReadBytes = 0;

        do
        {
            quint32 currentBytesToRead = ((bytesToRead - numberOfReadBytes) < sizeof(buffer)) ? (bytesToRead - numberOfReadBytes) : sizeof(buffer);
            currentReadBytes = inStream.readRawData(buffer, currentBytesToRead);
            numberOfReadBytes += currentReadBytes;

            if(currentReadBytes > 0)
            {
                for(qint32 i = 0; i < currentReadBytes; i++)
                {
                    fileContent.push_back(buffer[i]);
                }

            }
        }while((numberOfReadBytes < bytesToRead) && (currentReadBytes != 0));

        file.close();

    }
    return fileContent;
}


/**
* Returns the size of a file.
*
* * @param path
*      The file path.
* @param isRelativePath
*      True of path is a relative path.
* @return
*      The file size if the file exists. -1 if the file doesn't exists.
*
*/
qint64 ScriptFile::getFileSize(QString path, bool isRelativePath)
{
    qint64 size = -1;
    path = isRelativePath ? createAbsolutePath(path) : path;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
    {
        size = file.size();
        file.close();
    }

    return size;
}

/**
 * Checks if a file exists.
 * @param path
 *      The script path.
 * @param isRelativePath
 *      True of path is a relative path.
 * @return
 *      True on success.
 */
bool ScriptFile::checkFileExists(QString path, bool isRelativePath)
{
    path = isRelativePath ? createAbsolutePath(path) : path;
    QFileInfo fi(path);
    return fi.exists();
}



/**
 * Checks if a directory exists.
 * @param directory
 *      The directory.
 * @param isRelativePath
 *      True of path is a relative path.
 * @return
 *      True on success.
 */
bool ScriptFile::checkDirectoryExists(QString directory, bool isRelativePath)
{
    directory = isRelativePath ? createAbsolutePath(directory) : directory;
    QDir dir(directory);
    return dir.exists();
}

/**
 * Creates a directory.
 * @param directory
 *      The directory.
 * @param isRelativePath
 *      True of path is a relative path.
 * @return
 *      True on success.
 */
bool ScriptFile::createDirectory(QString directory, bool isRelativePath)
{
    directory = isRelativePath ? createAbsolutePath(directory) : directory;
    return QDir().mkdir(directory);
}

/**
 * Renames a directory.
 * @param directory
 *      The directory.
 * @param newName
 *      The new name.
 * @return
 *      True on success.
 */
bool ScriptFile::renameDirectory(QString directory, QString newName)
{
    return QDir().rename(directory, newName);
}

/**
 * Converts a relative path into an absolute path.
 * @param fileName
 *      The relative path.
 * @return
 *      The created absolute path.
 */
QString ScriptFile::createAbsolutePath(QString fileName)
{
    QFileInfo fi(m_scriptFileName);
    fileName = fi.absolutePath() + "/" + fileName;

    return fileName;
}

/**
 * Shows a script exception (message box) to the user.
 * @param exception
 *      The script exception.
 * @param scriptPath
 *      The script path.
 * @param parent
 *      The parent window.
 */
void ScriptFile::showExceptionInMessageBox(QScriptValue exception, QString scriptPath, QScriptEngine* scriptEngine, QWidget *parent, ScriptWindow *scriptWindow)
{
    QString textToShow;
    QString functionsAndProperies;


    QString exceptionString = exception.toString();
    QStringList list = exceptionString.split("'");
    bool errorOcured = true;

    if(list.length() >= 2)
    {
        list = list[1].split(".");
        QScriptValue object = scriptEngine->evaluate(list[0]);
        if(!object.isError())
        {
            ScriptThread::getAllObjectPropertiesAndFunctionsInternal(object, 0, &functionsAndProperies);

            if(!functionsAndProperies.isEmpty())
            {
                functionsAndProperies = "\n\n\nFunctions and properies of " + list[0] + ":\n" + functionsAndProperies;

                textToShow = exceptionString + "\n\nNote: All functions" +
                                               " and properties of " + list[0] + " are shown in the script window console.";
                errorOcured = false;
            }
        }
        else if(m_isWorkerScript && list[0].endsWith(": cust"))
        {
            errorOcured = false;
            textToShow = exceptionString + "\n\nNote: The cust object is only available in sequence scripts (send window).";
        }
        else if(!m_isWorkerScript && list[0].endsWith(": scriptThread"))
        {
            errorOcured = false;
            textToShow = exceptionString + "\n\nNote: The scriptThread object is only available in worker scripts (script window).";
        }
    }


    if(errorOcured)
    {
        textToShow = exceptionString;
    }

    emit disableMouseEventsSignal();
    emit enableMouseEventsSignal();

    if((scriptWindow != 0) && !functionsAndProperies.isEmpty())
    {
        emit appendTextToConsoleSignal(functionsAndProperies, true, true);
        parent = scriptWindow;
    }

    emit showMessageBoxSignal(QMessageBox::Critical, scriptPath,
                              QString::fromLatin1("%0:%1: %2")
                              .arg(scriptPath)
                              .arg(exception.property("lineNumber").toInt32())
                              .arg(textToShow), QMessageBox::Ok, parent);

    if((scriptWindow == 0) && !functionsAndProperies.isEmpty())
    {
        emit appendTextToConsoleSignal(functionsAndProperies, true, true);
    }
}

/**
 * Loads/includes one script (QtScript has no built in include mechanism).
 * @param scriptPath
 *      The script path.
 * @param scriptType
 *      The script type.
 * @param isRelativePath
 *      True of scriptPath is a relative path.
 * @param scriptEngine
 *      The script engine
 * @param checkForUnsavedData
 *      True if this function shall check for unsaved data.
 * @param scriptShallBeStopped
 *      True (out) if the script shall be stopped (user input). May be NULL.
 * @return
 *      True on success.
 */
bool ScriptFile::loadScript(QString scriptPath, bool isRelativePath, QScriptEngine* scriptEngine, QWidget *parent, ScriptWindow *scriptWindow,
                            bool checkForUnsavedData, bool* scriptShallBeStopped)
{
    bool hasSucceded = true;

    scriptPath = isRelativePath ? createAbsolutePath(scriptPath) : scriptPath;

    if(checkForUnsavedData)
    {
        QString unsavedInfoFile = ScriptWindow::getUnsavedInfoFileName(scriptPath);
        if(QFileInfo().exists(unsavedInfoFile))
        {//The file has unsaved changes.

            bool yesPressed = false;
            emit showYesNoDialogSignal(QMessageBox::Warning,"Warning", scriptPath + " is opened by an instance of ScriptEditor and contains unsaved changes. Execute anyway?",
                                       parent, &yesPressed);
            if(!yesPressed)
            {
                if(scriptShallBeStopped)
                {
                    *scriptShallBeStopped = true;
                }
                return false;
            }
        }
    }

    QFile scriptFile(scriptPath);
    if(!scriptFile.open(QIODevice::ReadOnly))
    {
        emit disableMouseEventsSignal();
        emit enableMouseEventsSignal();
        emit showMessageBoxSignal(QMessageBox::Critical, "error", "could not open script file: " + scriptPath, QMessageBox::Ok, parent);
        hasSucceded = false;

    }
    else
    {
        //set ScriptContext
        QScriptContext* context = scriptEngine->currentContext();
        QScriptContext* parentContext = context->parentContext();
        if(parentContext!=0)
        {
            context->setActivationObject(parentContext->activationObject());
            context->setThisObject(parentContext->thisObject());
        }

        QScriptValue result = scriptEngine->evaluate(scriptFile.readAll(), scriptPath);
        scriptFile.close();

        // If any Error, Display line number and error in a message box.
        if (result.isError())
        {
            QString str;
            QScriptValueIterator it(result);
            while (it.hasNext())
            {
                it.next();
                str += it.name() + "\n";
            }
            showExceptionInMessageBox(result, scriptPath, scriptEngine, parent, scriptWindow);

            hasSucceded = false;
        }

    }
    return hasSucceded;
}
/**
 * Rename a file exists.
 * @param path
 *      The file path.
 * @param newName
 *      The new name..
 * @return
 *      True on success.
 */
bool ScriptFile::renameFile(QString path, QString newName)
{
    return QFile::rename(path, newName);
}

/**
 * Deletes a file exists.
 * @param path
 *      The file path.
 * @param isRelativePath
 *      True if path is a relative path.
 * @return
 *      True on success.
 */
bool ScriptFile::deleteFile(QString path, bool isRelativePath)
{
    path = isRelativePath ? createAbsolutePath(path) : path;
    return QFile::remove(path);
}

/**
 * Deletes a directory (must be empty).
 * @param directory
 *      The directory.
 * @param isRelativePath
 *      True of directory is a relative path.
 * @return
 *      True on success.
 */
bool ScriptFile::deleteDirectory(QString directory, bool isRelativePath)
{
    directory = isRelativePath ? createAbsolutePath(directory) : directory;
    QDir dir;
    return dir.rmdir(directory);
}

/**
 * Removes the directory, including all its contents.
 * If a file or directory cannot be removed, deleteDirectoryRecursively() keeps going and attempts
 * to delete as many files and sub-directories as possible, then returns false.
 * If the directory was already removed, the method returns true (expected result already reached).
 * @param directory
 *      The directory.
 * @param isRelativePath
 *      True of directory is a relative path.
 * @return
 *      True on success.
 */
bool ScriptFile::deleteDirectoryRecursively(QString directory, bool isRelativePath)
{
    directory = isRelativePath ? createAbsolutePath(directory) : directory;
    QDir dir(directory);
    return dir.removeRecursively();
}

/**
 * Reads the content of a directory and his sub directories.
 * @param directory
 *      The directory.
 * @param isRelativePath
 *      True of directory is a relative path.
 * @param recursive
 *      If true, the content of the sub directories will be return too.
 * @param returnFiles
 *      If true, all files will be return.
 * @param returnDirectories
 *      If true, all directories will be return.
 * @return
 */
QStringList ScriptFile::readDirectory(QString directory, bool isRelativePath, bool recursive, bool returnFiles, bool returnDirectories)
{
    QStringList list;
    directory = isRelativePath ? createAbsolutePath(directory) : directory;

    QDir::Filters filter = 0;

    if(returnDirectories){filter |= QDir::Filter::Dirs;}
    if(returnFiles){filter |= QDir::Filter::Files;}


    QDirIterator iter(directory, filter, recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);


    if((iter.fileName() != ".") && (iter.fileName() != "..") && iter.fileName() != "")
    {
        list << iter.filePath();

    }

    while (iter.hasNext())
    {
        iter.next();
        if((iter.fileName() != ".") && (iter.fileName() != "..") && iter.fileName() != "")
        {
            list << iter.filePath();

        }

    }
    return list;
}



/**
 * Writes a text file.
 * @param path
 *      The file path.
 * @param isRelativePath
 *      True if the file path is relative to the executable.
 * @param content
 *      The content to write.
 * @param replaceFile
 *      If replaceFile is true, the existing file is overwritten, else the content is appended.
 * @parm startPosition
 *      If replaceFile is false then this is the start position at which the data will be written. For appending the
 *      data at the end of the file startPosition must be < 0.
 * @return
 *      True for success.
 */
bool ScriptFile::writeFile(QString path, bool isRelativePath, QString content, bool replaceFile, qint64 startPosition)
{
    bool hasSucceeded = true;

    path = isRelativePath ? createAbsolutePath(path) : path;

    QFile file(path);
    QIODevice::OpenMode mode = QIODevice::ReadWrite;

    if(replaceFile)
    {
        if(checkFileExists(path, false))
        {
            hasSucceeded = QFile::remove(path);
        }
        mode = QIODevice::ReadWrite;
        startPosition = 0;
    }
    else
    {
        mode = (startPosition < 0) ? QIODevice::Append :  QIODevice::ReadWrite;
    }

    if(hasSucceeded)
    {
        startPosition = (startPosition < 0) ? file.size() : startPosition;
        if(!file.open(mode) || !file.seek(startPosition))
        {
            hasSucceeded = false;
        }
        else
        {
            QTextStream outStream(&file);
            outStream << content;
            outStream.flush();
            file.close();
            hasSucceeded = true;
        }
    }

    return hasSucceeded;
}


/**
 * Writes a binary file.
 * @param path
 *      The file path.
 * @param isRelativePath
 *      True if the file path is relative to the executable.
 * @param content
 *      The content to write.
 * @param replaceFile
 *      If replaceFile is true, the existing file is overwritten, else the content is appended.
 * @parm startPosition
 *      If replaceFile is false then this is the start position at which the data will be written. For appending the
 *      data at the end of the file startPosition must be < 0.
 * @return
 *      True for success.
 */
bool ScriptFile::writeBinaryFile(QString path, bool isRelativePath, QVector<unsigned char> content, bool replaceFile, qint64 startPosition)
{
    bool hasSucceeded = true;

    path = isRelativePath ? createAbsolutePath(path) : path;

    QFile file(path);
    QIODevice::OpenMode mode = QIODevice::ReadWrite;

    if(replaceFile)
    {
        if(checkFileExists(path, false))
        {
            hasSucceeded = QFile::remove(path);
        }
        mode = QIODevice::ReadWrite;
        startPosition = 0;
    }
    else
    {
        mode = (startPosition < 0) ? QIODevice::Append :  QIODevice::ReadWrite;
    }

    if(hasSucceeded)
    {
        startPosition = (startPosition < 0) ? file.size() : startPosition;
        if(!file.open(mode) || !file.seek(startPosition))
        {
            hasSucceeded = false;
        }
        else
        {

            if(content.size() == file.write((const char*)content.constData(), content.size()))
            {
                file.flush();
                file.close();
                hasSucceeded = true;
            }
        }
    }
    return hasSucceeded;
}



/**
 * Adds all files from the directory dir and its sub directories to list.
 * @param dir
 *      The directory.
 * @param list
 *      All found files.
 */
static void recurseAddDir(QDir dir, QStringList & list)
{

    QStringList qsl = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);

    foreach (QString file, qsl)
    {
        QFileInfo finfo(QString("%1/%2").arg(dir.path()).arg(file));

        if (finfo.isSymLink())
            return;

        if (finfo.isDir())
        {
            recurseAddDir(QDir(finfo.filePath()), list);
        }
        else
        {
            list << finfo.filePath();
        }
    }
}


/**
 * Zips a directory.
 * @param fileName
 *      The zip file file name.
 * @param sourceDirName
 *      The source directory.
 * @param comment
 *      The zip file comment.
 * @return
 *      True on success.
 */
bool ScriptFile::zipDirectory(const QString& fileName, const QString sourceDirName, QProgressBar *progress, const QString& comment)
{

    QDir dir(sourceDirName);
    if (!dir.exists())
    {
        return false;
    }


    QStringList tmpList;
    recurseAddDir(sourceDirName, tmpList);

    QList<QStringList> fileList;
    for(auto el : tmpList)
    {
        QStringList entry;
        entry << el;
        entry << el.remove(0, QDir(sourceDirName).absolutePath().length() + 1);
        fileList << entry;
    }

    return zipFiles(fileName, fileList, progress, comment);
}


/**
 * Adds files to a zip file.
 * @param fileName
 * The file name of the zip file.
 * @param fileList
 *      Contains all files. An entry consists of a string pair.
 *      The first entry of this pair is the source file name (including the absolute file path) and the second
 *      is the file name inside (including the relative path) the zip file.
 * @param comment
 *      The zip file comment.
 * @return
 *      True on success.
 */
bool ScriptFile::zipFiles(QString fileName, QVariantList fileList, QString comment)
{
    bool result = false;
    QList<QStringList> createdList;

    for(int i = 0; i < fileList.length(); i++)
    {
        createdList << fileList[i].toStringList();
    }
    result = zipFiles(fileName, createdList, 0, comment);
    return result;
}

/**
 * Adds files to a zip file.
 * @param fileName
 * The file name of the zip file.
 * @param fileList
 *      Contains all files. An entry consists of a string pair.
 *      The first entry of this pair is the source file name (including the absolute file path) and the second
 *      is the file name inside the zip file (including the relative path).
 * @param progress
 *      Pointer to a progress bar. The value of this progress bar
 *      is increased by one for every finished file in fileList.
 * @param comment
 *      The zip file comment.
 * @return
 *      True on success.
 */
bool ScriptFile::zipFiles(const QString& fileName, const QList<QStringList> fileList, QProgressBar *progress, const QString& comment)
{

    QFile inFile;
    QuaZip zip(fileName);
    zip.setFileNameCodec("IBM866");

    if(fileList.isEmpty()){return false;}

    if (!zip.open(QuaZip::mdCreate)){return false;}

    QuaZipFile zipFile(&zip);

    foreach (QStringList el, fileList)
    {
        if(el.length() != 2){/*Invalid entry.*/return false;}

        QFileInfo fileInfo(el[0]);
        if (!fileInfo.isFile()){continue;}

        inFile.setFileName(fileInfo.absoluteFilePath());
        if (!inFile.open(QIODevice::ReadOnly)){return false;}

        if (!zipFile.open(QIODevice::WriteOnly, QuaZipNewInfo(el[1], fileInfo.fileName()),
                          NULL, 0,Z_DEFLATED, Z_DEFAULT_COMPRESSION)){return false;}

        qint64 readData = 0;
        while(inFile.size() > readData)
        {
            QByteArray readDataArray = inFile.read(MAX_READ_FROM_FILE);
            readData += readDataArray.length();
            zipFile.write(readDataArray);
        }

        if (zipFile.getZipError() != UNZ_OK){return false;}

        zipFile.close();

        if (zipFile.getZipError() != UNZ_OK){return false;}

        inFile.close();

        if(progress != 0){progress->setValue(progress->value() + 1);}
    }

    if (!comment.isEmpty()){ zip.setComment(comment);}

    zip.close();

    if (zip.getZipError() != 0){return false;}

    return true;
}

/**
 * Extracts a zip file.
 * @param fileName
 *      The zip file name.
 * @param destinationDirectory
 *      The destination directory.
 * @return
 *      True on success.
 */
bool ScriptFile::extractZipFile(const QString& fileName, const QString& destinationDirectory)
{
    QuaZip zip(fileName);

    if (!zip.open(QuaZip::mdUnzip)){return false;}

    if(!QDir(destinationDirectory).exists())
    {
        if(!QDir().mkdir(destinationDirectory)){return false;}
    }

    zip.setFileNameCodec("IBM866");

    QuaZipFileInfo info;
    QuaZipFile zipFile(&zip);

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        if (!zip.getCurrentFileInfo(&info)){return false;}

        if (!zipFile.open(QIODevice::ReadOnly)){return false;}

        QString name = QString("%1/%2").arg(destinationDirectory).arg(zipFile.getActualFileName());

        if (zipFile.getZipError() != UNZ_OK){return false;}

        QFile out(name);
        QString currentFolder = QFileInfo(name).path();

        if(!QDir(currentFolder).exists())
        {
            if(!QDir().mkpath(currentFolder)){return false;}
        }

        if(!out.open(QIODevice::WriteOnly)){return false;}

        qint64 readData = 0;
        while(zipFile.size() > readData)
        {
            QByteArray readDataArray = zipFile.read(MAX_READ_FROM_FILE);
            readData += readDataArray.length();
            out.write(readDataArray);
        }

        out.close();

        if (zipFile.getZipError() != UNZ_OK){return false;}

        if (!zipFile.atEnd()){return false;}

        zipFile.close();

        if (zipFile.getZipError() != UNZ_OK){return false;}
    }

    zip.close();

    if (zip.getZipError() != UNZ_OK){return false;}

    return true;
}
