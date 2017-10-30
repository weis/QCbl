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
import QtQuick.Controls.Styles 1.2
import QExplore.Components 1.0
import QtQuick.Window 2.1
import QtQuick.Dialogs 1.1


Item {
    id: page
    property QExplore qcblexplore

    signal saveDocuments()

    Component.onCompleted:  {
        qcblexplore.onDocsCountChanged.connect(countDocs);
        qcblexplore.onDocContentChanged.connect(documentChanged);
    }

    function countDocs ()
    {
        txtCount.displayCount(qcblexplore.docsCount);
    }

    function documentChanged (docId)
    {
       console.log("documentChanged: ", docId);
        if(jEditDoc.docId === docId)
        {
            jEditDoc.updateJEditorDoc();
            grEdit.title = jEditDoc.title;
        }
    }

    function onTabActivated(tab)
    {
        if(tab !== parent)
            return;

        countDocs();
    }

    anchors.fill: parent


    ColumnLayout {
        id: column
        anchors.fill: parent
        anchors.margins: column.spacing

        GroupBox {
            title: "Action"
            id: grAction
            Layout.fillWidth: true

            property string startDocId: explore.getStartDocId()
            property string nextDocId: startDocId

            ColumnLayout {

                anchors.fill: parent
                RowLayout {
                    Button {
                        id: btnRead
                        isDefault: true
                        text: "Read"
                        onClicked: {
                            grAction.startDocId = inpFrom.text
                            grAction.nextDocId = qcblexplore.getAllDocumentsQuery(grAction.startDocId, inpFetch.text);
                            if(grAction.startDocId !== "")
                            {
                                btnNew.enabled = true;
                                btnNext.enabled = true;
                            }
                        }
                    }

                    Button {
                        id: btnNext
                        enabled: false
                        text: "Next"
                        onClicked: {
                            grAction.startDocId = grAction.nextDocId;
                            grAction.nextDocId = qcblexplore.getAllDocumentsQuery(grAction.nextDocId, inpFetch.text);
                            if(grAction.startDocId !== "")
                            {
                                btnNew.enabled = true;
                                btnNext.enabled = true;
                            }
                        }
                    }

                    Button {
                        id: btnNew
                        enabled: false
                        isDefault: true
                        text: "New"
                        onClicked: {
                            qcblexplore.newDocument();
                            txtCount.displayCount();
                        }
                    }


                    BusyIndicator {
                        implicitWidth: 25;
                        implicitHeight: 25;
                        running: qcblexplore.repBusy;
                        Rectangle {
                            width: 10;height: 10;anchors.centerIn: parent;radius: width / 2;color: qcblexplore.statusFlag
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    Text {
                        id: txtCount
                        text: "Count: 0"

                        function displayCount() {
                            text =  "Count: " + qcblexplore.docsCount;
                        }
                    }

                }

                RowLayout {

                    Text {
                        text: "From:"
                     }

                    TextField {
                        id: inpFrom
                        text: grAction.startDocId
                        Layout.preferredWidth: mm(20)
                    }
                    Text {
                        text: "Fetch:"
                    }
                    TextField {
                        id: inpFetch
                        Layout.preferredWidth: mm(12)
                        inputMask: "900"
                        text: "20"
                    }

                }
            }
        }

        GroupBox {
            title: "Edit"
            id: grEdit
            Layout.fillWidth: true
            visible: false

            ColumnLayout {

                anchors.fill: parent
                RowLayout {
                    Button {
                        id: btnSaveDoc
                        isDefault: true
                        text: "Save"
                        enabled: jEditDoc.jsonTextChanged && jEditDoc.isCurrRev
                        onClicked: {
                            if (jEditDoc.isCurrRev) {
                                if (qcblexplore.updateDocument(jEditDoc.docItem, jEditDoc.getJson())) {
                                    jEditDoc.updateJEditorDoc();
                                    grEdit.title = jEditDoc.title;
                                    iconSuccess.icon = iconSuccess.icons.checkmark_circled;
                                    iconSuccess.color = "green"
                                } else {
                                    iconSuccess.icon = iconSuccess.icons.alert_circled;
                                    iconSuccess.color = "red"
                                }
                            }
                        }
                    }
                    Button {
                        id: btnExitEdit
                        enabled: true
                        isDefault: true
                        text: "Exit"
                        onClicked: {
                            jEditDoc.visible = false;
                            jEditDoc.clearJson();
                            result.visible = true;
                            grEdit.visible = false;
                            grAction.visible = true;
                        }
                    }
                    Button {
                        id: btnNextRevEdit;
                        implicitWidth: mm (5);
                        implicitHeight: mm (5);
                        style: ButtonStyle {
                            label: IconLbl {
                                icon: icons.chevron_left
                                size: mm (3.5);
                                color: "black";
                            }
                        }
                        onClicked: {
                            if (qcblexplore.getNextRevision(jEditDoc.docItem)) {
                                jEditDoc.updateJEditorDoc()
                                grEdit.title = jEditDoc.title;
                                iconSuccess.icon = ""
                            } else {
                                iconSuccess.icon = iconSuccess.icons.alert_circled;
                                iconSuccess.color = "gold"
                            }
                        }
                    }
                    Button {
                        id: btnCurrRevEdit;
                        implicitWidth: mm (5);
                        implicitHeight: mm (5);
                        style: ButtonStyle {
                            label: IconLbl {
                                icon: icons.chevron_up
                                size: mm (3.5);
                                color: "black";
                            }
                        }
                        onClicked: {
                            qcblexplore.getCurrRevision(jEditDoc.docItem)
                            jEditDoc.updateJEditorDoc()
                            grEdit.title = jEditDoc.title;
                            iconSuccess.icon = ""
                        }
                    }
                    BusyIndicator {
                        implicitWidth: 25;
                        implicitHeight: 25;
                        running: qcblexplore.repBusy;
                        Rectangle {
                            width: 10;height: 10;anchors.centerIn: parent;radius: width / 2;color: qcblexplore.statusFlag
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                    IconLbl {
                        id: iconSuccess
                        size: mm (5);
                        color: "transparent"
                    }
                }
            }
        }

        Item {
            id: result
            visible: true
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollView {
                anchors.fill: parent
                Component {
                    id: cntDelegate

                    Rectangle {
                        id: content
                        Component.onCompleted: {
                            model.explore = qcblexplore
                            saveDocuments.connect(saveDocItem);
                        }
                        Component.onDestruction: {
                            saveDocuments.disconnect(saveDocItem)
                        }

                        function saveDocItem() {
                            if (!cdel.checked)
                                model.text = tfname.text;
                        }

                        anchors {
                            left: parent.left;right: parent.right
                        }
                        height: column.implicitHeight + 4

                        border.width: 1
                        border.color: "lightsteelblue"

                        radius: 2

                        ColumnLayout {
                            id: column
                            anchors {
                                fill: parent;margins: 2
                            }
                            RowLayout {
                                Text {
                                    text: 'DocId:';color: "darkgray";Layout.preferredWidth: mm(9);horizontalAlignment: Text.AlignRight
                                }
                                Text {
                                    text: model.docId
                                }
                                Item {
                                    width: mm(2)
                                }
                                Text {
                                    text: 'Seq:';color: "darkgray";Layout.preferredWidth: mm(9);horizontalAlignment: Text.AlignRight
                                }
                                Text {
                                    text: model.sequenceNumber
                                }

                            }
                            RowLayout {
                                Text {
                                    text: 'RevId:';color: "darkgray";Layout.preferredWidth:  mm(9);horizontalAlignment: Text.AlignRight
                                }
                                Text {
                                    text: model.revision
                                    color: model.isCurrRevision ? "black" : "darkred"
                                }
                            }
                            RowLayout {

                                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                                RowLayout {
                                    Item {
                                        width: mm(9)
                                    }
                                    Button {
                                        property bool selected: false

                                        id: btnTrash;

                                        onClicked: selected = !selected
                                        implicitWidth: mm (5);
                                        implicitHeight: mm (5);
                                        style: ButtonStyle {
                                            label: IconLbl {
                                                id: lblTrash
                                                icon: icons.close
                                                size: mm (3.5);
                                                color: btnTrash.selected ? "red" : "black"
                                            }
                                        }

                                    }
                                    Button {
                                        id: btnEdit;

                                        implicitWidth: mm (5);
                                        implicitHeight: mm (5);

                                        style: ButtonStyle {
                                            label: IconLbl {
                                                icon: icons.document
                                                size: mm (3.5);
                                                color: "black"
                                            }
                                        }

                                        onClicked: {
                                            jEditDoc.docItem = model.item;
                                            jEditDoc.updateJEditorDoc();
                                            jEditDoc.visible = true;
                                            result.visible = false;
                                            grAction.visible = false;
                                            grEdit.title = jEditDoc.title;
                                            grEdit.visible = true;
                                        }
                                    }
                                    Button {
                                        id: btnNextRevDocs;
                                        implicitWidth: mm (5);
                                        implicitHeight: mm (5);
                                        style: ButtonStyle {
                                            label: IconLbl {
                                                icon: icons.chevron_left
                                                size: mm (3.5);
                                                color: "black";
                                            }
                                        }
                                        onClicked: {
                                            if (qcblexplore.getNextRevision(model.item)) {
                                                iconSuccessRow.icon = ""
                                            } else {
                                                iconSuccessRow.icon = iconSuccessRow.icons.alert_circled;
                                                iconSuccessRow.color = "gold"
                                            }
                                        }
                                    }
                                    Button {
                                        id: btnCurrRevDocs;
                                        implicitWidth: mm (5);
                                        implicitHeight: mm (5);
                                        style: ButtonStyle {
                                            label: IconLbl {
                                                icon: icons.chevron_up
                                                size: mm (3.5);
                                                color: "black";
                                            }
                                        }
                                        onClicked: {
                                            qcblexplore.getCurrRevision(model.item )
                                            iconSuccessRow.icon = "";
                                        }
                                    }
                                    Text {
                                        anchors.rightMargin: 10
                                        visible: btnTrash.selected
                                        text: "Remove:"
                                    }
                                    Button {
                                        id: btnRemoveDoc;
                                        visible: btnTrash.selected
                                        implicitHeight: mm(5)
                                        implicitWidth: mm(8)
                                        text: "Doc"
                                        onClicked: {
                                            if (!qcblexplore.removeDocument(model.item)) {
                                                iconSuccessRow.icon = iconSuccessRow.icons.alert_circled;
                                                iconSuccessRow.color = "red"
                                            }
                                        }
                                    }
                                    Button {
                                        id: btnRemoveRev;
                                        implicitHeight: mm(5)
                                        implicitWidth: mm(8)
                                        visible: btnTrash.selected
                                        text: "Rev"
                                        onClicked: {
                                            if (!qcblexplore.removeRevision(model.item)) {
                                                iconSuccessRow.icon = iconSuccessRow.icons.alert_circled;
                                                iconSuccessRow.color = "red"
                                            }
                                        }
                                    }
                                    Item {
                                        Layout.fillWidth: true
                                    }
                                    IconLbl {
                                        id: iconSuccessRow
                                        size: mm (5);
                                        color: "transparent"
                                    }

                                }
                            }
                        }

                    }
                }

                ListView {
                    id: view

                    anchors {
                        fill: parent;margins: 2
                    }

                    model: qcblexplore.docItems
                    delegate: cntDelegate

                    spacing: 4
                    cacheBuffer: 50
                }
            }
        }

        JsonEdit {
            id: jEditDoc
            readOnly: !isCurrRev

            function updateJEditorDoc()
            {
                docId = docItem.docId;
                revId = docItem.revision;
                seqNumber = docItem.sequenceNumber;
                isCurrRev = docItem.revision === docItem.currRevision;
                setJson(qcblexplore.readDocument(docId, revId));
                cursorPosition = 0;

                var docDisplay = docId.length > 12 ? docId.substring(0,10) + "..." : docId;
                var revDisplay = revId.length > 12 ? revId.substring(0,10) + "..." : revId;
                title = "DocID: " + docDisplay + ", RevID: " + revDisplay + ", Seq: " + seqNumber ;
            }


            visible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

}
