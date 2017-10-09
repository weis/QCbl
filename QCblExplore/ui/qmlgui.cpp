/****************************************************************************
**
** Copyright (c) 2017 DEM GmbH, weis@dem-gmbh.de

** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:

** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.

** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
****************************************************************************/

#include <QThread>
#include <QtCore>
#include <QDebug>

#include "qmlgui.h"
#include "../src/qexplore.h"
#include "../src/docitem.h"
#include "../src/documenthandler.h"



QmlGui::QmlGui(QObject* parent) :
    QObject(parent),
    m_qmlengine(new QQmlEngine()),
    m_wndMain(0)
{

    qmlRegisterType<QExplore> ("QExplore.Components", 1, 0, "QExplore");
    qmlRegisterType<DocItem> ("QExplore.Components", 1, 0, "DocItem");
    qmlRegisterType<DocumentHandler>("QExplore.Components", 1, 0, "DocumentHandler");


    QQmlContext* ctxt = m_qmlengine->rootContext();
    ctxt->setContextProperty("QmlGui", this);

    QObject::connect(m_qmlengine, SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));
}

QmlGui::~QmlGui()
{
}

QQmlContext* QmlGui::getQmlContext()
{
    return m_qmlengine->rootContext();
}

bool QmlGui::createMainWnd()
{
    QQuickWindow* w = loadAppWindow("qrc:///qml/Main.qml") ;
    bool success = w != 0;

    if (success)
    {
        w->show();
    }

    return success;
}

QQuickWindow* QmlGui::loadAppWindow(const QString& qurl)
{
    QQuickWindow* window = 0;
    QQmlComponent component(m_qmlengine);

    component.loadUrl(QUrl(qurl));
    if (component.isReady())
    {
        window = qobject_cast<QQuickWindow*>(component.create());
        if(window)
        {

            QSurfaceFormat surfaceFormat = window->requestedFormat();

            int major = surfaceFormat.majorVersion();
            int minor  = surfaceFormat.minorVersion();
            QString oglInfo =  QString ("Using OpenGL version %0.%1").arg(major).arg(minor);
            qInfo("%s", qPrintable(oglInfo));

            window->setPersistentOpenGLContext(true);
            window->setPersistentSceneGraph(true);

            QIcon ico = QIcon(":/img/dyadmin.ico");
            window->setIcon(ico);
        }
        else
            qCritical("%s", qPrintable(component.errorString()));
    }
    else
        qCritical("%s", qPrintable(component.errorString()));

    return window;
}


