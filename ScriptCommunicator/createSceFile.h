#ifndef CREATESCEFILE_H
#define CREATESCEFILE_H

#include <QMainWindow>
#include <QTableWidget>
#include <QProgressBar>

namespace Ui {
class CreateSceFile;
}

class CreateSceFile : public QMainWindow
{
    Q_OBJECT

public:
    explicit CreateSceFile(QWidget *parent, QTableWidget* scriptWindowScriptTable);
    ~CreateSceFile();

    ///Shows this window.
    void show(void);

    ///Is called if the send window has been resized.
    void resizeEvent(QResizeEvent * event);

    ///Returns the user interface pointer.
    Ui::CreateSceFile* getUI(void){return ui;}

    ///Returns m_configFileName.
    QString getConfigFileName(void){return m_configFileName;}

    ///Sets m_configFileName.
    void setConfigFileName(QString name){m_configFileName = name;}

    ///Loads the current config file (m_configFileName).
    void loadConfigFile(void);

    ///Returns true if the current config must be saved.
    bool configMustBeSaved();

    ///The index of sub dir columm in the files table.
    static const qint32 COLUMN_SUBDIR = 0;

    ///The index of type columm in thTe files table.
    static const qint32 COLUMN_TYPE = 1;

    ///The index of source columm in the files table.
    static const qint32 COLUMN_SOURCE = 2;

    ///Sets the window title.
    void setTitle(QString extraString);

signals:
    ///This signals is emitted if the global config (of the program) has to be saved.
    void configHasToBeSavedSignal(void);

public slots:

    ///Slot function for the save config menu.
    void saveConfigSlot();

    ///Slot function for the unload config menu.
    void unloadConfigSlot();

private slots:

    ///Select file button slot function.
    void selectFileSlot();

    ///Add folder button slot function.
    void addFolderSlot();

    ///Add file button slot function.
    void addFileSlot();

    ///Adds a row at the end of the files table.
    void addTableRow(QString folder, QString type, QString source);

    ///Remove file button slot function.
    void removeFileSlot();

    ///Add argument button slot function.
    void addArgumentSlot();

    ///File entry up button slot function.
    void fileEntryUpSlot();

    ///File entry down button slot function.
    void fileEntryDownSlot();

    ///Script table drop event function.
    void tableDropEventSlot(int row, int column, QStringList files);

    ///Arg entry up button slot function.
    void argEntryUpSlot();

    ///Arg entry down button slot function.
    void argEntryDownSlot();

    ///Remove argument button slot function.
    void removeArgumentSlot();

    ///Generate sce file menu slot function.
    void generateSlot(QStringList *copiedFilesOutput=0, QProgressBar* progress=0, bool* resultPointer=0);

    ///Generate scez file menu slot function.
    void generateCompressedFileSlot();

    ///The content of the sce file line edit has been changed.
    void sceFileTextChangedSlot(QString text);

    ////Is called if the content of a cell in the files table has been changed.
    void filesTableCellChangedSlot();

    ///Add all from script window button slot function.
    void addAllFromScriptWindow();

    ///Returns the file type of file.
    QString getFileTypeFromFile(QString file);

    ///Resizes all file table columns.
    void resizeTableColumnsSlot(void);

    ///Slot function for the load config menu.
    void loadConfigSlot();

    ///Slot function for the save as config menu.
    void saveConfigAsSlot();

    ///Is called if the value of one of the version combo boxes has been changed.
    void versionSpinboxValueChangedSlot(int value);

    ///The value of a type combo box has been changed.
    void typeTextChangedSlot(QString text);

private:

    ///Saves the current config.
    void saveCurrentConfig(void);

    ///Swaps the position of 2 file table rows.
    void swapFileTableRowPositions(int row1, int row2);

    ///Returns the sub directory which corresponds to the file type.
    QString getSubDirectoryFromType(QString type);

    ///Returns all library paths from the file table.
    QStringList getAllLibraryPathsFromFileTable();

    ///Returns all plugin paths from the file table.
    QStringList getAllPluginPathsFromFileTable();

    ///Sets the GUI elements to their default.
    void setGuiElementsToDefault();

    ///Creates the current configuration string.
    QString createCurrentConfigurationString();

    ///Returns true if the files table contains at minmium one executable script.
    bool isExecutableScriptInFilesTable();

    ///Pointer to the user interface.
    Ui::CreateSceFile *ui;

    ///Sets the state of several menus.
    void setMenuState(void);

    ///Creates an SCE file string.
    QString createSceFile();

    ///Pointer the script window script table.
    QTableWidget* m_scriptWindowScriptTable;

    ///The name of the current config file.
    QString m_configFileName;
};


#endif // CREATESCEFILE_H
