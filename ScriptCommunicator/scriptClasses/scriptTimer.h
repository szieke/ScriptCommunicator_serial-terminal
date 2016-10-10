#ifndef SCRIPTTIMER
#define SCRIPTTIMER

#include <QTimer>
#include "scriptObject.h"

///This wrapper class is used to access a QTimer from a script.
class ScriptTimer : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    explicit ScriptTimer(QObject *parent) : QObject(parent), m_timer()
    {
        //connect the necessary signals with the wrapper slots (in this slots the
        //events of the wrapper class are generated, the script can connect to this
        //wrapper events)
        connect(&m_timer, SIGNAL(timeout()),this, SIGNAL(timeoutSignal()));
        connect(&m_timer, SIGNAL(timeout()),this, SIGNAL(timeout()));


    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptTimer.api");
    }


    ///Sets the timer to single-shot or non-single-shot.
    ///A single-shot timer fires only once, non-single-shot timers fire every interval milliseconds.
    Q_INVOKABLE void setSingleShot(bool singleShot){m_timer.setSingleShot(singleShot);}

    ///Returns true if the timer is a single-shot timer.
    Q_INVOKABLE bool isSingleShot(void){return m_timer.isSingleShot();}

    ///Sets the timer interval.
    Q_INVOKABLE void setInterval(int msec){m_timer.setInterval(msec);}

    ///Returns the timer interval.
    Q_INVOKABLE int interval(void){return m_timer.interval();}

    ///Starts or restarts the timer with a timeout interval of msec milliseconds.
    ///If the timer is already running, it will be stopped and restarted.
    ///If the timer is a single-shot timer, then the timer will be activated only once.
    Q_INVOKABLE void start(int msec){m_timer.start(msec);}

    ///Starts or restarts the timer with the timeout interval set in setInterval.
    ///If the timer is already running, it will be stopped and restarted.
    ///If the timer is a single-shot timer, then the timer will be activated only once.
    Q_INVOKABLE void start(void){m_timer.start();}

    ///Stops the timer.
    Q_INVOKABLE void stop(void){m_timer.stop();}

    ///Returns true if the timer is running, otherwise false.
    Q_INVOKABLE bool isActive(void){return m_timer.isActive();}

    ///Returns the timer's remaining value in milliseconds left until the timeout.
    ///If the timer is inactive, the returned value will be -1. If the timer is overdue,
    ///the returned value will be 0.
    Q_INVOKABLE int remainingTime(void){return m_timer.remainingTime();}

signals:

    ///This signal is emitted if the timer times out.
    ///Scripts can connect a function to this signal.
    void timeoutSignal(void);

    ///This signal is emitted if the timer times out.
    ///Note: This signal is deprecated and shall not be used anymore.
    void timeout(void);

private:

    ///The wrapped timer.
    QTimer m_timer;

};
#endif // SCRIPTTIMER

