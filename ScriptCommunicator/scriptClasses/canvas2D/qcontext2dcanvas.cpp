/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qcontext2dcanvas.h"

#include "context2d.h"

#include <QPainter>
#include <QPaintEvent>
#include <QPrinter>
#include <QPrintDialog>


/**
 * Constructor
 * @param parent
 *      The parent object.
 */
QContext2DCanvas::QContext2DCanvas(bool runsInDebugger, QWidget *parent)
    : QWidget(parent), m_runsInDebugger(runsInDebugger)
{
    setMouseTracking(true);
    QObject::connect(&m_sizeTimer, SIGNAL(timeout()), this, SLOT(timerElapsed()));

    setFocusPolicy(Qt::StrongFocus);
}


/**
 * @Destructor.
 */
QContext2DCanvas::~QContext2DCanvas()
{
}

/**
 * Connects all necessary signals.
 * @param context
 *      The Context2D object which drawes the image which is rendered by the QContext2DCanvas class.
 */
void QContext2DCanvas::connectContextSignals(Context2D *context)
{
    Qt::ConnectionType directConnectionType = m_runsInDebugger ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

    m_context = context;
    QObject::connect(context, SIGNAL(changed(QImage)), this, SLOT(contentsChanged(QImage)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(setContextSizeSignal(int,int)), context, SLOT(setSize(int,int)), Qt::QueuedConnection);
    QObject::connect(context, SIGNAL(printSignal(QString)), this, SLOT(printSlot(QString)), directConnectionType);
}

/**
 * The content of the image has been changed.
 * @param image
 *      The image.
 */
void QContext2DCanvas::contentsChanged(const QImage &image)
{
    m_image = image;
    update();
}

/**
 * Prints the canvas widget.
 */
void QContext2DCanvas::printSlot(QString printDialogTitle)
{
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);

    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(printDialogTitle);

    if (dialog.exec() == QDialog::Accepted)
    {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = m_image.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(),
                            size.width(), size.height());
        painter.setWindow(m_image.rect());
        painter.drawImage(0, 0, m_image);
    }
}
/**
 * Paint event.
 * @param e
 *      The paint event.
 */
void QContext2DCanvas::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setClipRect(e->rect());
    p.drawImage(0, 0, m_image);
}


/**
 * Mouse move event.
 * @param e
 *      The mouse move event.
 */
void QContext2DCanvas::mouseMoveEvent(QMouseEvent *e)
{
    emit m_context->mouseMoveSignal(e->x(), e->y());

}

/**
 * Mouse press event.
 * @param e
 *      The mouse press event.
 */
void QContext2DCanvas::mousePressEvent(QMouseEvent *e)
{
    emit m_context->mousePressSignal(e->x(), e->y(), (quint32) e->button());
}

/**
 * Mouse release event.
 * @param e
 *      The mouse release event.
 */
void QContext2DCanvas::mouseReleaseEvent(QMouseEvent *e)
{

    emit m_context->mouseReleasSignal(e->x(), e->y(), (quint32) e->button());
}

/**
 * Key press event.
 * @param e
 *      The key press event.
 */
void QContext2DCanvas::keyPressEvent(QKeyEvent *e)
{
    emit m_context->keyPressSignal(e->text(), e->key(), (quint32)e->modifiers());
}

/**
 * Key release event.
 * @param e
 *      The key release event.
 */
void QContext2DCanvas::keyReleaseEvent(QKeyEvent *e)
{
    emit m_context->keyReleaseSignal(e->text(), e->key(), (quint32)e->modifiers());

}

/**
 * Resize event.
 * @param e
 *      The resize event.
 */
void QContext2DCanvas::resizeEvent(QResizeEvent *e)
{
    (void)e;
    m_sizeTimer.start(50);
}

/**
 * The resize timer has been elapsed.
 */
void QContext2DCanvas::timerElapsed()
{
    m_sizeTimer.stop();
    emit setContextSizeSignal(width(), height());

}


