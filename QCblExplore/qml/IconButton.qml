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

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2


Button {
    property alias icon: lblIcon.icon
    property alias icons: lblIcon.icons
    property int size: 10
    property color color: "black"
    property bool busy: false

    id: root;

    implicitWidth: size
    implicitHeight: size

    onBusyChanged: if (!busy) lblIcon.color = color

    IconLbl {

        Component.onCompleted: color = root.color

        id: lblIcon
        icon: icons.close
        size: root.size * 0.75

        anchors.centerIn: root
        SequentialAnimation {
            id: anim
            loops: Animation.Infinite;
            running: root.busy
            PropertyAnimation {
                target: lblIcon
                property: "color"
                from: root.color
                to: "transparent"
                duration: 400

            }
            PropertyAnimation {
                target: lblIcon
                property: "color"
                from: "transparent"
                to: root.color
                duration: 400
            }
        }
    }
}
