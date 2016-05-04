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


#include <Qsci/qsciscintilla.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_findDialog.h"

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
MainWindow::MainWindow(QStringList scripts) : ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    g_mainWindow = this;

    connect(ui->documentsTabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChangedSlot(int)));
    connect(ui->documentsTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(tabCloseRequestedSlot(int)));

    if(scripts.isEmpty())
    {
        addTab("");
    }
    else
    {
        for(auto el : scripts)
        {
            addTab(el);
        }
    }

    initActions();
    statusBar()->showMessage(tr("Ready"));

    setAcceptDrops(true);

    m_findDialog = new FindDialog(this);

    connect(m_findDialog->ui->findPushButton, SIGNAL(clicked()), this, SLOT(findButtonSlot()));
    connect(m_findDialog->ui->replacePushButton, SIGNAL(clicked()), this, SLOT(replaceButtonSlot()));
    connect(m_findDialog->ui->replaceAllPushButton, SIGNAL(clicked()), this, SLOT(replaceAllButtonSlot()));

    m_findDialog->ui->findWhatComboBox->setAutoCompletion(false);
    m_findDialog->ui->replaceComboBox->setAutoCompletion(false);

    connect(ui->functionsTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(functionListDoubleClicked(QTreeWidgetItem*,int)));

    m_findShortcut = new QShortcut(QKeySequence(Qt::Key_F3),this);

    connect(m_findShortcut, SIGNAL(activated()), this, SLOT(findButtonSlot()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * Adds a tab.
 */
void MainWindow::addTab(QString script)
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


        if(!script.isEmpty())
        {
            if(!loadFile(script))
            {
                ui->documentsTabWidget->removeTab(ui->documentsTabWidget->currentIndex());

                if(ui->documentsTabWidget->count() == 0)
                {
                    addTab("");
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
        ui->documentsTabWidget->setCurrentIndex(index);
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
        ui->documentsTabWidget->removeTab(index);

        if(ui->documentsTabWidget->count() == 0)
        {
            addTab("");
        }
        insertAllFunctionInListView();
    }
}

/**
 * Returns the corresponding ui file for a script.
 * @param scriptFile
 *      The script file.
 * @return
 *      The ui file un success. An empty string if not the ui file has not been found.
 */
QString MainWindow::getTheCorrespondingUiFileExists(QString scriptFile)
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
 * Enables or dissbles the edit ui action.
 */
void MainWindow::enableDisableActionEditUI()
{
    if(ui->documentsTabWidget->currentWidget() && ui->documentsTabWidget->currentWidget()->layout())
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());

        QString uiFile = getTheCorrespondingUiFileExists(textEditor->getDocumentName());
        if(uiFile.isEmpty())
        {
            ui->actionEditUI->setEnabled(false);
            ui->actionEditUI->setToolTip(QString("edit the corresponding user interface file (no corresponding ui file found)"));
        }
        else
        {
            ui->actionEditUI->setEnabled(true);
            ui->actionEditUI->setToolTip(QString("edit the corresponding user interface file (%1)").arg(QFileInfo(uiFile).fileName()));
        }
    }
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

        //Enable/disable the edit ui action.
        enableDisableActionEditUI();

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
        setWindowTitle(tr("%1 - %2[*]").arg("script editor").arg(textEditor->getDocumentName()));

        QMap<QString, bool> scripts = getAllIncludedScripts(ui->documentsTabWidget->currentIndex());
        if(scripts.isEmpty())
        {
            ui->actionOpenAllIncludedScripts->setEnabled(false);
        }
        else
        {
            ui->actionOpenAllIncludedScripts->setEnabled(true);
        }

        static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget())->initAutoCompletion(m_allFunction);
    }
    else
    {
        ui->actionOpenAllIncludedScripts->setEnabled(false);
        ui->actionEditUI->setEnabled(false);
        ui->actionEditUI->setToolTip("edit the corresponding user interface file");
    }
}

/**
 * /Cut action slot.
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
    addTab("");
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
 * Edit UI action slot.
 */
void MainWindow::editUiSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    QString uiFile = getTheCorrespondingUiFileExists(textEditor->getDocumentName());

    int ret = QMessageBox::question(this, tr("Edit user interface"), "Edit the ui in text mode?",
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
        addTab(uiFile);
    }
    else
    {
        startDesigner(uiFile);
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
        addTab(fileName);
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
                    addTab(file);
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
 * Close event.
 * @param event
 *      The close event.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    bool cancelled = false;
    for(quint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        if(!maybeSave(i))
        {
            cancelled = true;
        }
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
 * Returns all functions in a script file.
 * @param tabIndex
 *      The index of the tab.
 * @return
 *      All functions.
 */
QStringList MainWindow::getAllFunctions(int tabIndex)
{
    QStringList result;
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(tabIndex)->layout()->itemAt(0)->widget());
    QString text = textEditor->text();


    //Remove all '/**/' comments.
    QRegExp comment("/\\*(.|[\r\n])*\\*/");
    comment.setMinimal(true);
    comment.setPatternSyntax(QRegExp::RegExp);
    text.replace(comment, " ");

    //Remove all '//' comments.
    comment.setMinimal(false);
    comment.setPattern("//[^\n]*");
    text.remove(comment);



    QRegExp rx("function *(*)");
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
                QString func = text.mid(index, (endIndex - index) + 1);

                if(!func.contains("\n"))
                {//The string contains no new line.

                    func.remove(QRegExp("function "));
                    func.remove(" ");
                    result.append(func);
                }
            }
            index++;
        }
    }

    result.sort(Qt::CaseInsensitive );
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
            addTab(fileName);
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
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    QFont font = textEditor->lexer()->font(0);
    font.setPointSize(font.pointSize() - 1);
    textEditor->lexer()->setFont(font, -1);
}

/**
 * Zooms in on the text by by making the base font size one point larger and recalculating all font sizes.
 */
void MainWindow::zoomInSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    QFont font = textEditor->lexer()->font(0);
    font.setPointSize(font.pointSize() + 1);
    textEditor->lexer()->setFont(font, -1);
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
        QString function = item->data(0, FUNC_NAME_INDEX).toString();
        bool isOk = false;
        int index = item->data(0, TABINDEX_NAME_INDEX).toInt(&isOk);

        if(!function.isEmpty() && (index != -1))
        {
            function.replace(",", ".*,.*");
            function.replace("(", "(.*");
            function.replace(")", ".*)");
            function = QString("function.*[ |/]%1").arg(function);

            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());
            (void)textEditor->findFirst(function, true, true, false, true, true, 0,
                                  0, true, false);

            ui->documentsTabWidget->setCurrentIndex(index);
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
    }
    else
    {
         ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), shownName);
    }
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
    connect(ui->actionEditUI, SIGNAL(triggered()), this, SLOT(editUiSlot()));
    connect(ui->actionOpenAllIncludedScripts, SIGNAL(triggered()), this, SLOT(openAllIncludedScriptsSlot()));
}

/**
 * Reads the editor settings.
 */
void MainWindow::readSettings()
{
    QSettings settings("ScriptCommunicator", QString("ScriptEditor_%1").arg(INTERNAL_VERSION));

    if(settings.contains("pos") && settings.contains("size") && settings.contains("mainSplitter"))
    {
        QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
        QSize size = settings.value("size", QSize(600, 600)).toSize();
        ui->splitter->restoreState(settings.value("mainSplitter").toByteArray());
        resize(size);
        move(pos);

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
 * Inserts all function (form the current script file) into the function list view.
 */
void MainWindow::insertAllFunctionInListView()
{

    QTreeWidgetItem* root = ui->functionsTreeWidget->invisibleRootItem();
    bool hasFunctions = false;
    QMap<QString, bool> expandMap;
    m_allFunction.clear();

    for(int i = 0; i < root->childCount(); i++)
    {
        expandMap[root->child(i)->text(0)] = root->child(i)->isExpanded();
    }

    ui->functionsTreeWidget->clear();

    for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());

        if(!textEditor->getDocumentName().endsWith(".ui"))
        {
            QStringList functions = getAllFunctions(i);

            if(!functions.isEmpty())
            {
                hasFunctions = true;
                QTreeWidgetItem* fileElement = new QTreeWidgetItem(root);
                fileElement->setData(0, FUNC_NAME_INDEX, "");
                fileElement->setData(0, TABINDEX_NAME_INDEX, i);

                fileElement->setText(0, strippedName(textEditor->getDocumentName()));
                root->addChild(fileElement);
                for(auto el : functions)
                {
                    QTreeWidgetItem* funcElement = new QTreeWidgetItem(fileElement);
                    funcElement->setData(0, FUNC_NAME_INDEX, el);

                    funcElement->setData(0, TABINDEX_NAME_INDEX, i);
                    funcElement->setText(0, el);
                    fileElement->addChild(funcElement);

                    m_allFunction.append(el);
                }

                if(expandMap.contains(fileElement->text(0)))
                {
                    if(expandMap[fileElement->text(0)])
                    {
                        ui->functionsTreeWidget->expandItem(fileElement);
                    }
                }
                else
                {//New file element.
                    ui->functionsTreeWidget->expandItem(fileElement);
                }

            }
        }
    }


    if(!hasFunctions)
    {
        QTreeWidgetItem* fileElement = new QTreeWidgetItem(root);
        fileElement->setData(0, FUNC_NAME_INDEX, "");
        fileElement->setData(0, TABINDEX_NAME_INDEX, -1);
        fileElement->setText(0, "no functions");
    }


    static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget())->setFocus();
    static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget())->initAutoCompletion(m_allFunction);

}

/**
 * Loads a file.
 * @param fileName
 *      The file name.
 */
bool MainWindow::loadFile(const QString &fileName)
{

    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, tr("ScriptCommunicator script editor"),
                             tr("Cannot read file %1:\n%2")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->setText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File loaded"), 5000);
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
    QApplication::setOverrideCursor(Qt::WaitCursor);
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    out << textEditor->text();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("File saved"), 2000);

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
    textEditor->setDocumentName(fileName);
    textEditor->setModified(false);
    setWindowModified(false);

    QString tabShownName;
    QString windowShownName;
    if (fileName.isEmpty())
    {
        tabShownName = createNewDocumentTitle();
        windowShownName = tabShownName;
        ui->actionOpenAllIncludedScripts->setEnabled(false);
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
    }

    setWindowTitle(tr("%1 - %2[*]").arg("script editor").arg(windowShownName));

    ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), tabShownName);
    insertAllFunctionInListView();
    enableDisableActionEditUI();
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
