// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

// designer
#include "qdesigner.h"
#include "qdesigner_actions.h"
#include "qdesigner_server.h"
#include "qdesigner_settings.h"
#include "qdesigner_workbench.h"
#include "mainwindow.h"

#include <qdesigner_propertysheet_p.h>

#include <QtGui/qevent.h>
#include <QtWidgets/qmessagebox.h>
#include <QtGui/qicon.h>
#include <QtWidgets/qerrormessage.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qdir.h>
#include <QtCore/qfile.h>
#include <QtCore/qlibraryinfo.h>
#include <QtCore/qlocale.h>
#include <QtCore/qtextstream.h>
#include <QtCore/qtimer.h>
#include <QtCore/qtranslator.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdebug.h>
#include <QtCore/qcommandlineparser.h>
#include <QtCore/qcommandlineoption.h>

#include <QtDesigner/QDesignerComponents>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

static constexpr auto designerApplicationName = "Designer"_L1;
static constexpr auto designerDisplayName = "Qt Designer"_L1;
static constexpr auto designerWarningPrefix = "Designer: "_L1;
static QtMessageHandler previousMessageHandler = nullptr;

static void designerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Only Designer warnings are displayed as box
    QDesigner *designerApp = qDesigner;
    if (type != QtWarningMsg || !designerApp || !msg.startsWith(designerWarningPrefix)) {
        previousMessageHandler(type, context, msg);
        return;
    }
    designerApp->showErrorMessage(msg);
}

QDesigner::QDesigner(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setOrganizationName(u"QtProject"_s);
    QGuiApplication::setApplicationDisplayName(designerDisplayName);
    setApplicationName(designerApplicationName);
    QDesignerComponents::initializeResources();

#if !defined(Q_OS_MACOS) && !defined(Q_OS_WIN)
    setWindowIcon(QIcon(u":/qt-project.org/designer/images/designer.png"_s));
#endif
}

QDesigner::~QDesigner()
{
    delete m_workbench;
    delete m_server;
    delete m_client;
}

void QDesigner::showErrorMessage(const QString &message)
{
    // strip the prefix
    const QString qMessage =
        message.right(message.size() - int(designerWarningPrefix.size()));
    // If there is no main window yet, just store the message.
    // The QErrorMessage would otherwise be hidden by the main window.
    if (m_mainWindow) {
        showErrorMessageBox(qMessage);
    } else {
        const QMessageLogContext emptyContext;
        previousMessageHandler(QtWarningMsg, emptyContext, message); // just in case we crash
        m_initializationErrors += qMessage;
        m_initializationErrors += u'\n';
    }
}

void QDesigner::showErrorMessageBox(const QString &msg)
{
    // Manually suppress consecutive messages.
    // This happens if for example sth is wrong with custom widget creation.
    // The same warning will be displayed by Widget box D&D and form Drop
    // while trying to create instance.
    if (m_errorMessageDialog && m_lastErrorMessage == msg)
        return;

    if (!m_errorMessageDialog) {
        m_lastErrorMessage.clear();
        m_errorMessageDialog = new QErrorMessage(m_mainWindow);
        const QString title = QCoreApplication::translate("QDesigner", "%1 - warning").arg(designerApplicationName);
        m_errorMessageDialog->setWindowTitle(title);
        m_errorMessageDialog->setMinimumSize(QSize(600, 250));
    }
    m_errorMessageDialog->showMessage(msg);
    m_lastErrorMessage = msg;
}

QDesignerWorkbench *QDesigner::workbench() const
{
    return m_workbench;
}

QDesignerServer *QDesigner::server() const
{
    return m_server;
}

static void showHelp(QCommandLineParser &parser, const QString &errorMessage = QString())
{
    QString text;
    QTextStream str(&text);
    str << "<html><head/><body>";
    if (!errorMessage.isEmpty())
        str << "<p>" << errorMessage << "</p>";
    str << "<pre>" << parser.helpText().toHtmlEscaped() << "</pre></body></html>";
    QMessageBox box(errorMessage.isEmpty() ? QMessageBox::Information : QMessageBox::Warning,
                    QGuiApplication::applicationDisplayName(), text,
                    QMessageBox::Ok);
    box.setTextInteractionFlags(Qt::TextBrowserInteraction);
    box.exec();
}

struct Options
{
    QStringList files;
    QString resourceDir{QLibraryInfo::path(QLibraryInfo::TranslationsPath)};
    QStringList pluginPaths;
    bool server{false};
    quint16 clientPort{0};
    bool enableInternalDynamicProperties{false};
};

static inline QDesigner::ParseArgumentsResult
    parseDesignerCommandLineArguments(QCommandLineParser &parser, Options *options,
                                      QString *errorMessage)
{
    parser.setApplicationDescription(u"Qt Designer " QT_VERSION_STR "\n\nUI designer for QWidget-based applications."_s);
    const QCommandLineOption helpOption = parser.addHelpOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    const QCommandLineOption serverOption(u"server"_s,
                                          u"Server mode"_s);
    parser.addOption(serverOption);
    const QCommandLineOption clientOption(u"client"_s,
                                          u"Client mode"_s,
                                          u"port"_s);
    parser.addOption(clientOption);
    const QCommandLineOption resourceDirOption(u"resourcedir"_s,
                                          u"Resource directory"_s,
                                          u"directory"_s);
    parser.addOption(resourceDirOption);
    const QCommandLineOption internalDynamicPropertyOption(u"enableinternaldynamicproperties"_s,
                                          u"Enable internal dynamic properties"_s);
    parser.addOption(internalDynamicPropertyOption);
    const QCommandLineOption pluginPathsOption(u"plugin-path"_s,
                                               u"Default plugin path list"_s,
                                               u"path"_s);
    parser.addOption(pluginPathsOption);

    parser.addPositionalArgument(u"files"_s,
                                 u"The UI files to open."_s);
								 
	const QCommandLineOption darkModeOption(QStringLiteral("darkMode"),
			  QStringLiteral("Dark mode"));
    parser.addOption(darkModeOption);

    if (!parser.parse(QCoreApplication::arguments())) {
        *errorMessage = parser.errorText();
        return QDesigner::ParseArgumentsError;
    }

	if (parser.isSet(darkModeOption))
    {
        qDesigner->setDarkModeIsEnabled(true);
    }
	
    if (parser.isSet(helpOption))
        return QDesigner::ParseArgumentsHelpRequested;
    // There is no way to retrieve the complete help text from QCommandLineParser,
    // so, call process() to display it.
    if (parser.isSet(u"help-all"_s))
        parser.process(QCoreApplication::arguments()); // exits
    options->server = parser.isSet(serverOption);
    if (parser.isSet(clientOption)) {
        bool ok;
        options->clientPort = parser.value(clientOption).toUShort(&ok);
        if (!ok) {
            *errorMessage = u"Non-numeric argument specified for -client"_s;
            return QDesigner::ParseArgumentsError;
        }
    }
    if (parser.isSet(resourceDirOption))
        options->resourceDir = parser.value(resourceDirOption);
    const auto pluginPathValues = parser.values(pluginPathsOption);
    for (const auto &pluginPath : pluginPathValues)
        options->pluginPaths.append(pluginPath.split(QDir::listSeparator(), Qt::SkipEmptyParts));

    options->enableInternalDynamicProperties = parser.isSet(internalDynamicPropertyOption);
    options->files = parser.positionalArguments();
    return QDesigner::ParseArgumentsSuccess;
}

QDesigner::ParseArgumentsResult QDesigner::parseCommandLineArguments()
{
    QString errorMessage;
    Options options;
    QCommandLineParser parser;
    const ParseArgumentsResult result = parseDesignerCommandLineArguments(parser, &options, &errorMessage);
    if (result != ParseArgumentsSuccess) {
        showHelp(parser, errorMessage);
        return result;
    }
    // initialize the sub components
    if (options.clientPort)
        m_client = new QDesignerClient(options.clientPort, this);
    if (options.server) {
        m_server = new QDesignerServer();
        printf("%d\n", m_server->serverPort());
        fflush(stdout);
    }
    if (options.enableInternalDynamicProperties)
        QDesignerPropertySheet::setInternalDynamicPropertiesEnabled(true);

    std::unique_ptr<QTranslator> designerTranslator(new QTranslator(this));
    if (designerTranslator->load(QLocale(), u"designer"_s, u"_"_s, options.resourceDir)) {
        installTranslator(designerTranslator.release());
        std::unique_ptr<QTranslator> qtTranslator(new QTranslator(this));
        if (qtTranslator->load(QLocale(), u"qt"_s, u"_"_s, options.resourceDir))
            installTranslator(qtTranslator.release());
    }

    m_workbench = new QDesignerWorkbench(options.pluginPaths);

    emit initialized();
    previousMessageHandler = qInstallMessageHandler(designerMessageHandler); // Warn when loading faulty forms
    Q_ASSERT(previousMessageHandler);

    bool suppressNewFormShow = m_workbench->readInBackup();

    for (auto fileName : std::as_const(options.files)) {
        // Ensure absolute paths for recent file list to be unique
        const QFileInfo fi(fileName);
        if (fi.exists() && fi.isRelative())
            fileName = fi.absoluteFilePath();
        m_workbench->readInForm(fileName);
    }

    if (m_workbench->formWindowCount() > 0)
        suppressNewFormShow = true;

    // Show up error box with parent now if something went wrong
    if (m_initializationErrors.isEmpty()) {
        if (!suppressNewFormShow)
            m_workbench->showNewForm();
    } else {
        showErrorMessageBox(m_initializationErrors);
        m_initializationErrors.clear();
    }
    return result;
}

bool QDesigner::event(QEvent *ev)
{
    bool eaten;
    switch (ev->type()) {
    case QEvent::FileOpen:
        m_workbench->readInForm(static_cast<QFileOpenEvent *>(ev)->file());
        eaten = true;
        break;
    case QEvent::Close: {
        QCloseEvent *closeEvent = static_cast<QCloseEvent *>(ev);
        closeEvent->setAccepted(m_workbench->handleClose());
        if (closeEvent->isAccepted()) {
            // We're going down, make sure that we don't get our settings saved twice.
            if (m_mainWindow)
                m_mainWindow->setCloseEventPolicy(MainWindowBase::AcceptCloseEvents);
            eaten = QApplication::event(ev);
        }
        eaten = true;
        break;
    }
    default:
        eaten = QApplication::event(ev);
        break;
    }
    return eaten;
}

void QDesigner::setMainWindow(MainWindowBase *tw)
{
    m_mainWindow = tw;
}

MainWindowBase *QDesigner::mainWindow() const
{
    return m_mainWindow;
}

QT_END_NAMESPACE