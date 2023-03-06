#include "createSceFile.h"
#include "ui_createSceFile.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QBuffer>
#include <QXmlStreamWriter>
#include <QTextStream>
#include <QMessageBox>
#include "scriptFile.h"
#include "scriptwindow.h"
#include <QDomDocument>
#include <QCryptographicHash>


/**
 * Constructor.
 * @param parent
 *      The parent.
 * @param scriptWindowScriptTable
 *      Pointer to the script table in the script window.
 */
CreateSceFile::CreateSceFile(QWidget *parent, QTableWidget *scriptWindowScriptTable) :
    QMainWindow(parent), ui(new Ui::CreateSceFile), m_scriptWindowScriptTable(scriptWindowScriptTable), m_configFileName()
{
    ui->setupUi(this);

    QShortcut* shortcut = new QShortcut(QKeySequence("Ctrl+Shift+X"), this);
    QObject::connect(shortcut, SIGNAL(activated()), this, SLOT(close()));

    connect(ui->selectFile, SIGNAL(clicked()), this, SLOT(selectFileSlot()));
    connect(ui->addFile, SIGNAL(clicked()), this, SLOT(addFileSlot()));
    connect(ui->addFolder, SIGNAL(clicked()), this, SLOT(addFolderSlot()));
    connect(ui->removeFile, SIGNAL(clicked()), this, SLOT(removeFileSlot()));
    connect(ui->addArgument, SIGNAL(clicked()), this, SLOT(addArgumentSlot()));
    connect(ui->removeArgument, SIGNAL(clicked()), this, SLOT(removeArgumentSlot()));
    connect(ui->actionGenerateFileSce, SIGNAL(triggered(bool)), this, SLOT(generateSlot()));
    connect(ui->actionGenerateCompressedFile, SIGNAL(triggered(bool)), this, SLOT(generateCompressedFileSlot()));
    connect(ui->fileLineEdit, SIGNAL(textChanged(QString)), this, SLOT(sceFileTextChangedSlot(QString)));
    connect(ui->filesTableWidget, SIGNAL(cellChanged(int,int)), this, SLOT(filesTableCellChangedSlot()));
    connect(ui->addAllFromScriptTable, SIGNAL(clicked()), this, SLOT(addAllFromScriptWindow()));

    connect(ui->actionLoadConfig, SIGNAL(triggered()), this, SLOT(loadConfigSlot()));
    connect(ui->actionUnloadConfig, SIGNAL(triggered()), this, SLOT(unloadConfigSlot()));
    connect(ui->actionSaveConfig, SIGNAL(triggered()), this, SLOT(saveConfigSlot()));
    connect(ui->actionSaveConfigAs, SIGNAL(triggered()), this, SLOT(saveConfigAsSlot()));
    connect(ui->minScVersionMajor, SIGNAL(valueChanged(int)), this, SLOT(versionSpinboxValueChangedSlot(int)));
    connect(ui->minScVersionMinor, SIGNAL(valueChanged(int)), this, SLOT(versionSpinboxValueChangedSlot(int)));

    connect(ui->fileEntryDown, SIGNAL(clicked()), this, SLOT(fileEntryDownSlot()));
    connect(ui->fileEntryUp, SIGNAL(clicked()), this, SLOT(fileEntryUpSlot()));
    connect(ui->argEntryDown, SIGNAL(clicked()), this, SLOT(argEntryDownSlot()));
    connect(ui->argEntryUp, SIGNAL(clicked()), this, SLOT(argEntryUpSlot()));

    connect(ui->filesTableWidget, SIGNAL(dropEventSignal(int,int,QStringList)), this, SLOT(tableDropEventSlot(int,int,QStringList)));

    ui->progressBar->setValue(0);

    ui->minScVersionMajor->setValue(MainWindow::VERSION.split(".")[0].toUInt());
    ui->minScVersionMinor->setValue(MainWindow::VERSION.split(".")[1].toUInt());

    setMenuState();
    setTitle("");
}

/**
 * Destructor.
 */
CreateSceFile::~CreateSceFile()
{
    delete ui;
}

/**
 * Shows this window.
 */
void CreateSceFile::show(void)
{
    QWidget::show();
    resizeTableColumnsSlot();
}

/**
 * Returns true if the files table contains at minmium one executable script.
 */
bool CreateSceFile::isExecutableScriptInFilesTable()
{
    bool result = false;

    for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
    {
        QComboBox* box = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(i, COLUMN_TYPE));
        if(box->currentText() == "execute script")
        {
            result = true;
            break;
        }
    }

    return result;
}

/**
 * Sets the state of several menus.
 */
void CreateSceFile::setMenuState(void)
{
    ui->actionGenerateFileSce->setEnabled(false);
    ui->actionGenerateCompressedFile->setEnabled(false);

    if(!ui->fileLineEdit->text().isEmpty())
    {
        for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
        {
            QComboBox* box = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(i, COLUMN_TYPE));

            if((box->currentText() == "script") || (box->currentText() == "execute script"))
            {
                ui->actionGenerateFileSce->setEnabled(true);
                ui->actionGenerateCompressedFile->setEnabled(true);
                break;
            }
        }
    }
}

/**
 * Select file button slot function.
 */
void CreateSceFile::selectFileSlot()
{
    QString file = QFileDialog::QFileDialog::getSaveFileName(this, tr("Select file name"),
                                                             "", tr("Files (*)"));
    if(!file.isEmpty())
    {
        ui->fileLineEdit->setText(file.split(".")[0]);
    }

    setMenuState();
}

/**
 * Adds a row at the end of the files table.
 * @param folder
 *      The folder (relative the generated SCE file).
 * @param type
 *      The type of the file.
 * @param source
 *      The file source.
 */
void CreateSceFile::addTableRow(QString folder, QString type, QString source)
{
    if(!source.isEmpty())
    {
        ui->filesTableWidget->insertRow(ui->filesTableWidget->rowCount());
        int row = ui->filesTableWidget->rowCount() - 1;

        ui->filesTableWidget->setItem(row, COLUMN_SUBDIR,  new QTableWidgetItem(folder));
        ui->filesTableWidget->setItem(row, COLUMN_TYPE, new QTableWidgetItem());
        ui->filesTableWidget->setItem(row, COLUMN_SOURCE, new QTableWidgetItem(source));

        QComboBox* box = new QComboBox(ui->filesTableWidget);
        QStringList availTargets;
        availTargets << "script" << "execute script"  << "ui" << "lib" << "media" << "plugin" << "bin" << "etc";
        box->addItems(availTargets);
        box->setCurrentText(type);

        ui->filesTableWidget->setCellWidget(row, COLUMN_TYPE, box);
        connect(box, SIGNAL(currentTextChanged(QString)), this, SLOT(typeTextChangedSlot(QString)));
    }
}

/**
 * Returns the sub directory which corresponds to the file type.
 * @param type
 *      The file type.
 * @return
 *      The sub directory.
 */
QString CreateSceFile::getSubDirectoryFromType(QString type)
{
    QString result = type;

    if((type == "execute script") || (type == "script"))
    {
        result = "scripts";
    }
    else if(type == "lib")
    {
        result = "libs";
    }
    else if(type == "ui")
    {
        result = "scripts";
    }
    else if(type == "plugin")
    {
        result = "plugins";
    }

    return result;
}

/**
 * Add folder button slot function.
 */
void CreateSceFile::addFolderSlot()
{
    QString folder = QFileDialog::getExistingDirectory(this, "Select a folder");

    if(!folder.isEmpty())
    {
        ScriptFile helper(0, "", false);
        QStringList files = helper.readDirectory(folder, false, true, true, false);
        for(auto el : files)
        {
            QString subDir = QFileInfo(el).absolutePath();
            subDir.remove(QFileInfo(folder).absolutePath());

            QString type = getFileTypeFromFile(el);
            if(subDir.endsWith("/"))
            {
                subDir.remove(subDir.length() - 1, 1);
            }
            addTableRow(getSubDirectoryFromType(type) + subDir, type, el);
        }
    }
}

/**
 * Add file button slot function.
 */
void CreateSceFile::addFileSlot()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select files"),
                                                      "", tr("Files (*)"));
    if(!files.isEmpty())
    {
        for(auto el : files)
        {
            QString type = getFileTypeFromFile(el);
            addTableRow(getSubDirectoryFromType(type), type, el);
        }
    }

    setMenuState();
}

/**
 * Remove file button slot function.
 */
void CreateSceFile::removeFileSlot()
{
    int selectedRow = (ui->filesTableWidget->selectedItems().isEmpty())? -1 : ui->filesTableWidget->selectedItems()[0]->row();
    if(selectedRow != -1 )
    {
        ui->filesTableWidget->removeRow(selectedRow);
        setMenuState();
    }
}

/**
 * Resizes all file table columns.
 */
void CreateSceFile::resizeTableColumnsSlot(void)
{

    ui->filesTableWidget->resizeColumnsToContents();
    ui->filesTableWidget->resizeRowsToContents();

    ui->filesTableWidget->setColumnWidth(COLUMN_SOURCE, ui->filesTableWidget->width() -
                                         (ui->filesTableWidget->columnWidth(COLUMN_SUBDIR)
                                          + ui->filesTableWidget->columnWidth(COLUMN_TYPE)
                                          + 2 *ui->filesTableWidget->frameWidth()
                                          + ui->filesTableWidget->verticalHeader()->width()
                                          + (ui->filesTableWidget->verticalScrollBar()->isVisible() ?
                                                 ui->filesTableWidget->verticalScrollBar()->width() : 0)));
}

/**
 * Is called if the send window has been resized.
 * @param event
 *      The resize event.
 */
void CreateSceFile::resizeEvent(QResizeEvent * event)
{
    (void)event;
    resizeTableColumnsSlot();
}
/**
 * Add argument button slot function.
 */
void CreateSceFile::addArgumentSlot()
{
    bool okPressed;
    QString arg = QInputDialog::getMultiLineText(this, "enter argument", "", "", &okPressed);
    if(okPressed)
    {
        ui->argumentsListWidget->addItem(arg);
    }

    setMenuState();
}



/**
 * Swaps the position of 2 file table rows.
 * @param row1
 *      Row 1.
 * @param row2
 *      Row 2.
 */
void CreateSceFile::swapFileTableRowPositions(int row1, int row2)
{
    QComboBox * box1 = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(row1, COLUMN_TYPE));
    QComboBox * box2 = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(row2, COLUMN_TYPE));

    box1->blockSignals(true);
    box2->blockSignals(true);

    QList<QTableWidgetItem*> rowItems1,rowItems2;
    int colCount = ui->filesTableWidget->columnCount();

    ui->filesTableWidget->blockSignals(true);

    //Remove all cells from the two rows which position have to be swapped.
    for (int col = 0; col < colCount; ++col)
    {
        rowItems1 << ui->filesTableWidget->takeItem(row1, col);
        rowItems2 << ui->filesTableWidget->takeItem(row2, col);

    }

    //Insert all cells from the two rows which positions have to be swapped
    //at their new positions.
    for (int cola = 0; cola < colCount; ++cola)
    {
        ui->filesTableWidget->setItem(row2, cola, rowItems1.at(cola));
        ui->filesTableWidget->setItem(row1, cola, rowItems2.at(cola));

    }

    QString tmpFormat = box1->currentText();
    box1->setCurrentText(box2->currentText());
    box2->setCurrentText(tmpFormat);


    box1->blockSignals(false);
    box2->blockSignals(false);
    ui->filesTableWidget->blockSignals(false);
}

/**
 * File entry up button slot function.
 */
void CreateSceFile::fileEntryUpSlot()
{
    QList<QTableWidgetItem*> selectedItems = ui->filesTableWidget->selectedItems();
    if(!selectedItems.isEmpty())
    {
        int selectedRow = selectedItems[0]->row();
        if((selectedRow != -1) && (selectedRow != 0) )
        {
            swapFileTableRowPositions(selectedRow, selectedRow - 1);
            ui->filesTableWidget->clearSelection();
            ui->filesTableWidget->selectRow(selectedRow - 1);
        }
    }
}

/**
 * Script table drop event function.
 * @param row
 *      The row of the drop event.
 * @param column
 *      The column of the drop event.
 * @param files
 *      The file from the drop event.
 */
void CreateSceFile::tableDropEventSlot(int row, int column, QStringList files)
{
    (void)row;
    (void)column;

    for(auto el : files)
    {
        if(!el.isEmpty())
        {
            QString type = getFileTypeFromFile(el);
            addTableRow(getSubDirectoryFromType(type), type, el);
        }
    }
}

/**
 * File entry down button slot function.
 */
void CreateSceFile::fileEntryDownSlot()
{
    QList<QTableWidgetItem*> selectedItems = ui->filesTableWidget->selectedItems();
    if(!selectedItems.isEmpty())
    {
        int selectedRow = selectedItems[0]->row();
        if((selectedRow != -1) && (selectedRow != (ui->filesTableWidget->rowCount() - 1)))
        {
            swapFileTableRowPositions(selectedRow, selectedRow + 1);
            ui->filesTableWidget->clearSelection();
            ui->filesTableWidget->selectRow(selectedRow + 1);
        }
    }
}

/**
 * Arg entry up button slot function.
 */
void CreateSceFile::argEntryUpSlot()
{
    int row = ui->argumentsListWidget->currentRow();
    if((row != -1) && (row != 0))
    {
        QString tmpText = ui->argumentsListWidget->item(row)->text();
        ui->argumentsListWidget->item(row)->setText(ui->argumentsListWidget->item(row - 1)->text());
        ui->argumentsListWidget->item(row - 1)->setText(tmpText);

        ui->argumentsListWidget->clearSelection();
        ui->argumentsListWidget->setCurrentRow(row - 1);
    }
}

/**
 * Arg entry down button slot function.
 */
void CreateSceFile::argEntryDownSlot()
{
    int row = ui->argumentsListWidget->currentRow();
    if((row != -1) && (row != (ui->argumentsListWidget->count() - 1)))
    {
        QString tmpText = ui->argumentsListWidget->item(row)->text();
        ui->argumentsListWidget->item(row)->setText(ui->argumentsListWidget->item(row + 1)->text());
        ui->argumentsListWidget->item(row + 1)->setText(tmpText);

        ui->argumentsListWidget->clearSelection();
        ui->argumentsListWidget->setCurrentRow(row + 1);
    }
}

/**
 * Remove argument button slot function.
 */
void CreateSceFile::removeArgumentSlot()
{
    QList<QListWidgetItem*> items = ui->argumentsListWidget->selectedItems();
    for(int i = 0; i < items.length(); i++)
    {
        delete items[i];
    }

    setMenuState();
}

/**
 * Creates an SCE file string.
 * @return
 *      The string.
 */
QString CreateSceFile::createSceFile()
{
    QBuffer xmlBuffer;

    QXmlStreamWriter xmlWriter;
    xmlWriter.setAutoFormatting(true);
    xmlWriter.setAutoFormattingIndent(2);
    xmlBuffer.open(QIODevice::WriteOnly);
    xmlWriter.setDevice(&xmlBuffer);

    xmlWriter.writeStartElement("ExecutableConfig");
    xmlWriter.writeAttribute("version", MainWindow::VERSION);

    xmlWriter.writeStartElement("Scripts");
    for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
    {
        QComboBox* box = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(i, COLUMN_TYPE));

        if(box->currentText() == "execute script")
        {
            QString currentFolder = ui->filesTableWidget->item(i, COLUMN_SUBDIR)->text();
            QString source = ui->filesTableWidget->item(i, COLUMN_SOURCE)->text();

            xmlWriter.writeStartElement("Script");
            QString path = currentFolder.isEmpty() ? "./" + QFileInfo(source).fileName() : "./" + currentFolder + "/" + QFileInfo(source).fileName();
            xmlWriter.writeAttribute("path", path);
            xmlWriter.writeEndElement();
        }
    }
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("LibraryPaths");
    QStringList libPaths = getAllLibraryPathsFromFileTable();
    for(auto el : libPaths)
    {
        xmlWriter.writeStartElement("LibraryPath");
        xmlWriter.writeAttribute("path", "./" + el);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("PluginPaths");
    QStringList pluginPaths = getAllPluginPathsFromFileTable();
    for(auto el : pluginPaths)
    {
        xmlWriter.writeStartElement("PluginPath");
        xmlWriter.writeAttribute("path", "./" + el);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();


    xmlWriter.writeStartElement("ScriptArguments");
    for(int i = 0; i < ui->argumentsListWidget->count(); i++)
    {
        xmlWriter.writeStartElement("ScriptArgument");
        xmlWriter.writeAttribute("value", ui->argumentsListWidget->item(i)->text());
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("Options");
    xmlWriter.writeAttribute("withScriptWindow", QString("%1").arg(ui->withScriptWindowCheckBox->isChecked()));
    xmlWriter.writeAttribute("notMinimized", QString("%1").arg(ui->notMinimized->isChecked()));
    xmlWriter.writeAttribute("minScVersion", QString("%1.%2").arg(ui->minScVersionMajor->value()).arg(ui->minScVersionMinor->value()));
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();//ScriptCommunicatorExecutable

    return xmlBuffer.data();
}


/**
 * Is called if the content of a cell in the files table has been changed.
 */
void CreateSceFile::filesTableCellChangedSlot()
{
    resizeTableColumnsSlot();
}

/**
 * Returns the file type of file.
 * @param file
 *      The file.
 * @return
 *      The type ("script", "ui", "lib", "media", "plugin", "bin", "etc").
 */
QString CreateSceFile::getFileTypeFromFile(QString file)
{
    QString type = "etc";

#ifdef Q_OS_WIN32
    const QString libExtension = ".dll";
#else

#ifdef Q_OS_MAC
    const QString libExtension = ".dylib";
#else
    const QString libExtension = ".so";
#endif//#ifdef Q_OS_MAC

#endif

    if(file.contains(".js"))
    {
        type = "script";
    }
    else if(file.contains(".ui"))
    {
        type = "ui";
    }
    else if(file.contains(libExtension))
    {
        type = "lib";
    }
#ifdef Q_OS_MAC
    else if(file.contains(".framework"))
    {
        type = "lib";
    }
#endif

    return type;
}

/**
 * Add all from script window button slot function.
 */
void CreateSceFile::addAllFromScriptWindow()
{
    for(int i = 0; i < m_scriptWindowScriptTable->rowCount(); i++)
    {
        QTableWidgetItem* itemScriptPath = m_scriptWindowScriptTable->item(i, ScriptWindow::COLUMN_SCRIPT_PATH);
        QTableWidgetItem* itemUi = m_scriptWindowScriptTable->item(i, ScriptWindow::COLUMN_UI_PATH);

        addTableRow("scripts", "script", itemScriptPath->text());
        addTableRow("scripts", "ui", itemUi->text());
    }
}

/**
 * Generate scez file menu slot function.
 */
void CreateSceFile::generateCompressedFileSlot()
{
    bool success = false;
    QString sceFileName = ui->fileLineEdit->text() + "/" + QFileInfo(ui->fileLineEdit->text()).fileName() + ".sce";
    ScriptFile fileHelper(this, sceFileName, false);
    QStringList copiedFiles;

    if(!isExecutableScriptInFilesTable())
    {
        QMessageBox::information(this, "could not start generation", "No executable script defined.");
        return;
    }

    setEnabled(false);

    ui->progressBar->setValue(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(ui->filesTableWidget->rowCount() * 2);

    statusBar()->showMessage("generating scez file", 0);

    generateSlot(&copiedFiles, ui->progressBar, &success);
    if(success)
    {
        copiedFiles << sceFileName;
        QString zipFileName = ui->fileLineEdit->text() + ".scez";

        (void)fileHelper.deleteFile(zipFileName, false);

        QList<QStringList> fileList;
        QString rootDir = QDir(QFileInfo(sceFileName).path()).absolutePath();
        for(auto el : copiedFiles)
        {
            QStringList entry;
            entry << el;
            entry << QFileInfo(el).filePath().remove(0, rootDir.length() + 1);
            fileList << entry;
        }

        success = ScriptFile::zipFiles(zipFileName, fileList, ui->progressBar);
        if(!success)
        {
            (void)QFile::remove(zipFileName);
        }
        else
        {
            QFile inFile;
            inFile.setFileName(zipFileName);

            success = inFile.open(QIODevice::ReadOnly);
            if(success)
            {
                QCryptographicHash hashObject(QCryptographicHash::Sha512);
                success = hashObject.addData(&inFile);
                if(success)
                {   inFile.close();
                    success = inFile.open(QIODevice::Append);
                    if(success)
                    {
                        (void)inFile.seek(inFile.size());
                        QByteArray hash = hashObject.result();
                        success = (inFile.write(hash) == hash.length()) ? true : false;
                    }
                }

                inFile.close();
            }
        }
    }

    //Delete the sce folder.
    (void)QDir(ui->fileLineEdit->text()).removeRecursively();

    if(success)
    {
        QMessageBox::information(this, "information", "scez file creation succeeded");
        statusBar()->showMessage("scez file created", 5000);
    }
    else
    {
        statusBar()->showMessage("scez file creation failed", 5000);
    }

    setEnabled(true);
}


/**
 * Returns all library paths from the file table.
 */
QStringList CreateSceFile::getAllLibraryPathsFromFileTable()
{
    QStringList libPaths;
    //Get all lib directories.
    for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
    {
        QComboBox* box = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(i, COLUMN_TYPE));
        QString entry = box->currentText();
        bool isLibrary = false;
         QString path = ui->filesTableWidget->item(i, COLUMN_SUBDIR)->text();
        if(entry == "lib")
        {
            isLibrary = true;
        }
        if(path.contains(".framework"))
        {
            isLibrary = true;
            int index = path.indexOf(".framework");
            path = path.left(index + QString(".framework").length());

        }

        if(isLibrary)
        {

            if(!libPaths.contains(path))
            {
                libPaths << path;
            }
        }
    }


    return libPaths;
}

/**
 * Returns all plugin paths from the file table.
 */
QStringList CreateSceFile::getAllPluginPathsFromFileTable()
{
    QStringList paths;
    //Get all plugin directories.
    for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
    {
        QComboBox* box = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(i, COLUMN_TYPE));

        if(box->currentText() == "plugin")
        {
            QString path = ui->filesTableWidget->item(i, COLUMN_SUBDIR)->text();
            if(!paths.contains(path))
            {
                paths << path;
            }
        }
    }

    return paths;
}

/**
 * Generate sce file menu slot function.
 * @param copiedFilesOutput
 *      The copied files.
 * @param progress
 *      Pointer to a progress bar. The value of this progress bar
 *      is increased by one for every finished file in the files table.
 * @param resultPointer
 *      True on success.
 */
void CreateSceFile::generateSlot(QStringList* copiedFilesOutput, QProgressBar* progress, bool *resultPointer)
{
    QString sceFileName = ui->fileLineEdit->text() + "/" + QFileInfo(ui->fileLineEdit->text()).fileName() + ".sce";
    QFile file(sceFileName);
    file.remove();
    bool success = true;
    QStringList copiedFiles;

    if(!isExecutableScriptInFilesTable())
    {
        QMessageBox::information(this, "could not start generation", "No executable script defined.");
        return;
    }

    //Delete and create the sce folder.
    (void)QDir(ui->fileLineEdit->text()).removeRecursively();
    if(!QDir().mkpath(ui->fileLineEdit->text()))
    {
        QMessageBox::critical(this, "error", QString("could not create: %1").arg(ui->fileLineEdit->text()));
        return;
    }

    setEnabled(false);

    if(progress == 0)
    {
        progress = ui->progressBar;

        progress->setValue(0);
        progress->setMinimum(0);
        progress->setMaximum(ui->filesTableWidget->rowCount());
        statusBar()->showMessage("generating sce file", 0);
    }

    if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream data( &file );
        data.setEncoding(QStringConverter::Utf8);
        data << createSceFile();
        file.close();

        for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
        {
            QString currentFolder = ui->filesTableWidget->item(i, COLUMN_SUBDIR)->text();
            currentFolder = QFileInfo(sceFileName).absolutePath() + "/" + currentFolder;
            QString source = ui->filesTableWidget->item(i, COLUMN_SOURCE)->text();

            if(!QFile().exists(currentFolder))
            {
                if(!QDir().mkpath(currentFolder))
                {
                    QMessageBox::critical(this, "error", QString("could not create: %1").arg(currentFolder));
                    success = false;
                    break;
                }
            }

            QString scriptName = QFileInfo(source).fileName();
            QString destinationFile = currentFolder + "/" + scriptName;
            destinationFile = QFileInfo(destinationFile).absoluteFilePath();
            QFile::remove(destinationFile);

            copiedFiles << destinationFile;

            if(!QFile::copy(source, destinationFile))
            {
                QMessageBox::critical(this, "error", QString("could not copy: %1").arg(source));
                success = false;
                break;
            }
            progress->setValue(progress->value() + 1);
        }

    }// if(file.open(QIODevice::WriteOnly | QIODevice::Text))
    else
    {
        QMessageBox::critical(this, "error", QString("could not create: %1").arg(sceFileName));
        success = false;
    }

    if(success)
    {
        if(copiedFilesOutput == 0)
        {
            QMessageBox::information(this, "information", "sce file creation succeeded");
            statusBar()->showMessage("sce file created", 5000);
        }
    }
    else
    {
        statusBar()->showMessage("sce file creation failed", 5000);

        //Delete the sce folder.
        (void)QDir(ui->fileLineEdit->text()).removeRecursively();

        //Delete all copied files.
        for(auto el : copiedFiles)
        {
            (void)QFile::remove(el);
        }

        (void)QFile::remove(sceFileName);
    }

    if(copiedFilesOutput == 0)
    {
        setEnabled(true);
    }
    else
    {
        *copiedFilesOutput = copiedFiles;
    }

    if(resultPointer != 0)
    {
        *resultPointer = success;
    }
}

/**
 * The content of the sce file line edit has been changed.
 * @param text
 *      The new text.
 */
void CreateSceFile::sceFileTextChangedSlot(QString text)
{
    (void)text;
    setMenuState();
}

/**
 * Slot function for the load config menu.
 */
void CreateSceFile::loadConfigSlot()
{
    if(!m_configFileName.isEmpty())
    {
        saveConfigSlot();
    }

    QString tmpFileName = QFileDialog::getOpenFileName(this, tr("Open script config file"),
                                                       MainWindow::getAndCreateProgramUserFolder(), tr("XML files (*.xml);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        m_configFileName = tmpFileName;
        loadConfigFile();
        emit configHasToBeSavedSignal();
    }
}

/**
 * Is called if the value of one of the version combo boxes has been changed.
 * @param value
 */
void CreateSceFile::versionSpinboxValueChangedSlot(int value)
{
    QObject* obj = sender();
    QSpinBox* box = static_cast<QSpinBox*>(obj);
    if(value < 10)
    {
        box->setPrefix("0");
    }
    else
    {
        box->setPrefix("");
    }
}

/**
 * The value of a type combo box has been changed.
 * @param text
 *      The new text.
 */
void CreateSceFile::typeTextChangedSlot(QString text)
{
    (void)text;
    QObject* obj = sender();
    QComboBox* box = static_cast<QComboBox*>(obj);

    setMenuState();
    int row = 0;
    for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
    {
        if(box == static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(i, COLUMN_TYPE)))
        {
            row = i;
            break;
        }
    }
    QString subDir =  ui->filesTableWidget->item(row,COLUMN_SUBDIR)->text();
    int index = subDir.indexOf("/", 0);
    if(index != -1)
    {
        subDir.remove(0, index);
        subDir = getSubDirectoryFromType(box->currentText()) + subDir;
    }
    else
    {
        subDir = getSubDirectoryFromType(box->currentText());
    }


    ui->filesTableWidget->item(row,COLUMN_SUBDIR)->setText(subDir);
}

/**
 * Sets the window title.
 * @param extraString
 *      The string which is appended at the title.
 */
void CreateSceFile::setTitle(QString extraString)
{
    setWindowTitle("ScriptCommunicator " + MainWindow::VERSION + " - SCE File Dialog: " + extraString);
}

/**
 * Slot function for the save as config menu.
 */
void CreateSceFile::saveConfigAsSlot()
{
    QString tmpFileName = QFileDialog::getSaveFileName(this, tr("Save sce config file"),
                                                       MainWindow::getAndCreateProgramUserFolder(), tr("XML files (*.xml);;Files (*)"));
    if(!tmpFileName.isEmpty())
    {
        m_configFileName = tmpFileName;
        setTitle(m_configFileName);
        emit configHasToBeSavedSignal();
        saveCurrentConfig();
    }
}

/**
 * Slot function for the unload config menu.
 */
void CreateSceFile::unloadConfigSlot()
{
    if(!m_configFileName.isEmpty())
    {
        saveConfigSlot();
    }

    setGuiElementsToDefault();
    m_configFileName = "";
    setTitle(m_configFileName);
    emit configHasToBeSavedSignal();
}

/**
 * Creates the current configuration string.
 * @return
 *      The string.
 */
QString CreateSceFile::createCurrentConfigurationString()
{
    QBuffer xmlBuffer;

    QXmlStreamWriter xmlWriter;
    xmlWriter.setAutoFormatting(true);
    xmlWriter.setAutoFormattingIndent(2);
    xmlBuffer.open(QIODevice::WriteOnly);
    xmlWriter.setDevice(&xmlBuffer);

    xmlWriter.writeStartElement("SceConfiguration");

    xmlWriter.writeStartElement("Files");
    for(int i = 0; i < ui->filesTableWidget->rowCount(); i++)
    {
        QComboBox* box = static_cast<QComboBox*>(ui->filesTableWidget->cellWidget(i, COLUMN_TYPE));
        QString subDirectory = ui->filesTableWidget->item(i, COLUMN_SUBDIR)->text();
        QString source = MainWindow::convertToRelativePath(m_configFileName, ui->filesTableWidget->item(i, COLUMN_SOURCE)->text());

        xmlWriter.writeStartElement("File");
        xmlWriter.writeAttribute("subDirectory", subDirectory);
        xmlWriter.writeAttribute("type", box->currentText());
        xmlWriter.writeAttribute("source", source);
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("ScriptArguments");
    for(int i = 0; i < ui->argumentsListWidget->count(); i++)
    {
        xmlWriter.writeStartElement("ScriptArgument");
        xmlWriter.writeAttribute("value", ui->argumentsListWidget->item(i)->text());
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();

    xmlWriter.writeStartElement("ScriptWindowState");
    xmlWriter.writeAttribute("withScriptWindow", QString("%1").arg(ui->withScriptWindowCheckBox->isChecked()));
    xmlWriter.writeAttribute("notMinimized", QString("%1").arg(ui->notMinimized->isChecked()));
    xmlWriter.writeAttribute("minScVersion", QString("%1.%2").arg(ui->minScVersionMajor->value()).arg(ui->minScVersionMinor->value()));
    xmlWriter.writeAttribute("sceFileName", MainWindow::convertToRelativePath(m_configFileName, ui->fileLineEdit->text()));
    xmlWriter.writeEndElement();

    xmlWriter.writeEndElement();//SceConfiguration

    return xmlBuffer.data();
}

/**
 * Saves the current config.
 */
void CreateSceFile::saveCurrentConfig(void)
{
    QString configString = createCurrentConfigurationString();

    QFile file(m_configFileName);
    file.remove();
    if(!configString.isEmpty() && file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream data( &file );
        data.setEncoding(QStringConverter::Utf8);
        data << configString;
        file.close();

        QStringList showStrList = m_configFileName.split("/");
        statusBar()->showMessage(showStrList[showStrList.size() - 1] + " saved", 5000);
    }
    else
    {
        QMessageBox::critical(this, "save failed", m_configFileName);

        m_configFileName = "";
        setTitle(m_configFileName);
        emit configHasToBeSavedSignal();
    }
}

/**
 * Returns true if the current config must be saved.
 */
bool CreateSceFile::configMustBeSaved()
{
    bool result = false;

    if(m_configFileName.isEmpty())
    {
        if((ui->filesTableWidget->rowCount() != 0) ||
                (ui->argumentsListWidget->count() != 0) ||
                !ui->fileLineEdit->text().isEmpty())
        {
            result = true;
        }
    }
    else
    {
        result = true;
    }
    return result;
}

/**
 * Slot function for the save config menu.
 */
void CreateSceFile::saveConfigSlot()
{
    if(!m_configFileName.isEmpty())
    {
        saveCurrentConfig();
    }
    else
    {
        saveConfigAsSlot();
    }
}

/**
 * Sets the GUI elements to their default.
 */
void CreateSceFile::setGuiElementsToDefault()
{
    ui->filesTableWidget->setRowCount(0);
    ui->argumentsListWidget->clear();
    ui->fileLineEdit->setText("");
    ui->minScVersionMajor->setValue(MainWindow::VERSION.split(".")[0].toUInt());
    ui->minScVersionMinor->setValue(MainWindow::VERSION.split(".")[1].toUInt());
    ui->withScriptWindowCheckBox->setChecked(false);
    ui->notMinimized->setChecked(false);
}

/**
 * Loads the current config file (m_configFileName).
 */
void CreateSceFile::loadConfigFile(void)
{
    if(!m_configFileName.isEmpty())
    {
        setGuiElementsToDefault();
        setTitle(m_configFileName);

        if(!m_configFileName.isEmpty())
        {
            QFile file(m_configFileName);
            QDomDocument doc("SceConfiguration");

            if (file.open(QFile::ReadOnly))
            {
                file.close();

                if (!doc.setContent(&file))
                {
                    if(!file.readAll().isEmpty())
                    {
                        QMessageBox::critical(this, "parse error", "could not parse " + m_configFileName);

                        m_configFileName = "";
                        setTitle(m_configFileName);
                        emit configHasToBeSavedSignal();
                    }
                }
                else
                {

                    QDomElement docElem = doc.documentElement();
                    ui->filesTableWidget->blockSignals(true);

                    QDomNodeList nodeList = docElem.elementsByTagName("ScriptWindowState");
                    QDomNode nodeItem = nodeList.at(0);
                    ui->withScriptWindowCheckBox->setChecked((nodeItem.attributes().namedItem("withScriptWindow").nodeValue() == "1"));
                    ui->notMinimized->setChecked((nodeItem.attributes().namedItem("notMinimized").nodeValue() == "1"));
                    ui->fileLineEdit->setText(MainWindow::convertToAbsolutePath(m_configFileName, nodeItem.attributes().namedItem("sceFileName").nodeValue()));

                    QStringList versionList = nodeItem.attributes().namedItem("minScVersion").nodeValue().split(".");
                    if(versionList.length() == 2)
                    {
                        ui->minScVersionMajor->setValue(versionList[0].toUInt());
                        ui->minScVersionMinor->setValue(versionList[1].toUInt());
                    }

                    nodeList = docElem.elementsByTagName("File");
                    for (int i = 0; i < nodeList.size(); i++)
                    {
                        nodeItem = nodeList.at(i);
                        QString subDirectory = nodeItem.attributes().namedItem("subDirectory").nodeValue();
                        QString type = (nodeItem.attributes().namedItem("type").nodeValue());
                        QString source = MainWindow::convertToAbsolutePath(m_configFileName, nodeItem.attributes().namedItem("source").nodeValue());
                        addTableRow(subDirectory, type, source);
                    }

                    nodeList = docElem.elementsByTagName("ScriptArgument");
                    for (int i = 0; i < nodeList.size(); i++)
                    {
                        nodeItem = nodeList.at(i);
                        QString value = nodeItem.attributes().namedItem("value").nodeValue();
                        ui->argumentsListWidget->addItem(value);
                    }

                    ui->filesTableWidget->blockSignals(false);
                    resizeTableColumnsSlot();

                    QStringList showStrList = m_configFileName.split("/");
                    statusBar()->showMessage(showStrList[showStrList.size() - 1] + " loaded", 5000);
                    setMenuState();
                }
            }
        }
        else
        {
            QMessageBox::critical(this, "could not open file", m_configFileName);

            m_configFileName = "";
            setTitle(m_configFileName);
            emit configHasToBeSavedSignal();
        }
    }
}


