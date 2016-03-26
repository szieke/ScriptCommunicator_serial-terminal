/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2013-2015 Mattia Basaglia
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
#include "color_selector.hpp"
#include "color_dialog.hpp"
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>

namespace color_widgets {

class ColorSelector::Private
{
public:
    UpdateMode update_mode;
    ColorDialog *dialog;
    QColor old_color;

    Private(QWidget *widget) : dialog(new ColorDialog(widget))
    {
        dialog->setButtonMode(ColorDialog::OkCancel);
    }
};

ColorSelector::ColorSelector(QWidget *parent) :
    ColorPreview(parent), p(new Private(this))
{
    setUpdateMode(Continuous);
    p->old_color = color();

    connect(this,SIGNAL(clicked()),this,SLOT(showDialog()));
    connect(this,SIGNAL(colorChanged(QColor)),this,SLOT(update_old_color(QColor)));
    connect(p->dialog,SIGNAL(rejected()),this,SLOT(reject_dialog()));
    connect(p->dialog,SIGNAL(colorSelected(QColor)), this, SLOT(accept_dialog()));
    connect(p->dialog,SIGNAL(wheelFlagsChanged(ColorWheel::DisplayFlags)),
                SIGNAL(wheelFlagsChanged(ColorWheel::DisplayFlags)));

    setAcceptDrops(true);
}

ColorSelector::~ColorSelector()
{
    delete p;
}

ColorSelector::UpdateMode ColorSelector::updateMode() const
{
    return p->update_mode;
}

void ColorSelector::setUpdateMode(UpdateMode m)
{
    p->update_mode = m;
}

Qt::WindowModality ColorSelector::dialogModality() const
{
    return p->dialog->windowModality();
}

void ColorSelector::setDialogModality(Qt::WindowModality m)
{
    p->dialog->setWindowModality(m);
}

ColorWheel::DisplayFlags ColorSelector::wheelFlags() const
{
    return p->dialog->wheelFlags();
}

void ColorSelector::showDialog()
{
    p->old_color = color();
    p->dialog->setColor(color());
    connect_dialog();
    p->dialog->show();
}

void ColorSelector::setWheelFlags(ColorWheel::DisplayFlags flags)
{
    p->dialog->setWheelFlags(flags);
}

void ColorSelector::connect_dialog()
{
    if (p->update_mode == Continuous)
        connect(p->dialog, SIGNAL(colorChanged(QColor)), this, SLOT(setColor(QColor)), Qt::UniqueConnection);
    else
        disconnect_dialog();
}

void ColorSelector::disconnect_dialog()
{
    disconnect(p->dialog, SIGNAL(colorChanged(QColor)), this, SLOT(setColor(QColor)));
}

void ColorSelector::accept_dialog()
{
    setColor(p->dialog->color());
    p->old_color = color();
}

void ColorSelector::reject_dialog()
{
    setColor(p->old_color);
}

void ColorSelector::update_old_color(const QColor &c)
{
    if (!p->dialog->isVisible())
        p->old_color = c;
}

void ColorSelector::dragEnterEvent(QDragEnterEvent *event)
{
    if ( event->mimeData()->hasColor() ||
         ( event->mimeData()->hasText() && QColor(event->mimeData()->text()).isValid() ) )
        event->acceptProposedAction();
}


void ColorSelector::dropEvent(QDropEvent *event)
{
    if ( event->mimeData()->hasColor() )
    {
        setColor(event->mimeData()->colorData().value<QColor>());
        event->accept();
    }
    else if ( event->mimeData()->hasText() )
    {
        QColor col(event->mimeData()->text());
        if ( col.isValid() )
        {
            setColor(col);
            event->accept();
        }
    }
}

} // namespace color_widgets
