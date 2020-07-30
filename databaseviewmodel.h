#ifndef DATABASEVIEWMODEL_H
#define DATABASEVIEWMODEL_H

#include <algorithm>

#include <QObject>
#include <QAbstractListModel>

#include "private/databasetableviewmodel.h"
#include "private/databaseviewmodeldetail.h"
#include "private/databaseviewmodeldetail_vector.h"

/**
 * @brief The DatabaseViewModel class
 * @details
 */
template <typename Data>
class DatabaseViewModel : public DatabaseTableViewModel<Database<Data>, 0u, Filter<Data, FilterDetail::Blank>>
{
    //Q_OBJECT // Qt not support template QObjects

public:
    explicit DatabaseViewModel(
            const QString& databasePath,
            QStringList roles,
            bool reversed = false,
            QObject* parent = nullptr
            );

private:
    AsyncDatabase<Data> _database;

};

/* ******************************************************************
 * Public
 * ******************************************************************
 */

template <typename Data>
DatabaseViewModel<Data>::DatabaseViewModel(
        const QString& databasePath,
        QStringList roles,
        bool reversed,
        QObject* parent)
    : DatabaseTableViewModel<Database<Data>, 0u, Filter<Data, FilterDetail::Blank>>(
          reversed,
          parent)
    , _database(databasePath)
{
    this->setDatabase(&_database, roles);
}


/* ******************************************************************
 * Private
 * ******************************************************************
 */

namespace DatabaseViewModelDetail {

// FIXME add rvalue reference "&&" to Tuple type
template <typename Tuple, size_t...Is>
QVariantList tupleToVariantList(Tuple/*&&*/ t, std::index_sequence<Is...>) {
    QVariantList vl;
    ((
    vl.append(QVariant(std::get<Is>(t)))
    ), ...);
    return vl;
}

} // DatabaseViewModelDetail

#endif // DATABASEVIEWMODEL_H
