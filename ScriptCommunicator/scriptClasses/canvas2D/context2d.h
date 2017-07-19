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

#ifndef CONTEXT2D_H
#define CONTEXT2D_H


#include <QPainter>
#include <QPainterPath>
#include <QString>
#include <QStack>
#include <QMetaType>
#include <QTimerEvent>
#include <QVariant>
#include <QScriptValue>
#include <QScriptable>
#include "scriptObject.h"


// [3]
class CanvasGradient
{
public:
    CanvasGradient(const QGradient &v)
        : value(v) {}
    CanvasGradient() {}

    QGradient value;
};
// [3]

Q_DECLARE_METATYPE(CanvasGradient)
Q_DECLARE_METATYPE(CanvasGradient*)

class ImageData {
};

class QContext2DCanvas;

///Implements a subset of the HTML Canvas Api for worker scripts.
class Context2D : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

    ///Holds the current alpha value applied to rendering operations.
    ///The value must be in the range from 0.0 (fully transparent) to 1.0
    ///(fully opaque). The default value is 1.0.
    Q_PROPERTY(qreal globalAlpha READ globalAlpha WRITE setGlobalAlpha)

    ///Holds the current the current composition operation.
    /// The default value is source-over.
    ///Following values are possible:
    ///- "source-over"
    ///- "destination-over"
    ///- "clear"
    ///- "source"
    ///- "destination"
    ///- "source-in"
    ///- "destination-in"
    ///- "source-out"
    ///- "destination-out"
    ///- "source-atop"
    ///- "destination-atop"
    ///- "xor"
    ///- "plus"
    ///- "multiply"
    ///- "screen"
    ///- "overlay"
    ///- "darken"
    ///- "lighten"
    ///- "color-dodge"
    ///- "color-burn"
    ///- "hard-light"
    ///- "soft-light"
    ///- "difference"
    ///- "exclusion"
    ///Note: See QPainter::CompositionMode for more details.
    Q_PROPERTY(QString globalCompositeOperation READ globalCompositeOperation WRITE setGlobalCompositeOperation)

    ///Holds the current color or style to use for the lines around shapes.
    ///The style can be either a string containing a CSS color, a CanvasGradient or CanvasPattern object. Invalid values are ignored.
    ///The default value is '#000000'.
    Q_PROPERTY(QVariant strokeStyle READ strokeStyle WRITE setStrokeStyle)

    ///Holds the current style used for filling shapes.
    ///The style can be either a string containing a CSS color, a CanvasGradient or CanvasPattern object
    ///The default value is '#000000'.
    Q_PROPERTY(QVariant fillStyle READ fillStyle WRITE setFillStyle)

    ///Holds the current line width. Values that are not finite values greater than zero are ignored.
    ///The default value is 1.
    Q_PROPERTY(qreal lineWidth READ lineWidth WRITE setLineWidth)

    ///Holds the current line cap style. The possible line cap styles are:
    ///
    ///butt - the end of each line has a flat edge perpendicular to the direction of the line, this is the default line cap value.
    ///round - a semi-circle with the diameter equal to the width of the line must then be added on to the end of the line.
    ///square - a rectangle with the length of the line width and the width of half the line width, placed flat against the edge perpendicular to the direction of the line.
    Q_PROPERTY(QString lineCap READ lineCap WRITE setLineCap)

    ///Holds the current line join style. A join exists at any point in a subpath shared by two consecutive lines. When a subpath is closed, then a join also exists at its first point (equivalent to its last point) connecting the first and last lines in the subpath.
    ///The possible line join styles are:
    ///
    ///bevel - this is all that is rendered at joins.
    ///round - a filled arc connecting the two aforementioned corners of the join, abutting (and not overlapping) the aforementioned triangle, with the diameter equal to the line width and the origin at the point of the join, must be rendered at joins.
    ///miter - a second filled triangle must (if it can given the miter length) be rendered at the join, this is the default line join style.
    Q_PROPERTY(QString lineJoin READ lineJoin WRITE setLineJoin)

    ///Holds the current miter limit ratio. The default miter limit value is 10.0.
    Q_PROPERTY(qreal miterLimit READ miterLimit WRITE setMiterLimit)

    ///Holds the current shadow offset in the positive horizontal distance.
    ///The default value is 0.
    Q_PROPERTY(qreal shadowOffsetX READ shadowOffsetX WRITE setShadowOffsetX)

    ///Holds the current shadow offset in the positive vertical distance.
    ///The default value is 0.
    Q_PROPERTY(qreal shadowOffsetY READ shadowOffsetY WRITE setShadowOffsetY)

    ///Holds the current level of blur applied to shadows.
    ///The default value is 0.
    Q_PROPERTY(qreal shadowBlur READ shadowBlur WRITE setShadowBlur)

    ///Holds the current shadow color.
    ///The default value is '#000000'.
    Q_PROPERTY(QString shadowColor READ shadowColor WRITE setShadowColor)

    ///Holds the width of the canvas widget.
    Q_PROPERTY(int width READ width)

    ///Holds the height of the canvas widget.
    Q_PROPERTY(int height READ height)

    ///Holds the current text alignment
    ///The default value is Qt::AlignLeft. Possible values:
    /// - Qt::AlignLeft	0x0001	Aligns with the left edge.
    /// - Qt::AlignRight	0x0002	Aligns with the right edge.
    /// - Qt::AlignHCenter	0x0004	Centers horizontally in the available space.
    /// - Qt::AlignJustify	0x0008	Justifies the text in the available space.
    /// - Qt::AlignTop	0x0020	Aligns with the top.
    /// - Qt::AlignBottom	0x0040	Aligns with the bottom.
    /// - Qt::AlignVCenter	0x0080	Centers vertically in the available space.
    /// Qt::AlignBaseline	0x0100	Aligns with the baseline.
    Q_PROPERTY(int textAlign READ textAlign WRITE setTextAlign)

public:
    Context2D(QObject *parent = 0);

    ///Returns a semicolon separated list with all public functions, signals and properties.
    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptCanvas2DWidget.api");
    }

    ///Returns the current alpha value applied to rendering operations.
    qreal globalAlpha() const;

    ///Returns the current composition operation.
    QString globalCompositeOperation() const;

    ///Returns the current color or style to use for the lines around shapes.
    QVariant strokeStyle() const;

    ///Returns the current style used for filling shapes.
    QVariant fillStyle() const;

    ///Sets the current alpha value applied to rendering operations.
    void setGlobalAlpha(qreal alpha);

    ///Sets the current the current composition operation.
    void setGlobalCompositeOperation(const QString &op);

    ///Sets the current color or style to use for the lines around shapes.
    void setStrokeStyle(const QVariant &style);

    ///Sets the current style used for filling shapes.
    void setFillStyle(const QVariant &style);

    ///Returns the current line width.
    qreal lineWidth() const;

    ///Returns the current line cap style.
    QString lineCap() const;

    ///Returns the current line join style.
    QString lineJoin() const;

    ///Returns the current miter limit ratio.
    qreal miterLimit() const;

    ///Sets the current line width.
    void setLineWidth(qreal w);

    ///Sets the current line cap style.
    void setLineCap(const QString &s);

    ///Sets the current line join style.
    void setLineJoin(const QString &s);

    ///Sets the current miter limit ratio.
    void setMiterLimit(qreal m);

    ///Returns the current shadow offset in the positive horizontal distance.
    qreal shadowOffsetX() const;

    ///Returns the current shadow offset in the positive vertical distance.
    qreal shadowOffsetY() const;

    ///Returns the current level of blur applied to shadows.
    qreal shadowBlur() const;

    ///Returns the current shadow color.
    QString shadowColor() const;

    ///Sets the current shadow offset in the positive horizontal distance.
    void setShadowOffsetX(qreal x);

    ///Sets the current shadow offset in the positive vertical distance.
    void setShadowOffsetY(qreal y);

    ///Sets the current level of blur applied to shadows.
    void setShadowBlur(qreal b);

    ///Sets the current shadow color.
    void setShadowColor(const QString &str);

    ///Sets the current text alignement.
    void setTextAlign(int textAlign){m_state.textAlign = textAlign;}

    ///Returns the current text alignement.
    int textAlign(void){return m_state.textAlign;}


public slots:

    ///Pushes the current state onto the state stack.
    ///
    ///Before changing any state attributes, you should save the current state for future reference. The context maintains a stack
    ///of drawing states. Each state consists of the current transformation matrix, clipping region, and values of the following
    ///attributes:
    ///
    ///strokeStyle
    ///fillStyle
    ///globalAlpha
    ///lineWidth
    ///lineCap
    ///lineJoin
    ///miterLimit
    ///shadowOffsetX
    ///shadowOffsetY
    ///shadowBlur
    ///shadowColor
    ///globalCompositeOperation
    void save();

    ///Pops the top state on the stack, restoring the context to that state.
    void restore();

    ///Increases or decreases the size of each unit in the canvas grid by multiplying
    ///the scale factors to the current tranform matrix. x is the scale factor in the
    ///horizontal direction and y is the scale factor in the vertical direction.
    void scale(qreal x, qreal y);

    ///Rotate the canvas around the current origin by angle in radians and clockwise direction.
    void rotate(qreal angle);

    ///Translates the origin of the canvas by a horizontal distance of x, and a vertical distance of y,
    ///in coordinate space units.
    ///Translating the origin enables you to draw patterns of different objects on the canvas without
    ///having to measure the coordinates manually for each shape.
    void translate(qreal x, qreal y);

    ///This method is very similar to setTransform(), but instead of replacing the old transform matrix,
    ///this method applies the given tranform matrix to the current matrix by multiplying to it.
    void transform(qreal m11, qreal m12, qreal m21, qreal m22,
                   qreal dx, qreal dy);

    ///Changes the transformation matrix to the matrix given by the arguments as described below.
    ///Modifying the transformation matrix directly enables you to perform scaling, rotating,
    ///and translating transformations in a single step.
    ///Each point on the canvas is multiplied by the matrix before anything is drawn.
    ///For more details see the HTML Canvas 2D Context specification.
    void setTransform(qreal m11, qreal m12, qreal m21, qreal m22,
                      qreal dx, qreal dy);

    ///Returns a CanvasGradient object that represents a linear gradient that transitions the color along a
    ///line between the start point (x0, y0) and the end point (x1, y1).
    ///A gradient is a smooth transition between colors. There are two types of gradients: linear and radial.
    ///Gradients must have two or more color stops, representing color shifts positioned from 0 to 1 between
    ///to the gradient's starting and end points or circles.
    CanvasGradient createLinearGradient(qreal x0, qreal y0,
                                        qreal x1, qreal y1);

    ///Returns a CanvasGradient object that represents a radial gradient that paints along the cone given by
    ///the start circle with origin (x0, y0) and radius r0, and the end circle with origin (x1, y1) and radius r1.
    CanvasGradient createRadialGradient(qreal x0, qreal y0,
                                        qreal r0, qreal x1,
                                        qreal y1, qreal r1);

    ///Clears all pixels on the canvas in the given rectangle to transparent black.
    void clearRect(qreal x, qreal y, qreal w, qreal h);

    ///Paint the specified rectangular area using the fillStyle.
    void fillRect(qreal x, qreal y, qreal w, qreal h);

    ///Sets the current font.
    void setFont(QString family, int pixelSize = -1, int weight = -1, bool italic = false);

    ///Draws the given text beginning at the given position.
    void fillText(qreal x, qreal y, qreal w, qreal h, QString text);

    ///Stroke the specified rectangle's path using the strokeStyle, lineWidth,
    ///lineJoin, and (if appropriate) miterLimit attributes.
    void strokeRect(qreal x, qreal y, qreal w, qreal h);

    ///Resets the current path to a new path.
    void beginPath();

    ///Closes the current subpath by drawing a line to the beginning of the subpath, automatically starting a
    ///new path. The current point of the new path is the previous subpath's first point.
    void closePath();

    ///Creates a new subpath with the given point.
    void moveTo(qreal x, qreal y);

    ///Draws a line from the current position to the point (x, y).
    void lineTo(qreal x, qreal y);

    ///Adds a quadratic bezier curve between the current point and the endpoint (x, y) with the control point specified by (cpx, cpy).
    void quadraticCurveTo(qreal cpx, qreal cpy, qreal x, qreal y);

    ///Adds a cubic bezier curve between the current position and the given endPoint using the control points specified by (cp1x, cp1y)
    ///and (cp2x, cp2y). After the curve is added, the current position is updated to be at the end point (x, y) of the curve.
    void bezierCurveTo(qreal cp1x, qreal cp1y,
                       qreal cp2x, qreal cp2y, qreal x, qreal y);

    ///Adds an arc with the given control points and radius to the current subpath, connected to the previous point by a straight line.
    void arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal radius);

    ///Adds a rectangle at position (x, y), with the given width w and height h, as a closed subpath.
    void rect(qreal x, qreal y, qreal w, qreal h);

    ///Adds an arc to the current subpath that lies on the circumference of the circle whose center is at the point (x, y) and whose radius is radius.
    ///Both startAngle and endAngle are measured from the x-axis in radians.
    ///The anticlockwise parameter is true for each arc in the figure above because they are all drawn in the anticlockwise direction.
    void arc(qreal x, qreal y, qreal radius,
             qreal startAngle, qreal endAngle,
             bool anticlockwise);

    ///Fills the subpaths with the current fill style.
    void fill();

    ///Strokes the subpaths with the current stroke style.
    void stroke();

    ///Creates the clipping region from the current path. Any parts of the shape outside the clipping path are not displayed.
    void clip();

    ///Returns true if the given point is in the current path.
    bool isPointInPath(qreal x, qreal y) const;

    ///Clears the canvas widget.
    void clear();

    ///Resets the canvas widget.
    void reset();

    ///Returns the width of the canvas widget.
    int width() const;

    ///Returns the height of the canvas widget.
    int height() const;

    ///Save the canvas widget to an image file.
    bool saveToFile(QString fileName, QString imageType="");

    ///Opens a print dialog and prints the canvas widget.
    void print(QString printDialogTitle="");

    ///Sets the size of the canvas widget.
    ///This slot must not be used by script.
    void setSize(int width, int height);

signals:

    ///Is emitted if the user press a mouse button inside the widget.
    ///Note: mouseButton has the type Qt::MouseButton.
    ///Scripts can connect to this signal.
    void mousePressSignal(int xValue, int yValue, quint32 mouseButton);

    ///Is emitted if the user releases a mouse button inside the widget.
    ///Note: mouseButton has the type Qt::MouseButton.
    ///Scripts can connect to this signal.
    void mouseReleasSignal(int xValue, int yValue, quint32 mouseButton);

    ///Is emitted if the user moves the mouse inside the widget.
    ///Scripts can connect to this signal.
    void mouseMoveSignal(int xValue, int yValue);

    ///Is emitted if the user press a key inside the widget.
    ///Note:  modifiers has the type Qt::KeyboardModifiers.
    ///       keyCode has the type Qt::Key
    ///Scripts can connect to this signal.
    void keyPressSignal(QString keyText, int keyCode, quint32 modifiers);

    ///Is emitted if the user releases a key inside the widget.
    ///Note:  modifiers has the type Qt::KeyboardModifiers
    ///       keyCode has the type Qt::Key.
    ///Scripts can connect to this signal.
    void keyReleaseSignal(QString key, int keyCode, quint32 modifiers);

    ///Is emitted if the size of the canvas widget has been changed.
    void sizeChangeSignal(int width, int height);

    ///Is emitted if the canvas widget has been changed.
    ///This signal is private and must not be used inside a script.
    void changed(const QImage &image);

    ///Is emitted i print.
    ///This signal is private and must not be used inside a script.
    void printSignal(QString printDialogTitle);

protected:

    ///The change timer has been elapsed.
    void timerEvent(QTimerEvent *e);

private:
    ///Is called before a painting begins.
    void beginPainting();

    ///Is called if a painting is finished.
    const QImage &endPainting();

    ///Starts the change timer if the canvas widget has been changed.
    void scheduleChange();

    ///Id of the change timer.
    int m_changeTimerId;

    ///the image in which the canvas widget is drawing.
    QImage  m_image;

    ///The painter for drawing.
    QPainter m_painter;

    ///The painter path.
    QPainterPath m_path;

    enum DirtyFlag {
        DirtyTransformationMatrix = 0x00001,
        DirtyClippingRegion       = 0x00002,
        DirtyStrokeStyle          = 0x00004,
        DirtyFillStyle            = 0x00008,
        DirtyGlobalAlpha          = 0x00010,
        DirtyLineWidth            = 0x00020,
        DirtyLineCap              = 0x00040,
        DirtyLineJoin             = 0x00080,
        DirtyMiterLimit           = 0x00100,
        MDirtyPen                 = DirtyStrokeStyle
                                  | DirtyLineWidth
                                  | DirtyLineCap
                                  | DirtyLineJoin
                                  | DirtyMiterLimit,
        DirtyShadowOffsetX        = 0x00200,
        DirtyShadowOffsetY        = 0x00400,
        DirtyShadowBlur           = 0x00800,
        DirtyShadowColor          = 0x01000,
        DirtyGlobalCompositeOperation = 0x2000,
        DirtyFont                 = 0x04000,
        DirtyTextAlign            = 0x08000,
        DirtyTextBaseline         = 0x10000,
        AllIsFullOfDirt           = 0xfffff
    };

    struct State {
        State() : flags(0) {}
        QMatrix matrix;
        QPainterPath clipPath;
        QBrush strokeStyle;
        QBrush fillStyle;

        ///Holds the current alpha value applied to rendering operations.
        qreal globalAlpha;
        qreal lineWidth;
        Qt::PenCapStyle lineCap;
        Qt::PenJoinStyle lineJoin;
        qreal miterLimit;
        qreal shadowOffsetX;
        qreal shadowOffsetY;
        qreal shadowBlur;
        QColor shadowColor;
        QPainter::CompositionMode globalCompositeOperation;
        QFont font;
        int textAlign;
        int textBaseline;
        int flags;
    };

    ///The current state.
    State m_state;

    ///The state stack.
    QStack<State> m_stateStack;
};

#endif
