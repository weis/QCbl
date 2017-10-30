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

#include "docitem.h"
#include "qexplore.h"
#include <QDebug>
#include <QJsonDocument>
#include "qexplore.h"

DocItem::~DocItem()
{
    c4docobs_free((C4DocumentObserver*) m_docObserver);
}

DocItem::DocItem(QObject *parent)
    : QObject(parent)
    , m_explore(nullptr)
    , m_docObserver(nullptr)
{
}

QExplore *DocItem::explore() const
{
    return m_explore;
}

void DocItem::setExplore(QExplore* explore)
{
    if(m_explore == explore)
        return;

    m_explore = explore;
    emit exploreChanged();
}

void* DocItem::docObs() const
{
    return m_docObserver;
}

void DocItem::setDocObs(void* obs)
{
    m_docObserver = obs;
}


const QString& DocItem::docId() const
{
    return m_docId;
}

void DocItem::setDocId(const QString& docId)
{
    if(m_docId == docId)
        return;

    m_docId = docId;
    emit docIdChanged();
}

const QString& DocItem::revision() const
{
    return m_revision;
}

void DocItem::setRevision(const QString& revision)
{
    if(m_revision == revision)
        return;

    m_revision = revision;
    emit revisionChanged();
}


const QString& DocItem::currRevision() const
{
    return m_currRevision;
}

void DocItem::setCurrRevision(const QString& currRevision)
{
    if(m_currRevision == currRevision)
        return;

    m_currRevision = currRevision;
    emit currRevisionChanged();
}

quint64 DocItem::sequenceNumber() const
{
    return m_sequenceNumber;
}

void DocItem::setSequenceNumber(quint64 seq)
{
    if(m_sequenceNumber == seq)
        return;

    m_sequenceNumber = seq;
    emit sequenceNumberChanged();
}


const QString& DocItem::text() const
{
    return m_text;
}

void DocItem::setText(const QString& text)
{
    if(m_text == text)
        return;
    m_text = text;
    emit titleChanged();
}

const QString& DocItem::rowInfo() const
{
    return m_rowInfo;
}

void DocItem::setRowInfo(const QString& info)
{
    if(m_rowInfo == info)
        return;
    m_rowInfo = info;
    emit rowInfoChanged();
}

bool DocItem::queryFulltext() const
{
    return m_queryFulltext;
}

void DocItem::setQueryFulltext(bool val)
{
    m_queryFulltext = val;
}


