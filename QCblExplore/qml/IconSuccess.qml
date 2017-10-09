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

import QtQuick 2.0
import QtQuick.Window 2.1

IconLbl {
    readonly property int ok: 1
    readonly property int fail: -1
    readonly property int neutral: 0

    property int state : 0

    function setState(val)
    {
        if(val === -1) {

            icon = icons.alert_circled;
            color = "red"
        }
        else if(val === 1) {
            icon = icons.checkmark_circled;
            color = "green"
        }
        else
            color = "transparent";
    }

    icon: icons.checkmark_circled
    Component.onCompleted: color = "transparent"
    onStateChanged: setState(state)
}


