#ifndef DATABASE_H
#define DATABASE_H

#include <type_traits>
#include <utility>

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
//#include <QSqlField>
#include <QSqlError>
#include <QFile>
#include <QThread>
#include <QMutex>
#include <QVector>

#include "TupleConversions/structconversions.h"
#include "TupleConversions/conversions.h"
#include "TupleConversions/typelist.h"

#include "database_detail.h"
#include "filter.h"
#include "private/databaserecord.h"

#include "DebugOutput/debugoutput_disabled.h"
//#include <QDebug>
//#include <iostream>
//#define qDebug() while (false) std::cout


struct Wrong {
    const int i;
};

/**
 * @brief The Database class
 */
template <typename... T>
class Database
{
public:
    /**
     * @brief Database
     * @param databasePath Path to database file
     * @param maxDatabaseSize Maximum size of database file in kilobytes
     */
    explicit Database(
            const QString& databasePath,
            int maxDatabaseSize = 2097152 // 2 Mb
            );
    ~Database();

    // TODO make it private
//    using StoredTypes = Conversions::TypeList<T...>;
//    using Conversions::TypeChecker;
//    using Conversions::ValidIndexV;
//    using Conversions::TypeAtT;
//    using Conversions::SameTypeAtV;

    /**
     * @brief open
     * @return if database opened
     */
    //bool open();

    /**
     * @brief addRecord writes record to database
     * @param r record to write
     * @return if write was success
     */
    template <typename Type>
    std::enable_if_t<
        Conversions::TypeChecker<
            Type, Conversions::TypeList<T...>
            >::hasType,
        bool>
    addRecord(const Type& r);

    template <size_t tableIndex,
              typename Type = Conversions::TypeAtT<
                  Conversions::TypeList<T...>, tableIndex>
              >
    std::enable_if_t<
        Conversions::SameTypeAtV<
            Type, Conversions::TypeList<T...>, tableIndex
            >,
        bool>
    addRecord(const Type& r);

    template <typename L = Conversions::TypeList<T...> >
    std::enable_if_t<DatabaseDetail::OneLengthV<L>, bool>
    addRecord(const typename L::Head& r);// { return addRecord<0u>(r); }

    // TODO add transactions when writes several records
    // bool beginTransaction();
    // bool commitTransaction();

    /**
     * @brief numberOfRecords
     * @return -1 if method fails
     */
    template <typename Type, typename FilterType = Filter<Type, FilterDetail::Blank> >
    std::enable_if_t<
        Conversions::TypeChecker<
            Type, Conversions::TypeList<T...>
            >::hasType,
        uint>
    numberOfRecords(FilterType filter = FilterType()) const;

    template <
            size_t tableIndex,
            typename FilterType = Filter<
                typename Conversions::TypeAtT<
                    Conversions::TypeList<T...>, tableIndex
                    >,
                FilterDetail::Blank>>
    std::enable_if_t<
        Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
        uint>
    numberOfRecords(FilterType filter = FilterType()) const;

    template <
            typename L = Conversions::TypeList<T...>,
            typename FilterType = Filter<typename L::Head, FilterDetail::Blank>,
            typename H = typename L::Head
            >
    std::enable_if_t<DatabaseDetail::OneLength<L>::value, uint>
    //std::enable_if_t<DatabaseDetail::OneLengthV<L>, uint>
    numberOfRecords(FilterType filter = FilterType()) const;





    template <typename Type, typename FilterType = Filter<Type, FilterDetail::Blank> >
    std::enable_if_t<
        Conversions::TypeChecker<
            Type, Conversions::TypeList<T...>
            >::hasType,
        qint64>
    maxRowId() const;

    template <
            size_t tableIndex,
            typename FilterType = Filter<
                typename Conversions::TypeAtT<
                    Conversions::TypeList<T...>, tableIndex
                    >,
                FilterDetail::Blank>>
    std::enable_if_t<
        Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
        qint64>
    maxRowId() const;

    template <
            typename L = Conversions::TypeList<T...>,
            typename FilterType = Filter<typename L::Head, FilterDetail::Blank>,
            typename H = typename L::Head
            >
    std::enable_if_t<DatabaseDetail::OneLength<L>::value, qint64>
    //std::enable_if_t<DatabaseDetail::OneLengthV<L>, uint>
    maxRowId() const;

    template <typename Type, typename FilterType = Filter<Type, FilterDetail::Blank> >
    std::enable_if_t<
        Conversions::TypeChecker<
            Type, Conversions::TypeList<T...>>::hasType,
        QVector<DatabaseRecord<Type>> >
    read(unsigned offset, unsigned count,
         FilterType filter = FilterType());
    // TODO add without count
    //QVector<Type> read(unsigned offset);

    template <
            size_t tableIndex,
            typename FilterType = Filter<
                typename Conversions::TypeAtT<
                    Conversions::TypeList<T...>, tableIndex
                    >,
                FilterDetail::Blank>
            >
    QVector<DatabaseRecord<
    typename Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>
    >>
    read(unsigned offset, unsigned count,
         FilterType filter = FilterType());

    template <
            typename L = Conversions::TypeList<T...>,
            typename FilterType = Filter<typename L::Head, FilterDetail::Blank>
            >
    std::enable_if_t<
        DatabaseDetail::OneLength<L>::value,
        QVector<DatabaseRecord<typename L::Head>>
    >
    read(unsigned offset, unsigned count, FilterType filter = FilterType());// { return read<0u>(offset, count); }







    template <typename Type>
    std::enable_if_t<
        Conversions::TypeChecker<
            Type, Conversions::TypeList<T...>
            >::hasType,
        bool>
    updateRecord(const DatabaseRecord<Type>& r);

    template <size_t tableIndex,
              typename Type = Conversions::TypeAtT<
                  Conversions::TypeList<T...>, tableIndex>
              >
    std::enable_if_t<
        Conversions::SameTypeAtV<
            Type, Conversions::TypeList<T...>, tableIndex
            >,
        bool>
    updateRecord(const DatabaseRecord<Type>& r);

    template <typename L = Conversions::TypeList<T...> >
    std::enable_if_t<DatabaseDetail::OneLengthV<L>, bool>
    updateRecord(const DatabaseRecord<typename L::Head>& r);// { return addRecord<0u>(r); }







    /**
     * @brief exists
     * @return true if database file exists
     */
    bool exists() const;

    /**
     * @brief removeHalfRecords
     * Removes half records in database
     * @return if removing was success
     */
    template <typename Type>
    std::enable_if_t<
        Conversions::TypeChecker<
            Type, Conversions::TypeList<T...>
            >::hasType,
        bool>
    removeHalfRecords();

    template <size_t tableIndex>
    std::enable_if_t<
        Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
        bool>
    removeHalfRecords();

    template <typename L = Conversions::TypeList<T...>>
    std::enable_if_t<DatabaseDetail::OneLengthV<L>, bool>
    removeHalfRecords();

    /**
     * @brief clearDatabase
     * Removes all records in database
     * @return if removing was success
     */
    bool clearDatabase();

    /**
     * @brief clearDatabase
     * Removes all records in selected table
     * @return if removing was success
     */
    template <size_t tableIndex>
    std::enable_if_t<
        Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
        bool>
    clearTable();

private:
    /**
     * @brief createTables
     * @return true if database successfully created
     */
    bool createTables();

    template <size_t... Is>
    bool createTablesImpl(std::index_sequence<Is...>);

    /**
     * @brief createTables
     * @return true if database successfully created
     */
    template <size_t tableIndex>
    std::enable_if_t<
        Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
        bool>
    createTable();

    /**
     * @brief checkTypes
     * @return true if table corresponds to Type struct
     */
    bool checkTypes() const;

    /**
     * @brief checkTypes
     * @return true if table corresponds to Type struct
     */
    template <size_t... Is>
    bool checkTypesImpl(std::index_sequence<Is...>) const;

    /**
     * @brief checkTypes
     * @return true if table corresponds to Type struct
     */
    template <size_t tableIndex>
    std::enable_if_t<
        Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
        bool>
    checkType() const;

    /**
     * @brief vacuumDatabase
     * @return true if vacuum operation was success
     */
    bool vacuumDatabase();

    /**
     * @brief isValid return if database valid
     * @return false if cant create database
     * or Type struct fields dont corresponds to
     * database table column names,
     * else return true
     */
    bool isValid() const { return _valid; }

    template <size_t... Is>
    bool clearDatabaseImpl(std::index_sequence<Is...>);

    QString currentThreadConnectionName() const;

    /*
     * For test reasons
     */
    friend class TestDatabase;

private:
    mutable QVector<QString> _connections;
    mutable QMutex _connectionsMutex;

    /**
     * @brief _path
     * Path to database file
     */
    QString _path;

    /**
     * @brief maxDatabaseSize
     * maximum size of database file in kilobytes
     */
    const int _maxDatabaseSize;

    /**
     * @brief _valid
     * true if database valid
     */
    const bool _valid;

};

template<>
class Database<> {};

/* ******************************************************************
 * Public
 * ******************************************************************
 */

namespace DatabasePrivate {
inline const QString DB_TYPE = "QSQLITE";
inline const QString DB_NAME = "dbname";
}

template <typename... T>
Database<T...>::Database(
        const QString& databasePath,
        int maxDatabaseSize
        )
    : _connections{}
    , _connectionsMutex()
    , _path{databasePath}
    , _maxDatabaseSize{maxDatabaseSize}
    , _valid( (
          //QSqlDatabase::addDatabase(DatabasePrivate::DB_TYPE, currentThreadConnectionName()).setDatabaseName(databasePath),
          //QSqlDatabase::database().setDatabaseName(databasePath),
          QSqlDatabase::database(currentThreadConnectionName()).tables().isEmpty()
            ? createTables()
            : checkTypes()
          ) )
{

}


template <typename... T>
Database<T...>::~Database() {
    _connectionsMutex.lock();
    foreach (auto connection, _connections) {
        QSqlDatabase::removeDatabase(connection);
    }
    _connections.clear();
    _connectionsMutex.unlock();
}

template <typename... T>
template <typename Type>
std::enable_if_t<
    Conversions::TypeChecker<
        Type, Conversions::TypeList<T...>
        >::hasType,
    bool>
Database<T...>::addRecord(const Type& r) {
    const auto index = Conversions::ListIndexV<Type, Conversions::TypeList<T...>>;
    return addRecord<index>(r);
}

template <typename... T>
template <size_t tableIndex, typename Type>
std::enable_if_t<
    Conversions::SameTypeAtV<
        Type, Conversions::TypeList<T...>, tableIndex
        >,
    bool>
Database<T...>::addRecord(const Type& r) {
    if (!isValid()) {
        return false;
    }

    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery addQuery(db);

    addQuery.prepare(DatabaseDetail::addRecordQuery<Type, tableIndex>());
//    auto t = TupleConversions::makeTuple(r);
//    const size_t size = std::tuple_size_v<decltype(t)>;

//    DatabaseDetail::QueryFiller<
//            decltype(t), std::make_index_sequence<size>
//            >::populateSqlQuery(addQuery, t);
    DatabaseDetail::populateSqlQuery(addQuery, r);

    return addQuery.exec();
}

template <typename... T>
template <typename L>
std::enable_if_t<DatabaseDetail::OneLengthV<L>, bool>
Database<T...>::addRecord(const typename L::Head& r) {
    return addRecord<0u>(r);
}

template <typename... T>
template <typename Type, typename FilterType>
std::enable_if_t<
    Conversions::TypeChecker<
        Type, Conversions::TypeList<T...>
        >::hasType,
    uint>
Database<T...>::numberOfRecords(FilterType filter) const {
    const auto index = Conversions::ListIndexV<Type, Conversions::TypeList<T...>>;
    return numberOfRecords<index>(filter);
}

template <typename... T>
template <size_t tableIndex, typename FilterType>
std::enable_if_t<
    Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
    uint>
Database<T...>::numberOfRecords(FilterType filter) const {
    if (!isValid()) {
        qDebug() << "Database not valid";
        return -1;
    }
    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery numberQuery(db);
    //QString queryString = DatabaseDetail::maxRowIdQuery();
    QString queryString = DatabaseDetail::countQuery<tableIndex>();
    queryString += filter.query();
    numberQuery.prepare(queryString);
    if (numberQuery.exec()) {
        int idMaxId = numberQuery.record().indexOf("maxId");
        numberQuery.first();
        int numberOfLines = numberQuery.value(idMaxId).toInt();
        return numberOfLines;
    }
    return -1; // TODO change
}

template <typename... T>
template <typename L, typename FilterType, typename H>
std::enable_if_t<DatabaseDetail::OneLength<L>::value, uint>
Database<T...>::numberOfRecords(FilterType filter) const {
    qDebug() << "no template method" << L::length
        << DatabaseDetail::OneLength<L>::value;
    return numberOfRecords<0u>(filter);
}




template <typename... T>
template <typename Type, typename FilterType>
std::enable_if_t<
    Conversions::TypeChecker<
        Type, Conversions::TypeList<T...>
        >::hasType,
    qint64>
Database<T...>::maxRowId() const {
    const auto index = Conversions::ListIndexV<Type, Conversions::TypeList<T...>>;
    return maxRowId<index>();
}

template <typename... T>
template <size_t tableIndex, typename FilterType>
std::enable_if_t<
    Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
    qint64>
Database<T...>::maxRowId() const {
    if (!isValid()) {
        qDebug() << "Database not valid";
        return -1;
    }
    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery numberQuery(db);
    QString queryString = DatabaseDetail::maxRowIdQuery<tableIndex>();
    //QString queryString = DatabaseDetail::countQuery<tableIndex>();
    numberQuery.prepare(queryString);
    if (numberQuery.exec()) {
        int idMaxId = numberQuery.record().indexOf("maxId");
        numberQuery.first();
        int numberOfLines = numberQuery.value(idMaxId).toInt();
        return numberOfLines;
    }
    return -1; // TODO change
}

template <typename... T>
template <typename L, typename FilterType, typename H>
std::enable_if_t<DatabaseDetail::OneLength<L>::value, qint64>
Database<T...>::maxRowId() const {
    qDebug() << "no template method" << L::length
        << DatabaseDetail::OneLength<L>::value;
    return maxRowId<0u>();
}





template <typename... T>
template <typename Type, typename FilterType>
std::enable_if_t<
    Conversions::TypeChecker<
        Type, Conversions::TypeList<T...>>::hasType,
    QVector<DatabaseRecord<Type>> >
Database<T...>::read(uint offset, uint count, FilterType filter) {
    const auto index = Conversions::ListIndexV<Type, Conversions::TypeList<T...>>;
    return read<index>(offset, count, filter);
}

template <typename... T>
template <size_t tableIndex, typename FilterType>
QVector<DatabaseRecord<Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>>>
Database<T...>::read(uint offset, uint count, FilterType filter) {
    using Type = DatabaseRecord<
        Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>
        >;
    QVector<Type> result;
    if (!isValid()) {
        // TODO Do error handling
        qDebug() << "Database not valid";
        return result;
    }

    auto recordsCount = numberOfRecords<tableIndex>(filter);
    if (recordsCount < 0) {
        // TODO Do error handling
        qDebug() << "wrong records count";
        return result;
    }
    result.reserve(std::min(count, uint(recordsCount)));

    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery readQuery(db);
    auto queryString = DatabaseDetail::readQuery<tableIndex>(
                offset, count, filter.query());
//    qDebug() << AS_KV(queryString) << _path;
    readQuery.prepare(queryString);
    if (!readQuery.exec()) {
        // TODO Do error handling
        qDebug() << "Read query exec error occured"
            << readQuery.lastError().text();
        return result;
    }

    readQuery.first();
    readQuery.previous();

    while (readQuery.next())  {
        auto t = DatabaseDetail::extractRecord<Type>(readQuery.record());
        Type s = StructConversions::makeFromTuple<Type>(t);
        result.append(s);
    }

    return result;
}


template <typename... T>
template <typename L, typename FilterType>
std::enable_if_t<
    DatabaseDetail::OneLength<L>::value,
    QVector<DatabaseRecord<typename L::Head>>
>
Database<T...>::read(unsigned offset, unsigned count, FilterType filter) {
    return read<0u>(offset, count, filter);
}








template <typename... T>
template <typename Type>
std::enable_if_t<
    Conversions::TypeChecker<
        Type, Conversions::TypeList<T...>
        >::hasType,
    bool>
Database<T...>::updateRecord(const DatabaseRecord<Type>& r) {
    const auto index = Conversions::ListIndexV<Type, Conversions::TypeList<T...>>;
    return updateRecord<index>(r);
}

template <typename... T>
template <size_t tableIndex, typename Type>
std::enable_if_t<
    Conversions::SameTypeAtV<
        Type, Conversions::TypeList<T...>, tableIndex
        >,
    bool>
Database<T...>::updateRecord(const DatabaseRecord<Type>& r) {
    if (!isValid()) {
        return false;
    }

    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery updateQuery(db);
    const Type& t = r;
    auto queryString = DatabaseDetail::updateRecordQuery<Type, tableIndex>(r.rowId);
    updateQuery.prepare(queryString);
    DatabaseDetail::populateSqlQuery(updateQuery, t);
    const auto success = updateQuery.exec();

    //qDebug() << "update successful:" << success << queryString;
    if (!success) {
        qDebug() << updateQuery.lastError().text()
            << updateQuery.lastError().nativeErrorCode()
            << updateQuery.lastError().type()
            << updateQuery.lastQuery();
    }
    return success; //TODO
}

template <typename... T>
template <typename L>
std::enable_if_t<DatabaseDetail::OneLengthV<L>, bool>
Database<T...>::updateRecord(const DatabaseRecord<typename L::Head>& r) {
    return updateRecord<0u>(r);
}



template <typename... T>
bool Database<T...>::exists() const {
    QFile db(_path);
    return db.exists();
}

template <typename... T>
template <typename Type>
std::enable_if_t<
    Conversions::TypeChecker<
        Type, Conversions::TypeList<T...>
        >::hasType,
    bool>
Database<T...>::removeHalfRecords() {
    // TODO constexpr
    const auto index = Conversions::ListIndexV<Type, Conversions::TypeList<T...>>;
    return removeHalfRecords<index>();
}

template <typename... T>
template <size_t tableIndex>
std::enable_if_t<
    Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
    bool>
Database<T...>::removeHalfRecords() {
    if (!isValid()) {
        return false;
    }

    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery deleteQuery(db);

    QString query = DatabaseDetail::removeHalfRecordsQuery<tableIndex>();

    deleteQuery.prepare(query);

    return (deleteQuery.exec()
            ? vacuumDatabase()
            : false);
}

template <typename... T>
template <typename L>
std::enable_if_t<DatabaseDetail::OneLengthV<L>, bool>
Database<T...>::removeHalfRecords() {
    return removeHalfRecords<0u>();
}

template <typename... T>
bool Database<T...>::clearDatabase() {
    const auto count = sizeof...(T);
    bool result = clearDatabaseImpl(std::make_index_sequence<count>{});
    result &= vacuumDatabase();
    return result;
}

template<typename... T>
template <size_t tableIndex>
std::enable_if_t<
    Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
    bool>
Database<T...>::clearTable() {
    if (!isValid()) {
        return false;
    }

    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery clearQuery(db);

    QString queryString = "DELETE FROM ";
    queryString += DatabaseDetail::tableName<tableIndex>();
    clearQuery.prepare(queryString);

    return clearQuery.exec();
}

/* ******************************************************************
 * Private
 * ******************************************************************
 */

template <typename... T>
bool Database<T...>::createTables() {
    auto const count = sizeof...(T);
    return createTablesImpl(std::make_index_sequence<count>{});
}

template <typename... T>
template <size_t... Is>
bool Database<T...>::createTablesImpl(std::index_sequence<Is...>) {
    auto results = std::make_tuple(createTable<Is>()...);
    bool success = (std::get<Is>(results) & ...);
    return success;
}

template <typename... T>
template <size_t tableIndex>
std::enable_if_t<
    Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
    bool>
Database<T...>::createTable() {
    using Type = Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>;
    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    //qDebug() << "valid:" << db.isValid() << db.isOpen();

    QSqlQuery createQuery(db);
    auto queryString = DatabaseDetail::createTableQuery<Type, tableIndex>();

    //qDebug() << queryString;
    createQuery.prepare(queryString);
    //createQuery.prepare("CREATE TABLE database(_0event INTEGER);");

    bool success = createQuery.exec();
    qDebug()  << "table created:" << success
              << DatabaseDetail::tableName<tableIndex>()
              << DatabaseDetail::createTableQueryPart<Type>();
    if (!success) {
        qDebug() << createQuery.lastError().text()
            << createQuery.lastError().nativeErrorCode()
            << createQuery.lastError().type();
    }
    return success;
}

template <typename... T>
bool Database<T...>::checkTypes() const {
    auto const count = sizeof...(T);

    qDebug() << "check types";
    {
        auto connectionName = currentThreadConnectionName();
        auto db = QSqlDatabase::database(connectionName);
        if (db.tables().size() != count) {
            qDebug() << db.databaseName()
                << "wrong tables count. Wait for"
                << count << "but exist" << db.tables().size();
            return false;
        }
    }

    //TODO check each type in separate function
    //qDebug() << record;
    // TODO      

    const bool checkCorrect = checkTypesImpl(std::make_index_sequence<count>{});
    dbg << AS_KV(checkCorrect);
    return checkCorrect;
}

template <typename... T>
template<size_t... Is>
bool Database<T...>::checkTypesImpl(std::index_sequence<Is...>) const
{
    auto results = std::make_tuple(checkType<Is>()...);
    bool success = (std::get<Is>(results) & ...);
    return success;
}

template <typename... T>
template <size_t tableIndex>
std::enable_if_t<
    Conversions::ValidIndexV<Conversions::TypeList<T...>, tableIndex>,
    bool>
Database<T...>::checkType() const {
    using Type = Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>;

    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);

    auto const tablesCount = Conversions::TypeList<T...>::length;
    // BUG reverse table order
    auto table = db.tables().at(tablesCount - tableIndex - 1u);

    auto record = db.record(table);
    const auto structSize = StructConversions::StructExtractor<Type>::size;//to_tuple_size<Type>::value;
    if (structSize != record.count()) {
        qDebug() << db.databaseName()
            << "wrong table" << tableIndex << "columns count"
            << structSize
            << record.count();
        return false;
    }

    bool checkCorrect = true;
    auto columnNames = DatabaseDetail::columnNames<Type>();
    QStringList fieldNames;
    for (auto i = 0UL; i < structSize; ++i) {
        fieldNames << record.fieldName(i);
    }

    for (auto i = 0UL; i < structSize; ++i) {
        if (columnNames.at(i) != record.fieldName(i)) {
            qDebug() << db.databaseName()
                << "wrong column" << i << "name;"
                << "must be" << columnNames.at(i)
                << "but is" << record.fieldName(i);
            qDebug() << fieldNames;
            qDebug() << columnNames;
            checkCorrect = false;
            break;
        }
    }
    if (!checkCorrect) {
        qDebug() << db.databaseName()
            << "wrong column names check";
    }
    return checkCorrect;
}

template <typename... T>
bool Database<T...>::vacuumDatabase() {
    auto connectionName = currentThreadConnectionName();
    auto db = QSqlDatabase::database(connectionName);
    QSqlQuery vacuumQuery(db);
    vacuumQuery.prepare("VACUUM");
    return vacuumQuery.exec();
}

template <typename... T>
template <size_t... Is>
bool Database<T...>::clearDatabaseImpl(std::index_sequence<Is...>) {
    auto results = std::make_tuple(clearTable<Is>()...);
    bool success = (std::get<Is>(results) & ...);
    return success;
}

template <typename... T>
QString Database<T...>::currentThreadConnectionName() const {
    quintptr pThr = quintptr(QThread::currentThread());
    QString connectionName =
            QString(DatabasePrivate::DB_NAME)
            + QString::number(qHash(_path))
            + QString::number(pThr, 16);
    if (!QSqlDatabase::contains(connectionName)) {
        QSqlDatabase::addDatabase(DatabasePrivate::DB_TYPE, connectionName)
                .setDatabaseName(_path);
        _connectionsMutex.lock();
        _connections.append(connectionName);
        _connectionsMutex.unlock();
    }
    return connectionName;
}

#include "DebugOutput/undefdebug.h"

#endif // DATABASE_H

