#ifndef DOCITEM_H
#define DOCITEM_H

#include <QObject>
#include <QHash>

class QExplore;

class DocItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString docId READ docId WRITE setDocId NOTIFY docIdChanged)
    Q_PROPERTY(QString revision READ revision WRITE setRevision NOTIFY revisionChanged)
    Q_PROPERTY(QString currRevision READ currRevision WRITE setCurrRevision NOTIFY currRevisionChanged)
    Q_PROPERTY(bool isCurrRevision READ isCurrRevision NOTIFY revisionChanged)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY titleChanged)
    Q_PROPERTY(QString rowInfo READ rowInfo WRITE setRowInfo NOTIFY rowInfoChanged)
    Q_PROPERTY(QExplore* explore READ explore WRITE setExplore NOTIFY exploreChanged)
    Q_PROPERTY(DocItem* item READ item CONSTANT)
    Q_PROPERTY(bool queryFulltext READ queryFulltext CONSTANT)

public:
    DocItem(QObject *parent = 0);
    ~DocItem ();


    inline DocItem* item() { return this; }

    QExplore* explore() const;
    void setExplore(QExplore* explore) ;

    const QString& docId() const;
    void setDocId(const QString& docId) ;

    const QString& revision() const;
    void setRevision(const QString& revision) ;

    const QString& currRevision() const;
    void setCurrRevision(const QString& currRevision) ;

    inline bool isCurrRevision() {
        return m_currRevision  == m_revision;
    }

    const QString& text() const;
    void setText(const QString& text) ;

    const QString& rowInfo() const;
    void setRowInfo(const QString& info) ;

    bool queryFulltext() const;
    void setQueryFulltext(bool val);


Q_SIGNALS:
    void docIdChanged();
    void revisionChanged();
    void currRevisionChanged();
    void titleChanged();
    void rowInfoChanged();
    void jsonTextChanged();
    void matchesChanged();
    void exploreChanged();


private:

    QExplore*           m_explore;
    QString             m_docId;
    QString             m_revision;
    QString             m_currRevision;
    QString             m_text;
    QString             m_rowInfo;
    bool                m_queryFulltext;
};

#endif // DOCITEM_H
