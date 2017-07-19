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

#include "context2d.h"

#include <QVariant>
#include <QImageWriter>
#include <QPrinter>
#include <QPrintDialog>

#include <math.h>
static const double Q_PI   = 3.14159265358979323846;   // pi

#define DEGREES(t) ((t) * 180.0 / Q_PI)

#define qClamp(val, min, max) qMin(qMax(val, min), max)
static QList<qreal> parseNumbersList(QString::const_iterator &itr)
{
    QList<qreal> points;
    QString temp;
    while ((*itr).isSpace())
        ++itr;
    while ((*itr).isNumber() ||
           (*itr) == '-' || (*itr) == '+' || (*itr) == '.') {
        temp = QString();

        if ((*itr) == '-')
            temp += *itr++;
        else if ((*itr) == '+')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        if ((*itr) == '.')
            temp += *itr++;
        while ((*itr).isDigit())
            temp += *itr++;
        while ((*itr).isSpace())
            ++itr;
        if ((*itr) == ',')
            ++itr;
        points.append(temp.toDouble());
        //eat spaces
        while ((*itr).isSpace())
            ++itr;
    }

    return points;
}

QColor colorFromString(const QString &name)
{
    QString::const_iterator itr = name.constBegin();
    QList<qreal> compo;
    if (name.startsWith("rgba(")) {
        ++itr; ++itr; ++itr; ++itr; ++itr;
        compo = parseNumbersList(itr);
        if (compo.size() != 4) {
            return QColor();
        }
        //alpha seems to be always between 0-1
        compo[3] *= 255;
        return QColor((int)compo[0], (int)compo[1],
                      (int)compo[2], (int)compo[3]);
    } else if (name.startsWith("rgb(")) {
        ++itr; ++itr; ++itr; ++itr;
        compo = parseNumbersList(itr);
        if (compo.size() != 3) {
            return QColor();
        }
        return QColor((int)qClamp(compo[0], qreal(0), qreal(255)),
                      (int)qClamp(compo[1], qreal(0), qreal(255)),
                      (int)qClamp(compo[2], qreal(0), qreal(255)));
    } else {
        //QRgb color;
        //CSSParser::parseColor(name, color);
        return QColor(name);
    }
}


static QPainter::CompositionMode compositeOperatorFromString(const QString &compositeOperator)
{
    if ( compositeOperator == "source-over" ) {
        return QPainter::CompositionMode_SourceOver;
    } else if ( compositeOperator == "source-out" ) {
        return QPainter::CompositionMode_SourceOut;
    } else if ( compositeOperator == "source-in" ) {
        return QPainter::CompositionMode_SourceIn;
    } else if ( compositeOperator == "source-atop" ) {
        return QPainter::CompositionMode_SourceAtop;
    } else if ( compositeOperator == "destination-atop" ) {
        return QPainter::CompositionMode_DestinationAtop;
    } else if ( compositeOperator == "destination-in" ) {
        return QPainter::CompositionMode_DestinationIn;
    } else if ( compositeOperator == "destination-out" ) {
        return QPainter::CompositionMode_DestinationOut;
    } else if ( compositeOperator == "destination-over" ) {
        return QPainter::CompositionMode_DestinationOver;
    } else if ( compositeOperator == "darker" ) {
        return QPainter::CompositionMode_SourceOver;
    } else if ( compositeOperator == "lighter" ) {
        return QPainter::CompositionMode_SourceOver;
    } else if ( compositeOperator == "copy" ) {
        return QPainter::CompositionMode_Source;
    } else if ( compositeOperator == "xor" ) {
        return QPainter::CompositionMode_Xor;
    }

    return QPainter::CompositionMode_SourceOver;
}

static QString compositeOperatorToString(QPainter::CompositionMode op)
{
    switch (op) {
    case QPainter::CompositionMode_SourceOver:
        return "source-over";
    case QPainter::CompositionMode_DestinationOver:
        return "destination-over";
    case QPainter::CompositionMode_Clear:
        return "clear";
    case QPainter::CompositionMode_Source:
        return "source";
    case QPainter::CompositionMode_Destination:
        return "destination";
    case QPainter::CompositionMode_SourceIn:
        return "source-in";
    case QPainter::CompositionMode_DestinationIn:
        return "destination-in";
    case QPainter::CompositionMode_SourceOut:
        return "source-out";
    case QPainter::CompositionMode_DestinationOut:
        return "destination-out";
    case QPainter::CompositionMode_SourceAtop:
        return "source-atop";
    case QPainter::CompositionMode_DestinationAtop:
        return "destination-atop";
    case QPainter::CompositionMode_Xor:
        return "xor";
    case QPainter::CompositionMode_Plus:
        return "plus";
    case QPainter::CompositionMode_Multiply:
        return "multiply";
    case QPainter::CompositionMode_Screen:
        return "screen";
    case QPainter::CompositionMode_Overlay:
        return "overlay";
    case QPainter::CompositionMode_Darken:
        return "darken";
    case QPainter::CompositionMode_Lighten:
        return "lighten";
    case QPainter::CompositionMode_ColorDodge:
        return "color-dodge";
    case QPainter::CompositionMode_ColorBurn:
        return "color-burn";
    case QPainter::CompositionMode_HardLight:
        return "hard-light";
    case QPainter::CompositionMode_SoftLight:
        return "soft-light";
    case QPainter::CompositionMode_Difference:
        return "difference";
    case QPainter::CompositionMode_Exclusion:
        return "exclusion";
    default:
        break;
    }
    return QString();
}

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
void Context2D::save()
{
    m_stateStack.push(m_state);
}

///Pops the top state on the stack, restoring the context to that state.
void Context2D::restore()
{
    if (!m_stateStack.isEmpty()) {
        m_state = m_stateStack.pop();
        m_state.flags = AllIsFullOfDirt;
    }
}

///Increases or decreases the size of each unit in the canvas grid by multiplying
///the scale factors to the current tranform matrix. x is the scale factor in the
///horizontal direction and y is the scale factor in the vertical direction.
void Context2D::scale(qreal x, qreal y)
{
    m_state.matrix.scale(x, y);
    m_state.flags |= DirtyTransformationMatrix;
}

///Rotate the canvas around the current origin by angle in radians and clockwise direction.
void Context2D::rotate(qreal angle)
{
    m_state.matrix.rotate(DEGREES(angle));
    m_state.flags |= DirtyTransformationMatrix;
}

///Translates the origin of the canvas by a horizontal distance of x, and a vertical distance of y,
///in coordinate space units.
///Translating the origin enables you to draw patterns of different objects on the canvas without
///having to measure the coordinates manually for each shape.
void Context2D::translate(qreal x, qreal y)
{
    m_state.matrix.translate(x, y);
    m_state.flags |= DirtyTransformationMatrix;
}

///This method is very similar to setTransform(), but instead of replacing the old transform matrix,
///this method applies the given tranform matrix to the current matrix by multiplying to it.
void Context2D::transform(qreal m11, qreal m12, qreal m21, qreal m22,
                          qreal dx, qreal dy)
{
    QMatrix mat(m11, m12,
                m21, m22,
                dx, dy);
    m_state.matrix *= mat;
    m_state.flags |= DirtyTransformationMatrix;
}

///Changes the transformation matrix to the matrix given by the arguments as described below.
///Modifying the transformation matrix directly enables you to perform scaling, rotating,
///and translating transformations in a single step.
///Each point on the canvas is multiplied by the matrix before anything is drawn.
///For more details see the HTML Canvas 2D Context specification.
void Context2D::setTransform(qreal m11, qreal m12, qreal m21, qreal m22,
                             qreal dx, qreal dy)
{
    QMatrix mat(m11, m12,
                m21, m22,
                dx, dy);
    m_state.matrix = mat;
    m_state.flags |= DirtyTransformationMatrix;
}


////Returns the current the current composition operation.
QString Context2D::globalCompositeOperation() const
{
    return compositeOperatorToString(m_state.globalCompositeOperation);
}

////Sets the current the current composition operation.
void Context2D::setGlobalCompositeOperation(const QString &op)
{
    QPainter::CompositionMode mode =
        compositeOperatorFromString(op);
    m_state.globalCompositeOperation = mode;
    m_state.flags |= DirtyGlobalCompositeOperation;
}

///Returns the current color or style to use for the lines around shapes.
QVariant Context2D::strokeStyle() const
{
    return m_state.strokeStyle;
}

///Sets the current color or style to use for the lines around shapes.
void Context2D::setStrokeStyle(const QVariant &style)
{
    if (style.canConvert<CanvasGradient>()) {
        CanvasGradient cg = qvariant_cast<CanvasGradient>(style);
        m_state.strokeStyle = cg.value;
    } else {
        QColor color = colorFromString(style.toString());
        m_state.strokeStyle = color;
    }
    m_state.flags |= DirtyStrokeStyle;
}

///Returns the current style used for filling shapes.
QVariant Context2D::fillStyle() const
{
    return m_state.fillStyle;
}

///Sets the current style used for filling shapes.
void Context2D::setFillStyle(const QVariant &style)
{
    if (style.canConvert<CanvasGradient>()) {
        CanvasGradient cg = qvariant_cast<CanvasGradient>(style);
        m_state.fillStyle = cg.value;
    } else {
        QColor color = colorFromString(style.toString());
        m_state.fillStyle = color;
    }
    m_state.flags |= DirtyFillStyle;
}

///Returns the current alpha value applied to rendering operations.
qreal Context2D::globalAlpha() const
{
    return m_state.globalAlpha;
}

///Sets the current alpha value applied to rendering operations.
void Context2D::setGlobalAlpha(qreal alpha)
{
    m_state.globalAlpha = alpha;
    m_state.flags |= DirtyGlobalAlpha;
}


///Returns a CanvasGradient object that represents a linear gradient that transitions the color along a
///line between the start point (x0, y0) and the end point (x1, y1).
///A gradient is a smooth transition between colors. There are two types of gradients: linear and radial.
///Gradients must have two or more color stops, representing color shifts positioned from 0 to 1 between
///to the gradient's starting and end points or circles.
CanvasGradient Context2D::createLinearGradient(qreal x0, qreal y0,
                                               qreal x1, qreal y1)
{
    QLinearGradient g(x0, y0, x1, y1);
    return CanvasGradient(g);
}

///Returns a CanvasGradient object that represents a radial gradient that paints along the cone given by
///the start circle with origin (x0, y0) and radius r0, and the end circle with origin (x1, y1) and radius r1.
CanvasGradient Context2D::createRadialGradient(qreal x0, qreal y0,
                                               qreal r0, qreal x1,
                                               qreal y1, qreal r1)
{
    QRadialGradient g(QPointF(x1, y1), r0+r1, QPointF(x0, y0));
    return CanvasGradient(g);
}

///Returns the current line width.
qreal Context2D::lineWidth() const
{
    return m_state.lineWidth;
}

///Sets the current line width.
void Context2D::setLineWidth(qreal w)
{
    m_state.lineWidth = w;
    m_state.flags |= DirtyLineWidth;
}

///Returns the current line cap style.
QString Context2D::lineCap() const
{
    switch (m_state.lineCap) {
    case Qt::FlatCap:
        return "butt";
    case Qt::SquareCap:
        return "square";
    case Qt::RoundCap:
        return "round";
    default: ;
    }
    return QString();
}

void Context2D::setLineCap(const QString &capString)
{
    Qt::PenCapStyle style;
    if (capString == "round")
        style = Qt::RoundCap;
    else if (capString == "square")
        style = Qt::SquareCap;
    else //if (capString == "butt")
        style = Qt::FlatCap;
    m_state.lineCap = style;
    m_state.flags |= DirtyLineCap;
}


///Returns the current line join style.
QString Context2D::lineJoin() const
{
    switch (m_state.lineJoin) {
    case Qt::RoundJoin:
        return "round";
    case Qt::BevelJoin:
        return "bevel";
    case Qt::MiterJoin:
        return "miter";
    default: ;
    }
    return QString();
}

void Context2D::setLineJoin(const QString &joinString)
{
    Qt::PenJoinStyle style;
    if (joinString == "round")
        style = Qt::RoundJoin;
    else if (joinString == "bevel")
        style = Qt::BevelJoin;
    else //if (joinString == "miter")
        style = Qt::MiterJoin;
    m_state.lineJoin = style;
    m_state.flags |= DirtyLineJoin;
}

///Return the current miter limit ratio.
qreal Context2D::miterLimit() const
{
    return m_state.miterLimit;
}

///Sets the current miter limit ratio.
void Context2D::setMiterLimit(qreal m)
{
    m_state.miterLimit = m;
    m_state.flags |= DirtyMiterLimit;
}

///Sets the current shadow offset in the positive horizontal distance.
void Context2D::setShadowOffsetX(qreal x)
{
    m_state.shadowOffsetX = x;
    m_state.flags |= DirtyShadowOffsetX;
}

 ///Sets the current shadow offset in the positive vertical distance.
void Context2D::setShadowOffsetY(qreal y)
{
    m_state.shadowOffsetY = y;
    m_state.flags |= DirtyShadowOffsetY;
}

///Sets the current level of blur applied to shadows.
void Context2D::setShadowBlur(qreal b)
{
    m_state.shadowBlur = b;
    m_state.flags |= DirtyShadowBlur;
}

///Sets the current shadow color.
void Context2D::setShadowColor(const QString &str)
{
    m_state.shadowColor = colorFromString(str);
    m_state.flags |= DirtyShadowColor;
}

///Returns the current shadow offset in the positive horizontal distance.
qreal Context2D::shadowOffsetX() const
{
    return m_state.shadowOffsetX;
}

///Returns the current shadow offset in the positive vertical distance.
qreal Context2D::shadowOffsetY() const
{
    return m_state.shadowOffsetY;
}

///Returns the current level of blur applied to shadows.
qreal Context2D::shadowBlur() const
{
    return m_state.shadowBlur;
}

///Returns the current shadow color.
QString Context2D::shadowColor() const
{
    return m_state.shadowColor.name();
}

///Clears all pixels on the canvas in the given rectangle to transparent black.
void Context2D::clearRect(qreal x, qreal y, qreal w, qreal h)
{
    beginPainting();
    m_painter.save();
    m_painter.setMatrix(m_state.matrix, false);
    m_painter.setCompositionMode(QPainter::CompositionMode_Source);
    m_painter.fillRect(QRectF(x, y, w, h), QColor(0, 0, 0, 0));
    m_painter.restore();
    scheduleChange();
}


///Paint the specified rectangular area using the fillStyle.
void Context2D::fillRect(qreal x, qreal y, qreal w, qreal h)
{
    beginPainting();
    m_painter.save();
    m_painter.setMatrix(m_state.matrix, false);
    m_painter.fillRect(QRectF(x, y, w, h), m_painter.brush());
    m_painter.restore();
    scheduleChange();
}

/**
 * Sets the current font.
 * Possible value for weigth:
 *  - QFont::Thin	0
 * - QFont::ExtraLight	12
 * - QFont::Light	25
 * - QFont::Normal	50
 * - QFont::Medium	57
 * - QFont::DemiBold	63
 * - QFont::Bold	75
 * - QFont::ExtraBold	81
 * - QFont::Black	87
 */
void Context2D::setFont(QString family, int pixelSize, int weight, bool italic)
{
    m_state.font = QFont(family, m_state.font.pointSize(), weight, italic);
    m_state.font.setPixelSize(pixelSize);
    m_state.flags |= DirtyFont;
}

///Draws the given text beginning at the given position.
void Context2D::fillText(qreal x, qreal y, qreal w, qreal h, QString text)
{
    beginPainting();
    m_painter.save();
    m_painter.setMatrix(m_state.matrix, false);
    m_painter.setFont(m_state.font);
    m_painter.drawText(QRectF(x, y, w, h), m_state.textAlign, text);
    m_painter.restore();
    scheduleChange();
}

///Stroke the specified rectangle's path using the strokeStyle, lineWidth,
///lineJoin, and (if appropriate) miterLimit attributes.
void Context2D::strokeRect(qreal x, qreal y, qreal w, qreal h)
{
    QPainterPath path;
    path.addRect(x, y, w, h);
    beginPainting();
    m_painter.save();
    m_painter.setMatrix(m_state.matrix, false);
    m_painter.strokePath(path, m_painter.pen());
    m_painter.restore();
    scheduleChange();
}

///Resets the current path to a new path.
void Context2D::beginPath()
{
    m_path = QPainterPath();
}

///Closes the current subpath by drawing a line to the beginning of the subpath, automatically starting a
///new path. The current point of the new path is the previous subpath's first point.
void Context2D::closePath()
{
    m_path.closeSubpath();
}

///Creates a new subpath with the given point.
void Context2D::moveTo(qreal x, qreal y)
{
    QPointF pt = m_state.matrix.map(QPointF(x, y));
    m_path.moveTo(pt);
}

///Draws a line from the current position to the point (x, y).
void Context2D::lineTo(qreal x, qreal y)
{
    QPointF pt = m_state.matrix.map(QPointF(x, y));
    m_path.lineTo(pt);
}

///Adds a quadratic bezier curve between the current point and the endpoint (x, y) with the control point specified by (cpx, cpy).
void Context2D::quadraticCurveTo(qreal cpx, qreal cpy, qreal x, qreal y)
{
    QPointF cp = m_state.matrix.map(QPointF(cpx, cpy));
    QPointF xy = m_state.matrix.map(QPointF(x, y));
    m_path.quadTo(cp, xy);
}

///Adds a cubic bezier curve between the current position and the given endPoint using the control points specified by (cp1x, cp1y)
///and (cp2x, cp2y). After the curve is added, the current position is updated to be at the end point (x, y) of the curve.
void Context2D::bezierCurveTo(qreal cp1x, qreal cp1y,
                              qreal cp2x, qreal cp2y, qreal x, qreal y)
{
    QPointF cp1 = m_state.matrix.map(QPointF(cp1x, cp1y));
    QPointF cp2 = m_state.matrix.map(QPointF(cp2x, cp2y));
    QPointF end = m_state.matrix.map(QPointF(x, y));
    m_path.cubicTo(cp1, cp2, end);
}

///Adds an arc with the given control points and radius to the current subpath, connected to the previous point by a straight line.
void Context2D::arcTo(qreal x1, qreal y1, qreal x2, qreal y2, qreal radius)
{
    //FIXME: this is surely busted
    QPointF st  = m_state.matrix.map(QPointF(x1, y1));
    QPointF end = m_state.matrix.map(QPointF(x2, y2));
    m_path.arcTo(st.x(), st.y(),
                 end.x()-st.x(), end.y()-st.y(),
                 radius, 90);
}

///Adds a rectangle at position (x, y), with the given width w and height h, as a closed subpath.
void Context2D::rect(qreal x, qreal y, qreal w, qreal h)
{
    QPainterPath path; path.addRect(x, y, w, h);
    path = m_state.matrix.map(path);
    m_path.addPath(path);
}

///Adds an arc to the current subpath that lies on the circumference of the circle whose center is at the point (x, y) and whose radius is radius.
///Both startAngle and endAngle are measured from the x-axis in radians.
///The anticlockwise parameter is true for each arc in the figure above because they are all drawn in the anticlockwise direction.
void Context2D::arc(qreal xc, qreal yc, qreal radius,
                    qreal sar, qreal ear,
                    bool anticlockwise)
{
    //### HACK
    // In Qt we don't switch the coordinate system for degrees
    // and still use the 0,0 as bottom left for degrees so we need
    // to switch
    sar = -sar;
    ear = -ear;
    anticlockwise = !anticlockwise;
    //end hack

    float sa = DEGREES(sar);
    float ea = DEGREES(ear);

    double span = 0;

    double xs     = xc - radius;
    double ys     = yc - radius;
    double width  = radius*2;
    double height = radius*2;

    if (!anticlockwise && (ea < sa)) {
        span += 360;
    } else if (anticlockwise && (sa < ea)) {
        span -= 360;
    }

    //### this is also due to switched coordinate system
    // we would end up with a 0 span instead of 360
    if (!(qFuzzyCompare(span + (ea - sa) + 1, 1) &&
          qFuzzyCompare(qAbs(span), 360))) {
        span   += ea - sa;
    }

    QPainterPath path;
    path.moveTo(QPointF(xc + radius  * cos(sar),
                                yc - radius  * sin(sar)));

    path.arcTo(xs, ys, width, height, sa, span);
    path = m_state.matrix.map(path);
    m_path.addPath(path);
}

///Fills the subpaths with the current fill style.
void Context2D::fill()
{
    beginPainting();
    m_painter.fillPath(m_path, m_painter.brush());
    scheduleChange();
}

///Strokes the subpaths with the current stroke style.
void Context2D::stroke()
{
    beginPainting();
    m_painter.save();
    m_painter.setMatrix(m_state.matrix, false);
    QPainterPath tmp = m_state.matrix.inverted().map(m_path);
    m_painter.strokePath(tmp, m_painter.pen());
    m_painter.restore();
    scheduleChange();
}

///Creates the clipping region from the current path. Any parts of the shape outside the clipping path are not displayed.
void Context2D::clip()
{
    m_state.clipPath = m_path;
    m_state.flags |= DirtyClippingRegion;
}

///Returns true if the given point is in the current path.
bool Context2D::isPointInPath(qreal x, qreal y) const
{
    return m_path.contains(QPointF(x, y));
}


Context2D::Context2D(QObject *parent)
    : QObject(parent), m_changeTimerId(-1)
{
    reset();
}

///Is called if a painting is finished.
const QImage &Context2D::endPainting()
{
    if (m_painter.isActive())
        m_painter.end();
    return m_image;
}

///Is called before a painting begins.
void Context2D::beginPainting()
{
    if (!m_painter.isActive()) {
        m_painter.begin(&m_image);
        m_painter.setRenderHint(QPainter::Antialiasing);
        if (!m_state.clipPath.isEmpty())
            m_painter.setClipPath(m_state.clipPath);
        m_painter.setBrush(m_state.fillStyle);
        m_painter.setOpacity(m_state.globalAlpha);
        QPen pen;
        pen.setBrush(m_state.strokeStyle);
        if (pen.style() == Qt::NoPen)
            pen.setStyle(Qt::SolidLine);
        pen.setCapStyle(m_state.lineCap);
        pen.setJoinStyle(m_state.lineJoin);
        pen.setWidthF(m_state.lineWidth);
        pen.setMiterLimit(m_state.miterLimit);
        m_painter.setPen(pen);
    } else {
        if ((m_state.flags & DirtyClippingRegion) && !m_state.clipPath.isEmpty())
            m_painter.setClipPath(m_state.clipPath);
        if (m_state.flags & DirtyFillStyle)
            m_painter.setBrush(m_state.fillStyle);
        if (m_state.flags & DirtyGlobalAlpha)
            m_painter.setOpacity(m_state.globalAlpha);
        if (m_state.flags & DirtyGlobalCompositeOperation)
            m_painter.setCompositionMode(m_state.globalCompositeOperation);
        if (m_state.flags & MDirtyPen) {
            QPen pen = m_painter.pen();
            if (m_state.flags & DirtyStrokeStyle)
                pen.setBrush(m_state.strokeStyle);
            if (m_state.flags & DirtyLineWidth)
                pen.setWidthF(m_state.lineWidth);
            if (m_state.flags & DirtyLineCap)
                pen.setCapStyle(m_state.lineCap);
            if (m_state.flags & DirtyLineJoin)
                pen.setJoinStyle(m_state.lineJoin);
            if (m_state.flags & DirtyMiterLimit)
                pen.setMiterLimit(m_state.miterLimit);
            m_painter.setPen(pen);
        }
        m_state.flags = 0;
    }
}

///Clears the canvas widget.
void Context2D::clear()
{
    endPainting();
    m_image.fill(qRgba(0,0,0,0));
    scheduleChange();
}

 ///Resets the canvas widget.
void Context2D::reset()
{
    m_stateStack.clear();
    m_state.matrix = QMatrix();
    m_state.clipPath = QPainterPath();
    m_state.globalAlpha = 1.0;
    m_state.globalCompositeOperation = QPainter::CompositionMode_SourceOver;
    m_state.strokeStyle = Qt::black;
    m_state.fillStyle = Qt::black;
    m_state.lineWidth = 1;
    m_state.lineCap = Qt::FlatCap;
    m_state.lineJoin = Qt::MiterJoin;
    m_state.miterLimit = 10;
    m_state.shadowOffsetX = 0;
    m_state.shadowOffsetY = 0;
    m_state.shadowBlur = 0;
    m_state.shadowColor = qRgba(0, 0, 0, 0);
    m_state.flags = AllIsFullOfDirt;
    m_state.font = QFont();
    m_state.textAlign = Qt::AlignLeft;
    clear();
}

///Sets the size of the canvas widget.
///This slot must not be used by script.
void Context2D::setSize(int width, int height)
{
    endPainting();
    QImage newi(width, height, QImage::Format_ARGB32_Premultiplied);
    newi.fill(qRgba(0,0,0,0));
    QPainter p(&newi);
    p.drawImage(0, 0, m_image);
    p.end();
    m_image = newi;
    scheduleChange();

    emit sizeChangeSignal(width, height);
}

/**
 * Save the canvas widget to an image file.
 * If imageType is empty then the image format will be detected by inspecting the extension of fileName.
 * @param fileName
 *      The file name.
 * @param imageType
 *      The image type. Following types are supported.
 *      - BMP (Windows Bitmap)
 *      - JPG (Joint Photographic Experts Group)
 *      - PNG (Portable Network Graphics)
 *      - PBM (Portable Bitmap)
 *      - PGM (Portable Graymap)
 *      - PPM (Portable Pixmap)
 * @return
 *      True on success.
 */
bool Context2D::saveToFile(QString fileName, QString imageType)
{
    QImageWriter writer(fileName, imageType.toLocal8Bit());
    return writer.write(m_image);
}

/**
 * Opens a print dialog and prints the canvas widget.
 * @param printDialogTitle
 *      The print dialog title.
 */
void Context2D::print(QString printDialogTitle)
{
    emit printSignal(printDialogTitle);
}

///Returns the width of the canvas widget.
int Context2D::width() const
{
    return m_image.width();
}

///Returns the height of the canvas widget.
int Context2D::height() const
{
    return m_image.height();
}

///Starts the change timer if the canvas widget has been changed.
void Context2D::scheduleChange()
{
    if (m_changeTimerId == -1)
        m_changeTimerId = startTimer(0);
}

///The change timer has been elapsed.
void Context2D::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == m_changeTimerId) {
        killTimer(m_changeTimerId);
        m_changeTimerId = -1;
        emit changed(endPainting());
    } else {
        QObject::timerEvent(e);
    }
}
