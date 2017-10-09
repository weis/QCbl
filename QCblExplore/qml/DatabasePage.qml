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
import QtQuick.Dialogs 1.2
import QExplore.Components 1.0


Item {
    id: page
    property QExplore qcblexplore
    property bool docsCountVisible: false

    anchors.fill: parent

    Component.onCompleted:  {
        qcblexplore.onImportFinished.connect(importDone);
        qcblexplore.onWebLoadSuccessChanged.connect(webLoadStatus);
        qcblexplore.onDbOpenSuccessChanged.connect(openDbDone);
        qcblexplore.onDbLoadedChanged.connect(dbLoaded);
    }

    function openDbDone ()
    {
        suOpen.state = explore.dbOpenSuccess  ? suOpen.ok : suOpen.neutral;
    }

    function dbLoaded ()
    {

        btnDBOpen.busy = false;
        suOpen.state = explore.dbOpenSuccess  ? suOpen.ok : suOpen.neutral;
    }

    function importDone ()
    {
        rowImp.visible = false;
        suImportLocal.setState(suImportLocal.ok);
        btnRunImport.busy = false;
    }

    function webLoadStatus ()
    {
        if(isMobile())
        {
            suLoadWebM.setState(explore.webLoadSuccess ? suLoadWebM.ok : suLoadWebM.fail)
            btnRunLoadWebM.busy = false;
        }
        else
        {
            suLoadWeb.setState(explore.webLoadSuccess ? suLoadWeb.ok : suLoadWeb.fail)
            btnRunLoadWeb.busy = false;
        }

        rowImpW.visible = false;
    }


    FileDialog {
        id: fileDialog
        modality: Qt.NonModal
        title: "Select json file"
        selectExisting: true
        selectMultiple: false
        selectFolder: false
        nameFilters: [ "Json files (*.json)", "Zip (*.zip)", "All files (*)" ]
        selectedNameFilter: "All files (*.json)"
        sidebarVisible: true
        onAccepted: {
            tLocal.text =  explore.importPathToNative(fileUrls[0]) ;
        }
    }


    ColumnLayout {
        id: mainClmn
        spacing: 10
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.right: parent.right

        anchors.margins: spacing
        // Layout.fillWidth: true
        Rectangle {
            id: rcTop
            Layout.fillWidth: true
            color: "gray"
            height: 5
        }

        GroupBox {
            title: "Select"
            id: grDatabase
            anchors.top: rcTop.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    ComboBox {
                        id: cbDb
                        anchors.left: parent.left
                        anchors.right: btnDBRemove.left
                        anchors.rightMargin: mainClmn.spacing

                        Component.onCompleted: currentIndex = find(qcblexplore.dbname);
                        onCurrentIndexChanged: {
                            if(currentText !== "")
                            {
                                if(qcblexplore.closeC4Database())
                                    suOpen.setState(suOpen.neutral);
                                else
                                    suOpen.setState(suOpen.fail)  ;
                                explore.dbname = currentText;
                            }

                        }
                        Layout.preferredWidth: mm(28);
                        editable: false;

                        implicitHeight: mm (5);
                        textRole: "display";
                        model: qcblexplore.databases
                    }

                    IconButton {
                        id: btnDBRemove
                        anchors.right: btnDBAdd.left
                        size: mm(5)
                        icon: icons.ios_minus_outline
                        onClicked: {
                            qcblexplore.removeC4Database();
                            cbDb.currentIndex = 0;
                        }
                    }

                    IconButton {
                        id: btnDBAdd
                        anchors.right: btnDBOpen.left
                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            rowAddDb.visible = true;
                        }
                    }

                    IconButton {
                        id: btnDBOpen
                        anchors.right: suOpen.left
                        anchors.rightMargin: mainClmn.spacing
                        icon: icons.play
                        size: mm(5)

                        onClicked: {
                            busy = true;
                            explore.dbname = cbDb.currentText;
                            explore.selectDatabase();
                            explore.requestDocumentsCount();
                        }
                    }

                    IconSuccess {
                        id: suOpen
                        anchors.right: parent.right
                        size: mm(5)
                    }

                }

                RowLayout {
                    Text {
                        property string fileSize: qcblexplore.dbFileSize
                        property int dbDocsCount: qcblexplore.docsCount

                        id: txFileInfo
                        text: (fileSize.length > 0 ? "Size: " + fileSize : "") + (dbDocsCount >= 0 ? ", Documents: " + dbDocsCount : "")
                    }

                }

                RowLayout {
                    id: rowAddDb
                    visible: false

                    anchors.left: parent.left
                    anchors.right: parent.right

                    TextField {
                        id: dbName
                        anchors.left: parent.left
                        anchors.right: btnDBCreate.left
                        anchors.rightMargin: mainClmn.spacing

                        implicitHeight: mm (5)
                        z: 1
                    }

                    IconButton {
                        id: btnDBCreate
                        anchors.right: idFillDb.left

                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            if( dbName.text !== "")
                            {
                                suCreate.state = explore.createDatabase(dbName.text) ? suCreate.neutral : suCreate.fail;
                                if(suCreate.state  === suCreate.fail)
                                    return;
                                cbDb.currentIndex = cbDb.find(dbName.text);
                                dbName.text = "";
                            }
                            rowAddDb.visible = false;
                        }
                    }

                    Item {
                        id: idFillDb
                        width: mm(5)
                        anchors.right: suCreate.left
                        anchors.rightMargin: mainClmn.spacing
                    }

                    IconSuccess {
                        id: suCreate
                        anchors.right: parent.right
                        size: mm(5)
                    }

                }
            }
        }

        GroupBox {
            title: "Import"
            id: grImport

            anchors.top: grDatabase.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    ComboBox {
                        id: cbImport
                        anchors.left: parent.left
                        anchors.right: btnStopImport.left
                        anchors.rightMargin: mainClmn.spacing

                        Component.onCompleted: currentIndex = 0;
                        Layout.preferredWidth: mm(28);
                        editable: false;

                        implicitHeight: mm (5);
                        textRole: "display";
                        model: qcblexplore.importfiles
                    }

                    IconButton {
                        id: btnStopImport
                        anchors.right: btnRunImport.left
                        size: mm(5)
                        icon: icons.stop
                        visible: qcblexplore.importRunning
                        onClicked:qcblexplore.importAbort()
                    }

                    IconButton {
                        id: btnRunImport;
                        anchors.right: suImportLocal.left
                        anchors.rightMargin: mainClmn.spacing
                        icon: icons.play
                        size: mm(5)
                        onClicked: {rowImp.visible = true; busy = true; suImportLocal.setState(suImportLocal.neutral); explore.importJSON(cbImport.currentText) }
                    }

                    IconSuccess {
                        id: suImportLocal
                        anchors.right: parent.right
                        size: mm(5)
                    }

                }
                RowLayout {
                    id: rowImp
                    anchors.left: parent.left
                    anchors.right: parent.right
                    visible: false
                    enabled: false
                    ProgressBar { id: pbar ; value: qcblexplore.impProgress; Layout.fillWidth: true }
                }

            }

        }

        GroupBox {
            title: "Load"
            id: grLoad

            anchors.top: grImport.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    visible: isMobile()

                    anchors.left: parent.left
                    anchors.right: parent.right

                    TextField {
                        id: tUrlM
                        implicitHeight: mm (5)
                        anchors.left: parent.left
                        anchors.right: btnRunLoadWebM.left
                        anchors.rightMargin: mainClmn.spacing

                        cursorPosition: 0
                        text: "https://apps.dynasphere.de/57061/medien1/cbldata/names.zip"
                    }

                    IconButton {
                        id: btnRunLoadWebM
                        anchors.right: suLoadWebM.left
                        anchors.rightMargin: mainClmn.spacing
                        busy: false
                        icon: icons.play
                        size: mm(5)
                        onClicked: {
                            if(qcblexplore.importWebData(tUrlM.text)) {
                                busy = true;
                                suLoadWebM.color = "transparent";
                                rowImpW.visible = true;
                            }

                        }
                    }

                    IconSuccess {
                        id: suLoadWebM
                        anchors.right: parent.right

                        enabled: tUrlM.text.length > 0

                        size: mm(5)
                    }

                }

                RowLayout {
                    visible: !isMobile()

                    anchors.left: parent.left
                    anchors.right: parent.right


                    Text {
                        id: txt1
                        anchors.left: parent.left
                        Layout.preferredWidth: mm(10)
                        Layout.rightMargin: 5
                        text: "Web: "
                    }

                    TextField {
                        id: tUrl
                        implicitHeight: mm (5)
                        anchors.left: txt1.right
                        anchors.right: btnRunLoadWeb.left
                        anchors.rightMargin: mainClmn.spacing

                        cursorPosition: 0
                        // text: "https://raw.githubusercontent.com/arangodb/example-datasets/master/RandomUsers/names_100000.json"
                        text: "https://apps.dynasphere.de/57061/medien1/cbldata/names.zip"
                    }

                    IconButton {
                        id: btnRunLoadWeb
                        anchors.right: suLoadWeb.left
                        anchors.rightMargin: mainClmn.spacing
                        busy: false
                        icon: icons.play
                        size: mm(5)
                        onClicked: { busy = true;  suLoadWeb.color = "transparent"; rowImpW.visible = true;  qcblexplore.importWebData(tUrl.text) }
                    }

                    IconSuccess {
                        id: suLoadWeb
                        anchors.right: parent.right

                        enabled: tUrl.text.length > 0

                        size: mm(5)
                    }

                }

                RowLayout {
                    id: rowImpW
                    visible: false
                    enabled: false
                    ProgressBar { id: pbarW ; value: qcblexplore.loadProgress; Layout.fillWidth: true }
                }

                RowLayout {
                    visible: !isMobile()

                    Text {
                        id: txt2
                        Layout.preferredWidth: mm(10)
                        textFormat: Text.RichText
                        Layout.rightMargin: 5
                        color: "black"
                        text: "<style>a:link {color:\"black\"}</style><a href=\"local\">Local:</a>"
                        onLinkActivated: fileDialog.open()
                    }

                    TextField {
                        id: tLocal
                        implicitHeight: mm (5)

                        anchors.left: txt2.right
                        anchors.right: btnRunLoadLocal.left
                        anchors.rightMargin: mainClmn.spacing

                        cursorPosition: 0
                        Layout.fillWidth: true
                    }


                    IconButton {
                        id: btnRunLoadLocal
                        anchors.right: suLoadLocal.left
                        anchors.rightMargin: mainClmn.spacing

                        enabled: tLocal.text.length > 0
                        busy: false
                        icon: icons.play
                        size: mm(5)
                        onClicked: {
                            suLoadLocal.setState(qcblexplore.importLocalData(tLocal.text) ? suLoadLocal.ok : suLoadLocal.fail)
                        }
                    }

                    IconSuccess {
                        id: suLoadLocal
                        anchors.right: parent.right

                        size: mm(5)
                    }

                }

            }

        }

    }
}


