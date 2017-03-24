#include <math.h>

#include <QPainter>
#include <QGradient>
#include <QPaintDevice>
#include <QTimer>

#include "LED.h"

///Returns the class name of the custom widget.
const char* GetScriptCommunicatorWidgetName(void)
{
    return "LED";
}

/**
* Creates the wrapper classe
* @param scriptThread
*      Pointer to the script thread.
* @param customWidget
*      The custom widget.
* @param scriptRunsInDebugger
*       True if the script thread runs in a script debugger.
*/
QObject *CreateScriptCommunicatorWidget(QObject *scriptThread, QWidget *customWidget, bool scriptRunsInDebugger)
{
    return new ScriptLed(scriptThread, customWidget, scriptRunsInDebugger);
}
LED::
LED(QWidget* parent) :
	QWidget(parent),
    m_diameter(8),
    m_color(QColor("red")),
    m_alignment(Qt::AlignCenter),
    m_initialState(true),
	state_(true),
    m_alphaForOff(80),
    m_flashRate(200),
    m_flashing(false)
{
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(toggleState()));

    setDiameter(m_diameter);

}

LED::
~LED()
{
}

double LED::
diameter() const
{
    return m_diameter;
}

void LED::
setDiameter(double diameter)
{
    m_diameter = diameter;

    m_pixX = round(double(height())/heightMM());
    m_pixY = round(double(width())/widthMM());

    m_diamX = m_diameter*m_pixX;
    m_diamY = m_diameter*m_pixY;

	update();
}


QColor LED::
color() const
{
    return m_color;
}

void LED::
setColor(QColor color)
{
    m_color = color;
	update();
}

Qt::Alignment LED::
alignment() const
{
    return m_alignment;
}

void LED::
setAlignment(Qt::Alignment alignment)
{
    m_alignment = alignment;

	update();
}

void LED::
setFlashRate(int rate)
{
    m_flashRate = rate;
	update();
}

void LED::
setFlashing(bool flashing)
{
    m_flashing = flashing;
	update();

}

void LED::
startFlashing()
{
	setFlashing(true);
    //emit setFlashingeSignal(true);
}

void LED::
stopFlashing()
{
	setFlashing(false);
}


void LED::
setState(bool state)
{
	state_ = state;
	update();
    //emit toggleStateSignal();
}


void LED::setAlphaForOff(int value)
{
    m_alphaForOff = value;
}

void LED::
toggleState()
{
	state_ = !state_;
    update();
    emit stateChangedSignal(state_);
}

int LED::
heightForWidth(int width) const
{
	return width;
}

QSize LED::
sizeHint() const
{
    return QSize(m_diamX, m_diamY);
}

QSize LED::
minimumSizeHint() const
{
    return QSize(m_diamX, m_diamY);
}

void LED::
paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

	QPainter p(this);

    //return; // this is for transparency

	QRect geo = geometry();
	int width = geo.width();
	int height = geo.height();

	int x=0, y=0;
    if ( m_alignment & Qt::AlignLeft )
		x = 0;
    else if ( m_alignment & Qt::AlignRight )
        x = width-m_diamX;
    else if ( m_alignment & Qt::AlignHCenter )
        x = (width-m_diamX)/2;
    else if ( m_alignment & Qt::AlignJustify )
		x = 0;

    if ( m_alignment & Qt::AlignTop )
		y = 0;
    else if ( m_alignment & Qt::AlignBottom )
        y = height-m_diamY;
    else if ( m_alignment & Qt::AlignVCenter )
        y = (height-m_diamY)/2;

    QRadialGradient g(x+m_diamX/2, y+m_diamY/2, m_diamX*0.4,
        m_diamX*0.4, m_diamY*0.4);

	g.setColorAt(0, Qt::white);
	if ( state_ )
        g.setColorAt(1, m_color);
	else
    {
        QColor tmpColor = m_color;
         tmpColor.setAlpha(m_alphaForOff);
        g.setColorAt(1, tmpColor);
    }
	QBrush brush(g);

    p.setPen(m_color);
	p.setRenderHint(QPainter::Antialiasing, true);
	p.setBrush(brush);
    p.drawEllipse(x, y, m_diamX-1, m_diamY-1);

    if ( m_flashRate > 0 && m_flashing )
        m_timer->start(m_flashRate);
	else
        m_timer->stop();
}

bool LED::
state() const
{
    return state_;
}

bool LED::
isFlashing() const
{
    return m_flashing;
}
    
int LED::
flashRate() const
{
    return m_flashRate;
}


int LED::alphaForOff() const
{
    return m_alphaForOff;
}
