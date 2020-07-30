#ifndef DATABASETABLEVIEWMODEL_H
#define DATABASETABLEVIEWMODEL_H

#include <QQueue>
#include <QThread>

#include "taskedlistmodel.h"
#include "databaseviewmodeldetail.h"
#include "databaseviewmodeldetail_vector.h"

#include "CommonDatabase/TupleConversions/typelist.h"

#include "CommonDatabase/filter.h"
#include "asyncdatabase.h"

#include "DebugOutput/debugoutput_disabled.h"


template <typename Database, size_t tableIndex, typename FilterType>
class DatabaseTableViewModel;

template <size_t tableIndex, typename FilterType, typename... T>
class DatabaseTableViewModel<Database<T...>, tableIndex, FilterType> : public TaskedListModel
{
public:
    explicit DatabaseTableViewModel(
            AsyncDatabase<T...>* database,
            QStringList roles,
            DatabaseViewModelDetail::Direction d = DatabaseViewModelDetail::Direction::NonReversed,
            QObject* parent = nullptr
            );

protected:
    explicit DatabaseTableViewModel(
            bool reversed,
            QObject* parent = nullptr
            );
    void setDatabase(AsyncDatabase<T...>* database, QStringList roles);

private:
    void init(QStringList roles);

public:
    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override final;
    QHash<int, QByteArray> roleNames() const override;
    bool canFetchMore(const QModelIndex& parent) const override final;
    void fetchMore(const QModelIndex& parent) override final;

    void addRecord(const typename Conversions::TypeAt<
                   Conversions::TypeList<T...>, tableIndex
                   >::Type& data);

//    void addRecordAsync(const typename Conversions::TypeAt<
//                   Conversions::TypeList<T...>, tableIndex
//                   >::Type& data);

    DatabaseRecord<
        typename Conversions::TypeAt<
            Conversions::TypeList<T...>, tableIndex
        >::Type> recordAt(uint index);


    void updateRecord(
            DatabaseRecord<
                typename Conversions::TypeAt<
                    Conversions::TypeList<T...>, tableIndex
                >::Type
            > record);
    //void setDatabase(Database<T...>* database);
    void setFilter(FilterType filter);

protected:
    QVariantList rowData(uint rowIndex) const;

private:
    struct RowData {
        uint row;
        QVariantList columns;
    };

private:
    void initialFillModel();
    WriteToDatabaseResult<
        typename Conversions::TypeAt<
            Conversions::TypeList<T...>, tableIndex
            >::Type
        > writeToDatabase(
            const typename Conversions::TypeAt<
                Conversions::TypeList<T...>, tableIndex>::Type&
                data);
private:
    AsyncDatabase<T...>* _database;
    DatabaseViewModelDetail::Vector<
        DatabaseRecord<
        typename Conversions::TypeAt<
            Conversions::TypeList<T...>, tableIndex
            >::Type>,
        100u> _data;
    FilterType _filter;

    QHash<int, QByteArray> _roleNames;

//    qint64 _maxRowId;

    /**
     * @brief _row
     * @details Cache for row fields
     */
    mutable RowData _row;
    const bool _reversed;

    bool _canFetchMore;
    bool _nowFetch;

};

template <size_t tableIndex, typename FilterType, typename... T>
DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::DatabaseTableViewModel(
        AsyncDatabase<T...>* database,
        QStringList roles,
        DatabaseViewModelDetail::Direction direction,
        QObject* parent
        )
    : TaskedListModel(parent)
    , _database{ database }
    , _data{}
    , _filter()
    , _row{ 0, {} }
    , _reversed{ DatabaseViewModelDetail::Direction::Reversed == direction }
    , _canFetchMore{ false }
    , _nowFetch{ false }
{
    init(roles);
}

template<size_t tableIndex, typename FilterType, typename... T>
DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::DatabaseTableViewModel(
            bool reversed,
            QObject* parent
            )
    : TaskedListModel(parent)
    , _database{ nullptr }
    , _data{}
    , _filter()
    , _row{ 0, {} }
    , _reversed{ reversed }
    , _canFetchMore{ false }
    , _nowFetch{ false }
{}

template<size_t tableIndex, typename FilterType, typename... T>
void DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::setDatabase(
        AsyncDatabase<T...>* database,
        QStringList roles
        )
{
    Q_ASSERT(database != nullptr);
    _database = database;
    init(roles);
}

template<size_t tableIndex, typename FilterType, typename... T>
void DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::init(
        QStringList roles
        )
{
    using List = Conversions::TypeList<T...>;
    using Type = typename Conversions::TypeAt<List, tableIndex>::Type;
    Q_ASSERT_X(StructConversions::StructExtractor<Type>::size == roles.size(),
               "sizes must be the same", "");
    QStringList allRoles({"rowId"});
    allRoles.append(roles);
//    _maxRowId = _database->template maxRowId<tableIndex>();
    //qDebug() << allRoles;
    for (auto i = 0; i < allRoles.size(); ++i) {
        _roleNames.insert(Qt::UserRole + i + 1, allRoles.at(i).toLocal8Bit());
    }
    initialFillModel();
}

template <size_t tableIndex, typename FilterType, typename... T>
QVariant DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::data(
        const QModelIndex& index, int role) const
{
    const int rowIndex = index.row();
//    Q_ASSERT(row < _data.size());
//    if (row != _row.row) {
//        _row.row = row;
//        _row.columns = tupleToVariantList(_data.at(row));
//    }
    auto row = rowData(rowIndex);
    return row.at(role - Qt::UserRole - 1);

}

template <size_t tableIndex, typename FilterType, typename... T>
int DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::rowCount(
        const QModelIndex& /*parent*/) const
{
    return _data.size();
}

template <size_t tableIndex, typename FilterType, typename... T>
QHash<int, QByteArray>
DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::roleNames() const
{
    return _roleNames;
}

template <size_t tableIndex, typename FilterType, typename... T>
bool DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::canFetchMore(
        const QModelIndex& /*parent*/) const
{
    dbg << "Need fetch more:"
        << "data size:" << _data.size()
        << AS_KV(_nowFetch) << AS_KV(_canFetchMore) << "|" << AS_KV(tableIndex);

    return _nowFetch ? false : _canFetchMore;
}

template <size_t tableIndex, typename FilterType, typename... T>
void DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::fetchMore(
        const QModelIndex& /*parent*/)
{
    using StoredType = typename Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>;
    using VectorType = QVector<DatabaseRecord<StoredType>>;

    const auto currentDataCount = _data.size();

    auto guiCb = [this, currentDataCount](VectorType res, bool canFetchMore) {
        Q_ASSERT(this->thread() == QThread::currentThread());
        beginInsertRows(
                    QModelIndex(),
                    currentDataCount,
                    currentDataCount + res.length() - 1
                    );
        dbg << "insert rows from"
            << currentDataCount
            << "to"
            << currentDataCount + res.length() - 1
            << "current data size"
            << _data.size() << "|" << AS_KV(tableIndex);
        _data.append(res);
        dbg << "new data size" << _data.size() << "|" << AS_KV(tableIndex);
        endInsertRows();
        _nowFetch = false;
        _canFetchMore = canFetchMore;
    };

    const auto filter = _filter;
    const auto db = _database->internalDatabase();

    auto dbTask = [this, db, guiCb, filter, currentDataCount]() {
        const auto recordCount = db->template numberOfRecords<tableIndex>(filter);
        auto countToFetch = std::min(
                    recordCount - currentDataCount,
                    DatabaseViewModelDetail::sizeToFetch);
        Q_ASSERT(countToFetch > 0);
        bool canFetchMore = recordCount > countToFetch + currentDataCount;
        auto fetchedData = db->template read<tableIndex>(currentDataCount, countToFetch, _filter);
        if (_reversed) {
            std::reverse(fetchedData.begin(), fetchedData.end());
        }
        auto guiTask = [fetchedData, guiCb, canFetchMore]() {
            guiCb(fetchedData, canFetchMore);
        };
        addGuiTask(guiTask);
    };

    _nowFetch = true;
    dbg << "add need fetch more task" << "|" << AS_KV(tableIndex);
    auto th = _database->internalThread();
    th->work(dbTask);
}

/* ******************************************************************
 * Protected
 * ******************************************************************
 */

namespace DatabaseTableViewModelDetail {

// FIXME add rvalue reference "&&" to Tuple type
template <typename Tuple, size_t...Is>
QVariantList tupleToVariantList(Tuple/*&&*/ t, std::index_sequence<Is...>) {
    QVariantList vl;
    ((
    vl.append(QVariant(std::get<Is>(t)))
    ), ...);
    return vl;
}

template <typename Data>
static constexpr QVariantList structToVariantList(const Data& d) {
    auto t = TupleConversions::makeTuple(d);
    return DatabaseTableViewModelDetail::tupleToVariantList(
                t,
                std::make_index_sequence<
                    StructConversions::StructExtractor<Data>::size
                    >{}
                );
}


} // DatabaseTableViewModelDetail


template <size_t tableIndex, typename FilterType, typename... T>
QVariantList
DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::rowData(
        uint rowIndex) const
{
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT(rowIndex < uint(_data.size()));
    if ((rowIndex != _row.row) || _row.columns.isEmpty()) {
        _row.row = rowIndex;
        _row.columns = DatabaseTableViewModelDetail::structToVariantList(
                    _data.at(rowIndex));
    }
    return _row.columns;
}



template <size_t tableIndex, typename FilterType, typename... T>
void DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::addRecord(
        const typename Conversions::TypeAt<
            Conversions::TypeList<T...>, tableIndex>::Type&
            data
        )
{
    const bool filterPassed = _filter.tryPass(data);
    using DbRec = DatabaseRecord<
    typename Conversions::TypeAt<
        Conversions::TypeList<T...>, tableIndex
    >::Type>;
    auto guiCb = [this, filterPassed](DbRec r) {
        Q_ASSERT(this->thread() == QThread::currentThread());
        const bool needUpdateView = filterPassed
                && (_reversed || _data.isEmpty() || (_data.last().rowId + 1u == r.rowId));
        if (!needUpdateView) {
            return;
        }
        const int index = _reversed ? 0 : _data.length();
        beginInsertRows(QModelIndex(), index, index);
//        qDebug() << "add record at index" << index
//            << "table" << tableIndex
//            << "rowId:" << maxRowId
//            << AS_KV(data);
        if (!_reversed) {
            _data.append(r);
        }
        else {
            _data.prepend(r); // Fast prepending vector
            _row.row++;
        }
        endInsertRows();
        dbg << " record at index" << index << "inserted"
            << "|" << AS_KV(tableIndex);
    };
    auto db = _database->internalDatabase();
    auto dbTask = [this, db, guiCb, data]() {
        const bool success = db->template addRecord<tableIndex>(data);
        const auto maxRowId = db->template maxRowId<tableIndex>();
        const DbRec r(maxRowId, data);
        auto task = [guiCb, r]() {
            guiCb(r);
        };
        if (success) {
            addGuiTask(task);
        }
        else {
            dbg << "ERROR: write record failed";
        }
    };
    dbg << "begin add record" << "|" << AS_KV(tableIndex);
    auto th = _database->internalThread();
    th->work(dbTask);
}

template <size_t tableIndex, typename FilterType, typename... T>
void DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::setFilter(
        FilterType filter)
{
    dbg << "set new filter" << "|" << AS_KV(tableIndex);
    Q_ASSERT(this->thread() == QThread::currentThread());
    if (filter != _filter) {
        dbg << "old row count" << rowCount() << "|" << AS_KV(tableIndex);
        _filter = filter;
        initialFillModel();
        dbg << "current row count" << rowCount() << "|" << AS_KV(tableIndex);
    }
    else {
        dbg << "old filter equals to new";
    }
}

template<size_t tableIndex, typename FilterType, typename... T>
DatabaseRecord<typename Conversions::TypeAt<Conversions::TypeList<T...>, tableIndex>::Type>
DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::recordAt(
        uint index)
{
    Q_ASSERT(this->thread() == QThread::currentThread());
    Q_ASSERT_X(index < _data.size(),
               "Wrong index",
               qPrintable(QString("index: ")+QString::number(index)
               + QString("; size: ") + QString::number(_data.size())
               + QString("; table: ") + QString::number(tableIndex))
               );
    return _data.at(index);
}

template<size_t tableIndex, typename FilterType, typename... T>
void DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::updateRecord(
        DatabaseRecord<
            typename Conversions::TypeAt<
                Conversions::TypeList<T...>, tableIndex
            >::Type
        > record)
{
    using Type = DatabaseRecord<typename Conversions::TypeAt<
        Conversions::TypeList<T...>, tableIndex
    >::Type>;
    const Type data = record;
    // WARNING unused wariable
    const bool filterPassed = _filter.tryPass(data);
    Q_UNUSED(filterPassed);
//    dbg << "filter passed" << filterPassed << "for table:" << tableIndex;

    //qDebug() << AS_KV(needUpdateView);

    auto guiCb = [this, record]() {
        Q_ASSERT(this->thread() == QThread::currentThread());

        // DatabaseRecord comparison by rowId
        const int index = _data.indexOf(record);

        if (index != -1) {
            _data[index] = record;
            //qDebug() << "updating" << AS_KV(index) << AS_KV(tableIndex);
            QModelIndex modelIndex = createIndex(index, 0);
            emit dataChanged(modelIndex, modelIndex);
        }
    };

    auto db = _database->internalDatabase();
    auto dbTask = [this, db, guiCb, record]() {
        const bool success = db->updateRecord(record);
        if (success) {
            addGuiTask(guiCb);
        }
        else {
            dbg << "updating record success";
        }
    };

    auto th = _database->internalThread();
    th->work(dbTask);
}


/* ******************************************************************
 * Private
 * ******************************************************************
 */

template <size_t tableIndex, typename FilterType, typename... T>
void DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::initialFillModel()
{
    if (_database == nullptr) {
        Q_ASSERT(false);
        return;
    }
    _data.clear();
    if (_reversed) {
        _data.reserveSpace();
    }

    using StoredType = typename Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>;
    using VectorType = QVector<DatabaseRecord<StoredType>>;

    auto guiCb = [this](VectorType res, bool canFetchMore) {
        Q_ASSERT(this->thread() == QThread::currentThread());
        beginResetModel();

        _data.append(res);
        if (!_data.isEmpty()) {
            _row.row = 0u;
            _row.columns = DatabaseTableViewModelDetail::structToVariantList(
                        _data.at(0u));
        }
        _nowFetch = false;
        _canFetchMore = canFetchMore;

        endResetModel();

        dbg << "initial fill model finished"
            << "count" << _data.size() << res.size() << "|" << AS_KV(tableIndex);

    };

    const auto filter = _filter;
    auto db = _database->internalDatabase();

    auto dbTask = [this, db, guiCb, filter]() {
        auto recordCount = db->template numberOfRecords<tableIndex>(filter);
        uint countToRead = std::min(
                    recordCount,
                    DatabaseViewModelDetail::sizeToFetch
                    );
        if (countToRead == 0) {
            auto guiClearCb = [this]() {
                Q_ASSERT(this->thread() == QThread::currentThread());
                beginResetModel();
                _canFetchMore = false;
                endResetModel();
                dbg << "initial table" << tableIndex << "cleared";
            };
            auto guiTask = [guiClearCb]() {
                guiClearCb();
            };
            addGuiTask(guiTask);
            return;
        }
        bool canFetchMore = recordCount > countToRead;
        VectorType res;
        if (Q_LIKELY(!_reversed)) {
            res = db->template read<tableIndex>(
                        0u, countToRead, filter);
        }
        else {
            res = db->template read<tableIndex>(
                        recordCount - countToRead,
                        countToRead,
                        filter);
            std::reverse(res.begin(), res.end());
        }
        auto guiTask = [res, guiCb, canFetchMore]() {
            guiCb(res, canFetchMore);
        };
        addGuiTask(guiTask);
    };

    dbg << "begin initial fill model" << "|" << AS_KV(tableIndex);
    _nowFetch = false;
    auto th = _database->internalThread();
    th->work(dbTask);
}


template <size_t tableIndex, typename FilterType, typename... T>
WriteToDatabaseResult<
    typename Conversions::TypeAt<
        Conversions::TypeList<T...>, tableIndex
        >::Type
    >
DatabaseTableViewModel<Database<T...>, tableIndex, FilterType>::writeToDatabase(
        const typename Conversions::TypeAt<
            Conversions::TypeList<T...>, tableIndex>::Type&
            data)
{
//    qDebug() << "writing to database" << QThread::currentThread();
    Q_ASSERT(false);
    auto rowCount = 0u;
    bool filterPassed = _filter.tryPass(data);
//    qDebug() << "filter passed" << filterPassed << "for table:" << tableIndex;
    bool needUpdateView = filterPassed
            && (_reversed
                || ((rowCount = _database->template numberOfRecords<tableIndex>(_filter))
                    == uint(_data.size())));

//    qDebug() << AS_KV(needUpdateView) << AS_KV(_reversed);

//    using Type = typename Conversions::TypeAt<
//            Conversions::TypeList<T...>, tableIndex
//            >::Type;
    bool success = _database->template addRecord<tableIndex>(data);
    auto maxRowId = _database->template maxRowId<tableIndex>();

    WriteToDatabaseResult<
            typename Conversions::TypeAt<
                Conversions::TypeList<T...>, tableIndex
                >::Type> r
    {
        DatabaseRecord(maxRowId, data),
        rowCount,
        bool(needUpdateView && success)
    };

    return r;
}

#include "DebugOutput/undefdebug.h"

#endif // DATABASETABLEVIEWMODEL_H
