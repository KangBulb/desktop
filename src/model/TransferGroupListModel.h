//
// Created by veli on 1/22/19.
//

#ifndef TREBLESHOT_TRANSFERGROUPLISTMODEL_H
#define TREBLESHOT_TRANSFERGROUPLISTMODEL_H

#include <QtCore/QAbstractListModel>
#include <iostream>
#include <src/util/AppUtils.h>
#include <src/database/object/TransferGroup.h>
#include <QtCore/QDateTime>
#include <src/database/object/NetworkDevice.h>
#include <QIcon>
#include <QtGui/QIconEngine>

class TransferGroupListModel
        : public QAbstractTableModel {
    QList<TransferGroup *> *m_list;

public:
    enum ColumnNames {
        Devices,
        Size,
        Status,
        Date
    };

    explicit TransferGroupListModel(QObject *parent = nullptr)
            : QAbstractTableModel(parent)
    {
        auto *db = AppUtils::getDatabase();
        auto *selection = new SqlSelection;

        selection->setTableName(DbStructure::TABLE_TRANSFERGROUP);

        m_list = db->castQuery(*selection, new TransferGroup());
    }

    ~TransferGroupListModel() override
    {
        delete m_list;
    }

    int columnCount(const QModelIndex &parent) const override
    {
        return sizeof(ColumnNames);
    }

    int rowCount(const QModelIndex &parent) const override
    {
        return m_list->size();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        if (role == Qt::DisplayRole) {
            if (orientation == Qt::Horizontal) {
                switch (section) {
                    case ColumnNames::Status:
                        return tr("Status");
                    case ColumnNames::Devices:
                        return tr("Devices");
                    case ColumnNames::Date:
                        return tr("Date");
                    case ColumnNames::Size:
                        return tr("Size");
                    default:
                        return QString("?");
                }
            } else
                return QString("%1").arg(section);
        }

        return QVariant();
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::DisplayRole) {
            const TransferGroup *currentGroup = m_list->at(index.row());

            switch (index.column()) {
                case ColumnNames::Devices: {
                    auto *selection = new SqlSelection();

                    selection->setTableName(DbStructure::TABLE_TRANSFERASSIGNEE)
                            ->setWhere(QString("`%1` = ?").arg(DbStructure::FIELD_TRANSFERASSIGNEE_GROUPID));
                    selection->whereArgs << currentGroup->groupId;

                    QList<TransferAssignee *> *assigneeList = AppUtils::getDatabase()->castQuery(*selection, new TransferAssignee());
                    QString devicesString;

                    for (const TransferAssignee *assignee : assigneeList->toStdList()) {
                        try {
                            auto *device = new NetworkDevice(assignee->deviceId);

                            AppUtils::getDatabase()->reconstruct(device);

                            if (devicesString.length() > 0)
                                devicesString.append(", ");

                            devicesString.append(device->nickname);

                            delete device;
                        } catch (...) {
                            // We will not add this device to the list.
                            std::cout << "An assignee failed to reconstruct";
                        }
                    }

                    if (devicesString.length() == 0)
                        devicesString.append("-");

                    delete assigneeList;

                    return devicesString;
                }
                case ColumnNames::Status: {

                }
                case ColumnNames::Size:
                case ColumnNames::Date:
                    return QDateTime::fromTime_t(static_cast<uint>(currentGroup->dateCreated))
                            .toString("ddd, d MMM");
                default:
                    return QString("Data id %1x%2")
                            .arg(index.row())
                            .arg(index.column());
            }
        } else if (role == Qt::DecorationRole) {
            switch (index.column())
                case ColumnNames::Devices:
                    return QIcon(":/icon/arrow_down");
        }

        return QVariant();
    }
};


#endif //TREBLESHOT_TRANSFERGROUPLISTMODEL_H
