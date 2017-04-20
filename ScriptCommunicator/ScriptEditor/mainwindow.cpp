/***************************************************************************
**                                                                        **
**  ScriptEditor is a simple script editor for ScriptCommunicator scripts.**
**  Copyright (C) 2016 Stefan Zieker                                      **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Stefan Zieker                                        **
**  Website/Contact: http://sourceforge.net/projects/scriptcommunicator/  **
****************************************************************************/

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPoint>
#include <QSettings>
#include <QSize>
#include <QStatusBar>
#include <QTextStream>
#include <QToolBar>
#include "Qsci/qscilexerjavascript.h"
#include <QMimeData>
#include <QProcess>
#include <QMessageBox>
#include <QSplitter>
#include "version.h"
#include <QFontDialog>
#include "Qsci/qscistyle.h"


#include <Qsci/qsciscintilla.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_findDialog.h"
#include "esprima/esprima.h"


///Pointer to the main window.
MainWindow* g_mainWindow = 0;

///Return a main window pointer.
MainWindow* getMainWindow()
{
    return g_mainWindow;
}


/**
 * Constructor.
 * @param script
 *      The file which should be loaded.
 */
MainWindow::MainWindow(QStringList scripts) : ui(new Ui::MainWindow), m_parseTimer(), m_parseThread(0), m_parsingFinished(true),
    m_lockFiles(), m_unsavedInfoFiles(), m_checkForFileChangesTimer()
{
    ui->setupUi(this);

    g_mainWindow = this;

    connect(ui->documentsTabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChangedSlot(int)));
    connect(ui->documentsTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequestedSlot(int)));

    if(scripts.isEmpty())
    {
        addTab("", false);
    }
    else
    {
        for(auto el : scripts)
        {
            addTab(el, true);
        }
    }

    initActions();
    statusBar()->showMessage(tr("Ready"));

    setAcceptDrops(true);

    m_findDialog = new FindDialog(this);

    connect(m_findDialog->ui->findPushButton, SIGNAL(clicked()), this, SLOT(findButtonSlot()));
    connect(m_findDialog->ui->replacePushButton, SIGNAL(clicked()), this, SLOT(replaceButtonSlot()));
    connect(m_findDialog->ui->replaceAllPushButton, SIGNAL(clicked()), this, SLOT(replaceAllButtonSlot()));
    connect(&m_parseTimer, SIGNAL(timeout()), this, SLOT(parseTimeout()));
    connect(&m_checkForFileChangesTimer, SIGNAL(timeout()), this, SLOT(checkForFileChanges()));


    m_findDialog->ui->findWhatComboBox->setAutoCompletion(false);
    m_findDialog->ui->replaceComboBox->setAutoCompletion(false);

    connect(ui->outlineTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(functionListDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui->uiTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(uiViewDoubleClicked(QTreeWidgetItem*,int)));

    m_findShortcut = new QShortcut(QKeySequence(Qt::Key_F3),this);

    connect(m_findShortcut, SIGNAL(activated()), this, SLOT(findButtonSlot()));

    m_parseThread = new ParseThread();
    m_parseThread->moveToThread(m_parseThread);
    m_parseThread->start();

    qRegisterMetaType<QMap<QString,QStringList>>("QMap<QString,QStringList>");
    qRegisterMetaType<QMap<QString,QString>>("QMap<QString,QString>");
    qRegisterMetaType<QMap<QString,QVector<ParsedEntry>>>("QMap<QString,QVector<ParsedEntry>>");

    connect(this, SIGNAL(parseSignal(QMap<QString, QString>,QMap<QString, QString>,bool)), m_parseThread,
            SLOT(parseSlot(QMap<QString, QString>,QMap<QString, QString>,bool)), Qt::QueuedConnection);
    connect(m_parseThread, SIGNAL(parsingFinishedSignal(QMap<QString,QStringList>,QMap<QString,QStringList>, QMap<QString, QStringList>,QMap<QString, QVector<ParsedEntry>>,bool)),
            this, SLOT(parsingFinishedSlot(QMap<QString,QStringList>,QMap<QString,QStringList>, QMap<QString, QStringList>,QMap<QString, QVector<ParsedEntry>>,bool)), Qt::QueuedConnection);

     m_parseTimer.start(2000);
     m_checkForFileChangesTimer.start(2000);
}

MainWindow::~MainWindow()
{
    m_parseThread->exit();
    QThread::msleep(10);
    m_parseThread->deleteLater();
    delete ui;
}

/**
 * Is called if the parsing is finished.
 * Note: autoCompletionApiFiles contains the auto-completion entries for all parsed files.
 * @param autoCompletionEntries
 *      Contains all auto-completion entries (all but for the parsed api files).
 * @param autoCompletionApiFiles
 *      Contains the auto-completion entries for all parsed api files.
 */
void MainWindow::parsingFinishedSlot(QMap<QString, QStringList> autoCompletionEntries, QMap<QString, QStringList> autoCompletionApiFiles,
                                     QMap<QString, QStringList> parsedUiObjects, QMap<QString,QVector<ParsedEntry>> parsedEntries, bool doneParsing)
{
    if(doneParsing)
    {
        SingleDocument* currentEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        currentEditor->initAutoCompletion(m_allFunctions, autoCompletionEntries, autoCompletionApiFiles);

        insertAllUiObjectsInUiView(parsedUiObjects);

        if(!checkForErrorsInScripts())
        {
            insertAllFunctionAndVariablesInScriptView(parsedEntries);
        }

        for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
            textEditor->setFileMustBeParsed(false);
        }
    }

    m_parsingFinished = true;
}

/**
 * Is call by m_checkForFileChangesTimer and checks for changes in the loaded files.
 */
void MainWindow::checkForFileChanges(void)
{
    m_checkForFileChangesTimer.stop();

    for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());

        QFileInfo fileInfo(textEditor->getDocumentName());
        if(textEditor->getDocumentName() != "")
        {
            if(fileInfo.lastModified() != textEditor->getLastModified())
            {//Document was changed from elsewhere

                int ret = QMessageBox::question(this, tr("ScriptCommunicator script editor"), textEditor->getDocumentName() +
                                                " has been modified by another programm. Reload this file",
                                               QMessageBox::Yes | QMessageBox::Default,
                                               QMessageBox::No);

                if (ret == QMessageBox::Yes)
                {
                    QFile file(textEditor->getDocumentName());
                    if (file.open(QFile::ReadOnly))
                    {
                        QTextStream in(&file);
                        in.setCodec("UTF-8");
                        textEditor->setText(in.readAll());
                        file.close();

                        textEditor->setModified(false);
                        removeSavedInfoFile(textEditor->getDocumentName());


                        if(ui->documentsTabWidget->currentIndex() == i)
                        {
                            //Call the slot function manually.
                            tabIndexChangedSlot(i);
                        }
                        else
                        {
                            ui->documentsTabWidget->setCurrentIndex(i);
                        }

                        //Set the tab text,
                        ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), strippedName(textEditor->getDocumentName()));
                        statusBar()->showMessage(tr("File reloaded"), 10000);
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("ScriptCommunicator script editor"),
                                             tr("Cannot read file %1:\n%2")
                                             .arg(textEditor->getDocumentName())
                                             .arg(file.errorString()));
                    }
                }

                textEditor->updateLastModified();
            }
        }
    }

    m_checkForFileChangesTimer.start(2000);
}

/**
 * Is called if the parse timer times out.
 */
void MainWindow::parseTimeout(void)
{

    if(m_parsingFinished)
    {
        m_parseTimer.stop();

        QMap<QString, QString> loadedScripts;
        QMap<QString, QString> loadedUiFiles;
        bool fileMustBeParsed = false;

        //Note: The document from the current tab must be the first entry in loadedScripts (if variable with the same
        //name exists in differnt documents).
        SingleDocument* currentEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());

        if(currentEditor->getFileMustBeParsed())
        {
            fileMustBeParsed = true;
        }
        if(!currentEditor->getDocumentName().endsWith(".ui"))
        {
            loadedScripts[currentEditor->getDocumentName()] = currentEditor->text();
        }
        else
        {
            loadedUiFiles[currentEditor->getDocumentName()] = currentEditor->text();
        }

        //Get the text of all open documents.
        for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
            if(currentEditor != textEditor)
            {
                if(!textEditor->getDocumentName().endsWith(".ui"))
                {
                    loadedScripts[textEditor->getDocumentName()] = textEditor->text();
                }
                else
                {
                    loadedUiFiles[textEditor->getDocumentName()] = textEditor->text();
                }
            }

            if(textEditor->getFileMustBeParsed())
            {
                fileMustBeParsed = true;
            }
        }

        m_parsingFinished = false;
        emit parseSignal(loadedUiFiles, loadedScripts, fileMustBeParsed);

        m_parseTimer.start(2000);
    }
}

/**
 * Adds a tab.
 */
void MainWindow::addTab(QString script, bool setTabIndex)
{
    int index = -1;
    if(!checkIfDocumentAlreadyLoaded(script, index))
    {

        QWidget* newTab = new QWidget();
        ui->documentsTabWidget->addTab(newTab, strippedName(script));

        QVBoxLayout* vBoxlayout	= new QVBoxLayout();
        SingleDocument* textEditor = new SingleDocument(this, newTab);
        vBoxlayout->addWidget(textEditor);
        newTab->setLayout(vBoxlayout);
        ui->documentsTabWidget->setCurrentWidget(newTab);

        connect(textEditor, SIGNAL(copyAvailable(bool)), ui->actionCut, SLOT(setEnabled(bool)));
        connect(textEditor, SIGNAL(copyAvailable(bool)),  ui->actionCopy, SLOT(setEnabled(bool)));

        connect(textEditor, SIGNAL(zoomInSignal()), this, SLOT(zoomInSlot()));
        connect(textEditor, SIGNAL(zoomOutSignal()), this, SLOT(zoomOutSlot()));


        if(!script.isEmpty())
        {
            if(!loadFile(script))
            {
                ui->documentsTabWidget->removeTab(ui->documentsTabWidget->currentIndex());

                if(ui->documentsTabWidget->count() == 0)
                {
                    addTab("", false);
                }
            }
        }
        else
        {
            setCurrentFile("");
        }
    }
    else
    {
        if(setTabIndex)
        {
            ui->documentsTabWidget->setCurrentIndex(index);
        }
    }

}

/**
 * Is called if a tab shall be closed.
 * @param index
 *      The tab index.
 */
void MainWindow::tabCloseRequestedSlot(int index)
{
    if(maybeSave(index))
    {
        removeFileLock(index);

        ui->documentsTabWidget->removeTab(index);

        if(ui->documentsTabWidget->count() == 0)
        {
            addTab("", true);
        }
        m_parseTimer.start(200);
    }

}

/**
 * Returns the corresponding ui file for a script.
 * @param scriptFile
 *      The script file.
 * @return
 *      The ui file un success. An empty string if not the ui file has not been found.
 */
QString MainWindow::getTheCorrespondingUiFile(QString scriptFile)
{
    QString uiFile;
    qint32 pos = scriptFile.lastIndexOf(".");

    if(pos != -1)
    {//Dot has been found.

        uiFile = scriptFile.left(pos);
    }
    else
    {
        uiFile = scriptFile;
    }

    uiFile += ".ui";
    if(!QFile::exists(uiFile))
    {
        uiFile = "";
    }

    return uiFile;
}


/**
 * Is called if current tab index has been changed.
 * @param index
 *      The new index.
 */
void MainWindow::tabIndexChangedSlot(int index)
{
    if(ui->documentsTabWidget->currentWidget() && ui->documentsTabWidget->currentWidget()->layout())
    {
        (void)index;
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());

        if(textEditor->hasSelectedText())
        {
            ui->actionCut->setEnabled(true);
            ui->actionCopy->setEnabled(true);
        }
        else
        {
            ui->actionCut->setEnabled(false);
            ui->actionCopy->setEnabled(false);
        }
        setWindowModified(textEditor->isModified());

        QString nameInTitle;
        if(textEditor->getDocumentName().isEmpty())
        {
            ui->actionReload->setEnabled(false);
            nameInTitle = ui->documentsTabWidget->tabText(ui->documentsTabWidget->currentIndex());
        }
        else
        {
            ui->actionReload->setEnabled(true);
            nameInTitle = textEditor->getDocumentName();
        }

        setWindowTitle(tr("ScriptCommunicator %1 - Script Editor %2[*]").arg(SCRIPT_COMMUNICATOR_VERSION).arg(nameInTitle));

        QMap<QString, bool> scripts = getAllIncludedScripts(ui->documentsTabWidget->currentIndex());
        if(scripts.isEmpty())
        {
            ui->actionOpenAllIncludedScripts->setEnabled(false);
        }
        else
        {
            ui->actionOpenAllIncludedScripts->setEnabled(true);
        }
    }
    else
    {
        ui->actionOpenAllIncludedScripts->setEnabled(false);
    }
}

/**
 * Reload action slot.
 */
void MainWindow::reloadSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    QFile file(textEditor->getDocumentName());
    if (file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        textEditor->setText(in.readAll());
        file.close();

        textEditor->setModified(false);
        removeSavedInfoFile(textEditor->getDocumentName());

        //Call the slot function manually.
        tabIndexChangedSlot(ui->documentsTabWidget->currentIndex());

        //Set the tab text,
        ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), strippedName(textEditor->getDocumentName()));
        statusBar()->showMessage(tr("File reloaded"), 10000);

        textEditor->updateLastModified();
        textEditor->setFileMustBeParsed(true);
        m_parseTimer.start(200);
    }
    else
    {
        QMessageBox::warning(this, tr("ScriptCommunicator script editor"),
                             tr("Cannot read file %1:\n%2")
                             .arg(textEditor->getDocumentName())
                             .arg(file.errorString()));
    }
}

/**
 * Cut action slot.
 */
void MainWindow::cutSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->cut();
}

/**
 * Copy action slot.
 */
void MainWindow::copySlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->copy();
}

/**
 * Paste action slot.
 */
void MainWindow::pasteSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->paste();
}

/**
 * Undo action slot.
 */
void MainWindow::undoSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->undo();
}

/**
 * Redo action slot.
 */
void MainWindow::redoSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->redo();
}

/**
 * New action slot.
 */
void MainWindow::newSlot()
{
    addTab("", true);
}


/**
 * Starts the designer.
 * @param uiFile
 *      The ui file.
 */
void MainWindow::startDesigner(QString uiFile)
{
    QString program;
#ifdef Q_OS_LINUX
    program = getScriptEditorFilesFolder() + "/designer";
#elif defined Q_OS_MAC

    QFileInfo fi("/Applications/Qt Creator.app");
    if(fi.exists())
    {
        QStringList arguments;
        arguments << uiFile;

        QProcess *myProcess = new QProcess(this);
        myProcess->start("/Applications/Qt Creator.app/Contents/MacOS/Qt Creator", arguments);

        myProcess->waitForFinished(10000);
        if(myProcess->exitCode() != 0)
        {
            QMessageBox::critical(this, "error starting QtCreator", "could not start QtCreator");
        }

    }
    else
    {
        QString text = "To edit a user interface you need QtCreator.";
        text.append("<br>Download QtCreator from here: <a href=\"http://sourceforge.net/projects/scriptcommunicator/files/Mac%20OS%20X/qt-creator-opensource-mac-x86_64-3.6.0.dmg/download\">");
        text.append("http://sourceforge.net/projects/scriptcommunicator/files/Mac%20OS%20X/qt-creator-opensource-mac-x86_64-3.6.0.dmg/download</a>");
        text.append("<br>Open the dmg file and copy the app to the Applications folder.");
        QMessageBox::critical(this, "could not find QtCreator", text);
    }

    return;
#else
    program = getScriptEditorFilesFolder() + "/designer.exe";
#endif
    QStringList arguments;
    arguments << uiFile;

    QProcess *myProcess = new QProcess(this);
    bool processCreated = myProcess->startDetached(program, arguments);

    if(!processCreated)
    {
        QMessageBox::critical(this, "error starting QtDesigner", "could not start QtDesigner ");
    }

}


/**
 * Open all included action slot.
 */
void MainWindow::openAllIncludedScriptsSlot()
{

    QMap<QString, bool> scripts = getAllIncludedScripts(ui->documentsTabWidget->currentIndex());
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    int currentIndex = ui->documentsTabWidget->currentIndex();
    QMap<QString, bool>::iterator i;
    for (i = scripts.begin(); i != scripts.end(); ++i)
    {
        QString fileName;
        if(i.value() == true)
        {//Relative path.

            fileName = QFileInfo(textEditor->getDocumentName()).absolutePath() + "/" + i.key();
        }
        else
        {
            fileName = i.key();
        }
        addTab(fileName, true);
    }
    ui->documentsTabWidget->setCurrentIndex(currentIndex);
}

/**
 * Returns the folder ich which the ScriptEditor files
 * are locared
 * @return
 *      The folder.
 */
QString MainWindow::getScriptEditorFilesFolder(void)
{
    return QCoreApplication::applicationDirPath();

}

/**
 * Starts an other instance of ScriptEditor.
 * @param arguments
 *      The program arguments.
 */
void MainWindow::startScriptEditor(QStringList arguments)
{

    QString scriptEditor;

#ifdef Q_OS_LINUX
    scriptEditor = getScriptEditorFilesFolder() + "/ScriptEditor";
#elif defined Q_OS_MAC
    scriptEditor = getScriptEditorFilesFolder() + "/ScriptEditor";
#else//Windows
    scriptEditor = getScriptEditorFilesFolder() + "/ScriptEditor.exe";
#endif


    QProcess *myProcess = new QProcess(this);

    //Start the script editor.
    if(!myProcess->startDetached(scriptEditor, arguments))
    {
        QMessageBox::critical(this, "error starting script editor", "could not start: " + scriptEditor);
    }
}

/**
 * Drag enter event.
 * @param event
 *      The drag enter event.
 */
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/**
 * Returns true if the document in fileName has already been loaded.
 * @param fileName
 *      The file name.
 * @param index
 *      The tab index of the already loaded document.
 * @return
 *      True if already loaded.
 */
bool MainWindow::checkIfDocumentAlreadyLoaded(QString fileName, int& index)
{
    bool result = false;
    index = -1;

    for(quint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());

        if((textEditor->getDocumentName() == fileName) && !fileName.isEmpty())
        {
            result = true;
            index = i;
            break;
        }
    }

    return result;

}

/**
 * Drop event.
 * @param event
 *      The drop event.
 */
void MainWindow::dropEvent(QDropEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
#ifdef Q_OS_LINUX
        QString files = event->mimeData()->text().remove("file://");
#else
        QString files = event->mimeData()->text().remove("file:///");
#endif
        QStringList list = files.split("\n");

        for(auto file : list)
        {
            if(!file.isEmpty())
            {
                SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
                if(textEditor->getDocumentName().isEmpty() && !textEditor->isModified())
                {
                    if(!loadFile(file))
                    {
                        setCurrentFile("");
                    }
                }
                else
                {
                    addTab(file, true);
                }
            }
        }

        event->acceptProposedAction();
    }
}




/**
 * Show event.
 * @param event
 *      The show event.
 */
void MainWindow::showEvent(QShowEvent *event)
{
    event->accept();

    QList<int> elSizes = ui->splitter->sizes();
    if(elSizes[1] > 50)
    {
        elSizes[0] += elSizes[1] - 50;
        elSizes[1] = 50;
    }
    ui->splitter->setSizes(elSizes);


    readSettings();

    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->setFocus();

}

/**
 * Removes the saved info file.
 * @param name
 *      The name of the file to which the insaved info file belongs to.
 */
void MainWindow::removeSavedInfoFile(QString name)
{

    if(m_unsavedInfoFiles.contains(name))
    {
        QString tmpDirectory = getTmpDirectory(name);
        QString unsavedInfoFileName = getUnsavedInfoFileName(name);

        m_unsavedInfoFiles[name]->close();
        QFile::remove(unsavedInfoFileName);
        delete m_unsavedInfoFiles[name];
        m_unsavedInfoFiles.remove(name);

        if(QDir(tmpDirectory).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
        {//The tmp directory is empty.
            QDir().rmdir(tmpDirectory);
        }
    }
}

/**
 * Removes the lock of a loaded script file.
 * @param index
 *      The tab index of the file.
 */
void MainWindow::removeFileLock(int index)
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());
    QString name = textEditor->getDocumentName().isEmpty() ? ui->documentsTabWidget->tabText(index) : textEditor->getDocumentName();

    if(m_lockFiles.contains(name))
    {
        QString tmpDirectory = getTmpDirectory(name);
        QString lockFileName = getLockFileName(name);

        m_lockFiles[name]->close();
        QFile::remove(lockFileName);
        delete m_lockFiles[name];
        m_lockFiles.remove(name);

        if(QDir(tmpDirectory).entryInfoList(QDir::NoDotAndDotDot|QDir::AllEntries).count() == 0)
        {//The tmp directory is empty.
            QDir().rmdir(tmpDirectory);
        }
    }

    removeSavedInfoFile(name);
}

/**
 * Close event.
 * @param event
 *      The close event.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    m_parseTimer.stop();

    bool cancelled = false;
    for(quint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        if(!maybeSave(i))
        {
            cancelled = true;
        }
        removeFileLock(i);
    }

    if(!cancelled)
    {
        writeSettings();
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

/**
 * Returns all included scripts from a script file.
 * @param tabIndex
 *      The index of the tab.
 * @return
 *      All scripts.
 */
QMap<QString, bool> MainWindow::getAllIncludedScripts(int tabIndex)
{
    QMap<QString, bool> result;
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(tabIndex)->layout()->itemAt(0)->widget());
    QString text = textEditor->text();

    //Remove all '/**/' comments.
    QRegExp comment("/\\*(.|[\r\n])*\\*/");
    comment.setMinimal(true);
    comment.setPatternSyntax(QRegExp::RegExp);
    text.remove(comment);

    //Remove all '//' comments.
    comment.setMinimal(false);
    comment.setPattern("//[^\n]*");
    text.remove(comment);

    bool custSearched = false;
    QRegExp rx("scriptThread.loadScript(*)");
    rx.setPatternSyntax(QRegExp::Wildcard);
    int index = 0;


    while(index != -1)
    {
        index = text.indexOf(rx,index);

        if(index != -1)
        {
            int endIndex = text.indexOf(")", index);
            if(endIndex != -1)
            {
                QString script = text.mid(index, (endIndex - index));

                int startIndex = script.indexOf("(", 0);
                script.remove(0, startIndex + 1);
                script.remove('"');
                script.remove("'");
                QStringList list = script.split(",");

                if(list.count() > 1)
                {
                    result[list[0]] = list[1].contains("true") ? true : false;
                }
                else
                {
                    result[list[0]] = true;
                }
            }
            index++;
        }
        else
        {
            if(!custSearched)
            {
                custSearched = true;
                index = 0;
                rx.setPattern("cust.loadScript(*)");
            }

        }

    }

    return result;
}


/**
 * Adds a string to the search string list (m_lastSearchString).
 * @param findText
 *      The new search text.
 */
void MainWindow::addStringToTheSearchStringList(QString findText)
{

    if(!findText.isEmpty())
    {
        int pos = -1;

        for(int index = 0; index < m_lastSearchString.length(); index++)
        {
            if(m_lastSearchString[index].indexOf(findText, 0, Qt::CaseSensitive) != -1)
            {//String found.
                pos = index;
                break;
            }
        }
        if(-1 != pos)
        {//The current search string is included in the list.
            m_lastSearchString.removeAt(pos);
            m_findDialog->ui->findWhatComboBox->removeItem(pos);
        }

        m_findDialog->ui->findWhatComboBox->insertItem(0, findText);
        m_lastSearchString.push_front(findText);

        if(m_lastSearchString.size() > 20)
        {
            m_lastSearchString.removeLast();
            m_findDialog->ui->findWhatComboBox->removeItem(m_findDialog->ui->findWhatComboBox->count() - 1);
        }

        m_findDialog->ui->findWhatComboBox->setCurrentIndex(0);

    }
}

/**
 * Adds a string to the replace string list (m_lastReplaceString).
 * @param replaceText
 *      The new replace text.
 */
void MainWindow::addStringToTheReplaceList(QString replaceText)
{

    if(!replaceText.isEmpty())
    {
        int pos = -1;

        for(int index = 0; index < m_lastReplaceString.length(); index++)
        {
            if(m_lastReplaceString[index].indexOf(replaceText, 0, Qt::CaseSensitive) != -1)
            {//String found.
                pos = index;
                break;
            }
        }
        if(-1 != pos)
        {//The current search string is included in the list.
            m_lastReplaceString.removeAt(pos);
            m_findDialog->ui->replaceComboBox->removeItem(pos);
        }

        m_findDialog->ui->replaceComboBox->insertItem(0, replaceText);
        m_lastReplaceString.push_front(replaceText);

        if(m_lastReplaceString.size() > 20)
        {
            m_lastReplaceString.removeLast();
            m_findDialog->ui->replaceComboBox->removeItem(m_findDialog->ui->replaceComboBox->count() - 1);
        }

        m_findDialog->ui->replaceComboBox->setCurrentIndex(0);
    }
}

/**
 * Is called if the find button the the find dialog has been clicked.
 */
void MainWindow::replaceButtonSlot()
{
    QString replaceText =m_findDialog->ui->replaceComboBox->currentText();
    QString findText = m_findDialog->ui->findWhatComboBox->currentText();

    addStringToTheReplaceList(replaceText);
    addStringToTheSearchStringList(findText);

    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    if(textEditor->hasSelectedText())
    {
        textEditor->replaceSelectedText(replaceText);
    }

    findTextInDocument(findText);
}

/**
 * Is called if the replace all button the the find dialog has been clicked.
 */
void MainWindow::replaceAllButtonSlot()
{
    QString replaceText =m_findDialog->ui->replaceComboBox->currentText();
    QString findText = m_findDialog->ui->findWhatComboBox->currentText();

    addStringToTheReplaceList(replaceText);
    addStringToTheSearchStringList(findText);

    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->setCursorPosition(0,0);
    while(textEditor->findFirst(findText, false, m_findDialog->ui->matchCaseCheckBox->isChecked(),
                                m_findDialog->ui->matchWholeWordCheckBox->isChecked(), false, true, -1, -1, false, false))
    {
        textEditor->replaceSelectedText(replaceText);
    }
}

/**
 * Finds ans marks a text in the current script file.
 * @param findText
 *      The text which shall be found.
 * @return
 *      True if the text has been found.
 */
bool MainWindow::findTextInDocument(QString findText)
{

    bool hasBeenFound = false;
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());

    if(textEditor->hasSelectedText())
    {
        int line = 0;
        int index = 0;
        textEditor->getCursorPosition(&line, &index);

        if(m_findDialog->ui->directionDownRadioButton->isChecked())
        {
            index++;
        }
        else
        {
            index--;
        }

        textEditor->setCursorPosition(line, index);
    }
    hasBeenFound = textEditor->findFirst(findText, false, m_findDialog->ui->matchCaseCheckBox->isChecked(),
                                         m_findDialog->ui->matchWholeWordCheckBox->isChecked(), true, m_findDialog->ui->directionDownRadioButton->isChecked(), -1,
                                         -1, true, false);


    return hasBeenFound;
}

/**
 * Is called if the edit ui button has been clicked.
 */
void MainWindow::editUiButtonSlot()
{
    QList<QTreeWidgetItem*>items = ui->uiTreeWidget->selectedItems();
    if(!items.isEmpty())
    {
        bool isOk;
        ParsedUiObject* entry  = (ParsedUiObject*)items[0]->data(0, PARSED_ENTRY).toULongLong(&isOk);

        int index = 0;
        if((entry != 0) && !checkIfDocumentAlreadyLoaded(entry->uiFile, index))
        {
            int ret = QMessageBox::question(this, tr("Edit user interface"), "Edit " + strippedName(entry->uiFile) + " in text mode?",
                                           QMessageBox::Yes | QMessageBox::Default,
                                           QMessageBox::No);

            if (ret == QMessageBox::Yes)
            {
                addTab(entry->uiFile, true);
            }
            else
            {
                startDesigner(entry->uiFile);
            }
        }
    }
}

/**
 * Is called if the find button the the find dialog has been clicked.
 */
void MainWindow::findButtonSlot()
{
    QString findText = m_findDialog->ui->findWhatComboBox->currentText();

    addStringToTheSearchStringList(findText);

    //Find the text.
    findTextInDocument(findText);
}

/**
 * Opens the find dialog.
 */
void MainWindow::find()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    if(textEditor->hasSelectedText())
    {
        m_findDialog->ui->findWhatComboBox->setCurrentText(textEditor->selectedText());
    }
    m_findDialog->show();

    int currentHeight = m_findDialog->height();
    m_findDialog->setMaximumHeight(currentHeight);
}

/**
 * Opens a new file.
 */
void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty())
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        if(textEditor->getDocumentName().isEmpty())
        {
            if(!loadFile(fileName))
            {
                setCurrentFile("");
            }
        }
        else
        {
            addTab(fileName, true);
        }
    }
}

/**
 * Opens a set font dialog.
 */
void MainWindow::setFont()
{
    bool ok;
    m_currentFont = QFontDialog::getFont(&ok, m_currentFont, this);
    if (ok)
    {
        for(quint32 i = 0; i < ui->documentsTabWidget->count(); i++)
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
            textEditor->lexer()->setFont(m_currentFont, -1);
        }
    }

}

/**
 * Saves the current script file (under the current file name).
 * @return
 *      True on success.
 */
bool MainWindow::save()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    if(textEditor->getDocumentName().isEmpty())
    {
        return saveAs();
    }
    else
    {
        return saveFile(textEditor->getDocumentName());
    }
}

/**
 * Saves the current script file (under a new file name).
 * @return
 *      True on success.
 */
bool MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save script file", "", "script files (*.js);;Files (*.*)");
    if (fileName.isEmpty())
    {
        return false;
    }

    return saveFile(fileName);
}


/**
 * Zooms out on the text by by making the base font size one point smaller and recalculating all font sizes.
 */
void MainWindow::zoomOutSlot()
{
    m_currentFont.setPointSize(m_currentFont.pointSize() - 1);
    for(quint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
        textEditor->lexer()->setFont(m_currentFont, -1);
    }
}

/**
 * Zooms in on the text by by making the base font size one point larger and recalculating all font sizes.
 */
void MainWindow::zoomInSlot()
{
    m_currentFont.setPointSize(m_currentFont.pointSize() + 1);
    for(quint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
        textEditor->lexer()->setFont(m_currentFont, -1);
    }
}


/**
 * Is called if the user double clicks on the ui view.
 * @param item
 *      The clicked item.
 */
void MainWindow::uiViewDoubleClicked(QTreeWidgetItem* item, int column)
{
    (void)column;

    if(item)
    {
        bool isOk = false;
        ParsedUiObject* entry  = (ParsedUiObject*)item->data(0, PARSED_ENTRY).toULongLong(&isOk);

        if((entry != 0) && !entry->objectName.isEmpty())
        {
            int index = 0;
            if(!checkIfDocumentAlreadyLoaded(entry->uiFile, index))
            {
                int ret = QMessageBox::question(this, tr("Edit user interface"), "Edit " + strippedName(entry->uiFile) + " in text mode?",
                                               QMessageBox::Yes | QMessageBox::Default,
                                               QMessageBox::No);

                if (ret == QMessageBox::Yes)
                {
                    addTab(entry->uiFile, true);
                }
                else
                {
                    startDesigner(entry->uiFile);
                }
            }


            if(checkIfDocumentAlreadyLoaded(entry->uiFile, index))
            {
                SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());
                (void)textEditor->findFirst("name=\"" + entry->objectName, false, true, false, true, true, 0, 0, true, false);

                ui->documentsTabWidget->setCurrentIndex(index);
                textEditor->setFocus();
            }
        }
    }

}

/**
 * Is called if the user double clicks on the function list.
 * @param item
 *      The clicked item.
 */
void MainWindow::functionListDoubleClicked(QTreeWidgetItem* item, int column)
{
    (void)column;

    if(item)
    {
        bool isOk = false;
        ParsedEntry* entry  = (ParsedEntry*)item->data(0, PARSED_ENTRY).toULongLong(&isOk);

        if(entry != 0)
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(entry->tabIndex)->layout()->itemAt(0)->widget());
            textEditor->setCursorPosition(entry->line, 0);
            (void)textEditor->findFirst(entry->name, false, true, false, true, true, entry->line,
                                 0, true, false);

            ui->documentsTabWidget->setCurrentIndex(entry->tabIndex);
            textEditor->setFocus();
        }
    }

}

/**
 * Creates a document title for a new document
 * @return
 *      The document title.
 */
QString MainWindow::createNewDocumentTitle(void)
{
    bool titleCreated = false;
    QString newTitle = "untitled";
    quint32 counter = 1;

    while(!titleCreated)
    {
        bool titleFound = false;

        for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
        {//Search the current title.

            QString searchText = newTitle + QString("%1.js").arg(counter);
            if((searchText == ui->documentsTabWidget->tabText(i)) ||
                (searchText + "*" == ui->documentsTabWidget->tabText(i)))
            {
                titleFound = true;
            }
        }

        if(!titleFound)
        {
            newTitle += QString("%1.js").arg(counter);
            titleCreated = true;
        }
        counter++;
    }

    return newTitle;
}



/**
 * Checks if the current script file has been changed. If the files has been changed
 * it displays this in the window title.
 */
void MainWindow::documentWasModified()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    setWindowModified(textEditor->isModified());
    textEditor->setMarginWidth(0, QString("00%1").arg(textEditor->lines()));
    textEditor->setFileMustBeParsed(true);

    QString shownName;
    if (textEditor->getDocumentName().isEmpty())
    {
        shownName = ui->documentsTabWidget->tabText(ui->documentsTabWidget->currentIndex());

    }
    else
    {
        shownName = strippedName(textEditor->getDocumentName());
    }
    if(textEditor->isModified())
    {
        if(!shownName.endsWith("*"))
        {
            ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), shownName + "*");
        }

        QString fileName = textEditor->getDocumentName();
        if(!fileName.isEmpty() && !m_unsavedInfoFiles.contains(fileName))
        {//No unsaved info file created yet.
            QFile* unsavedInfoFile = new QFile(getUnsavedInfoFileName(fileName));
            unsavedInfoFile->open(QIODevice::WriteOnly);
            m_unsavedInfoFiles[fileName] = unsavedInfoFile;
        }
    }
    else
    {
         ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), shownName);
    }

    m_parseTimer.start(2000);
}

/**
 * Initializes all actions.
 */
void MainWindow::initActions()
{
    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionSaveAs, SIGNAL(triggered()), this, SLOT(saveAs()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionFind, SIGNAL(triggered()), this, SLOT(find()));
    connect(ui->actionZoomIn, SIGNAL(triggered()), this, SLOT(zoomInSlot()));
    connect(ui->actionZoomOut, SIGNAL(triggered()), this, SLOT(zoomOutSlot()));
    connect(ui->actionCut, SIGNAL(triggered()), this, SLOT(cutSlot()));
    connect(ui->actionCopy, SIGNAL(triggered()), this, SLOT(copySlot()));
    connect(ui->actionPaste, SIGNAL(triggered()), this, SLOT(pasteSlot()));
    connect(ui->actionUndo, SIGNAL(triggered()), this, SLOT(undoSlot()));
    connect(ui->actionRedo, SIGNAL(triggered()), this, SLOT(redoSlot()));
    connect(ui->actionNew, SIGNAL(triggered()), this, SLOT(newSlot()));
    connect(ui->actionOpenAllIncludedScripts, SIGNAL(triggered()), this, SLOT(openAllIncludedScriptsSlot()));
    connect(ui->actionSetFont, SIGNAL(triggered()), this, SLOT(setFont()));
    connect(ui->actionEditUi , SIGNAL(triggered()), this, SLOT(editUiButtonSlot()));
    connect(ui->actionReload , SIGNAL(triggered()), this, SLOT(reloadSlot()));

    ui->actionEditUi->setEnabled(false);

}

/**
 * Reads the editor settings.
 */
void MainWindow::readSettings()
{
    QSettings settings("ScriptCommunicator", QString("ScriptEditor_%1").arg(INTERNAL_VERSION));

    if(settings.contains("pos") && settings.contains("size") && settings.contains("mainSplitter") && settings.contains("fontFamily"))
    {
        QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
        QSize size = settings.value("size", QSize(600, 600)).toSize();
        ui->splitter->restoreState(settings.value("mainSplitter").toByteArray());
        resize(size);
        move(pos);

        m_currentFont.setFamily(settings.value("fontFamily", QFont().family()).toString());
        bool ok;
        m_currentFont.setPointSize(settings.value("fontPointSize", QFont().pointSize()).toInt(&ok));
        m_currentFont.setWeight(settings.value("fontWeight", QFont().weight()).toInt(&ok));
        m_currentFont.setItalic(settings.value("fontItalic", QFont().italic()).toBool());

        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        textEditor->lexer()->readSettings(settings);
    }
    else
    {
        QList<int> list = ui->splitter->sizes();

        double size = list[0] + list[1];
        list[0] = static_cast<int>(size * 0.2);
        list[1] = static_cast<int>(size - list[0]);
        ui->splitter->setSizes(list);
    }


}

/**
 * Writes/saves the editor settings.
 */
void MainWindow::writeSettings()
{
    QSettings settings("ScriptCommunicator", QString("ScriptEditor_%1").arg(INTERNAL_VERSION));
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("mainSplitter", ui->splitter->saveState());
    settings.setValue("fontFamily", m_currentFont.family());
    settings.setValue("fontPointSize", m_currentFont.pointSize());
    settings.setValue("fontWeight", m_currentFont.weight());
    settings.setValue("fontItalic", m_currentFont.italic());

    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->lexer()->writeSettings(settings);

}

/**
 * Displays a dialog if the current script has been changed.
 * @return
 *      True if the current file should be saved.
 */
bool MainWindow::maybeSave(int index)
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());
    if (textEditor->isModified())
    {
        QString name = textEditor->getDocumentName().isEmpty() ? ui->documentsTabWidget->tabText(index) : textEditor->getDocumentName();
        int ret = QMessageBox::warning(this, tr("Save?"), name + " has been modified.\nDo you want to save your changes?",
                                       QMessageBox::Yes | QMessageBox::Default,
                                       QMessageBox::No,
                                       QMessageBox::Cancel | QMessageBox::Escape);

        if (ret == QMessageBox::Yes)
        {
            return save();
        }
        else if (ret == QMessageBox::Cancel)
        {
            return false;
        }
    }
    return true;
}

/**
 * Clears the outline window.
 */
void MainWindow::clearOutlineWindow(void)
{
    QTreeWidgetItemIterator it(ui->outlineTreeWidget);
    while (*it)
    {
        bool isOk = false;
        ParsedEntry* entry  = (ParsedEntry*)(*it)->data(0, PARSED_ENTRY).toULongLong(&isOk);
        if(entry != 0)
        {
            (*it)->setData(0, PARSED_ENTRY, (quint64)0);
            delete entry;
        }
      ++it;
    }

    ui->outlineTreeWidget->clear();
}


/**
 * Clears the ui window.
 */
void MainWindow::clearUiWindow(void)
{
    QTreeWidgetItemIterator it(ui->uiTreeWidget);
    while (*it)
    {
        bool isOk = false;
        ParsedUiObject* entry  = (ParsedUiObject*)(*it)->data(0, PARSED_ENTRY).toULongLong(&isOk);
        if(entry != 0)
        {
            (*it)->setData(0, PARSED_ENTRY, (quint64)0);
            delete entry;
        }
      ++it;
    }

    ui->uiTreeWidget->clear();
}
/**
 * Inserts all UI objects in the ui view.
 *
 * @param parsedUiObjects
 *      All parsed UI objects.
 */
void MainWindow::insertAllUiObjectsInUiView(QMap<QString, QStringList> parsedUiObjects)
{
    static QString savedCompleteTreeString = "";
    QString currenCompleteTreeString = "";

    QTreeWidgetItem* root = ui->uiTreeWidget->invisibleRootItem();
    QMap<QString, bool> expandMap;

    QMap<QString, QStringList>::const_iterator iter = parsedUiObjects.constBegin();
    while (iter != parsedUiObjects.constEnd())
    {
        for(auto el : iter.value())
        {
            currenCompleteTreeString += el;
        }
        iter++;
    }

    if(savedCompleteTreeString == currenCompleteTreeString)
    {///The tree wigdet content has not changed.
        return;
    }

    //Save the expanded state of all tree widget elements.
    for(int i = 0; i < root->childCount(); i++)
    {
        expandMap[root->child(i)->text(0)] = root->child(i)->isExpanded();
    }

    savedCompleteTreeString = currenCompleteTreeString;
    clearUiWindow();

    ui->actionEditUi->setEnabled(parsedUiObjects.isEmpty() ? false : true);

    iter = parsedUiObjects.constBegin();
    bool firstFile = true;
    while (iter != parsedUiObjects.constEnd())
    {
        QTreeWidgetItem* fileElement = new QTreeWidgetItem(root);
        ParsedUiObject* tmpEntry = new ParsedUiObject();
        tmpEntry->objectName = "";
        tmpEntry->uiFile = iter.key();
        fileElement->setData(0, PARSED_ENTRY, (quint64)tmpEntry);
        fileElement->setText(0, strippedName(iter.key()));
        fileElement->setToolTip(0, iter.key());
        root->addChild(fileElement);

        if(firstFile)
        {
            fileElement->setSelected(true);
            firstFile = false;
        }


        for(auto el : iter.value())
        {
            QTreeWidgetItem* funcElement = new QTreeWidgetItem(fileElement);
            QString textInTreeWidget = "UI_" + el;
            tmpEntry = new ParsedUiObject();
            tmpEntry->objectName = el;
            tmpEntry->uiFile = iter.key();
            funcElement->setData(0, PARSED_ENTRY, (quint64)tmpEntry);
            fileElement->addChild(funcElement);

            funcElement->setText(0, textInTreeWidget);
            funcElement->setToolTip(0, textInTreeWidget);
        }

        if(expandMap.contains(fileElement->text(0)))
        {
            //Restore the expanded state of all tree widget elements.
            if(expandMap[fileElement->text(0)])
            {
                ui->uiTreeWidget->expandItem(fileElement);
            }
        }
        else
        {//New file element, expand all items.
            ui->uiTreeWidget->expandItem(fileElement);
        }

        iter++;
    }
}

/**
 * Check for errors in the loaded scripts.
 *
 * @return
 *      True if one script contains an error.
 */
bool MainWindow::checkForErrorsInScripts(void)
{
    bool errorFound = false;

    for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());

        if(!textEditor->getDocumentName().endsWith(".ui"))
        {
            QString text = textEditor->text();

            textEditor->clearAnnotations();
            esprima::Pool pool;
            esprima::Program *program = NULL;
            try
            {
                program = esprima::parse(pool, text.toLocal8Bit().constData());
            }
            catch(esprima::ParseError e)
            {
                QsciStyle myStyle(-1,"Annotation",QColor(255,0,0),QColor(255,150,150),QFont("Courier New",-1,-1,true),true);
                textEditor->annotate(e.lineNumber - 1, e.description.c_str(),myStyle);
                errorFound = true;
            }
        }
    }

    return errorFound;
}

/**
 * Inserts all function and global variables (form the current script file) into the function script view.
 */
void MainWindow::insertAllFunctionAndVariablesInScriptView(QMap<QString,QVector<ParsedEntry>> parsedEntries)
{
    static QString savedCompleteTreeString = "";
    QString currentCompleteTreeString = "";
    QTreeWidgetItem* root = ui->outlineTreeWidget->invisibleRootItem();
    QMap<QString, bool> expandMap;
    m_allFunctions.clear();

    QMap<QString,QVector<ParsedEntry>>::const_iterator iter = parsedEntries.constBegin();
    while (iter != parsedEntries.constEnd())
    {
        currentCompleteTreeString += iter.key();
        for(auto el : iter.value())
        {
            currentCompleteTreeString += el.name;
        }
        iter++;
    }

   if(savedCompleteTreeString == currentCompleteTreeString)
   {///The tree wigdet content has not changed.
       return;
   }

    savedCompleteTreeString = currentCompleteTreeString;

    //Save the expanded state of all tree widget elements.
    for(int i = 0; i < root->childCount(); i++)
    {
        expandMap[root->child(i)->text(0)] = root->child(i)->isExpanded();
    }

    clearOutlineWindow();

    iter = parsedEntries.constBegin();
    while (iter != parsedEntries.constEnd())
    {
        int index = -1;
        checkIfDocumentAlreadyLoaded(iter.key(), index);

        QTreeWidgetItem* fileElement = new QTreeWidgetItem(root);
        fileElement->setData(0, PARSED_ENTRY, 0);
        fileElement->setText(0, strippedName(iter.key()));
        fileElement->setToolTip(0, iter.key());
        root->addChild(fileElement);

        for(auto el : iter.value())
        {

            QTreeWidgetItem* funcElement = new QTreeWidgetItem(fileElement);
            QString textInTreeWidget = el.name;
            ParsedEntry* tmpEntry = new ParsedEntry();
            *tmpEntry = el;
            tmpEntry->tabIndex = index;
            funcElement->setData(0, PARSED_ENTRY, (quint64)tmpEntry);
            fileElement->addChild(funcElement);

            if(el.isFunction)
            {
                m_allFunctions.append(el.name);

                textInTreeWidget += "(";

                for(auto param : el.params)
                {
                    textInTreeWidget += param + ",";
                }
                if(textInTreeWidget.right(1) == ",")
                {
                    textInTreeWidget.remove(textInTreeWidget.length() - 1, 1);
                }
                textInTreeWidget += ")";
            }

            funcElement->setText(0, textInTreeWidget);
            funcElement->setToolTip(0, textInTreeWidget);

        }


        if(expandMap.contains(fileElement->text(0)))
        {
            //Restore the expanded state of all tree widget elements.
            if(expandMap[fileElement->text(0)])
            {
                ui->outlineTreeWidget->expandItem(fileElement);
            }
        }
        else
        {//New file element, expand all items.
            ui->outlineTreeWidget->expandItem(fileElement);
        }

        iter++;
    }
}

/**
 * Loads a file.
 * @param fileName
 *      The file name.
 */
bool MainWindow::loadFile(const QString &fileName)
{
    QString tmpDirectory = getTmpDirectory(fileName);
    QString lockFileName = getLockFileName(fileName);

    if(QFileInfo().exists(lockFileName))
    {//The file is already opened.

        QMessageBox message(QMessageBox::Warning, "Warning", fileName + " is already opened by an other instance of ScriptCommunicator. Open anyway?", QMessageBox::Yes | QMessageBox::No, this);
        if(message.exec() == QMessageBox::No)
        {
            return false;
        }
    }

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, tr("ScriptCommunicator script editor"),
                             tr("Cannot read file %1:\n%2")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    //Create the tmp directory.
    QDir().mkdir(tmpDirectory);

    //Create the lock file.
    QFile* lockFile = new QFile(lockFileName);
    lockFile->open(QIODevice::WriteOnly);
    m_lockFiles[fileName] = lockFile;


    QTextStream in(&file);
    in.setCodec("UTF-8");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->setText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    textEditor->updateLastModified();
    textEditor->setFileMustBeParsed(true);
    statusBar()->showMessage(tr("File loaded"), 5000);

    m_parseTimer.start(200);
    return true;
}

/**
 * Saves the current file.
 * @param fileName
 *      The file name.
 * @return
 *      True on success.
 */
bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, tr("ScriptCommunicator script editor"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    QApplication::setOverrideCursor(Qt::WaitCursor);
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    out << textEditor->text();
    file.close();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);

    removeSavedInfoFile(fileName);

    textEditor->updateLastModified();
    textEditor->setFileMustBeParsed(true);
    m_parseTimer.start(200);

    return true;
}

/**
 * Sets the current file (name).
 * @param fileName
 *      The new file name.
 */
void MainWindow::setCurrentFile(const QString &fileName)
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->setDocumentName(fileName, m_currentFont);
    textEditor->setModified(false);
    setWindowModified(false);

    QString tabShownName;
    QString windowShownName;
    if (fileName.isEmpty())
    {
        tabShownName = createNewDocumentTitle();
        windowShownName = tabShownName;
        ui->actionOpenAllIncludedScripts->setEnabled(false);
        ui->actionReload->setEnabled(false);
    }
    else
    {
        tabShownName = strippedName(fileName);
        windowShownName = fileName;

        QMap<QString, bool> scripts = getAllIncludedScripts(ui->documentsTabWidget->currentIndex());
        if(scripts.isEmpty())
        {
            ui->actionOpenAllIncludedScripts->setEnabled(false);
        }
        else
        {
            ui->actionOpenAllIncludedScripts->setEnabled(true);
        }

        ui->actionReload->setEnabled(true);
    }

    setWindowTitle(tr("ScriptCommunicator %1 - Script Editor %2[*]").arg(SCRIPT_COMMUNICATOR_VERSION).arg(windowShownName));

    ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), tabShownName);
    ui->documentsTabWidget->setTabToolTip(ui->documentsTabWidget->currentIndex(), fileName);
}

/**
 * Returns the current file name without the path.
 * @param fullFileName
 *      The full file name.
 * @return
 *      The stripped file name.
 */
QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}
