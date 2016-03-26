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

#ifndef QCONTEXT2DCANVAS_H
#define QCONTEXT2DCANVAS_H

#include <qscriptengine.h>
#include <qscriptcontext.h>
#include <qscriptvalue.h>
#include <QTimer>

#include <QWidget>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QKeyEvent;
QT_END_NAMESPACE
class Environment;
class Context2D;


///This widget renders the image which is created by a Context2D object.
class QContext2DCanvas : public QWidget
{
    Q_OBJECT
public:
    QContext2DCanvas(bool runsInDebugger, QWidget *parent = 0);
    ~QContext2DCanvas();

    ///Connects all necessary signals.
    void connectContextSignals(Context2D *context);

signals:
    ///Is emitted if the size of the widget has been changed.
    void setContextSizeSignal(int width, int height);

protected:
    ///Paint event.
    virtual void paintEvent(QPaintEvent *e);

    ///Mouse move event.
    virtual void mouseMoveEvent(QMouseEvent *e);

    ///Mouse press event.
    virtual void mousePressEvent(QMouseEvent *e);

    ///Mouse release event.
    virtual void mouseReleaseEvent(QMouseEvent *e);

    ///Key press event.
    virtual void keyPressEvent(QKeyEvent *e);

    ///Key release event.
    virtual void keyReleaseEvent(QKeyEvent *e);

    ///Resize event.
    virtual void resizeEvent(QResizeEvent *e);

public slots:

    ///Prints the canvas widget.
    void printSlot(QString printDialogTitle);


private slots:
    void contentsChanged(const QImage &image);
    void timerElapsed();


private:

    ///The image which is rendered by this class.
    QImage m_image;

    ///Resize timer.
    QTimer m_sizeTimer;

    ///The context object which draws/creates the image.
    Context2D *m_context;

    ///True if the script runs in the script debugger.
    bool m_runsInDebugger;
};

#endif
