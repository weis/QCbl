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
** IMsPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
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
import QExplore.Components 1.0

Rectangle {
    id: root

    property DocItem docItem
    property string docId
    property string revId
    property string title
    property bool   isCurrRev
    property alias cursorPosition: textArea.cursorPosition
    property alias text: textArea.text
    property alias  readOnly:  textArea.readOnly


    color: 'white'

    radius: 10



    function setJson(jsonText)
    {
        var csrPosOld = textArea.cursorPosition;
        textArea.text =  jsonText;
        textArea.cursorPosition = csrPosOld;
        jsonTextChanged = false;
    }

    function getJson()
    {
        return textArea.text;
    }

    function clearJson()
    {
        jsonTextChanged = false;
        textArea.cursorPosition = 0;
        textArea.text = "";
    }


    property bool jsonTextChanged: false

    border { width: 1; color: "lightsteelblue" }

    Item {
        anchors { fill: parent; }
        clip: true
        id: textRoot


        TextArea {
            id: textArea
            backgroundVisible: false
            Accessible.name: "document"


            onTextChanged: jsonTextChanged = true


            anchors {
                margins: 5
                fill: parent
            }
            wrapMode: TextEdit.NoWrap


            selectByMouse: true
            font: Qt.font({ family:"Courier", pointSize: 10 });

            inputMethodHints: Qt.ImhNoPredictiveText

            style: TextAreaStyle {
                textColor: "black"
                selectionColor: "#0C75BC"
                backgroundColor: "transparent"
                renderType: Text.NativeRendering
            }

            DocumentHandler {
                id: document
                cursorPosition: textArea.cursorPosition
                selectionStart: textArea.selectionStart
                selectionEnd: textArea.selectionEnd

                target: textArea
            }

            Rectangle {
                id: editorCurrentLineHighlight
                anchors {
                    left: parent.left
                    margins: 0
                }
                visible: textArea.focus
                width: textArea.flickableItem.width
                height: textArea.cursorRectangle.height

                y: textArea.cursorRectangle.y - textArea.flickableItem.contentY
                color: '#15151510'
            }

        }


    }

}

