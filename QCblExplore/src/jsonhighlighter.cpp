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

#include "jsonhighlighter.h"
#include <QColor>

void JSONHighlighter::highlightBlock(const QString &text)
{
    QTextCharFormat propertyFormat;
    propertyFormat.setForeground(QColor("black"));

    QTextCharFormat quotedStringFormat;
    quotedStringFormat.setForeground(QColor("green"));

    QTextCharFormat numericConstantFormat;
    numericConstantFormat.setForeground(QColor("blue"));

    QTextCharFormat boolFormat;
    boolFormat.setForeground(QColor("blue"));

    QRegularExpression numericPattern(":\\s*([0-9]+\\.?[0-9]*)");
    QRegularExpression quotedStringPattern("\"(.*?[^\\\\])??((\\\\\\\\)+)?+\"");
    QRegularExpression propertyPattern("\\s*\\\"*(.*?)\\\"*\\s*:.*");
    QRegularExpression boolPattern("([Tt][Rr][Uu][Ee]|[Ff][Aa][Ll][Ss][Ee])");

    applyHighlight(text, numericPattern, numericConstantFormat, 1);
    applyHighlight(text, quotedStringPattern, quotedStringFormat, 1);
    applyHighlight(text, propertyPattern, propertyFormat, 1);
    applyHighlight(text, boolPattern, boolFormat, 1);
 }

void JSONHighlighter::applyHighlight(const QString &text, QRegularExpression &re, QTextCharFormat &format, int group)
{
    QRegularExpressionMatchIterator iter = re.globalMatch(text);
    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        setFormat(match.capturedStart(group), match.capturedLength(group), format);
    }
}
