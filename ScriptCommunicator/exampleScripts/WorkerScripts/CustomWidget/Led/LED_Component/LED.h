#ifndef _LED_H_
#define _LED_H_

#include <QtDesigner/QtDesigner>
#include <QWidget>

class QTimer;

/********************Exported functions needed by ScriptCommunicator.**********************/
/**
* Creates the wrapper classe
* @param scriptThread
*      Pointer to the script thread.
* @param customWidget
*      The custom widget.
* @param scriptRunsInDebugger
*       True if the script thread runs in a script debugger.
*/
extern "C" Q_DECL_EXPORT QObject *CreateScriptCommunicatorWidget(QObject *scriptThread, QWidget* customWidget, bool scriptRunsInDebugger);

///Returns the class name of the custom widget.
extern "C" Q_DECL_EXPORT const char* GetScriptCommunicatorWidgetName(void);

/*************************************************************************************/

///The custom widget class.
class QDESIGNER_WIDGET_EXPORT LED : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(double diameter READ diameter WRITE setDiameter) // mm
    Q_PROPERTY(QColor color READ color WRITE setColor)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool state READ state WRITE setState)
    Q_PROPERTY(bool flashing READ isFlashing WRITE setFlashing)
    Q_PROPERTY(int flashRate READ flashRate WRITE setFlashRate)
    Q_PROPERTY(int alphaForOff READ alphaForOff WRITE setAlphaForOff)

public:
    explicit LED(QWidget* parent=0);
    ~LED();

    double diameter() const;
    void setDiameter(double diameter);

    QColor color() const;

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment alignment);

    bool state() const;

    bool isFlashing() const;
    
    int flashRate() const;

    int alphaForOff() const;

public slots:
    void setState(bool state);
    void toggleState();
    void setFlashing(bool flashing);
    void setFlashRate(int rate);
    void startFlashing();
    void stopFlashing();
    void setAlphaForOff(int value);
    void setColor(QColor color);

    //QtDesigner sometimes sets the visible property to false (a bug in QtDesigner?) therefore setVisible is ignored here.
    void setVisible(bool visible){(void) visible; QWidget::setVisible(true);}

public:
    int heightForWidth(int width) const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

signals:
    void toggleStateSignal();
    void setFlashingeSignal(bool flashing);

    //Script communication...return
    void stateChangedSignal(bool state);

protected:
    void paintEvent(QPaintEvent* event);

private:
    double m_diameter;
    QColor m_color;
    Qt::Alignment m_alignment;
    bool m_initialState;
    bool state_;


    //
    // Pixels per mm for x and y...
    //
    int m_pixX, m_pixY;

    //
    // Scaled values for x and y diameter.
    //
    int m_diamX, m_diamY;
    QTimer* m_timer;
    int m_alphaForOff;
    int m_flashRate;
    bool m_flashing;
};


///The custom widget wrapper class.
///A worker script access the custom widget through is class.
///Important:
///A function from a custom widget class must not be called directly from a worker-script(a worker script runs in his own thread
///and the custom widget runs like all GUI elements in the main thread). Instead a slot must be called (by a signal).
///This signal must be connected with Qt::QueuedConnection or Qt::BlockingQueuedConnection.
///
///If the worker script runs in a debugger then Qt::DirectConnection instead of Qt::BlockingQueuedConnection must be used (
///the debugger and therefore the worker script runs in the main thread). Qt::BlockingQueuedConnection would cause a dead-lock.
class  ScriptLed : public QObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    ///Note: ScriptCommunicator uses this property for showing more information
    ///during an exception.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptLed(QObject* scriptThread, QWidget* led, bool scriptRunsInDebugger) : QObject(scriptThread), m_led(static_cast<LED*>(led))
    {
        //Note: If scriptRunsInDebugger is true Qt::DirectConnection instead of Qt::BlockingQueuedConnection
        //must be used (the wrapper ScriptLed and the LED widget are running in the main thread
        //if scriptRunsInDebugger true).
        (void)scriptRunsInDebugger;
        connect(this, SIGNAL(toggleStateSignal()), m_led, SLOT(toggleState()), Qt::QueuedConnection);
        connect(this, SIGNAL(setFlashingeSignal(bool)), m_led, SLOT(setFlashing(bool)), Qt::QueuedConnection);
        connect(this, SIGNAL(setStateSignal(bool)), m_led, SLOT(setState(bool)), Qt::QueuedConnection);
        connect(this, SIGNAL(setColorSignal(QColor)), m_led, SLOT(setColor(QColor)), Qt::QueuedConnection);
        connect(this, SIGNAL(setFlashRateSignal(int)), m_led, SLOT(setFlashRate(int)), Qt::QueuedConnection);

    }
    ScriptLed()
    {

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    ///Note: ScriptCommunicator uses this property/function for showing more information
    ///during an exception.
    virtual QString getPublicScriptElements(void)
    {
        return "void toggleState(void);void setFlashing(bool flashing);"
                "void setState(bool state);void stateChangedSlot(bool state);stateChangedSignal(bool state);"
                "setColorRgb(quint8 red, quint8 green, quint8 blue);setFlashRate(int rate)";
    }

    ///Toggles the state of the LED (on/off).
    Q_INVOKABLE void toggleState(void){emit toggleStateSignal();}

    ///Turns the flashing of the LED on or off.
    Q_INVOKABLE void setFlashing(bool flashing){emit setFlashingeSignal(flashing);}

    ///Sets the state of the LED (on/off).
    Q_INVOKABLE void setState(bool state){emit setStateSignal(state);}

    ///Sets the color of the LED.
    Q_INVOKABLE void setColorRgb(quint8 red, quint8 green, quint8 blue)
    {
        emit setColorSignal(QColor(red, green, blue));
    }

    ///Sets the flahing rate of the LED (ms).
    Q_INVOKABLE void  setFlashRate(int rate){emit setFlashRateSignal(rate);}



signals:

    ///Is emitted if the state of the led has been changed.
    void stateChangedSignal(bool state);


    ///This signals are private and must not be used inside a script.
    void toggleStateSignal(void);
    void setFlashingeSignal(bool flashing);
    void setStateSignal(bool state);
    void setColorSignal(QColor color);
    void setFlashRateSignal(int rate);


private:
    LED* m_led;

};



#endif
