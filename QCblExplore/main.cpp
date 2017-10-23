	/****************************************************************************
**
** The MIT License (MIT)
**
** Copyright (c) 2017 DEM GmbH, weis@dem-gmbh.de
**
** $BEGIN_LICENSE:MIT$
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**
** $END_LICENSE$
**
****************************************************************************/



#include "logging/loginternal.h"
#include "ui/qmlgui.h"

#include <QGuiApplication>

int main(int argc, char* argv[])
{
    QGuiApplication qapp(argc, argv);
    QGuiApplication::setOrganizationName("DEM GmbH");
    QGuiApplication::setOrganizationDomain("www.dynasphere.de");
    QGuiApplication::setApplicationName("QCblExplore");
    QGuiApplication::setApplicationVersion("0.1");

    // LogInternal logging;

    QmlGui* qmlGui = new QmlGui();

    int rc = -1;
    if (qmlGui->createMainWnd())
        rc = qapp.exec();


    return rc;
}
