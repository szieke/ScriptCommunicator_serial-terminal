/***************************************************************************
**                                                                        **
**  ScriptCommunicator, is a tool for sending/receiving data with several **
**  interfaces.                                                           **
**  Copyright (C) 2014 Stefan Zieker                                      **
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

#include <QApplication>

#include "mainwindow.h"
#include <QStringList>
#include <scriptFile.h>
#include <QDir>
#include <QCryptographicHash>
#include <QLibrary>
#include <QProcess>
#include <QStandardPaths>


#ifdef Q_OS_WIN32
#include <Windows.h>
#endif

///Is set to true if a thread has been terminated.
///This variabke is used un the main function.
bool g_aThreadHasBeenTerminated = false;

///The current SCEZ folder.
QString g_currentScezFolder = "";

#ifdef Q_OS_WIN32
static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString &message)
{
    (void)type;
    (void)context;
    OutputDebugString(reinterpret_cast<const wchar_t *>(message.utf16()));
}
#endif

///Deletes the current SCEZ folder.
void deleteCurrentScezFolder(void)
{
    if(!g_currentScezFolder.isEmpty())
    {
        QString program = QCoreApplication::applicationDirPath() + "/DeleteFolder";
        program = QCoreApplication::applicationDirPath() + "/DeleteFolder";

#ifdef QT_DEBUG
        QDir(g_currentScezFolder).removeRecursively();
#else
        QProcess::startDetached(program, QStringList() << g_currentScezFolder, QString(""));
#endif
        g_currentScezFolder = "";
    }
}

int main(int argc, char *argv[])
{

#if QT_VERSION >= QT_VERSION_CHECK(5,6,0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication* a = new QApplication(argc, argv);
    QStringList extraPluginPaths;
    QStringList scriptArguments;
    QStringList extraLibPaths;
    QString configFile;
    int result = 0;

    QStringList scripts;
    bool withScriptWindow = false;
    bool scriptWindowIsMinimized = true;
    QString minimumScVersion;
    QString iconFile;

#ifdef Q_OS_WIN32
    qInstallMessageHandler(messageHandler);
#endif


    //Parse all arguments.
    for(int i = 1; i < argc; i++)
    {
        QString currentArg = QString::fromUtf8(argv[i]);

        if(currentArg.indexOf("-") == 0)
        {//The current argument is not a script.

            if(QString("-withScriptWindow") == currentArg)
            {
                withScriptWindow = true;
            }
            else if(QString("-notMinimized") == currentArg)
            {
                scriptWindowIsMinimized = false;
            }
            else if(currentArg.startsWith("-P"))
            {//Extra plugin path.

                extraPluginPaths << currentArg.remove("-P");
            }
            else if(currentArg.startsWith("-L"))
            {//Extra library path.

                extraLibPaths << currentArg.remove("-L");
            }
            else if(currentArg.startsWith("-C"))
            {//Config file.

                configFile = currentArg.remove("-C");
            }
            else if(currentArg.startsWith("-A"))
            {//Script argument (script can read these arguments withScriptThread::getScriptArguments).

                scriptArguments << currentArg.remove("-A");
            }
            else if(currentArg.startsWith("-minScVersion"))
            {
                minimumScVersion = currentArg.remove("-minScVersion");
            }
            else if(currentArg.startsWith("-I"))
            {
                iconFile = currentArg.remove("-I");
            }
            else
            {
                QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", QString("unknown command line argument: ") + currentArg);
                QApplication::setActiveWindow(&box);
                box.exec();
                return -1;
            }
        }
        else
        {//The current argument is a script, a SCE or a SCEZ File.

            if(currentArg.endsWith(".sce"))
            {//The current argument is an SCE file.

                if(!MainWindow::parseSceFile(currentArg.replace("\\", "/"), &scripts, &extraPluginPaths, &scriptArguments,
                                             &withScriptWindow, &scriptWindowIsMinimized, &minimumScVersion, &extraLibPaths))
                {
                    return -1;
                }

            }
            else if(currentArg.endsWith(".scez"))
            {
                if(MainWindow::checkScezFileHash(currentArg))
                {
                    g_currentScezFolder = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + QString("%1").arg(QDateTime::currentDateTime().currentMSecsSinceEpoch());
                    if(ScriptFile::extractZipFile(currentArg, g_currentScezFolder))
                    {
                        QString currentSceFile;
                        QStringList dirContent = ScriptFile(0, "", false).readDirectory(g_currentScezFolder, false, false, true, false);

                        for(auto el : dirContent)
                        {
                            if(el.endsWith(".sce"))
                            {
                                currentSceFile = el;
                                break;
                            }
                        }

                        if(!currentSceFile.isEmpty())
                        {
                            if(!MainWindow::parseSceFile(currentSceFile.replace("\\", "/"), &scripts, &extraPluginPaths, &scriptArguments,
                                                         &withScriptWindow, &scriptWindowIsMinimized, &minimumScVersion, &extraLibPaths))
                            {
                                return -1;
                            }
                        }
                        else
                        {
                            QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", QString("no sce file found in: ") + currentArg);
                            QApplication::setActiveWindow(&box);
                            box.exec();
                            return -1;
                        }
                    }
                    else
                    {
                        QMessageBox box(QMessageBox::Warning, "ScriptCommunicator", QString("could not unzip: ") + currentArg);
                        QApplication::setActiveWindow(&box);
                        box.exec();
                        return -1;
                    }

                }//if(MainWindow::checkScezFileHash(currentArg))
                else
                {
                    return -1;
                }

            }
            else
            {
#ifdef Q_OS_WIN32
                scripts << currentArg.replace("\\", "/");
#else
                scripts << currentArg;
#endif
            }
        }
    }

    if(!MainWindow::checkParsedScVersion(minimumScVersion))
    {
        return -1;
    }

    //Load all libraries in extraLibPaths.

    bool oneLibraryLoadFailed = true;
    for(int i = 0; (i < 5) && oneLibraryLoadFailed; i++)
    {
        oneLibraryLoadFailed = false;
        for(auto el : extraLibPaths)
        {
            QDir dir(el);
            QStringList foundEntries = dir.entryList(QDir::Files);

            //Load all found files.
            for(qint32 i = 0; i < foundEntries.length(); i++)
            {
                QString libName = foundEntries[i];

                QLibrary lib(el + "/" + libName);
                if(!lib.load())
                {
                    oneLibraryLoadFailed = true;
                }

            }
        }
    }


    try
    {

        MainWindow* w = new MainWindow(scripts, withScriptWindow, scriptWindowIsMinimized, extraPluginPaths,
                                       scriptArguments, configFile, iconFile);

        if(scripts.isEmpty())
        {
            w->show();
        }
        result = a->exec();

        if(!w->closedByScript())
        {//No script has called exitScriptCommunicator.

            if(!scripts.isEmpty())
            {
                if(!withScriptWindow)
                {
                    //If no script window is shown, the last running script
                    //will exit ScriptCommunicator(in withScriptWindow)
                    while(1)
                    {
                        a->processEvents();
                    }
                }

                w->close();
            }
        }

        if(w->closedByScript())
        {//A script has called exitScriptCommunicator.

            //Get the exit code.
            result = w->getExitCode();
        }




        //Note: The main window must be deleted before QApplication.
        delete w;

        if(!g_aThreadHasBeenTerminated)
        {//No thread has been ternminated.
            //If a thread has been terminated then the Destructor of QApplication will crash.
            delete a;
        }

    }
    catch(...)
    {

    }

    deleteCurrentScezFolder();

    return result;
}
