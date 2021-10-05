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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "findDialog.h"
#include <QListWidget>
#include <QSplitter>
#include <QShortcut>
#include <QFileInfo>
#include "singledocument.h"
#include <QTreeWidgetItem>
#include "parseThread.h"
#include <QInputDialog>


namespace Ui {
class MainWindow;
}



///Main window class.
class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class QsciScintilla;
    friend class SingleDocument;

public:
    MainWindow(QStringList scripts);

    ~MainWindow();

    ///Adds a tab.
    bool addTab(QString script, bool setTabIndex);

    ///Returns the folder ich which the ScriptEditor files
    QString getScriptEditorFilesFolder(void);

    ///Starts an other instance of ScriptEditor.
    void startScriptEditor(QStringList arguments);

    ///Returns all included scripts from a script file.
    QMap<QString, bool> getAllIncludedScripts(int tabIndex);

    ///Finds ans marks a text in the current script file.
    bool findTextInDocument(QString findText);

    ///Adds a string to the replace string list (m_lastReplaceString).
    void addStringToTheReplaceList(QString replaceText);

    ///Adds a string to the search string list (m_lastSearchString).
    void addStringToTheSearchStringList(QString findText);

    ///Returns true if the document in fileName has already been loaded.
    bool checkIfDocumentAlreadyLoaded(QString fileName, int &index);

    ///The index in the data map (tree widget) of the parsed entries pointer (ParsedEntry).
    static const quint32 PARSED_ENTRY = Qt::UserRole + 3;

    ///The internal script editor version.
    static const quint32 INTERNAL_VERSION = 2;

    ///Returns the corresponding ui file for a script.
    static QString getTheCorrespondingUiFile(QString scriptFile);

    ///Clears the current indicator.
    void clearCurrentIndicator(void);

signals:
    ///Is emitted if the parsing of the source files shall be started.
    void parseSignal(QMap<QString, QString> loadedUiFile,QMap<int, QString> loadedScripts, QMap<int, QString> loadedScriptsIndex,
                     bool loadedFileChanged, bool parseOnlyUIFiles);

protected:
    ///Close event.
    void closeEvent(QCloseEvent *event);

    ///Show event.
    void showEvent(QShowEvent *event);

    ///Drag enter event.
    void dragEnterEvent(QDragEnterEvent *event);

    ///Drop event.
    void dropEvent(QDropEvent *event);


private slots:

    ///Is called if the user double clicks a postion in the editor.
    void handleDoubleClicksInEditor(int position, int line, int modifiers);

    ///Is called if the parsing is finished.
    ///Note: autoCompletionApiFiles contains the auto-completion entries for all parsed files.
    void parsingFinishedSlot(QMap<QString, QStringList> autoCompletionEntries, QMap<QString, QStringList> autoCompletionApiFiles,
                             QMap<QString, QStringList> parsedUiObjects, QMap<int, QVector<ParsedEntry> > parsedEntries, bool doneParsing, bool parseOnlyUIFiles);

    ///Opens a new file.
    void open();

    ///Opens a set font dialog.
    void setFont();

    ///Saves the current script file (under the current file name).
    bool save();

    ///Saves the current script file (under a new file name).
    bool saveAs();

    ///Is called if the find button the the find dialog has been clicked.
    void findButtonSlot();

    ///Is called if the find all or the replace all button in the find dialog has been clicked.
    void findReplaceAllButtonSlot(bool replace=false);

    ///Is called if the edit ui button has been clicked.
    void editUiButtonSlot();

    ///Is called if the replace button the the find dialog has been clicked.
    void replaceButtonSlot();

    ///Is called if the replace all button the the find dialog has been clicked.
    void replaceAllButtonSlot();

    ///Opens the find dialog.
    void find();

    ///Is called if the modified property of a document has been changed.
    void modificationChangedSlot(void);

    ///Checks if the current script file has been changed. If the files has been changed
    ///it displays this in the window title.
    void documentWasModified(int index=-1);

    ///Is called if the user double clicks on the ui view.
    void uiViewDoubleClicked(QTreeWidgetItem* item, int column);

    ///Is called if the user double clicks on the find result list.
    void findResultsDoubleClicked(QTreeWidgetItem* item, int column);

    ///Is called if the user double clicks on the function list.
    void functionListDoubleClicked(QTreeWidgetItem*item, int column);

    ///Zooms out on the text by by making the base font size one point smaller
    ///and recalculating all font sizes.
    void zoomOutSlot();

    ///Zooms in on the text by by making the base font size one point larger
    ///and recalculating all font sizes.
    void zoomInSlot();

    ///Is called if current tab index has been changed.
    void tabIndexChangedSlot(int index);

    ///Is called if an info tab shall be closed.
    void infoTabCloseRequestedSlot(int index);

    ///Is called if the user moves a tab.
    void tabMoved(int from, int to);

    ///Is called if a documents tab shall be closed.
    void documentsTabCloseRequestedSlot(int index);

    ///Reload action slot.
    void reloadSlot();

    ///Execute the go to line.
    void executeGoToLineSlot(int code);

    ///Go to line action slot.
    void goToLineSlot();

    ///Cut action slot.
    void cutSlot();

    ///Copy action slot.
    void copySlot();

    ///Paste action slot.
    void pasteSlot();

    ///Undo action slot.
    void undoSlot();

    ///Redo action slot.
    void redoSlot();

    ///New action slot.
    void newSlot();

    ///Close document action slot.
    void closeDocumentSlot();

    ///Open all included action slot.
    void openAllIncludedScriptsSlot();

    ///Is call by m_checkForFileChangesTimer and checks for changes in the loaded files.
    void checkForFileChanges(void);

    ///Is called if the parse timer times out.
    void parseTimeout(bool parseOnlyUIFiles = false);

    ///Is called if the mouse move timer times out.
    void mouseMoveTimerSlot();

    ///Is called if an indicator is clicked.
    void indicatorClickedSlot(int line, int index, Qt::KeyboardModifiers modifier);

    ///Is called if an indicator is clicked.
    void indicatorClickTimerSlot();

    ///Is called by m_showEventTimer.
    void showEventTimerSlot();

    ///Is called if the use dark style menu is pressed.
    void useDarkStyleMenuPressedSlot(void);

    ///Is called if the set application font size menu is pressed.
    void setApplicationFontSizeSlot();

private:

    ///The user interface.
    Ui::MainWindow *ui;

    ///Sets the state of the load all scripts button.
    void setStateLoadAllIncludedScriptsButton(void);

    ///Removes the lock of a loaded script file.
    void removeFileLock(int index);

    ///Removes the saved info file.
    void removeSavedInfoFile(QString name);

    ///Initializes all actions.
    void initActions();

    ///Reads the editor settings.
    void readSettings();

    ///Writes/saves the editor settings.
    void writeSettings();

    ///Displays a dialog if the current script has been changed.
    bool maybeSave(int index);

    ///Returns true if outlineTreeWidget contains an element with the given name.
    bool checkIfElementsInOutlineTree(QString name);

    ///Loads a file.
    bool loadFile(const QString &fileName);

    ///Inserts all UI objects in the ui view
    void insertAllUiObjectsInUiView(QMap<QString, QStringList> parsedUiObjects);

    ///Inserts a subelement into the script view.
    bool inserSubElementsToScriptView(QTreeWidgetItem* parent, QVector<ParsedEntry> parsedEntries, QString parentName);

    ///Inserts a file element in the scripts outline.
    void insertFileElementForTabIndex(int tabIndex);

    ///Inserts all parsed elements in the  script view and displays all parse errors (annotations).
    bool insertFillScriptViewAndDisplayErrors(QMap<int, QVector<ParsedEntry> > &parsedEntries);

    ///Clears the outline window.
    void clearOutlineWindow(int tabIndex);

    ///Clears the ui window.
    void clearUiWindow(void);

    ///Starts the designer.
    void startDesigner(QString uiFile);

    ///Saves the current file.
    bool saveFile(const QString &fileName);

    ///Sets the current file (name).
    void setCurrentFile(const QString &fileName);

    ///Returns the tmp directory for a file.
    QString getTmpDirectory(QString fileName){return QFileInfo(fileName).path() +"/.tmp";}

    ///Returns the lock file name for a file.
    QString getLockFileName(QString fileName){return getTmpDirectory(fileName) + "/" + QFileInfo(fileName).fileName() + ".lock";}

    ///Returns the unsaved info file name for a file.
    QString getUnsavedInfoFileName(QString fileName){return getTmpDirectory(fileName) + "/" + QFileInfo(fileName).fileName() + ".unsaved";}

    ///Creates a document title for a new document.
    QString createNewDocumentTitle(void);

    ///Returns the current file name without the path.
    QString strippedName(const QString &fullFileName);

    ///The find dialog object.
    FindDialog* m_findDialog;

    ///This list contains the last search string.
    QVector<QString> m_lastSearchString;

    ///This list contains the last replace string.
    QVector<QString> m_lastReplaceString;

    ///Shortcut for finding text in the current script file.
    QShortcut* m_findShortcut;

    ///The parse timer.
    QTimer m_parseTimer;

    ///The parse thread.
    ParseThread* m_parseThread;

    ///The current font.
    QFont m_currentFont;

    ///True if the parse thread has finished.
    bool m_parsingFinished;

    ///The lock files for all open documents.
    QMap<QString, QFile*> m_lockFiles;

    ///The unsaved info files for all documents which have unsaved changes.
    QMap<QString, QFile*> m_unsavedInfoFiles;

    ///This timer checks for changes in the loades files.
    QTimer m_checkForFileChangesTimer;

    ///The last mouse move event.
    QMouseEvent m_lastMouseMoveEvent;

    ///This timer is started if a mouse move event occurs (calls mouseTimerSlot).
    QTimer m_mouseEventTimer;

    ///True if the ctrl key is pressed.
    bool m_ctrlIsPressed;

    ///This timer is started if an indicator is clicked. (calls indicatorClickTimerSlot)
    QTimer m_indicatorClickTimer;

    ///The position of the last click indicator event.
    long m_lastIndicatorClickPosition;

    ///The saved auto completion entries.
    QMap<QString, QStringList> m_autoCompletionEntries;

    ///True if parse error shall be displayed.
    bool m_showParseError;

    ///Scripts wich shalll be loaded after start (from command-line).
    QStringList m_scriptsToLoadAfterStart;

    ///The goto line dialog.
    QInputDialog m_goToLineDialog;

    ///This timer starts if the showEvent occurs.
    QTimer m_showEventTimer;

    ///True if the dark style shall be used.
    bool m_useDarkStyle;

    ///The application font size.
    int m_applicationFontSize;

};

///Return a main window pointer.
extern MainWindow* getMainWindow();

#endif
