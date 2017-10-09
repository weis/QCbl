#ifndef QEXPLORELISTS_H
#define QEXPLORELISTS_H


QQmlListProperty<DocItem> QExplore::searchItems()
{

    return QQmlListProperty<DocItem>(this, 0,
                                     &QExplore::searchItemAppend,
                                     &QExplore::searchItemCount,
                                     &QExplore::searchItemAt,
                                     &QExplore::searchItemClear
                                     );

}

void QExplore::clearSearchItems(bool emitqml)
{
    for (QList<DocItem*>::ConstIterator  it = m_searchitems.constBegin();
         it != m_searchitems.constEnd();
         it++)
    {
        DocItem* o = *it;
        o->deleteLater();
    }
    m_searchitems.clear();
    if(emitqml)
        emit searchItemsChanged();
}

void QExplore::searchItemAppend(QQmlListProperty<DocItem>* l,
                          DocItem* o)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    if (qexplore)
    {
        qexplore->m_searchitems.append(o);
        emit qexplore->searchItemsChanged();
    }
}

void QExplore::searchItemClear(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    if (qexplore && qexplore->m_searchitems.count() > 0 )
    {
        qexplore->clearSearchItems();
    }
}

int QExplore::searchItemCount(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    return qexplore->m_searchitems.count();
}

DocItem* QExplore::searchItemAt(QQmlListProperty<DocItem>* l, int index)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    return qexplore->m_searchitems.at(index);
}


QQmlListProperty<DocItem> QExplore::docItems()
{

    return QQmlListProperty<DocItem>(this, 0,
                                     &QExplore::itemAppend,
                                     &QExplore::itemCount,
                                     &QExplore::itemAt,
                                     &QExplore::itemClear
                                     );

}

void QExplore::clearDocItems(bool emitqml)
{
    for (QList<DocItem*>::ConstIterator  it = m_docitems.constBegin();
         it != m_docitems.constEnd();
         it++)
    {
        DocItem* o = *it;
        o->deleteLater();
    }
    m_docitems.clear();
    if(emitqml)
        emit docItemsChanged();
}

void QExplore::itemAppend(QQmlListProperty<DocItem>* l,
                          DocItem* o)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    if (qexplore)
    {
        qexplore->m_docitems.append(o);
        emit qexplore->docItemsChanged();
    }
}

void QExplore::itemClear(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    if (qexplore && qexplore->m_docitems.count() > 0 )
    {
        qexplore->clearDocItems();
    }
}

int QExplore::itemCount(QQmlListProperty<DocItem>* l)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    return qexplore->m_docitems.count();
}

DocItem* QExplore::itemAt(QQmlListProperty<DocItem>* l, int index)
{
    QExplore* qexplore = qobject_cast<QExplore*>(l->object);
    return qexplore->m_docitems.at(index);
}


#endif // QEXPLORELISTS_H
