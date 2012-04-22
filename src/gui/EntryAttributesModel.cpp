/*
 *  Copyright (C) 2012 Felix Geyer <debfx@fobos.de>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 or (at your option)
 *  version 3 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntryAttributesModel.h"

#include "core/Entry.h"
#include "core/Tools.h"

EntryAttributesModel::EntryAttributesModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_entryAttributes(0)
{
}

void EntryAttributesModel::setEntryAttributes(EntryAttributes* entryAttributes)
{
    beginResetModel();

    if (m_entryAttributes) {
        m_entryAttributes->disconnect(this);
    }

    m_entryAttributes = entryAttributes;

    if (m_entryAttributes) {
        updateAttributes();
        connect(m_entryAttributes, SIGNAL(customKeyModified(QString)), SLOT(attributeChange(QString)));
        connect(m_entryAttributes, SIGNAL(aboutToBeAdded(QString)), SLOT(attributeAboutToAdd(QString)));
        connect(m_entryAttributes, SIGNAL(added(QString)), SLOT(attributeAdd()));
        connect(m_entryAttributes, SIGNAL(aboutToBeRemoved(QString)), SLOT(attributeAboutToRemove(QString)));
        connect(m_entryAttributes, SIGNAL(removed(QString)), SLOT(attributeRemove()));
        connect(m_entryAttributes, SIGNAL(aboutToBeReset()), SLOT(aboutToReset()));
        connect(m_entryAttributes, SIGNAL(reset()), SLOT(reset()));
    }

    endResetModel();
}

int EntryAttributesModel::rowCount(const QModelIndex& parent) const
{
    if (!m_entryAttributes || parent.isValid()) {
        return 0;
    }
    else {
        return m_attributes.size();
    }
}

int EntryAttributesModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return 2;
}

QVariant EntryAttributesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation == Qt::Horizontal) && (role == Qt::DisplayRole)) {
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Value");
        }
    }

    return QVariant();
}

QVariant EntryAttributesModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        QString key = m_attributes.at(index.row());

        if (index.column() == 0) {
            return key;
        }
        else {
            return m_entryAttributes->value(key);
        }
    }

    return QVariant();
}

void EntryAttributesModel::attributeChange(QString key)
{
    int row = m_attributes.indexOf(key);
    Q_ASSERT(row != -1);
    Q_EMIT dataChanged(index(row, 0), index(row, columnCount()-1));
}

void EntryAttributesModel::attributeAboutToAdd(QString key)
{
    QList<QString> rows = m_attributes;
    rows.append(key);
    qSort(rows);
    int row = rows.indexOf(key);
    beginInsertRows(QModelIndex(), row, row);
}

void EntryAttributesModel::attributeAdd()
{
    updateAttributes();
    endInsertRows();
}

void EntryAttributesModel::attributeAboutToRemove(QString key)
{
    int row = m_attributes.indexOf(key);
    beginRemoveRows(QModelIndex(), row, row);
}

void EntryAttributesModel::attributeRemove()
{
    updateAttributes();
    endRemoveRows();
}

void EntryAttributesModel::aboutToReset()
{
    beginResetModel();
}

void EntryAttributesModel::reset()
{
    endResetModel();
}

void EntryAttributesModel::updateAttributes()
{
    m_attributes.clear();

    Q_FOREACH (const QString& key, m_entryAttributes->keys()) {
        if (!EntryAttributes::isDefaultAttribute(key)) {
            m_attributes.append(key);
        }
    }
}