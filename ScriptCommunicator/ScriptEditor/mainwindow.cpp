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
    m_lockFiles(), m_unsavedInfoFiles(), m_checkForFileChangesTimer(),
    m_lastMouseMoveEvent(QEvent::None,QPointF(),Qt::NoButton, Qt::NoButton, Qt::NoModifier), m_mouseEventTimer(),
    m_ctrlIsPressed(false), m_indicatorClickTimer(), m_lastIndicatorClickPosition(0), m_showParseError(true),
    m_scriptsToLoadAfterStart(scripts)
{
    ui->setupUi(this);

    g_mainWindow = this;

    m_goToLineDialog.setInputMode(QInputDialog::IntInput);
    m_goToLineDialog.setLabelText("go to line:");
    m_goToLineDialog.setIntRange(1, 1000000000);
    m_goToLineDialog.setIntValue(1);
    m_goToLineDialog.setIntStep(1);

    connect(&m_goToLineDialog, SIGNAL(finished(int)), this, SLOT(executeGoToLineSlot(int)));

    connect(ui->documentsTabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabIndexChangedSlot(int)));
    connect(ui->documentsTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(documentsTabCloseRequestedSlot(int)));
    connect(ui->infoTabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(infoTabCloseRequestedSlot(int)));
    connect(ui->documentsTabWidget->tabBar(), SIGNAL(tabMoved(int,int)), this, SLOT(tabMoved(int,int)));


    initActions();
    statusBar()->showMessage(tr("Ready"));

    setAcceptDrops(true);

    m_findDialog = new FindDialog(this);

    connect(m_findDialog->ui->findPushButton, SIGNAL(clicked()), this, SLOT(findButtonSlot()));
    connect(m_findDialog->ui->findAllPushButton, SIGNAL(clicked()), this, SLOT(findReplaceAllButtonSlot()));
    connect(m_findDialog->ui->replacePushButton, SIGNAL(clicked()), this, SLOT(replaceButtonSlot()));
    connect(m_findDialog->ui->replaceAllPushButton, SIGNAL(clicked()), this, SLOT(replaceAllButtonSlot()));
    connect(&m_parseTimer, SIGNAL(timeout()), this, SLOT(parseTimeout()));
    connect(&m_checkForFileChangesTimer, SIGNAL(timeout()), this, SLOT(checkForFileChanges()));
    connect(&m_mouseEventTimer, SIGNAL(timeout()), this, SLOT(mouseMoveTimerSlot()));
    connect(&m_indicatorClickTimer, SIGNAL(timeout()), this, SLOT(indicatorClickTimerSlot()));
    connect(&m_showEventTimer, SIGNAL(timeout()), this, SLOT(showEventTimerSlot()));


    m_findDialog->ui->findWhatComboBox->setAutoCompletion(false);
    m_findDialog->ui->replaceComboBox->setAutoCompletion(false);

    connect(ui->outlineTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(functionListDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui->uiTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(uiViewDoubleClicked(QTreeWidgetItem*,int)));
    connect(ui->findResults, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(findResultsDoubleClicked(QTreeWidgetItem*,int)));

    m_findShortcut = new QShortcut(QKeySequence(Qt::Key_F3),this);

    connect(m_findShortcut, SIGNAL(activated()), this, SLOT(findButtonSlot()));

    m_parseThread = new ParseThread();
    m_parseThread->moveToThread(m_parseThread);
    m_parseThread->start();

    qRegisterMetaType<QMap<QString,QStringList>>("QMap<QString,QStringList>");
    qRegisterMetaType<QMap<QString,QString>>("QMap<QString,QString>");
    qRegisterMetaType<QMap<int,QString>>("QMap<int,QString>");
    qRegisterMetaType<QMap<int,QVector<ParsedEntry>>>("QMap<int,QVector<ParsedEntry>>");

    connect(this, SIGNAL(parseSignal(QMap<QString, QString>,QMap<int, QString>,QMap<int, QString>, bool,bool)), m_parseThread,
            SLOT(parseSlot(QMap<QString, QString>,QMap<int, QString>,QMap<int, QString>,bool,bool)), Qt::QueuedConnection);
    connect(m_parseThread, SIGNAL(parsingFinishedSignal(QMap<QString,QStringList>,QMap<QString,QStringList>, QMap<QString, QStringList>,QMap<int, QVector<ParsedEntry>>,bool,bool)),
            this, SLOT(parsingFinishedSlot(QMap<QString,QStringList>,QMap<QString,QStringList>, QMap<QString, QStringList>,QMap<int, QVector<ParsedEntry>>,bool,bool)), Qt::QueuedConnection);
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
 * @param parsedUiObjects
 *      Contains the parsed UI objects.
 * @param parsedEntries
 *      Contains all parsed entries.
 * @param doneParsing
 *      True if any file has been parsed.
 * @param parseOnlyUIFiles
 *      True if only ui file have been parsed.
 */
void MainWindow::parsingFinishedSlot(QMap<QString, QStringList> autoCompletionEntries, QMap<QString, QStringList> autoCompletionApiFiles,
                                     QMap<QString, QStringList> parsedUiObjects, QMap<int,QVector<ParsedEntry>> parsedEntries, bool doneParsing,
                                     bool parseOnlyUIFiles)
{


    SingleDocument* currentEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());

    if(doneParsing &&  !currentEditor->isListActive())
    {

        if(!parseOnlyUIFiles)
        {
            bool hasError = insertFillScriptViewAndDisplayErrors(parsedEntries);

            if(hasError)
            {//One document contains an error.

                QMap<QString, QStringList>::const_iterator  iter = autoCompletionEntries.constBegin();
                while (iter != autoCompletionEntries.constEnd())
                {
                    m_autoCompletionEntries[iter.key()] = iter.value();
                    iter++;
                }

            }
            else
            {
                m_autoCompletionEntries = autoCompletionEntries;
            }

            for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
            {
                SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
                textEditor->initAutoCompletion(m_autoCompletionEntries, autoCompletionApiFiles);
            }

            m_showParseError = true;


        }


        insertAllUiObjectsInUiView(parsedUiObjects);
    }


    m_parsingFinished = true;
}

/**
 * Is called by m_checkForFileChangesTimer and checks for changes in the loaded files.
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

                int ret = QMessageBox::question(this, tr("Script Editor"), textEditor->getDocumentName() +
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
                            clearCurrentIndicator();
                            ui->documentsTabWidget->setCurrentIndex(i);
                        }

                        //Set the tab text.
                        ui->documentsTabWidget->setTabText(ui->documentsTabWidget->currentIndex(), strippedName(textEditor->getDocumentName()));
                        statusBar()->showMessage(tr("File reloaded"), 10000);

                        m_parseTimer.start(200);
                    }
                    else
                    {
                        QMessageBox::warning(this, tr("Script Editor"),
                                             tr("Cannot read file %1:\n%2")
                                             .arg(textEditor->getDocumentName())
                                             .arg(file.errorString()));
                    }
                }

                textEditor->updateLastModified();
            }
        }
    }

    parseTimeout(true);
    m_checkForFileChangesTimer.start(2000);
}

/**
 * Clears the current indicator.
 */
void MainWindow::clearCurrentIndicator(void)
{
    if(ui->documentsTabWidget->widget(ui->documentsTabWidget->currentIndex()) && ui->documentsTabWidget->widget(ui->documentsTabWidget->currentIndex())->layout())
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(ui->documentsTabWidget->currentIndex())->layout()->itemAt(0)->widget());
        textEditor->removeUndlineFromWordWhichCanBeClicked();
    }
}

/**
 * Is called if an indicator is clicked.
 */
void MainWindow::indicatorClickTimerSlot()
{
    m_indicatorClickTimer.stop();
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    int line = textEditor->lineAt(m_lastMouseMoveEvent.pos());
    handleDoubleClicksInEditor(m_lastIndicatorClickPosition, line, m_ctrlIsPressed ? QsciScintillaBase::SCMOD_CTRL : QsciScintillaBase:: SCMOD_NORM);

}

/**
 * Is called if an indicator is clicked.
 * @param line
 *      The line of the indicator.
 * @param index
 *      The index of the indicator.
 * @param modifier
 *      The keyboard modifier.
 *
 */
void MainWindow::indicatorClickedSlot(int line, int index, Qt::KeyboardModifiers modifier)
{
    if(ui->documentsTabWidget->currentWidget() && ui->documentsTabWidget->currentWidget()->layout() &&
            (modifier & Qt::ControlModifier))
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        m_lastIndicatorClickPosition = textEditor->positionFromLineIndex(line, index);
        m_indicatorClickTimer.start(200);
    }
}


/**
 * Returns true if outlineTreeWidget contains an element with the given name.
 *
 * @param name
 *      The name.
 */
bool MainWindow::checkIfElementsInOutlineTree(QString name)
{
    //Check if the word is in the outline.
    bool wordIsInOutline = false;
    QTreeWidgetItemIterator iter(ui->outlineTreeWidget);
    while (*iter)
    {
        bool isOk = false;
        ParsedEntry* entry  = (ParsedEntry*)(*iter)->data(0, PARSED_ENTRY).toULongLong(&isOk);

      if (entry && (entry->completeName == name))
      {
          wordIsInOutline = true;
          break;
      }

      ++iter;
    }

    return wordIsInOutline;
}

/**
 * Is called if the mouse move timer times out.
 */
void MainWindow::mouseMoveTimerSlot()
{

    m_mouseEventTimer.stop();

    if(ui->documentsTabWidget->widget(ui->documentsTabWidget->currentIndex()) && ui->documentsTabWidget->widget(ui->documentsTabWidget->currentIndex())->layout())
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(ui->documentsTabWidget->currentIndex())->layout()->itemAt(0)->widget());
        QString word = textEditor->wordAtPoint(m_lastMouseMoveEvent.pos());
        long pos = textEditor->SendScintilla(QsciScintillaBase::SCI_POSITIONFROMPOINTCLOSE, m_lastMouseMoveEvent.x(), m_lastMouseMoveEvent.y());
        int line = textEditor->lineAt(m_lastMouseMoveEvent.pos());

        clearCurrentIndicator();

        //Cancel any existing call tip.
        textEditor->SendScintilla(QsciScintillaBase::SCI_CALLTIPCANCEL);
        if(m_ctrlIsPressed)
        {
            QString completeWord = textEditor->wordAtPosition(pos, true);
            if(!completeWord.isEmpty())
            {
                bool wordIsInOutline = false;

                QString contextString = textEditor->getContextString(line);
                if(!contextString.isEmpty())
                {
                    int index = 0;

                    if(completeWord.startsWith("this."))
                    {
                        //Remove this.
                        completeWord.remove(0, 5);
                        index = contextString.lastIndexOf("::");
                        if(index != -1)
                        {
                            contextString.remove(index, contextString.length() - index);
                        }
                    }
                    contextString = contextString.replace("::", ".");

                    do
                    {
                        //Check the local variables.
                        wordIsInOutline = checkIfElementsInOutlineTree(contextString + "." + completeWord);

                        index = contextString.lastIndexOf(".");

                        if(index != -1)
                        {
                            contextString.remove(index, contextString.length() - index);
                        }

                    }while((index != -1) && !wordIsInOutline);
                }

                if(!wordIsInOutline)
                {
                    //Check if the word is in the outline.
                    wordIsInOutline = checkIfElementsInOutlineTree(completeWord);
                }


                if(!wordIsInOutline)
                {
                    QTreeWidgetItemIterator iter(ui->uiTreeWidget);
                    while (*iter)
                    {
                        bool isOk = false;
                        ParsedUiObject* entry  = (ParsedUiObject*)(*iter)->data(0, PARSED_ENTRY).toULongLong(&isOk);

                      if ("UI_" + entry->objectName == completeWord)
                      {
                          wordIsInOutline = true;
                          break;
                      }

                      ++iter;
                    }
                }

                if(wordIsInOutline)
                {
                    textEditor->underlineWordWhichCanBeClicked(pos);
                }
            }
        }
        else
        {
            if(textEditor->lexer() && !word.isEmpty())
            {
                //Show the calltip (if possible).
              if(!textEditor->callTip(pos))
              {
                  QString lineText = textEditor->text(line);
                  int index = lineText.indexOf(word);
                  index = lineText.indexOf("(", index + 1);
                  pos = textEditor->positionFromLineIndex(line, index + 1);
                  (void)textEditor->callTip(pos);
              }
            }
        }

    }
}

/**
 * Is called if the parse timer times out.
 *
 * @param parseOnlyUIFiles
 *      True if only UI files shall be parsed.
 */
void MainWindow::parseTimeout(bool parseOnlyUIFiles)
{
    if(m_parsingFinished)
    {

        bool parseTimerWasActive = m_parseTimer.isActive();
        m_parseTimer.stop();


        QMap<int, QString> loadedScripts;
        QMap<QString, QString> loadedUiFiles;
        QMap<int, QString> loadedScriptsIndex;
        bool fileMustBeParsed = false;

        //Get the text of all open documents.
        for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
            if(!textEditor->getDocumentName().endsWith(".ui"))
            {
                loadedScripts[i] = textEditor->text();
                loadedScriptsIndex[i] = textEditor->getDocumentName();
            }
            else
            {
                loadedUiFiles[textEditor->getDocumentName()] = textEditor->text();
            }


            if(textEditor->getFileMustBeParsed())
            {
                fileMustBeParsed = true;
            }
            textEditor->setFileMustBeParsed(false);
        }

        m_parsingFinished = false;
        emit parseSignal(loadedUiFiles, loadedScripts, loadedScriptsIndex, fileMustBeParsed, parseOnlyUIFiles);

        //Restart the parse timer if it was active and this call parses only the ui files.
        if(parseOnlyUIFiles & parseTimerWasActive)
        {
            m_parseTimer.start(200);
        }
    }

}


/**
 * Is called if the user double clicks a postion in the editor.
 * @param position
 *      The position in the editor.
 * @param line
 *      The line in the editor.
 * @param modifiers
 *      The keyboard modifiers.
 */
void MainWindow::handleDoubleClicksInEditor(int position, int line, int modifiers)
{
    (void)position;
    const bool ctrl = (modifiers & QsciScintillaBase::SCMOD_CTRL) != 0;

    if(ui->documentsTabWidget->currentWidget() && ui->documentsTabWidget->currentWidget()->layout() && ctrl)
    {
        m_indicatorClickTimer.stop();

        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        QString text = textEditor->wordAtPosition(m_lastIndicatorClickPosition, true);


        bool isLocalVariable = false;
        QString searchString = textEditor->getContextString(line);
        if(!searchString.isEmpty())
        {
            int index = 0;
            if(text.startsWith("this."))
            {
                //Remove this.
                text.remove(0, 5);
                index = searchString.lastIndexOf("::");
                if(index != -1)
                {
                    searchString.remove(index, searchString.length() - index);
                }
            }
            searchString = searchString.replace("::", ".");


            do
            {
               //Check the local variables.
               isLocalVariable = checkIfElementsInOutlineTree(searchString + "." + text);

               if(isLocalVariable)
               {
                   searchString += "." + text;
               }
               else
               {
                   index = searchString.lastIndexOf(".");
                   if(index != -1)
                   {
                       searchString.remove(index, searchString.length() - index);
                   }
               }

            }while((index != -1) && !isLocalVariable);

        }

        if(!isLocalVariable)
        {
            searchString = text;
        }

        //Iterate over all elements in the scripts outline.
        QTreeWidgetItemIterator iter(ui->outlineTreeWidget);
        while (*iter)
        {
            bool isOk = false;
            ParsedEntry* entry  = (ParsedEntry*)(*iter)->data(0, PARSED_ENTRY).toULongLong(&isOk);
            if(entry)
            {
                //The double clicked word is in the scripts outline.
                if (entry->completeName == searchString)
                {
                    functionListDoubleClicked((*iter), 0);
                  break;
                }
                else
                {
                    QTreeWidgetItemIterator iter(ui->uiTreeWidget);
                    while (*iter)
                    {
                        bool isOk = false;
                        ParsedUiObject* entry  = (ParsedUiObject*)(*iter)->data(0, PARSED_ENTRY).toULongLong(&isOk);

                      if ("UI_" + entry->objectName == searchString)
                      {
                          uiViewDoubleClicked((*iter), 0);
                          break;
                      }

                      ++iter;
                    }

                }
            }

          ++iter;
        }
    }
}

/**
 * Adds a tab.
 *
 * @param script
 *      The script which shall be added.
 * @param setTabIndex
 *      If the script is already loaded, the corresponding tab is activates if setTabIndex is true.
 * @return
 *      True if a tab has been added
 */
bool MainWindow::addTab(QString script, bool setTabIndex)
{
    int index = -1;
    bool tabAdded = false;

    if(!checkIfDocumentAlreadyLoaded(script, index))
    {
        tabAdded = true;

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

        connect(textEditor, SIGNAL(modificationChanged(bool)), this, SLOT(modificationChangedSlot()));

        connect(textEditor,SIGNAL(SCN_DOUBLECLICK(int,int,int)),
                 this,SLOT(handleDoubleClicksInEditor(int,int,int)));
        connect(textEditor, SIGNAL(indicatorClicked(int,int,Qt::KeyboardModifiers)), this,
                SLOT(indicatorClickedSlot(int,int,Qt::KeyboardModifiers)));


        if(!script.isEmpty())
        {
            if(!loadFile(script))
            {
                ui->documentsTabWidget->removeTab(ui->documentsTabWidget->currentIndex());

                if(ui->documentsTabWidget->count() == 0)
                {
                    addTab("", false);
                }
                else
                {
                    tabAdded = false;
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
            clearCurrentIndicator();
            ui->documentsTabWidget->setCurrentIndex(index);
        }
    }

    return tabAdded;
}


/**
 * Saves the expanded state of all tree widget elements.
 *
 * @param parent
 *      The parent tree widget item.
 * @param expandMap
 *      The map in which the expanded states are saved.
 * @param key
 *      The key of the parent tree widget in the expand map.
 */
static void saveExpandedState(QTreeWidgetItem* parent, QMap<QString, bool>& expandMap, QString key)
{
    for(int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem* subEl = parent->child(i);
        expandMap[key + ":" + subEl->text(0)] = subEl->isExpanded();


        if(subEl->childCount() > 0)
        {
            saveExpandedState(subEl, expandMap, key + subEl->text(0));
        }
    }
}

/**
 * Restores the expanded state of all tree widget elements.
 *
 * @parm treeWidget
 *      The tree widget.
 * @param parent
 *      The parent tree widget item.
 * @param expandMap
 *      The map in which the expanded states are saved.
 * @param key
 *      The key of the parent tree widget in the expand map.
 */
static void restoreExpandedState(QTreeWidget *treeWidget, QTreeWidgetItem* parent, QMap<QString, bool>& expandMap, QString key)
{
    for(int i = 0; i < parent->childCount(); i++)
    {
        QTreeWidgetItem* subEl = parent->child(i);
        if(expandMap.contains(key + ":" + subEl->text(0)))
        {
            if(expandMap[key + ":" + subEl->text(0)])
            {
                treeWidget->expandItem(subEl);
            }
            else
            {
                treeWidget->collapseItem(subEl);
            }
        }

        if(subEl->childCount() > 0)
        {
            restoreExpandedState(treeWidget, subEl, expandMap, key + subEl->text(0));
        }
    }
}

/**
 * Is called if the user moves a tab.
 * @param from
 *      The old tab index.
 * @param to
 *      The new tab index.
 */
void MainWindow::tabMoved(int from, int to)
{
    int treeIndexFrom = -1;
    int treeIndexTo = -1;


    QTreeWidgetItem* root = ui->outlineTreeWidget->invisibleRootItem();

    SingleDocument* textEditorFrom = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(from)->layout()->itemAt(0)->widget());
    SingleDocument* textEditorTo = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(to)->layout()->itemAt(0)->widget());

    for(int i = 0; i < root->childCount(); i++)
    {
        QTreeWidgetItem* item = ui->outlineTreeWidget->topLevelItem(i);
        if(textEditorFrom->getDocumentName() == item->toolTip(0))
        {
            treeIndexFrom = i;
        }

        if(textEditorTo->getDocumentName() == item->toolTip(0))
        {
            treeIndexTo = i;
        }
    }

    if((treeIndexFrom != -1) && (treeIndexTo != -1))
    {
        //Save the expanded state of all tree widget elements.
        QMap<QString, bool> expandMap;
        saveExpandedState(root, expandMap, "");

        QTreeWidgetItem* item = ui->outlineTreeWidget->takeTopLevelItem(treeIndexFrom);
        ui->outlineTreeWidget->insertTopLevelItem(treeIndexTo, item);

        //Restore the expanded state of all tree widget elements.
        restoreExpandedState(ui->outlineTreeWidget, root, expandMap, "");
    }

    QTreeWidgetItemIterator it(ui->outlineTreeWidget);
    while (*it)
    {
        bool isOk = false;
        ParsedEntry* entry  = (ParsedEntry*)(*it)->data(0, PARSED_ENTRY).toULongLong(&isOk);
        if(entry != 0)
        {
            if(entry->tabIndex == from)
            {
                entry->tabIndex = to;
            }
            else if (to > from)
            {
                if ((entry->tabIndex <= to) && (entry->tabIndex > from))
                {
                    entry->tabIndex--;
                }
            }
            else if (to < from)
            {
                if ((entry->tabIndex >= to) && (entry->tabIndex < from))
                {
                    entry->tabIndex++;
                }
            }


        }
      ++it;
    }

}

/**
 * Is called if an info tab shall be closed.
 * @param index
 *      The tab index.
 */
void MainWindow::infoTabCloseRequestedSlot(int index)
{
    (void)index;

    //Hide the find/replace list.
    QList<int> list = ui->splitter2->sizes();
    double size = list[0] + list[1];
    list[1] = 0;
    list[0] = static_cast<int>(size);
    ui->splitter2->setSizes(list);
}

/**
 * Is called if a documents tab shall be closed.
 * @param index
 *      The tab index.
 */
void MainWindow::documentsTabCloseRequestedSlot(int index)
{
    if(maybeSave(index))
    {
        removeFileLock(index);

        QWidget* tab = ui->documentsTabWidget->widget(index);
        ui->documentsTabWidget->removeTab(index);
        delete tab;

        if(ui->documentsTabWidget->count() == 0)
        {
            addTab("", true);
        }

        for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
            textEditor->setFileMustBeParsed(true);
        }
        m_showParseError = true;
        m_parseTimer.start(200);
    }

    clearOutlineWindow(index);
    setStateLoadAllIncludedScriptsButton();

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

        textEditor->setFocus();
    }

    setStateLoadAllIncludedScriptsButton();
}


/**
 * Sets the state of the load all scripts button.
 */
void MainWindow::setStateLoadAllIncludedScriptsButton(void)
{
    bool containsIncludedScripts = false;
    for(qint32 index = 0; index < ui->documentsTabWidget->count(); index++)
    {
        if(ui->documentsTabWidget->widget(index) && ui->documentsTabWidget->widget(index)->layout())
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());
            QMap<QString, bool> scripts = getAllIncludedScripts(index);
            QMap<QString, bool>::iterator iter;
            for (iter = scripts.begin(); iter != scripts.end(); ++iter)
            {
                QString fileName;
                if(iter.value() == true)
                {//Relative path.

                    fileName = QFileInfo(textEditor->getDocumentName()).absolutePath() + "/" + iter.key();
                }
                else
                {
                    fileName = iter.key();
                }
                int tmpIndex;
                if(!checkIfDocumentAlreadyLoaded(fileName, tmpIndex))
                {
                    containsIncludedScripts = true;
                }
            }
        }
    }
    ui->actionOpenAllIncludedScripts->setEnabled(containsIncludedScripts);
}

/**
 * Execute the go to line.
 */
void MainWindow::executeGoToLineSlot(int code)
{
    if(code == 1)
    {//OK pressed.

        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        textEditor->goToLine( m_goToLineDialog.intValue() - 1);
    }
}

/**
 * Go to line action slot.
 */
void MainWindow::goToLineSlot()
{
    m_goToLineDialog.show();
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
        m_showParseError = true;
        m_parseTimer.start(200);
    }
    else
    {
        QMessageBox::warning(this, tr("Script Editor"),
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
    textEditor->removeUndlineFromWordWhichCanBeClicked();
}

/**
 * Copy action slot.
 */
void MainWindow::copySlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->copy();
    textEditor->removeUndlineFromWordWhichCanBeClicked();
}

/**
 * Paste action slot.
 */
void MainWindow::pasteSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
    textEditor->paste();
    textEditor->removeUndlineFromWordWhichCanBeClicked();
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
 * Close document action slot.
 */
void MainWindow::closeDocumentSlot()
{
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(ui->documentsTabWidget->currentIndex())->layout()->itemAt(0)->widget());
    if(!(textEditor->getDocumentName().isEmpty() && !textEditor->isModified()))
    {
        documentsTabCloseRequestedSlot(ui->documentsTabWidget->currentIndex());
    }
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
    int currentIndex = ui->documentsTabWidget->currentIndex();
    bool fileLoaded = false;

    do
    {
        fileLoaded = false;

        for(qint32 index = 0; index < ui->documentsTabWidget->count(); index++)
        {
            QMap<QString, bool> scripts = getAllIncludedScripts(index);
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());
            QMap<QString, bool>::iterator iter;
            for (iter = scripts.begin(); iter != scripts.end(); ++iter)
            {
                QString fileName;
                if(iter.value() == true)
                {//Relative path.

                    fileName = QFileInfo(textEditor->getDocumentName()).absolutePath() + "/" + iter.key();
                }
                else
                {
                    fileName = iter.key();
                }

                if(addTab(fileName, true))
                {
                    fileLoaded = true;
                }
            }
        }
    }while(fileLoaded);
    clearCurrentIndicator();
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
 * Is called by m_showEventTimer.
 */
void MainWindow::showEventTimerSlot()
{
    m_showEventTimer.stop();

    QList<int> elSizes = ui->splitter->sizes();
    if(elSizes[1] > 50)
    {
        elSizes[0] += elSizes[1] - 50;
        elSizes[1] = 50;
    }
    ui->splitter->setSizes(elSizes);


    readSettings();

    m_checkForFileChangesTimer.start(2000);

    parseTimeout(false);

    qApp->installEventFilter(this);

}

/**
 * Show event.
 * @param event
 *      The show event.
 */
void MainWindow::showEvent(QShowEvent *event)
{
    event->accept();
    m_showEventTimer.start(100);
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

    if(ui->documentsTabWidget->widget(tabIndex) && ui->documentsTabWidget->widget(tabIndex)->layout())
    {
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
    if(textEditor->hasSelectedText() && (textEditor->selectedText() == findText))
    {
        textEditor->replaceSelectedText(replaceText);
        (void)findTextInDocument(findText);
    }
    else
    {

        if(!findTextInDocument(findText))
        {
            QMessageBox::information(this, tr("Script Editor"),
                                     QString("Can't find %1").arg(findText));
        }
    }
}

/**
 * Is called if the replace all button the the find dialog has been clicked.
 */
void MainWindow::replaceAllButtonSlot()
{
    findReplaceAllButtonSlot(true);
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
    if(!findTextInDocument(findText))
    {
        QMessageBox::information(this, tr("Script Editor"),
                             QString("Can't find %1").arg(findText));
    }
}



/**
 * Is called if the find all or the replace all button in the find dialog has been clicked.
 */
void MainWindow::findReplaceAllButtonSlot(bool replace)
{
    QString findText = m_findDialog->ui->findWhatComboBox->currentText();
    QString replaceText =m_findDialog->ui->replaceComboBox->currentText();
    addStringToTheSearchStringList(findText);
    QList<int> modifiedDocuments;
    int oldTabIndex = ui->documentsTabWidget->currentIndex();

    if(replace)
    {
        addStringToTheReplaceList(replaceText);
    }

    ui->findResults->clear();
    int counter = 0;

    //Find/replace in all documents.
    for(qint32 i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());
        QTreeWidgetItem* fileElement = NULL;
        int foundLine = 0;
        int column = 0;
        int oldCursorLine, oldCursorIndex;
        textEditor->getCursorPosition(&oldCursorLine, &oldCursorIndex);

        //Find all occurrences.
        while(textEditor->findFirst(findText, false, m_findDialog->ui->matchCaseCheckBox->isChecked(),
                                                 m_findDialog->ui->matchWholeWordCheckBox->isChecked(), false, true, foundLine
                                    ,column, false))
        {

            textEditor->getCursorPosition(&foundLine, &column);
            if(!fileElement)
            {
                //Add the entry for the document/file.
                fileElement = new QTreeWidgetItem(ui->findResults->invisibleRootItem());
                ParsedEntry* dummyEntry = new ParsedEntry();
                dummyEntry->type = PARSED_ENTRY_TYPE_FILE;
                dummyEntry->tabIndex = i;

                fileElement->setData(0, PARSED_ENTRY, (quint64)dummyEntry);
                fileElement->setText(0, ui->documentsTabWidget->tabText(i));
                fileElement->setToolTip(0, textEditor->getDocumentName());
                ui->findResults->invisibleRootItem()->addChild(fileElement);
                ui->findResults->expandItem(fileElement);
                if(ui->documentsTabWidget->tabText(i).endsWith(".ui"))
                {
                    fileElement->setIcon(0, QIcon(":/images/ui16.png"));
                }
                else
                {
                    fileElement->setIcon(0, QIcon(":/images/document.png"));
                }
            }

            //Add the entry for the found/replaced text.
            QTreeWidgetItem* el = new QTreeWidgetItem(fileElement);
            QString textInTreeWidget = QString("Line %1:  %2").arg(foundLine).arg(textEditor->text(foundLine));
            textInTreeWidget.replace("\r", "");
            textInTreeWidget.replace("\n", "");

            ParsedEntry* tmpEntry = new ParsedEntry();
            tmpEntry->name = findText;
            tmpEntry->line = foundLine;
            tmpEntry->column = column - findText.length();
            if(tmpEntry->column > 0)
            {
                tmpEntry->column--;
            }
            tmpEntry->tabIndex = i;
            tmpEntry->findWholeWord = m_findDialog->ui->matchWholeWordCheckBox->isChecked();
            tmpEntry->findWithCase = m_findDialog->ui->matchCaseCheckBox->isChecked();

            el->setData(0, PARSED_ENTRY, (quint64)tmpEntry);
            fileElement->addChild(el);
            el->setText(0, textInTreeWidget);
            el->setToolTip(0, textInTreeWidget);
            counter++;

            if(replace)
            {
                //Replace the found text.
                textEditor->replaceSelectedText(replaceText);
                tmpEntry->name = replaceText;

                if(!modifiedDocuments.contains(i))
                {
                    modifiedDocuments.append(i);
                }
            }

        }

        textEditor->setCursorPosition(oldCursorLine, oldCursorIndex);
    }
    QString textInTab;
    if(replace)
    {
        if(counter == 1)
        {
            textInTab = "Replace all result - %1 occurrence replaced";
        }
        else if((counter == 0) || (counter > 1))
        {
            textInTab = "Replace all result - %1 occurrences replaced";
        }


        for(auto index : modifiedDocuments)
        {
            documentWasModified(index);
        }

        ui->documentsTabWidget->setCurrentIndex(oldTabIndex);
        clearCurrentIndicator();
    }
    else
    {
        if(counter == 1)
        {
            textInTab = "Find all result - %1 occurrence found";
        }
        else if((counter == 0) || (counter > 1))
        {
            textInTab = "Find all result - %1 occurrences found";
        }

    }

    ui->infoTabWidget->setTabText(0, textInTab.arg(counter));

    if(counter > 0)
    {
        //Show the find/replace list.
        QList<int> list = ui->splitter2->sizes();
        double size = list[0] + list[1];
        if(list[1] < (size * 0.1))
        {
            list[1] = static_cast<int>(size * 0.2);
            list[0] = static_cast<int>(size - list[1]);
            ui->splitter2->setSizes(list);
        }
    }
    else
    {
        QMessageBox::information(this, tr("Script Editor"), QString("Can't find %1").arg(findText));
    }
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
            textEditor->setLineNumberMarginFont(m_currentFont);
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
        textEditor->setLineNumberMarginFont(m_currentFont);

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
        textEditor->setLineNumberMarginFont(m_currentFont);
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

        //Check of entry is valid.
        if((entry != 0))
        {
            clearCurrentIndicator();

            int index = 0;
            if(!checkIfDocumentAlreadyLoaded(entry->uiFile, index))
            {//The ui file is not already loaded.

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
            {//The ui file is loaded.

                SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());

                if(!entry->objectName.isEmpty())
                {
                    //Serach for the clicked item/text.
                    if(textEditor->findFirst("name=\"" + entry->objectName, false, true, false, true, true, 0, 0, true, false))
                    {
                        int foundLine, column;
                        textEditor->getCursorPosition(&foundLine, &column);
                        textEditor->ensureLineVisible(foundLine);
                    }
                }

                ui->documentsTabWidget->setCurrentIndex(index);
                textEditor->setFocus();
            }
        }
    }

}

/**
 * Is called if the user double clicks on the find result list.
 * @param item
 *      The clicked item.
 */
void MainWindow::findResultsDoubleClicked(QTreeWidgetItem* item, int column)
{
    (void)column;

    if(item)
    {
        bool isOk = false;
        ParsedEntry* entry  = (ParsedEntry*)item->data(0, PARSED_ENTRY).toULongLong(&isOk);

        //Check if entry is valid.
        if(entry != 0)

        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(entry->tabIndex)->layout()->itemAt(0)->widget());
            if(entry->type != PARSED_ENTRY_TYPE_FILE)
            {
                //Find the clicked item/text.
                if(textEditor->findFirst(entry->name, false, entry->findWithCase,
                                            entry->findWholeWord, true, true, entry->line, entry->column, true, false))
                {
                    int foundLine, column;
                    textEditor->getCursorPosition(&foundLine, &column);
                    textEditor->ensureLineVisible(foundLine);
                }
            }

            clearCurrentIndicator();
            ui->documentsTabWidget->setCurrentIndex(entry->tabIndex);
            textEditor->setFocus();
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

        //Check if entry is valid.
        if(entry != 0)
        {
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(entry->tabIndex)->layout()->itemAt(0)->widget());

            if(entry->type != PARSED_ENTRY_TYPE_FILE)
            {
                QString regEx;
                if(entry->type == PARSED_ENTRY_TYPE_FUNCTION)
                {
                    regEx = QString("function.*[ |/]%1.*[ |/](").arg(entry->name);

                }
                else if(entry->type == PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION)
                {
                    regEx = QString("%1*[ |/]=*[ |/]function").arg(entry->name);

                }
                else if((entry->type == PARSED_ENTRY_TYPE_MAP_VAR) ||
                        (entry->type == PARSED_ENTRY_TYPE_MAP_FUNC))
                {
                    regEx = QString("%1*[\" |/:]").arg(entry->name);

                }
                else if(entry->type == PARSED_ENTRY_TYPE_PROTOTYPE_FUNC)
                {
                    regEx = QString("prototype.%1").arg(entry->name);

                }
                else if(entry->type == PARSED_ENTRY_TYPE_CONST)
                {
                    regEx = QString("const.*[ |/]%1").arg(entry->name);

                }
                else
                {
                  regEx = QString("var.*[ |/]%1").arg(entry->name);
                }

                bool itemFound = true;

                //Find the clicked item/text.
                if(!textEditor->findFirst(regEx, true, true, false, true, true, entry->line, 0, true, false))
                {
                    //The item was not found. Search with a simple string (instead of regular expression).
                    itemFound = textEditor->findFirst(entry->name, false, true, false, true, true, entry->line, 0, true, false);
                }

                if(itemFound)
                {
                    //Goto the found item/text.
                    int foundLine, column;
                    textEditor->getCursorPosition(&foundLine, &column);
                    textEditor->ensureLineVisible(foundLine);

                    int startPos = textEditor->SendScintilla(QsciScintillaBase::SCI_GETSELECTIONSTART);
                    QString word = textEditor->selectedText();
                    int index = word.indexOf(entry->name);
                    startPos += index;
                    textEditor->setSelectionFromPosition(startPos, startPos + entry->name.length());
                }
            }
            clearCurrentIndicator();
            ui->documentsTabWidget->setCurrentIndex(entry->tabIndex);
            textEditor->setFocus();
        }
    }

}

/**
 * Creates a document title for a new document.
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
 * Is called if the modified property of a document has been changed.
 */
void MainWindow::modificationChangedSlot(void)
{
    documentWasModified(ui->documentsTabWidget->currentIndex());
}


/**
 * Checks if the current script file has been changed. If the files has been changed
 * it displays this in the window title.
 */
void MainWindow::documentWasModified(int index)
{
    SingleDocument* textEditor;

    if(index == -1)
    {
        textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        index = ui->documentsTabWidget->currentIndex();
    }
    else
    {
        textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(index)->layout()->itemAt(0)->widget());
    }
    textEditor->removeUndlineFromWordWhichCanBeClicked();
    setWindowModified(textEditor->isModified());
    textEditor->setMarginWidth(0, QString("00%1").arg(textEditor->lines()));
    textEditor->setFileMustBeParsed(true);

    QString shownName;
    if (textEditor->getDocumentName().isEmpty())
    {
        shownName = ui->documentsTabWidget->tabText(index);

    }
    else
    {
        shownName = strippedName(textEditor->getDocumentName());
    }
    if(textEditor->isModified())
    {
        if(!shownName.endsWith("*"))
        {
            ui->documentsTabWidget->setTabText(index, shownName + "*");
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
         ui->documentsTabWidget->setTabText(index, shownName);
    }

    m_showParseError = false;
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
    connect(ui->actionCloseDocument, SIGNAL(triggered()), this, SLOT(closeDocumentSlot()));
    connect(ui->actionOpenAllIncludedScripts, SIGNAL(triggered()), this, SLOT(openAllIncludedScriptsSlot()));
    connect(ui->actionSetFont, SIGNAL(triggered()), this, SLOT(setFont()));
    connect(ui->actionEditUi , SIGNAL(triggered()), this, SLOT(editUiButtonSlot()));
    connect(ui->actionReload , SIGNAL(triggered()), this, SLOT(reloadSlot()));
    connect(ui->actionGoToLine , SIGNAL(triggered()), this, SLOT(goToLineSlot()));

    ui->actionEditUi->setEnabled(false);

}

/**
 * Reads the editor settings.
 */
void MainWindow::readSettings()
{
    QSettings settings("ScriptCommunicator", QString("ScriptEditor_%1").arg(INTERNAL_VERSION));
    QList<int> list;
    double size;

    if(settings.contains("pos") && settings.contains("size") && settings.contains("mainSplitter") && settings.contains("fontFamily")
            && settings.contains("mainWindowState"))
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

        if(m_scriptsToLoadAfterStart.isEmpty())
        {
            addTab("", false);
        }
        else
        {
            for(auto el : m_scriptsToLoadAfterStart)
            {
                addTab(el, true);
            }
        }

        restoreState(settings.value("mainWindowState", QByteArray()).toByteArray());

        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->currentWidget()->layout()->itemAt(0)->widget());
        textEditor->lexer()->readSettings(settings);
        textEditor->setFocus();
    }
    else
    {
        if(m_scriptsToLoadAfterStart.isEmpty())
        {
            addTab("", false);
        }
        else
        {
            for(auto el : m_scriptsToLoadAfterStart)
            {
                addTab(el, true);
            }
        }

        list = ui->splitter->sizes();
        size = list[0] + list[1];
        list[0] = static_cast<int>(size * 0.2);
        list[1] = static_cast<int>(size - list[0]);
        ui->splitter->setSizes(list);
    }

    //Hide the find/replace list.
    list = ui->splitter2->sizes();
    size = list[0] + list[1];
    list[1] = 0;
    list[0] = static_cast<int>(size);
    ui->splitter2->setSizes(list);


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
    settings.setValue("mainWindowState", saveState());

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
void MainWindow::clearOutlineWindow(int tabIndex)
{
    QTreeWidgetItemIterator it(ui->outlineTreeWidget);
    while (*it)
    {
        bool isOk = false;
        ParsedEntry* entry  = (ParsedEntry*)(*it)->data(0, PARSED_ENTRY).toULongLong(&isOk);
        if((entry != 0) && (tabIndex == entry->tabIndex))
        {
            (*it)->setData(0, PARSED_ENTRY, (quint64)0);
            delete entry;
        }
      ++it;
    }

    //Remove the file item.
    ui->outlineTreeWidget->takeTopLevelItem(tabIndex);
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
        //Add the element for the current ui files.
        QTreeWidgetItem* fileElement = new QTreeWidgetItem(root);
        ParsedUiObject* tmpEntry = new ParsedUiObject();
        tmpEntry->objectName = "";
        tmpEntry->uiFile = iter.key();
        fileElement->setData(0, PARSED_ENTRY, (quint64)tmpEntry);
        fileElement->setText(0, strippedName(iter.key()));
        fileElement->setToolTip(0, iter.key());
        fileElement->setIcon(0, QIcon(":/images/ui16.png"));
        root->addChild(fileElement);

        if(firstFile)
        {
            fileElement->setSelected(true);
            firstFile = false;
        }


        //Add all gui elements from the current ui file.
        for(auto el : iter.value())
        {
            QTreeWidgetItem* funcElement = new QTreeWidgetItem(fileElement);
            QString textInTreeWidget = "UI_" + el;
            tmpEntry = new ParsedUiObject();
            tmpEntry->objectName = el;
            tmpEntry->uiFile = iter.key();
            funcElement->setData(0, PARSED_ENTRY, (quint64)tmpEntry);
            funcElement->setIcon(0, QIcon(":/images/var.png"));
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
 * Inserts a subelement into the script view.
 * @param parent
 *      The parent element.
 * @param parsedEntries
 *      The parsed sub entries.
 */
bool MainWindow::inserSubElementsToScriptView(QTreeWidgetItem* parent, QVector<ParsedEntry> parsedEntries, QString parentName)
{
    bool hasError = false;
    for(auto el : parsedEntries)
    {
        SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(el.tabIndex)->layout()->itemAt(0)->widget());

        if(el.type == PARSED_ENTRY_TYPE_PARSE_ERROR)
        {//Do nothing.
            hasError = true;

        }
        else
        {
            QTreeWidgetItem* funcElement = new QTreeWidgetItem(parent);
            QString textInTreeWidget = el.name;
            ParsedEntry* tmpEntry = new ParsedEntry();
            *tmpEntry = el;
            tmpEntry->completeName = (parentName.isEmpty()) ? tmpEntry->name : parentName + "." + tmpEntry->name;
            funcElement->setData(0, PARSED_ENTRY, (quint64)tmpEntry);
            parent->addChild(funcElement);

            if((el.type == PARSED_ENTRY_TYPE_FUNCTION) || (el.type == PARSED_ENTRY_TYPE_CLASS_FUNCTION)
                    || (el.type == PARSED_ENTRY_TYPE_MAP_FUNC) || (el.type == PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION) ||
                    (el.type == PARSED_ENTRY_TYPE_PROTOTYPE_FUNC))
            {
                textEditor->addFunction(el);

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
                funcElement->setIcon(0, QIcon(":/images/func.png"));
            }
            else
            {
                funcElement->setIcon(0, QIcon(":/images/var.png"));

                if(!el.valueType.isEmpty())
                {
                    textInTreeWidget += ": " + el.valueType;
                }
            }

            funcElement->setText(0, textInTreeWidget);
            funcElement->setToolTip(0, textInTreeWidget);

            if(!tmpEntry->subElements.isEmpty())
            {
                //Add all sub elements.
                hasError = inserSubElementsToScriptView(funcElement, tmpEntry->subElements, tmpEntry->completeName);
            }

            for(int i = 0; i < funcElement->columnCount(); i++)
            {
                funcElement->sortChildren(i, Qt::AscendingOrder);
            }
        }

    }

    return hasError;
}


/**
 * Converts the parsed entries to a string.
 *
 * @param parsedEntries
 *      The parsed entries.
 * @param currentCompleteTreeString
 *      The created string.
 */
static void parsedEntryToString(const QVector<ParsedEntry>& parsedEntries,
                                QString& currentCompleteTreeString)
{
    for(auto el : parsedEntries)
    {
        currentCompleteTreeString += el.name + QString("%1,%2").arg(el.type).arg(el.line);
        if((el.type == PARSED_ENTRY_TYPE_FUNCTION) || (el.type == PARSED_ENTRY_TYPE_CLASS_FUNCTION)
                || (el.type == PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION))
        {
            //Add all parameters to the string.
            for(auto param : el.params)
            {
                currentCompleteTreeString += param;
            }
        }

        if(!el.subElements.isEmpty())
        {
            //Add all sub elements to the string.
            parsedEntryToString(el.subElements,currentCompleteTreeString);
        }

        //Add the elemement name to the string.
        currentCompleteTreeString += el.name;

        //Add the value type to the string.
        currentCompleteTreeString += el.valueType;

        //Add the start/end line to the string.
        currentCompleteTreeString += QString("%1%2").arg(el.line).arg(el.endLine);
    }
}

/**
 * Inserts a file element in the scripts outline.
 *
 * @param tabIndex
 *      The index of the tab to which the file element belongs to.
 */
void MainWindow::insertFileElementForTabIndex(int tabIndex, QColor textColor)
{
    QTreeWidgetItem* root = ui->outlineTreeWidget->invisibleRootItem();
    SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(tabIndex)->layout()->itemAt(0)->widget());

    //Add the element for the current documents tab.
    QTreeWidgetItem* fileElement = new QTreeWidgetItem();
    ParsedEntry* dummyEntry = new ParsedEntry();
    dummyEntry->type = PARSED_ENTRY_TYPE_FILE;
    dummyEntry->tabIndex = tabIndex;

    fileElement->setData(0, PARSED_ENTRY, (quint64)dummyEntry);

    QString textData = ui->documentsTabWidget->tabText(tabIndex);
    if(textData.endsWith("*"))
    {
        //Remove the last "*".
        textData.remove(textData.length() - 1, 1);
    }
    fileElement->setText(0, textData);
    fileElement->setToolTip(0, textEditor->getDocumentName());
    fileElement->setIcon(0, QIcon(":/images/document.png"));
    fileElement->setTextColor(0, textColor);

    root->insertChild(tabIndex, fileElement);
    ui->outlineTreeWidget->expandItem(fileElement);
    textEditor->clearAllFunctions();
}

/**
 * Inserts all parsed elements in the  script view and displays all parse errors (annotations).
 *
 * @param parsedEntries
 *      The parsed entries.
 */
bool MainWindow::insertFillScriptViewAndDisplayErrors(QMap<int,QVector<ParsedEntry>>& parsedEntries)
{
    bool hasError = false;
    static QMap<int, QString> savedCompleteTreeStrings;
    QMap<int, QString> currentCompleteTreeStrings;
    QTreeWidgetItem* root = ui->outlineTreeWidget->invisibleRootItem();
    QMap<QString, bool> expandMap;

    if(root->childCount() != savedCompleteTreeStrings.size())
    {
        savedCompleteTreeStrings.clear();
    }

    //Save the expanded state of all tree widget elements.
    saveExpandedState(root, expandMap, "");

    //Insert for all tabs a file element.
    for(int i = 0; i < ui->documentsTabWidget->count(); i++)
    {
        if(!parsedEntries.contains(i) && m_showParseError)
        {
            clearOutlineWindow(i);
        }

        if(ui->outlineTreeWidget->topLevelItem(i) == 0)
        {
            insertFileElementForTabIndex(i, QColor(0,0,0));
        }
        else
        {
            QTreeWidgetItem* fileElement = ui->outlineTreeWidget->topLevelItem(i);
            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(i)->layout()->itemAt(0)->widget());


            QString textData = ui->documentsTabWidget->tabText(i);
            if(textData.endsWith("*"))
            {
                //Remove the last "*".
                textData.remove(textData.length() - 1, 1);
            }
            fileElement->setText(0, textData);
            fileElement->setToolTip(0, textEditor->getDocumentName());
        }
    }

    //Add all parsed entries to the script outline.
    QMap<int,QVector<ParsedEntry>>::const_iterator  iter = parsedEntries.constBegin();
    while (iter != parsedEntries.constEnd())
    {
        //Add all elements for the current documents tab.
        if(ui->documentsTabWidget->widget(iter.key()))
        {

            SingleDocument* textEditor = static_cast<SingleDocument*>(ui->documentsTabWidget->widget(iter.key())->layout()->itemAt(0)->widget());


            //Clear all annotations.
            textEditor->clearAnnotations();

            if(root->child(iter.key()))
            {
                root->child(iter.key())->setTextColor(0, QColor(0,0,0));
            }

            if(iter.value().isEmpty())
            {
                //Do nothing.

            }
            else if(iter.value()[0].type != PARSED_ENTRY_TYPE_PARSE_ERROR)
            {

                if(!currentCompleteTreeStrings.contains(iter.key()))
                {
                    currentCompleteTreeStrings[iter.key()] = "";
                }

                if(!savedCompleteTreeStrings.contains(iter.key()))
                {
                    savedCompleteTreeStrings[iter.key()] = "";
                }


                currentCompleteTreeStrings[iter.key()] += textEditor->getDocumentName() + ui->documentsTabWidget->tabText(iter.key());
                parsedEntryToString(iter.value(), currentCompleteTreeStrings[iter.key()]);

                if(savedCompleteTreeStrings[iter.key()] == currentCompleteTreeStrings[iter.key()])
                {//The tree wigdet content has not changed.
                    iter++;
                    continue;
                }


                savedCompleteTreeStrings[iter.key()] = currentCompleteTreeStrings[iter.key()];

                clearOutlineWindow(iter.key());

                insertFileElementForTabIndex(iter.key(), QColor(0,0,0));

                QTreeWidgetItem* fileElement = ui->outlineTreeWidget->topLevelItem(iter.key());

                if(inserSubElementsToScriptView(fileElement, iter.value(), ""))
                {
                    fileElement->setTextColor(0, QColor(255,0,0));
                }

                for(int i = 0; i < fileElement->columnCount(); i++)
                {
                    fileElement->sortChildren(i, Qt::AscendingOrder);
                }

            }
            else
            {

                hasError = true;
                savedCompleteTreeStrings[iter.key()] = "";

                if(m_showParseError)
                {

                    QsciStyle myStyle(-1,"Annotation",QColor(255,0,0),QColor(255,150,150),m_currentFont,true);
                    textEditor->annotate(iter.value()[0].line - 1, iter.value()[0].name,myStyle);

                    if(root->child(iter.key()))
                    {
                        root->child(iter.key())->setTextColor(0, QColor(255,0,0));
                    }
                    else
                    {
                        //Add the element for the current documents tab.
                        insertFileElementForTabIndex(iter.key(), QColor(255,0,0));
                    }
                }

            }


        }

        iter++;
    }

    //Remove all ui files from the script outline.
    bool elementDeleted = false;
    do
    {
        elementDeleted = false;

        for(int i = 0; i < root->childCount(); i++)
        {
            QTreeWidgetItem* item = ui->outlineTreeWidget->topLevelItem(i);
            if(item->text(0).endsWith(".ui"))
            {
                //Remove and delete the item.
                ui->outlineTreeWidget->takeTopLevelItem(i);
                delete item;
                elementDeleted = true;
                break;
            }
        }
    }while(elementDeleted);

    //Restore the expanded state of all tree widget elements.
    restoreExpandedState(ui->outlineTreeWidget, root, expandMap, "");

    return hasError;
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
        QMessageBox::warning(this, tr("Script Editor"),
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

    //Parse all files.
    m_showParseError = true;
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
        QMessageBox::warning(this, tr("Script Editor"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return false;
    }

    //Add the UTF-8 bom.
    const char bom[] = {(char)0xEF, (char)0xBB, (char)0xBF};
    file.write(bom, 3);

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

    //Parse all files.
    m_showParseError = true;
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
        ui->actionReload->setEnabled(false);
    }
    else
    {
        tabShownName = strippedName(fileName);
        windowShownName = fileName;
        ui->actionReload->setEnabled(true);
    }

    setStateLoadAllIncludedScriptsButton();

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
