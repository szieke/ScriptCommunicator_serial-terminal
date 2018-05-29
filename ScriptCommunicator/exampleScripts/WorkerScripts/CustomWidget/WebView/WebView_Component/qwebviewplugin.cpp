/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwebviewplugin.h"
#include "scriptwebwidget.h"

#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>

#include <QtCore/qplugin.h>
#include <QWebView>

static const char *toolTipC = "A widget for displaying a web page, from the Qt WebKit Widgets module.";

QMap<QWebView*, ScriptWebWidgetSlots*> m_pointerMap;

QT_BEGIN_NAMESPACE

CustomWebViewPlugin::CustomWebViewPlugin(QObject *parent) :
    QObject(parent),
    m_initialized(false)
{
}

QString CustomWebViewPlugin::name() const
{
    return QStringLiteral("QWebView");
}

QString CustomWebViewPlugin::group() const
{
    return QStringLiteral("Display Widgets");
}

QString CustomWebViewPlugin::toolTip() const
{
    return tr(toolTipC);
}

QString CustomWebViewPlugin::whatsThis() const
{
    return tr(toolTipC);
}

QString CustomWebViewPlugin::includeFile() const
{
    return QStringLiteral("QtWebKitWidgets/QWebView");
}

QIcon CustomWebViewPlugin::icon() const
{
    return QIcon(QStringLiteral(":/qt-project.org/qwebview/images/qwebview.png"));
}

bool CustomWebViewPlugin::isContainer() const
{
    return false;
}

QWidget *CustomWebViewPlugin::createWidget(QWidget *parent)
{
    QWebView* el = new QWebView(parent);
    ScriptWebWidgetSlots* slotObject = new ScriptWebWidgetSlots(el);

    m_pointerMap[el] = slotObject;
    return el;
}

bool CustomWebViewPlugin::isInitialized() const
{
    return m_initialized;
}

void CustomWebViewPlugin::initialize(QDesignerFormEditorInterface * /*core*/)
{
    if (m_initialized)
        return;

    m_initialized = true;
}

QString CustomWebViewPlugin::domXml() const
{
    return QStringLiteral("\
    <ui language=\"c++\">\
        <widget class=\"QWebView\" name=\"webView\">\
            <property name=\"url\">\
                <url>\
                    <string>about:blank</string>\
                </url>\
            </property>\
            <property name=\"geometry\">\
                <rect>\
                    <x>0</x>\
                    <y>0</y>\
                    <width>300</width>\
                    <height>200</height>\
                </rect>\
            </property>\
        </widget>\
    </ui>");
}

QT_END_NAMESPACE
