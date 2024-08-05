// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mainwindow.h"
#include "qdesigner.h"
#include "qdesigner_actions.h"
#include "qdesigner_workbench.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_toolwindow.h"
#include "qdesigner_settings.h"
#include "qttoolbardialog.h"

#include <QtDesigner/abstractformwindow.h>

#include <QtWidgets/qtoolbar.h>
#include <QtWidgets/qmdisubwindow.h>
#include <QtWidgets/qstatusbar.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qlayout.h>
#include <QtWidgets/qdockwidget.h>

#include <QtGui/qaction.h>
#include <QtGui/qactiongroup.h>
#include <QtGui/qevent.h>

#include <QtCore/qurl.h>
#include <QtCore/qdebug.h>
#include <QtCore/qmimedata.h>

#include <algorithm>

#include <QResource>
#include <QFile>

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

using ActionList = QList<QAction *>;

// Helpers for creating toolbars and menu

static void addActionsToToolBar(const ActionList &actions, QToolBar *t)
{
    for (QAction *action : actions) {
        if (action->property(QDesignerActions::defaultToolbarPropertyName).toBool())
            t->addAction(action);
    }
}

static QToolBar *createToolBar(const QString &title, const QString &objectName,
                               const ActionList &actions)
{
    QToolBar *rc =  new QToolBar;
    rc->setObjectName(objectName);
    rc->setWindowTitle(title);
    addActionsToToolBar(actions, rc);
    return rc;
}

// ---------------- MainWindowBase

MainWindowBase::MainWindowBase(QWidget *parent, Qt::WindowFlags flags) :
    QMainWindow(parent, flags)
{
#ifndef Q_OS_MACOS
    setWindowIcon(qDesigner->windowIcon());
#endif

    if(qDesigner->darkModeIsEnabled())
    {
        (void)QResource::registerResource(QCoreApplication::applicationDirPath() + "/stylesheet.rcc");
#ifdef Q_OS_LINUX
        QFile file(QCoreApplication::applicationDirPath() + "/stylesheetDesignerLinux.qss");
#else
        QFile file(QCoreApplication::applicationDirPath() + "/stylesheet.qss");
#endif
        QString styleSheet;
        if(file.exists())
        {

            file.open(QFile::ReadOnly);
            styleSheet= QLatin1String(file.readAll());
            qApp->setStyleSheet(styleSheet);

        }
    }
}

void MainWindowBase::closeEvent(QCloseEvent *e)
{
    switch (m_policy) {
    case AcceptCloseEvents:
        QMainWindow::closeEvent(e);
        break;
      case EmitCloseEventSignal:
        emit closeEventReceived(e);
        break;
    }
}

QList<QToolBar *> MainWindowBase::createToolBars(const QDesignerActions *actions, bool singleToolBar)
{
    // Note that whenever you want to add a new tool bar here, you also have to update the default
    // action groups added to the toolbar manager in the mainwindow constructor
    QList<QToolBar *> rc;
    if (singleToolBar) {
        //: Not currently used (main tool bar)
        QToolBar *main = createToolBar(tr("Main"), u"mainToolBar"_s,
                                       actions->fileActions()->actions());
        addActionsToToolBar(actions->editActions()->actions(), main);
        addActionsToToolBar(actions->toolActions()->actions(), main);
        addActionsToToolBar(actions->formActions()->actions(), main);
        rc.push_back(main);
    } else {
        rc.append(createToolBar(tr("File"), u"fileToolBar"_s,
                                actions->fileActions()->actions()));
        rc.append(createToolBar(tr("Edit"), u"editToolBar"_s,
                                actions->editActions()->actions()));
        rc.append(createToolBar(tr("Tools"), u"toolsToolBar"_s,
                                actions->toolActions()->actions()));
        rc.append(createToolBar(tr("Form"), u"formToolBar"_s,
                                actions->formActions()->actions()));
    }
    return rc;
}

QString MainWindowBase::mainWindowTitle()
{
    return tr("Qt Designer");
}

// Use the minor Qt version as settings versions to avoid conflicts
int MainWindowBase::settingsVersion()
{
    const int version = QT_VERSION;
    return (version & 0x00FF00) >> 8;
}

// ----------------- DockedMdiArea

DockedMdiArea::DockedMdiArea(const QString &extension, QWidget *parent) :
    QMdiArea(parent),
    m_extension(extension)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    setLineWidth(1);
    setAcceptDrops(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

QStringList DockedMdiArea::uiFiles(const QMimeData *d) const
{
    // Extract dropped UI files from Mime data.
    QStringList rc;
    if (!d->hasFormat("text/uri-list"_L1))
        return rc;
    const auto urls = d->urls();
    if (urls.isEmpty())
        return rc;
    for (const auto &url : urls) {
        const QString fileName = url.toLocalFile();
        if (!fileName.isEmpty() && fileName.endsWith(m_extension))
            rc.push_back(fileName);
    }
    return rc;
}

bool DockedMdiArea::event(QEvent *event)
{
    // Listen for desktop file manager drop and emit a signal once a file is
    // dropped.
    switch (event->type()) {
    case QEvent::DragEnter: {
        QDragEnterEvent *e = static_cast<QDragEnterEvent*>(event);
        if (!uiFiles(e->mimeData()).isEmpty()) {
            e->acceptProposedAction();
            return true;
        }
    }
        break;
    case QEvent::Drop: {
        QDropEvent *e = static_cast<QDropEvent*>(event);
        const QStringList files = uiFiles(e->mimeData());
        for (const auto &f : files)
            emit fileDropped(f);
        e->acceptProposedAction();
        return true;
    }
        break;
    default:
        break;
    }
    return QMdiArea::event(event);
}

// ------------- ToolBarManager:

static void addActionsToToolBarManager(const ActionList &al, const QString &title, QtToolBarManager *tbm)
{
    for (QAction *action : al)
        tbm->addAction(action, title);
}

ToolBarManager::ToolBarManager(QMainWindow *configureableMainWindow,
                                         QWidget *parent,
                                         QMenu *toolBarMenu,
                                         const QDesignerActions *actions,
                                         const QList<QToolBar *> &toolbars,
                                         const QList<QDesignerToolWindow *> &toolWindows) :
    QObject(parent),
    m_configureableMainWindow(configureableMainWindow),
    m_parent(parent),
    m_toolBarMenu(toolBarMenu),
    m_manager(new QtToolBarManager(this)),
    m_configureAction(new QAction(tr("Configure Toolbars..."), this)),
    m_toolbars(toolbars)
{
    m_configureAction->setMenuRole(QAction::NoRole);
    m_configureAction->setObjectName(u"__qt_configure_tool_bars_action"_s);
    connect(m_configureAction, &QAction::triggered, this, &ToolBarManager::configureToolBars);

    m_manager->setMainWindow(configureableMainWindow);

    for (QToolBar *tb : std::as_const(m_toolbars)) {
        const QString title = tb->windowTitle();
        m_manager->addToolBar(tb, title);
        addActionsToToolBarManager(tb->actions(), title, m_manager);
    }

    addActionsToToolBarManager(actions->windowActions()->actions(), tr("Window"), m_manager);
    addActionsToToolBarManager(actions->helpActions()->actions(), tr("Help"), m_manager);

    // Filter out the device profile preview actions which have int data().
    ActionList previewActions = actions->styleActions()->actions();
    auto it = previewActions.begin();
    for ( ; (*it)->isSeparator() || (*it)->data().metaType().id() == QMetaType::Int; ++it) ;
    previewActions.erase(previewActions.begin(), it);
    addActionsToToolBarManager(previewActions, tr("Style"), m_manager);

    const QString dockTitle = tr("Dock views");
    for (QDesignerToolWindow *tw : toolWindows) {
        if (QAction *action = tw->action())
            m_manager->addAction(action, dockTitle);
    }

    addActionsToToolBarManager(actions->fileActions()->actions(), tr("File"), m_manager);
    addActionsToToolBarManager(actions->editActions()->actions(), tr("Edit"), m_manager);
    addActionsToToolBarManager(actions->toolActions()->actions(), tr("Tools"), m_manager);
    addActionsToToolBarManager(actions->formActions()->actions(), tr("Form"), m_manager);

    m_manager->addAction(m_configureAction, tr("Toolbars"));
    updateToolBarMenu();
}

// sort function for sorting tool bars alphabetically by title [non-static since called from template]

bool toolBarTitleLessThan(const QToolBar *t1, const QToolBar *t2)
{
    return t1->windowTitle() < t2->windowTitle();
}

void ToolBarManager::updateToolBarMenu()
{
    // Sort tool bars alphabetically by title
    std::stable_sort(m_toolbars.begin(), m_toolbars.end(), toolBarTitleLessThan);
    // add to menu
    m_toolBarMenu->clear();
    for (QToolBar *tb : std::as_const(m_toolbars))
        m_toolBarMenu->addAction(tb->toggleViewAction());
    m_toolBarMenu->addAction(m_configureAction);
}

void ToolBarManager::configureToolBars()
{
    QtToolBarDialog dlg(m_parent);
    dlg.setToolBarManager(m_manager);
    dlg.exec();
    updateToolBarMenu();
}

QByteArray ToolBarManager::saveState(int version) const
{
    return m_manager->saveState(version);
}

bool ToolBarManager::restoreState(const QByteArray &state, int version)
{
    return m_manager->restoreState(state, version);
}

// ---------- DockedMainWindow

DockedMainWindow::DockedMainWindow(QDesignerWorkbench *wb,
                                   QMenu *toolBarMenu,
                                   const QList<QDesignerToolWindow *> &toolWindows)
{
    setObjectName(u"MDIWindow"_s);
    setWindowTitle(mainWindowTitle());

    const QList<QToolBar *> toolbars = createToolBars(wb->actionManager(), false);
    for (QToolBar *tb : toolbars)
        addToolBar(tb);
    DockedMdiArea *dma = new DockedMdiArea(wb->actionManager()->uiExtension());
    connect(dma, &DockedMdiArea::fileDropped,
            this, &DockedMainWindow::fileDropped);
    connect(dma, &QMdiArea::subWindowActivated,
            this, &DockedMainWindow::slotSubWindowActivated);
    setCentralWidget(dma);

    QStatusBar *sb = statusBar();
    Q_UNUSED(sb);

    m_toolBarManager = new ToolBarManager(this, this, toolBarMenu, wb->actionManager(), toolbars, toolWindows);
}

QMdiArea *DockedMainWindow::mdiArea() const
{
    return static_cast<QMdiArea *>(centralWidget());
}

void DockedMainWindow::slotSubWindowActivated(QMdiSubWindow* subWindow)
{
    if (subWindow) {
        QWidget *widget = subWindow->widget();
        if (QDesignerFormWindow *fw = qobject_cast<QDesignerFormWindow*>(widget)) {
            emit formWindowActivated(fw);
            mdiArea()->setActiveSubWindow(subWindow);
        }
    }
}

// Create a MDI subwindow for the form.
QMdiSubWindow *DockedMainWindow::createMdiSubWindow(QWidget *fw, Qt::WindowFlags f, const QKeySequence &designerCloseActionShortCut)
{
    QMdiSubWindow *rc = mdiArea()->addSubWindow(fw, f);
    // Make action shortcuts respond only if focused to avoid conflicts with
    // designer menu actions
    if (designerCloseActionShortCut == QKeySequence(QKeySequence::Close)) {
        const ActionList systemMenuActions = rc->systemMenu()->actions();
        for (auto *a : systemMenuActions) {
            if (a->shortcut() == designerCloseActionShortCut) {
                a->setShortcutContext(Qt::WidgetShortcut);
                break;
            }
        }
    }
    return rc;
}

DockedMainWindow::DockWidgetList DockedMainWindow::addToolWindows(const DesignerToolWindowList &tls)
{
    DockWidgetList rc;
    for (QDesignerToolWindow *tw : tls) {
        QDockWidget *dockWidget = new QDockWidget;
        dockWidget->setObjectName(tw->objectName() + "_dock"_L1);
        dockWidget->setWindowTitle(tw->windowTitle());
        addDockWidget(tw->dockWidgetAreaHint(), dockWidget);
        dockWidget->setWidget(tw);
        rc.push_back(dockWidget);
    }
    return rc;
}

// Settings consist of MainWindow state and tool bar manager state
void DockedMainWindow::restoreSettings(const QDesignerSettings &s, const DockWidgetList &dws, const QRect &desktopArea)
{
    const int version = settingsVersion();
    m_toolBarManager->restoreState(s.toolBarsState(DockedMode), version);

    // If there are no old geometry settings, show the window maximized
    s.restoreGeometry(this, QRect(desktopArea.topLeft(), QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)));

    const QByteArray mainWindowState = s.mainWindowState(DockedMode);
    const bool restored = !mainWindowState.isEmpty() && restoreState(mainWindowState, version);
    if (!restored) {
        // Default: Tabify less relevant windows bottom/right.
        tabifyDockWidget(dws.at(QDesignerToolWindow::SignalSlotEditor),
                         dws.at(QDesignerToolWindow::ActionEditor));
        tabifyDockWidget(dws.at(QDesignerToolWindow::ActionEditor),
                         dws.at(QDesignerToolWindow::ResourceEditor));
    }
}

void DockedMainWindow::saveSettings(QDesignerSettings &s) const
{
    const int version = settingsVersion();
    s.setToolBarsState(DockedMode, m_toolBarManager->saveState(version));
    s.saveGeometryFor(this);
    s.setMainWindowState(DockedMode, saveState(version));
}

QT_END_NAMESPACE