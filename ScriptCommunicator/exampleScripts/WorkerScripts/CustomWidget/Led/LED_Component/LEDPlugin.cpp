#include "LED.h"
#include "LEDPlugin.h"

#include <QtPlugin>

LEDPlugin::
LEDPlugin(QObject* parent) :
	QObject(parent),
	initialized(false)
{
}

QString LEDPlugin::
name() const
{
	return "LED";
}

QString LEDPlugin::
group() const
{
    return tr("Custom Widgets");
}

QString LEDPlugin::
toolTip() const
{
	return tr("An LED");
}

QString LEDPlugin::
whatsThis() const
{
	return tr("An LED");
}

QString LEDPlugin::
includeFile() const
{
	return "LED.h";
}

QIcon LEDPlugin::
icon() const
{
    return QIcon(":/icons/led.png");
}

// custom startup properties
QString LEDPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"LED\" name=\"led\">\n" //pay attention to this line do not confuse class with name

           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>32</width>\n"
           "    <height>32</height>\n"
           "   </rect>\n"
           "  </property>\n"

           "  <property name=\"visible\">\n"
           "   <bool>false</bool>\n"
           "  </property>\n"

           " </widget>\n"
           "</ui>\n";
}

bool LEDPlugin::
isContainer() const
{
    return false; //ok
}

QWidget * LEDPlugin::
createWidget(QWidget *parent)
{
	return new LED(parent);
}

#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
Q_EXPORT_PLUGIN2(ledplugin, LEDPlugin)
#endif
