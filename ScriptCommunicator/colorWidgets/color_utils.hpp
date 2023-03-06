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
#include <QColor>
#include <qmath.h>

namespace color_widgets {
namespace detail {


inline float color_chromaF(const QColor& c)
{
    float max = qMax(c.redF(), qMax(c.greenF(), c.blueF()));
    float min = qMin(c.redF(), qMin(c.greenF(), c.blueF()));
    return max - min;
}

inline float color_lumaF(const QColor& c)
{
    return 0.30 * c.redF() + 0.59 * c.greenF() + 0.11 * c.blueF();
}
QColor color_from_lch(float hue, float chroma, float luma, float alpha = 1 );

inline QColor rainbow_lch(float hue)
{
    return color_from_lch(hue,1,1);
}

inline QColor rainbow_hsv(float hue)
{
    return QColor::fromHsvF(hue,1,1);
}

inline float color_lightnessF(const QColor& c)
{
    return ( qMax(c.redF(),qMax(c.greenF(),c.blueF())) +
             qMin(c.redF(),qMin(c.greenF(),c.blueF())) ) / 2;
}

inline float color_HSL_saturationF(const QColor& col)
{
    float c = color_chromaF(col);
    float l = color_lightnessF(col);
    if ( qFuzzyCompare(l+1,1) || qFuzzyCompare(l+1,2) )
        return 0;
    return c / (1-qAbs(2*l-1));
}

QColor color_from_hsl(float hue, float sat, float lig, float alpha = 1 );

} // namespace detail
} // namespace color_widgets
