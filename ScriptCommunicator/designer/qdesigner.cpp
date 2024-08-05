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

static const char *designerApplicationName = "Designer";
static const char designerDisplayName[] = "Qt Designer";
static const char *designerWarningPrefix = "Designer: ";
static QtMessageHandler previousMessageHandler = nullptr;

static void designerMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Only Designer warnings are displayed as box
    QDesigner *designerApp = qDesigner;
    if (type != QtWarningMsg || !designerApp || !msg.startsWith(QLatin1String(designerWarningPrefix))) {
        previousMessageHandler(type, context, msg);
        return;
    }
    designerApp->showErrorMessage(msg);
}

QDesigner::QDesigner(int &argc, char **argv)
    : QApplication(argc, argv),
      m_server(nullptr),
      m_client(nullptr),
      m_workbench(0), m_suppressNewFormShow(false), m_darkModeIsEnabled(false)
{
    setOrganizationName(QStringLiteral("QtProject"));
    QGuiApplication::setApplicationDisplayName(QLatin1String(designerDisplayName));
    setApplicationName(QLatin1String(designerApplicationName));
    QDesignerComponents::initializeResources();

#if !defined(Q_OS_OSX) && !defined(Q_OS_WIN)
    setWindowIcon(QIcon(QStringLiteral(":/qt-project.org/designer/images/designer.png")));
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
        message.right(message.size() - int(qstrlen(designerWarningPrefix)));
    // If there is no main window yet, just store the message.
    // The QErrorMessage would otherwise be hidden by the main window.
    if (m_mainWindow) {
        showErrorMessageBox(qMessage);
    } else {
        const QMessageLogContext emptyContext;
        previousMessageHandler(QtWarningMsg, emptyContext, message); // just in case we crash
        m_initializationErrors += qMessage;
        m_initializationErrors += QLatin1Char('\n');
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
        const QString title = QCoreApplication::translate("QDesigner", "%1 - warning").arg(QLatin1String(designerApplicationName));
        m_errorMessageDialog->setWindowTitle(title);
        m_errorMessageDialog->setMinimumSize(QSize(600, 250));
        m_errorMessageDialog->setWindowFlags(m_errorMessageDialog->windowFlags() & ~Qt::WindowContextHelpButtonHint);
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
    bool server{false};
    quint16 clientPort{0};
    bool enableInternalDynamicProperties{false};
};

static inline QDesigner::ParseArgumentsResult
    parseDesignerCommandLineArguments(QCommandLineParser &parser, Options *options,
                                      QString *errorMessage)
{
    parser.setApplicationDescription(QStringLiteral("Qt Designer ")
        + QLatin1String(QT_VERSION_STR)
        + QLatin1String("\n\nUI designer for QWidget-based applications."));
    const QCommandLineOption helpOption = parser.addHelpOption();
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    const QCommandLineOption serverOption(QStringLiteral("server"),
                                          QStringLiteral("Server mode"));
    parser.addOption(serverOption);
    const QCommandLineOption clientOption(QStringLiteral("client"),
                                          QStringLiteral("Client mode"),
                                          QStringLiteral("port"));
    parser.addOption(clientOption);
    const QCommandLineOption resourceDirOption(QStringLiteral("resourcedir"),
                                          QStringLiteral("Resource directory"),
                                          QStringLiteral("directory"));
    parser.addOption(resourceDirOption);
    const QCommandLineOption internalDynamicPropertyOption(QStringLiteral("enableinternaldynamicproperties"),
                                          QStringLiteral("Enable internal dynamic properties"));
    parser.addOption(internalDynamicPropertyOption);

    parser.addPositionalArgument(QStringLiteral("files"),
                                 QStringLiteral("The UI files to open."));

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
            *errorMessage = QStringLiteral("Non-numeric argument specified for -client");
            return QDesigner::ParseArgumentsError;
        }
    }
    if (parser.isSet(resourceDirOption))
        options->resourceDir = parser.value(resourceDirOption);
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
    if (designerTranslator->load(QLocale(), QStringLiteral("designer"), QStringLiteral("_"), options.resourceDir)) {
        installTranslator(designerTranslator.release());
        std::unique_ptr<QTranslator> qtTranslator(new QTranslator(this));
        if (qtTranslator->load(QLocale(), QStringLiteral("qt"), QStringLiteral("_"), options.resourceDir))
            installTranslator(qtTranslator.release());
    }

    m_workbench = new QDesignerWorkbench();

    emit initialized();
    previousMessageHandler = qInstallMessageHandler(designerMessageHandler); // Warn when loading faulty forms
    Q_ASSERT(previousMessageHandler);

    m_suppressNewFormShow = m_workbench->readInBackup();

    if (!options.files.isEmpty()) {
        const QStringList::const_iterator cend = options.files.constEnd();
        for (QStringList::const_iterator it = options.files.constBegin(); it != cend; ++it) {
            // Ensure absolute paths for recent file list to be unique
            QString fileName = *it;
            const QFileInfo fi(fileName);
            if (fi.exists() && fi.isRelative())
                fileName = fi.absoluteFilePath();
            m_workbench->readInForm(fileName);
        }
    }
    if ( m_workbench->formWindowCount())
        m_suppressNewFormShow = true;

    // Show up error box with parent now if something went wrong
    if (m_initializationErrors.isEmpty()) {
        if (!m_suppressNewFormShow && QDesignerSettings(m_workbench->core()).showNewFormOnStartup())
            QTimer::singleShot(100, this, &QDesigner::callCreateForm); // won't show anything if suppressed
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
        // Set it true first since, if it's a Qt 3 form, the messagebox from convert will fire the timer.
        m_suppressNewFormShow = true;
        if (!m_workbench->readInForm(static_cast<QFileOpenEvent *>(ev)->file()))
            m_suppressNewFormShow = false;
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

void QDesigner::callCreateForm()
{
    if (!m_suppressNewFormShow)
        m_workbench->actionManager()->createForm();
}

QT_END_NAMESPACE
