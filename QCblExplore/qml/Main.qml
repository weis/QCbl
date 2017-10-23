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
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.2
import QExplore.Components 1.0
import QtQuick.Window 2.1

ApplicationWindow {
    title: qsTr("QCBL Experimental")
    flags: Qt.Window
    id: app

    property real availWidth:  Screen.desktopAvailableWidth
    property real availHeight: Screen.desktopAvailableHeight

    function isMobile ()
    {
        return Qt.platform.os === "ios" || Qt.platform.os === "android"
    }

    function mm (size)
    {
        return size * Screen.pixelDensity
    }


    minimumWidth:  isMobile() ? availWidth : mm (100)
    minimumHeight: isMobile() ? availHeight : mm (90)


    width:  isMobile() ? availWidth : mm (120)
    height: isMobile() ? availHeight : mm (150)

    signal tabActivated(var page)

    onClosing: explore.shutDown();

    QExplore
    {
        id: explore
        onRepStatusChanged: console.log("RepStatus =", explore.repStatus )
    }

    ColumnLayout
    {
        anchors.fill: parent
        Rectangle {
            id: rcTitle
            visible: isMobile()
            anchors.top : parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: mm(5)
            color: "white"
            Text {
                textFormat: Text.RichText
                Layout.rightMargin: 5
                color: "black"
                text: "<b>QCBL Experimtental</b>"

                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 5

            }

        }


        SplitView
        {
            id: spv
            anchors.top : rcTitle.visible ?  rcTitle.bottom : parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.topMargin: 5

            orientation: Qt.Vertical


            TabView {
                onCurrentIndexChanged: tabActivated(getTab(currentIndex));
                Layout.fillWidth: true
                height: app.height - mm(18)

                id: tabView
                tabPosition: Qt.TopEdge
                Tab {
                    id: tabDb
                    title: "Database"
                    DatabasePage {
                        id: dbPage
                        qcblexplore: explore
                    }
                }
                Tab {
                    id: docTab
                    title: "Docs"
                    enabled: explore.dbLoaded

                    DocumentsPage {
                        id: docPage
                        qcblexplore: explore;
                        Component.onCompleted: app.tabActivated.connect(onTabActivated);
                    }
                }
                Tab {
                    id: qTab
                    title: "Query"
                    enabled: explore.dbLoaded

                    QueryPage {
                        qcblexplore: explore;
                        Component.onCompleted: app.tabActivated.connect(onTabActivated);
                    }

                }
                Tab {
                    id: replTab
                    title: "Rep"
                    enabled: explore.dbLoaded
                    ReplicationPage {
                        qcblexplore: explore;
                    }
                }
            }

            Item
            {
                Layout.fillWidth: true
                Layout.minimumHeight: mm(15)

                TextArea
                {
                    id: tmessage
                    anchors.fill: parent
                    anchors.margins: 0
                    text: explore.message
                    wrapMode: Text.NoWrap
                }
            }

        }
    }

}
