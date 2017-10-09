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

#include "documenthandler.h"

#include <QtGui/QTextDocument>
#include <QtGui/QTextCursor>
#include <QtGui/QFontDatabase>
#include <QtCore/QFileInfo>

DocumentHandler::DocumentHandler()
    : m_target(0)
    , m_doc(0)
    , m_cursorPosition(-1)
    , m_selectionStart(0)
    , m_selectionEnd(0)
    , m_highlighter(0)
{
}

void DocumentHandler::setTarget(QQuickItem *target)
{
    m_doc = 0;
    m_highlighter = 0;
    m_target = target;
    if (!m_target)
        return;

    QVariant doc = m_target->property("textDocument");
    if (doc.canConvert<QQuickTextDocument*>()) {
        QQuickTextDocument *qqdoc = doc.value<QQuickTextDocument*>();
        if (qqdoc)
            m_doc = qqdoc->textDocument();
            m_highlighter = new JSONHighlighter(m_doc);
            //m_highlighter = new Highlighter(m_doc);
    }
    emit targetChanged();
}

void DocumentHandler::setText(const QString &arg)
{
    if (m_text != arg) {
        m_text = arg;
        emit textChanged();
    }
}

QString DocumentHandler::text() const
{
    return m_text;
}

void DocumentHandler::setCursorPosition(int position)
{
    if (position == m_cursorPosition)
        return;

    m_cursorPosition = position;

    reset();
}

void DocumentHandler::reset()
{

}


QTextCursor DocumentHandler::textCursor() const
{
    QTextCursor cursor = QTextCursor(m_doc);
    if (m_selectionStart != m_selectionEnd) {
        cursor.setPosition(m_selectionStart);
        cursor.setPosition(m_selectionEnd, QTextCursor::KeepAnchor);
    } else {
        cursor.setPosition(m_cursorPosition);
    }
    return cursor;
}

void DocumentHandler::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
}

void DocumentHandler::setSelectionStart(int position)
{
    m_selectionStart = position;
}

void DocumentHandler::setSelectionEnd(int position)
{
    m_selectionEnd = position;
}

