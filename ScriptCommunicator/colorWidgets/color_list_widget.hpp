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
#ifndef COLOR_LIST_WIDGET_HPP
#define COLOR_LIST_WIDGET_HPP

#include "abstract_widget_list.hpp"
#include "color_wheel.hpp"

namespace color_widgets {

class ColorListWidget : public AbstractWidgetList
{
    Q_OBJECT

    Q_PROPERTY(QList<QColor> colors READ colors WRITE setColors NOTIFY colorsChanged )
    Q_PROPERTY(ColorWheel::DisplayFlags wheelFlags READ wheelFlags WRITE setWheelFlags NOTIFY wheelFlagsChanged)

public:
    explicit ColorListWidget(QWidget *parent = 0);
    ~ColorListWidget();

    QList<QColor> colors() const;
    void setColors(const QList<QColor>& colors);

    void swap(int a, int b);

    void append();

    ColorWheel::DisplayFlags wheelFlags() const;

signals:
    void colorsChanged(const QList<QColor>&);
    void wheelFlagsChanged(ColorWheel::DisplayFlags flags);

public slots:
    void setWheelFlags(ColorWheel::DisplayFlags flags);

private slots:
    void emit_changed();
    void handle_removed(int);
    void color_changed(int row);

private:
    class Private;
    Private * const p;
    void  append_widget(int col);
};

} // namespace color_widgets

#endif // COLOR_LIST_WIDGET_HPP
