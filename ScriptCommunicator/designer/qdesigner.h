// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#ifndef QDESIGNER_H
#define QDESIGNER_H

#include <QtCore/qpointer.h>
#include <QtWidgets/qapplication.h>

QT_BEGIN_NAMESPACE

#define qDesigner \
    (static_cast<QDesigner*>(QCoreApplication::instance()))

class QDesignerWorkbench;
class QDesignerToolWindow;
class MainWindowBase;
class QDesignerServer;
class QDesignerClient;
class QErrorMessage;
class QCommandLineParser;
struct Options;

class QDesigner: public QApplication
{
    Q_OBJECT
public:
    enum ParseArgumentsResult {
        ParseArgumentsSuccess,
        ParseArgumentsError,
        ParseArgumentsHelpRequested
    };

    QDesigner(int &argc, char **argv);
    ~QDesigner() override;

    ParseArgumentsResult parseCommandLineArguments();

    QDesignerWorkbench *workbench() const;
    QDesignerServer *server() const;
    MainWindowBase *mainWindow() const;
    void setMainWindow(MainWindowBase *tw);

    bool darkModeIsEnabled(){return m_darkModeIsEnabled;}
    void setDarkModeIsEnabled(bool isEnabled){m_darkModeIsEnabled = isEnabled;}

protected:
    bool event(QEvent *ev) override;

signals:
    void initialized();

public slots:
    void showErrorMessage(const QString &message);

private slots:
    void callCreateForm();

private:
    void showErrorMessageBox(const QString &);

    QDesignerServer *m_server;
    QDesignerClient *m_client;
    QDesignerWorkbench *m_workbench;
    QPointer<MainWindowBase> m_mainWindow;
    QPointer<QErrorMessage> m_errorMessageDialog;

    QString m_initializationErrors;
    QString m_lastErrorMessage;
    bool m_suppressNewFormShow;

    bool m_darkModeIsEnabled;
};

QT_END_NAMESPACE

#endif // QDESIGNER_H
