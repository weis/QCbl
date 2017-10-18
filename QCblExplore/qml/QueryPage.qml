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
import QtQuick.Dialogs 1.2

import QExplore.Components 1.0

Item {
    id: page
    property QExplore qcblexplore
    property bool shiftkey: false


//    emit indexRunningChanged_t();
//    emit indexRunningChanged_v();

    Component.onCompleted:  {
        qcblexplore.onDbLoadedChanged.connect(dbLoaded);
        qcblexplore.onIndexRunningChanged_t.connect(idxTextDone);
        qcblexplore.onIndexRunningChanged_v.connect(idxValueDone);
    }

    Keys.onPressed: {
        shiftkey = event.key === Qt.Key_Shift
    }

    function idxTextDone() {
        if(explore.error)
            suIndexSuccess_t.setState(suIndexSuccess_t.fail)
        else
            suIndexSuccess_t.setState(suIndexSuccess_t.ok)
    }

    function idxValueDone() {
        if(explore.error)
            suIndexSuccess_v.setState(suIndexSuccess_v.fail)
        else
            suIndexSuccess_v.setState(suIndexSuccess_v.ok)
    }

    function dbLoaded() {
        if(!qcblexplore.databaseOpen)
            return;

        btnShow.visible = true;
        btnHide.visible = false;
        grIdxStandard.visible = false;
        grIdxText.visible = false;

        cbIndex_v.currentIndex = 0

        cbIndex_t.currentIndex = 0

    }

    function onTabActivated(tab) {
        if (tab !== parent)
            return;

        if(qcblexplore.hasQueries())
        {
            var index = cbQuery.find(qcblexplore.queryName());
            if(index < 0)
                index = 0;
            cbQuery.currentIndex = index;
            explore.queryVars(cbQuery.currentText);
            grSearch.visible = explore.hasQueryVar1;
        }
        else
        {
            grSearch.visible = false;
            cbQuery.currentIndex = 0;
        }
    }

    function showIndices(show)
    {
        quind.visible = show;
        grIdxStandard.visible = quind.visible  ? quind.open : show;
        grIdxText.visible  = quind.visible  ? quind.open : show;
    }

    anchors.fill: parent

    ColumnLayout {
        id: mainClmn
        anchors.fill: parent
        anchors.margins: mainClmn.spacing

        GroupBox {
            title: "Edit"
            id: grEditJson
            Layout.fillWidth: true
            visible: false

            ColumnLayout {

                anchors.fill: parent
                RowLayout {
                    Button {
                        id: btnExitEdit
                        enabled: true
                        isDefault: true
                        text: "Exit"
                        onClicked: {
                            jViewDoc.visible = false;
                            jViewDoc.clearJson();
                            result.visible = true;

                            grEditJson.visible = false;
                            grEditIndex.visible = false;
                            grEditQuery.visible = false;
                            grSearch.visible = true;
                            showIndices(true);
                            grQueries.visible = true;
                        }
                    }
                    Item {
                        Layout.fillWidth: true
                    }
                }
            }
        }

        GroupBox {
            property bool textIndex: false

            title: "Edit"
            id: grEditIndex
            Layout.fillWidth: true
            visible: false

            ColumnLayout {

                anchors.fill: parent
                RowLayout {
                    Button {
                        id: btnSaveDocIdx
                        isDefault: true
                        text: "Save"
                        enabled: jEditIQ.jsonTextChanged
                        onClicked: {
                            var success = grEditIndex.textIndex ? explore.saveIndexContent_t(jEditIQ.fname, jEditIQ.text, shiftkey)
                                                                : explore.saveIndexContent_v(jEditIQ.fname, jEditIQ.text, shiftkey);
                            if (success) {
                                iconSuccessIdx.icon = iconSuccessIdx.icons.checkmark_circled;
                                iconSuccessIdx.color = "green"
                                jEditIQ.jsonTextChanged = false
                            } else {
                                iconSuccessIdx.icon = iconSuccessIdx.icons.alert_circled;
                                iconSuccessIdx.color = "red"
                            }
                        }
                    }

                    Button {
                        id: btnExitEditIdx
                        enabled: true
                        isDefault: true
                        text: "Exit"
                        onClicked: {
                            jEditIQ.visible = false;
                            grQueries.visible = true;
                            showIndices(true);
                            grSearch.visible = explore.hasQueryVar1;
                            result.visible = true;
                            grEditIndex.visible = false;
                        }
                    }

                    Button {
                        id: btnDeleteIdx
                        enabled: true
                        isDefault: true
                        text: "Delete"
                        onClicked: {
                            jEditIQ.visible = false;
                            showIndices(true);
                            grSearch.visible = explore.hasQueryVar1;
                            result.visible = true;
                            grEditIndex.visible = false;
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                    IconLbl {
                        id: iconSuccessIdx
                        size: mm(5);
                        color: "transparent"
                    }
                }
            }
        }

        GroupBox {
            title: "Edit"
            id: grEditQuery
            Layout.fillWidth: true
            visible: false
            onVisibleChanged: {
                iconSuccessQuery.color = "transparent"
            }

            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    Button {

                        id: btnSaveDocQuery
                        isDefault: true
                        text: "Save"
                        enabled: jEditIQ.jsonTextChanged
                        onClicked: {
                            if (explore.saveQueryContent(jEditIQ.fname, jEditIQ.text, shiftkey)) {
                                explore.queryVars(jEditIQ.fname)
                                iconSuccessQuery.icon = iconSuccessQuery.icons.checkmark_circled;
                                iconSuccessQuery.color = "green"
                                jEditIQ.jsonTextChanged = false
                            } else {
                                iconSuccessQuery.icon = iconSuccessQuery.icons.alert_circled;
                                iconSuccessQuery.color = "red"
                            }
                        }
                    }

                    Button {
                        id: btnExitEditQuery
                        enabled: true
                        isDefault: true
                        text: "Exit"
                        onClicked: {
                            jEditIQ.visible = false;

                            grEditJson.visible = false;
                            grEditIndex.visible = false;
                            grEditQuery.visible = false;
                            grSearch.visible = true;
                            grQueries.visible = true;
                            grSearch.visible = explore.hasQueryVar1;
                            showIndices(true);
                            result.visible = true;
                        }
                    }

                    Button {
                        id: btnDeleteQuery
                        isDefault: true
                        text: "Delete"

                        onClicked: {
                            delConfirmDlg.name = jEditIQ.fname;
                            delConfirmDlg.type = "Query";
                            delConfirmDlg.visible = true;
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    IconLbl {
                        id: iconSuccessQuery
                        icon: iconSuccessQuery.icons.checkmark_circled
                        size: mm(5);
                        color: "transparent"
                    }
                }
            }
        }

        GroupBox {
            title: "Queries"
            Layout.fillWidth: true
            id: grQueries
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    ComboBox {
                        id: cbQuery;
                        anchors.left: parent.left
                        anchors.right: btnEditQuery.left
                        anchors.rightMargin: mainClmn.spacing


                        onCurrentIndexChanged: {
                            explore.queryVars(currentText);
                            grSearch.visible = explore.hasQueryVar1;
                        }
                        implicitHeight: mm(5);
                        textRole: "display";
                        model: qcblexplore.queries
                    }

                    IconButton {
                        id: btnEditQuery
                        size: mm(5)
                        icon: icons.document
                        anchors.right: btnClearQuery.left

                        onClicked: {
                            showIndices(false);
                            grSearch.visible = false;
                            result.visible = false;
                            grEditQuery.title = "Query \"" + cbQuery.currentText + "\"";
                            grEditQuery.visible = true;
                            jEditIQ.fname = cbQuery.currentText;
                            jEditIQ.visible = true;

                            jEditIQ.setJson(explore.queryContent(cbQuery.currentText));
                        }
                    }

                    IconButton {
                        id: btnClearQuery
                        icon: icons.ios_minus_outline
                        size: mm(5)
                        anchors.right: btnQueryCreate_S.left

                        onClicked: {
                            explore.clearSearchResult()
                        }
                    }

                    IconButton {
                        id: btnQueryCreate_S
                        anchors.right: btnRunQuery.left

                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            rlAddQuery.visible = true;
                        }
                    }

                    IconButton {
                        id: btnRunQuery;
                        busy: explore.queryRunning
                        onBusyChanged: if (!busy) suQuerySuccess.state = explore.queryError ? suQuerySuccess.fail : suQuerySuccess.ok
                        icon: icons.play
                        size: mm(5)

                        anchors.right: suQuerySuccess.left
                        anchors.rightMargin: mainClmn.spacing

                        onClicked: {
                            suQuerySuccess.state = suQuerySuccess.neutral;
                            explore.runQuery(cbQuery.currentText);
                        }
                    }

                    IconSuccess {
                        id: suQuerySuccess
                        size: mm(5)
                        anchors.right: parent.right
                    }

                }

                RowLayout {
                    id: rlAddQuery
                    visible: false

                    anchors.left: parent.left
                    anchors.right: parent.right

                    TextField {
                        id: queryName
                        anchors.left: parent.left
                        anchors.right: btnQueryCreate.left
                        anchors.rightMargin: mainClmn.spacing

                        implicitHeight: mm (5)
                        z: 1
                    }

                    IconButton {
                        id: btnQueryCreate
                        anchors.right: idFillQu.left

                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            if(queryName.text !== "") {
                                explore.createQueryName(queryName.text);
                                cbQuery.currentIndex = cbQuery.find(queryName.text);
                                queryName.text = "";
                            }
                            rlAddQuery.visible = false;
                        }
                    }


                    Item {
                        id: idFillQu
                        width: mm(20)
                        anchors.right: parent.right
                    }

                }

            }
        }

        GroupBox {
            id: grSearch
            visible: explore.hasQueryVar1
            title: "Search .."
            Layout.fillWidth: true
            GridLayout {
                anchors.fill: parent
                columns: 3
                flow: GridLayout.LeftToRight
                Text { id: qArg1;Layout.rightMargin: 5;text: explore.queryVar1 + ":" }
                TextField {
                    Layout.fillWidth: true
                    text: explore.queryDefault1
                    onTextChanged: {
                        explore.queryText1 = text
                        suQuerySuccess.state =  suQuerySuccess.neutral
                    }
                    implicitHeight: mm(5)
                    z: 1
                }
                CheckBoxRect {
                    size: mm (5)
                    checked: explore.queryI1
                    onCheckedChanged: explore.queryI1 = checked
                }

                Text { id: qArg2;visible: explore.hasQueryVar2;Layout.rightMargin: 5;text: explore.queryVar2 + ":" }
                TextField {
                    Layout.fillWidth: true
                    text: explore.queryDefault2
                    onTextChanged: {
                        explore.queryText2 = text
                        suQuerySuccess.state =  suQuerySuccess.neutral
                    }
                    visible: explore.hasQueryVar2
                    implicitHeight: mm(5)
                    z: 1
                }
                CheckBoxRect {
                    visible: explore.hasQueryVar2
                    size: mm (5)
                    checked: explore.queryI2
                    onCheckedChanged: explore.queryI2 = checked
                }
                Text { id: qArg3;visible: explore.hasQueryVar3;Layout.rightMargin: 5;text: explore.queryVar3 + ":" }
                TextField {
                    Layout.fillWidth: true
                    text: explore.queryDefault3
                    onTextChanged: {
                        explore.queryText3 = text
                        suQuerySuccess.state =  suQuerySuccess.neutral
                    }
                    visible: explore.hasQueryVar3
                    implicitHeight: mm(5)
                    z: 1
                }
                CheckBoxRect {
                    visible: explore.hasQueryVar3
                    checked: explore.queryI3
                    size: mm (5)
                    onCheckedChanged: explore.queryI3 = checked

                }
            }
        }

        Item {
            id: quind
            Layout.fillWidth: true
            property bool open: false

            RowLayout
            {

                IconButton {
                    id: btnShow
                    visible: true
                    size: mm(5)
                    icon: icons.ios_plus_outline
                    onClicked: {
                        quind.open = true
                        btnShow.visible = false;
                        btnHide.visible = true;
                        grIdxStandard.visible = true;
                        grIdxText.visible = true;
                    }
                }
                IconButton {
                    id: btnHide
                    visible: false
                    size: mm(5)
                    icon: icons.ios_minus_outline
                    onClicked: {
                        quind.open = false
                        btnShow.visible = true;
                        btnHide.visible = false;
                        grIdxStandard.visible = false;
                        grIdxText.visible = false;
                    }
                }

                Text {
                    text: "Indices"
                }

            }

            height: mm(5)
        }

        GroupBox {
            title: "Value Indices"
            Layout.fillWidth: true
            id: grIdxStandard
            visible: false
            ColumnLayout {
                anchors.fill: parent

                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    ComboBox {
                        id: cbIndex_v;

                        anchors.left: parent.left
                        anchors.right: btnEditIndex_v.left
                        anchors.rightMargin: mainClmn.spacing

                        implicitHeight: mm(5);
                        textRole: "display";
                        model: qcblexplore.indices_v
                    }

                    IconButton {
                        id: btnEditIndex_v
                        size: mm(5)
                        anchors.right: btnDropIndex_v.left

                        icon: icons.document

                        onClicked: {
                            showIndices(false);
                            grSearch.visible = false;
                            grQueries.visible = false;
                            result.visible = false;
                            jEditIQ.fname = cbIndex_v.currentText;
                            grEditIndex.textIndex = false;
                            grEditIndex.title = "Index \"" + cbIndex_v.currentText + "\"";
                            grEditIndex.visible = true;

                            jEditIQ.visible = true;
                            jEditIQ.setJson(explore.indexContent_v(cbIndex_v.currentText));
                        }
                    }

                    IconButton {
                        id: btnDropIndex_v;
                        icon: icons.ios_minus_outline
                        size: mm(5)
                        anchors.right: btnIndexCreate_sv.left
                        onClicked: {
                            suIndexSuccess_v.state = explore.deleteIndex(cbIndex_v.currentText, false) ? suIndexSuccess_v.ok : suIndexSuccess_v.fail;
                        }
                    }

                    IconButton {
                        id: btnIndexCreate_sv
                        anchors.right: btnCreateIndex_v.left

                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            rlAddIndex_v.visible = true;
                        }
                    }


                    IconButton {
                        id: btnCreateIndex_v;
                        icon: icons.play
                        size: mm(5)
                        anchors.right: suIndexSuccess_v.left
                        anchors.rightMargin: mainClmn.spacing


                        busy: explore.indexRunning_v
                        onBusyChanged: if (!busy) suIndexSuccess_v.state = explore.indexError ? suIndexSuccess_v.fail : suIndexSuccess_v.ok

                        onClicked: {
                            suIndexSuccess_v.state = suIndexSuccess_v.neutral;
                            if(!explore.createIndex(cbIndex_v.currentText, false)) suIndexSuccess_v.setState(suIndexSuccess_v.fail) ;
                        }
                    }

                    IconSuccess {
                        id: suIndexSuccess_v
                        anchors.right: parent.right

                        size: mm(5)
                    }

                }

                RowLayout {
                    id: rlAddIndex_v
                    visible: false

                    anchors.left: parent.left
                    anchors.right: parent.right

                    TextField {
                        id: indexName_v
                        anchors.left: parent.left
                        anchors.right: btnIndexCreateName_v.left
                        anchors.rightMargin: mainClmn.spacing

                        implicitHeight: mm (5)
                        z: 1
                    }

                    IconButton {
                        id: btnIndexCreateName_v
                        anchors.right: idFillI_v.left

                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            if(indexName_v.text !== "") {
                                explore.createIndexName(indexName_v.text, false);
                                cbIndex_v.currentIndex = cbIndex_v.find(indexName_v.text);
                                indexName_v.text = "";
                            }
                            rlAddIndex_v.visible = false;
                        }
                    }


                    Item {
                        id: idFillI_v
                        width: mm(20)
                        anchors.right: parent.right
                    }

                }

            }
        }

        GroupBox {
            title: "Text Indices"
            Layout.fillWidth: true
            id: grIdxText
            visible: false
            ColumnLayout {
                anchors.fill: parent
                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    ComboBox {
                        id: cbIndex_t;
                        anchors.left: parent.left
                        anchors.right: btnEditIndex_t.left
                        anchors.rightMargin: mainClmn.spacing

                        implicitHeight: mm(5);
                        textRole: "display";
                        model: qcblexplore.indices_t
                    }

                    IconButton {
                        id: btnEditIndex_t;
                        size: mm(5)
                        anchors.right: btnDropIndex_t.left
                        icon: icons.document

                        onClicked: {
                            showIndices(false);
                            grQueries.visible = false;
                            grSearch.visible = false;
                            result.visible = false;
                            jEditIQ.fname = cbIndex_t.currentText;
                            grEditIndex.textIndex = true;
                            grEditIndex.title = "Index \"" + cbIndex_t.currentText + "\"";
                            grEditIndex.visible = true;

                            jEditIQ.visible = true;
                            jEditIQ.setJson(explore.indexContent_t(cbIndex_t.currentText));
                        }
                    }

                    IconButton {
                        id: btnDropIndex_t
                        icon: icons.ios_minus_outline
                        size: mm(5)
                        anchors.right: btnIndexCreate_st.left

                        onClicked: {

                            suIndexSuccess_t.state = explore.deleteIndex(cbIndex_t.currentText, true) ? suIndexSuccess_t.ok : suIndexSuccess_t.fail;
                        }
                    }

                    IconButton {
                        id: btnIndexCreate_st
                        anchors.right: btnCreateIndex_t.left

                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            rlAddIndex_t.visible = true;
                        }
                    }


                    IconButton {
                        id: btnCreateIndex_t;
                        icon: icons.play
                        size: mm(5)

                        anchors.right: suIndexSuccess_t.left
                        anchors.rightMargin: mainClmn.spacing

                        busy: explore.indexRunning_t
                        onBusyChanged: if (!busy) suIndexSuccess_t.state = explore.indexError ? suIndexSuccess_t.fail : suIndexSuccess_t.ok

                        onClicked: {
                            suIndexSuccess_t.state = suIndexSuccess_t.neutral;
                            var currentText = cbIndex_t.currentText;
                            if(!explore.createIndex(cbIndex_t.currentText, true)) suIndexSuccess_t.setState(suIndexSuccess_v.fail) ;
                        }
                    }

                    IconSuccess {
                        id: suIndexSuccess_t
                        anchors.right: parent.right
                        size: mm(5)
                    }

                }

                RowLayout {
                    id: rlAddIndex_t
                    visible: false

                    anchors.left: parent.left
                    anchors.right: parent.right

                    TextField {
                        id: indexName_t
                        anchors.left: parent.left
                        anchors.right: btnIndexCreateName_t.left
                        anchors.rightMargin: mainClmn.spacing

                        implicitHeight: mm (5)
                        z: 1
                    }

                    IconButton {
                        id: btnIndexCreateName_t
                        anchors.right: idFillI_t.left

                        size: mm(5)
                        icon: icons.ios_plus_outline
                        onClicked: {
                            if(indexName_t.text !== "") {
                                explore.createIndexName(indexName_t.text, true);
                                cbIndex_t.currentIndex = cbIndex_t.find(indexName_t.text);
                                indexName_t.text = "";
                            }
                            rlAddIndex_t.visible = false;
                        }
                    }


                    Item {
                        id: idFillI_t
                        width: mm(20)
                        anchors.right: parent.right
                    }

                }
            }
        }

        Item {
            id: result
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollView {
                anchors.fill: parent
                Component {
                    id: cntDelegate

                    Rectangle {
                        id: content

                        anchors { left: parent.left;right: parent.right }
                        height: mainClmn.implicitHeight + 4

                        border.width: 1
                        border.color: "lightsteelblue"

                        radius: 2

                        ColumnLayout {
                            id: mainClmn
                            anchors { fill: parent;margins: 2 }
                            RowLayout {
                                Text { Layout.preferredHeight: mm(5);
                                    text: "Result:"
                                    color: "red";
                                    Layout.preferredWidth: mm(10)
                                    horizontalAlignment: Text.AlignRight
                                    verticalAlignment: Text.AlignTop }
                                TextField { visible: !model.queryFulltext;Layout.preferredHeight: mm(5);Layout.fillWidth: true;text: model.text;readOnly: true }
                                TextArea {
                                    visible: model.queryFulltext
                                    textFormat: TextEdit.RichText
                                    Layout.preferredHeight: mm(10);

                                    Layout.fillWidth: true;
                                    text: model.text;
                                    readOnly: true
                                }
                            }
                            RowLayout {
                                Text { Layout.preferredHeight: mm(5);
                                    text: 'DocId:';
                                    color: "red";
                                    Layout.preferredWidth: mm(10)
                                    horizontalAlignment: Text.AlignRight }
                                TextField { Layout.preferredHeight: mm(5);
                                    text: model.docId;
                                    readOnly: true }
                                Item { Layout.fillWidth: true; }
                                Text { Layout.preferredWidth: mm(20);
                                    Layout.preferredHeight: mm(5);
                                    text: model.rowInfo;
                                    color: "gray";
                                    horizontalAlignment: Text.AlignRight }
                                Button {
                                    id: btnEdit;

                                    implicitWidth: mm(5);
                                    implicitHeight: mm(5);

                                    style: ButtonStyle {
                                        label: IconLbl {
                                            icon: icons.document
                                            size: mm(3.5);
                                            color: "black"
                                        }
                                    }

                                    onClicked: {
                                        jViewDoc.docItem = model.item;
                                        jViewDoc.jEditorUpdate();
                                        result.visible = false;
                                        grEditIndex.visible = false;
                                        grEditQuery.visible = false;
                                        grSearch.visible = false;
                                        showIndices(false);
                                        grEditJson.title = jViewDoc.title;
                                        grEditJson.visible = true;
                                        jViewDoc.visible = true;
                                    }
                                }

                            }

                        }
                    }
                }

                ListView {
                    id: view

                    anchors { fill: parent;margins: 2 }

                    model: qcblexplore.searchItems
                    delegate: cntDelegate

                    spacing: 4
                    cacheBuffer: 50
                }
            }
        }

        JsonEdit {
            property string fname

            id: jEditIQ
            visible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        JsonEdit {
            id: jViewDoc

            readOnly: true

            function jEditorUpdate() {
                docId = docItem.docId;
                revId = docItem.revision;
                setJson(qcblexplore.readDocument(docId, revId));
                cursorPosition = 0;

                var docDisplay = docId.length > 12 ? docId.substring(0, 10) + "..." : docId;
                var revDisplay = revId.length > 12 ? revId.substring(0, 10) + "..." : revId;
                title = "DocID: " + docDisplay + ", RevID: " + revDisplay;
            }

            visible: false
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

    }

    MessageDialog {
        id: delConfirmDlg

        property string name
        property string type
        visible: false
        title: "Delete " + type
        text: "Shure to delete " + name + "?"
        standardButtons: StandardButton.Yes | StandardButton.Abort
        onYes: {
            var success = false;
            if (type === "Index") {
                success = explore.deleteIndexContent_v(name)
                if (success) cbIndex_v.currentIndex = -1
            } else if (type === "Query") {
                success = explore.deleteQueryContent(name)
                if (success) cbQuery.currentIndex = -1
            }
            if (success) {
                jEditIQ.visible = false;
                grIQ.visible = true;
                grSearch.visible = explore.hasQueryVar1;
                result.visible = true;
                grEditQuery.visible = false;
                grEditIndex.visible = false;
            } else {
                iconSuccessQuery.icon = iconSuccessQuery.icons.alert_circled;
                iconSuccessQuery.color = "red"
            }
            visible = false;
        }
    }
}
