#ifndef SEARCHCONSOLE_H
#define SEARCHCONSOLE_H

#include <QDialog>
#include "mainwindow.h"
#include <QTimer>

namespace Ui {
class SearchConsole;
}

class SearchConsole : public QObject
{
    Q_OBJECT
    friend class MainWindow;

public:
    explicit SearchConsole(MainWindow* mainWindow);
    ~SearchConsole();

    ///Returns the last search strings (separated with ;).
    QString getLastSearchStrings(void);

    ///Sets the last search strings.
    void setLastSearchStrings(QString string);

    ///This function activtes or deactivates the search button
    ///(depends on the current console tab and the current seaarch text).
    void activateDeactiveSearchButton();

private slots:

    ///Button find slot.
    void findButtonClickedSlot(void);

    ///Is call if the user changes the search text.
    void currentSearchTextChangedSlot(QString text);

    ///Is called if the result timer has elapsed
    void timerSlot(void);

private:

    ///The max. number of search-strings which are saved (in  m_lastSearchString).
    static const qint32 MAX_LAST_SEARCH_STRING = 20;

    ///Pointer to the main window.
    MainWindow* m_mainWindow;

    ///The result timer.
    QTimer m_resultTimer;

    QVector<QString> m_lastSearchString;
};

#endif // SEARCHCONSOLE_H
