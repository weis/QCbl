#ifndef JSONHIGHLIGHTER_H
#define JSONHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>


class JSONHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    JSONHighlighter(QTextDocument* parent) : QSyntaxHighlighter(parent)
    {
    }

protected:
    virtual void highlightBlock(const QString &text);

private:
    void applyHighlight(const QString &text, QRegularExpression &re, QTextCharFormat &format, int group);
};

#endif //  JSONHIGHLIGHTER_H
