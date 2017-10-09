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
import QtQuick.Controls 1.2
import QExplore.Components 1.0

Item {
    id: page
    property QExplore qcblexplore

    anchors.fill: parent

    ColumnLayout {
        id: column
        anchors.fill: parent
        anchors.margins: column.spacing

        GroupBox {
            id: repUrl
            anchors.top: parent.top
            title: "Remote Host"
            Layout.fillWidth: true
            RowLayout {
                anchors { fill: parent}
                TextField { id:tfRepUrl;  text: qcblexplore.repUrl; onTextChanged: qcblexplore.repUrl=text;   Layout.fillWidth: true;  z: 1 }
            }
        }

        GroupBox {
            anchors.top: repUrl.bottom
            title: "Action"
            Layout.fillWidth: true
            ColumnLayout
            {
                anchors { fill: parent}
                RowLayout {
                    id: row1

                    Button { text: "Start"; isDefault: true;  enabled: !qcblexplore.repBusy
                        onClicked:
                        {
                           qcblexplore.startReplication(cbContinous.checked)
                        }
                    }
                    Button { text: "Stop"; isDefault: true; enabled: qcblexplore.repStatus=="Idle" || qcblexplore.repStatus=="Active"
                        onClicked:
                        {
                            qcblexplore.stopReplication();
                        }
                    }
                    Item { Layout.fillWidth: true; }
                    Text { text: "Continous" }
                    CheckBox { id: cbContinous; checked: true }
                    Rectangle { width:15; height:15 ; radius: width / 2;  color: qcblexplore.statusFlag }
                }
                RowLayout {
                    id: row2
                    ProgressBar { minimumValue: 0; maximumValue: 100; value: qcblexplore.repProgress; Layout.fillWidth: true }
                }
            }
        }

    }

}
