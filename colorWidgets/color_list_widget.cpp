/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2012-2015 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "color_list_widget.hpp"
#include "color_selector.hpp"

namespace color_widgets {

class ColorListWidget::Private
{
public:
    QList<QColor>               colors;
    QSignalMapper               mapper;
    ColorWheel::DisplayFlags  wheel_flags;
};

ColorListWidget::ColorListWidget(QWidget *parent)
    : AbstractWidgetList(parent), p(new Private)
{
    connect(this, SIGNAL(removed(int)), SLOT(handle_removed(int)));
    connect(&p->mapper, SIGNAL(mapped(int)), SLOT(color_changed(int)));
    p->wheel_flags = ColorWheel::defaultDisplayFlags();
}

ColorListWidget::~ColorListWidget()
{
    delete p;
}

QList<QColor> ColorListWidget::colors() const
{
    return p->colors;
}

void ColorListWidget::setColors(const QList<QColor> &colors)
{
    clear();
    p->colors = colors;
    for(int i = 0;i < colors.size();i++ )
        append_widget(i);
    emit colorsChanged(colors);
}

void ColorListWidget::swap(int a, int b)
{
    ColorSelector* sa = widget_cast<ColorSelector>(a);
    ColorSelector* sb = widget_cast<ColorSelector>(b);
    if ( sa && sb )
    {
        QColor ca = sa->color();
        sa->setColor(sb->color());
        sb->setColor(ca);
        emit colorsChanged(p->colors);
    }
}

void ColorListWidget::append()
{
    p->colors.push_back(Qt::black);
    append_widget(p->colors.size()-1);
    emit colorsChanged(p->colors);
}

void ColorListWidget::emit_changed()
{
    emit colorsChanged(p->colors);
}

void ColorListWidget::handle_removed(int i)
{
    p->colors.removeAt(i);
    emit colorsChanged(p->colors);
}

void ColorListWidget::color_changed(int row)
{
    ColorSelector *cs = widget_cast<ColorSelector>(row);
    if ( cs )
    {
        p->colors[row] = cs->color();
        emit colorsChanged(p->colors);
    }
}

void ColorListWidget::append_widget(int col)
{
    ColorSelector* cbs = new ColorSelector;
    cbs->setDisplayMode(ColorPreview::AllAlpha);
    cbs->setColor(p->colors[col]);
    //connect(cbs,SIGNAL(colorChanged(QColor)),SLOT(emit_changed()));
    p->mapper.setMapping(cbs,col);
    connect(cbs,SIGNAL(colorChanged(QColor)),&p->mapper,SLOT(map()));
    connect(this,SIGNAL(wheelFlagsChanged(ColorWheel::DisplayFlags)),
            cbs,SLOT(setWheelFlags(ColorWheel::DisplayFlags)));
    appendWidget(cbs);
    setRowHeight(count()-1,22);
}

ColorWheel::DisplayFlags ColorListWidget::wheelFlags() const
{
    return p->wheel_flags;
}

void ColorListWidget::setWheelFlags(ColorWheel::DisplayFlags flags)
{
    if ( p->wheel_flags != flags )
    {
        p->wheel_flags = flags;
        emit wheelFlagsChanged(flags);
    }
}

} // namespace color_widgets
