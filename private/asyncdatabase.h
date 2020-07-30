#ifndef ASYNCDATABASE_H
#define ASYNCDATABASE_H

#include <QObject>
#include "../database.h"
#include "workerthread.h"

template <typename... T>
class AsyncDatabase : public QObject
{

public:
    explicit AsyncDatabase(
            const QString& databasePath,
            QObject* parent = nullptr);
    ~AsyncDatabase();

    Database<T...>* internalDatabase() const;
    WorkerThread* internalThread() const;

    template <size_t tableIndex, typename FilterType>
    void read(
            unsigned offset, unsigned count,
            std::function<void(QVector<DatabaseRecord<
            typename Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>
            >>)> cb,
            FilterType filter = FilterType());

    template <size_t tableIndex, typename FilterType>
    void numberOfRecords(
            std::function<void(uint)> cb,
            FilterType filter = FilterType());

private:
    Database<T...>* _database;
    WorkerThread* _thread;

};


/* ******************************************************************
 * Public
 * ******************************************************************
 */

template <typename... T>
AsyncDatabase<T...>::AsyncDatabase(
        const QString& databasePath,
        QObject* parent)
    : QObject(parent)
    , _database{ new Database<T...>(databasePath)}
    , _thread{ new WorkerThread() }
{
//    _database->move
}

template <typename... T>
AsyncDatabase<T...>::~AsyncDatabase() {
    delete _thread;
//    dbg << "thread deleted";
    delete _database;
}

template<typename... T>
Database<T...>* AsyncDatabase<T...>::internalDatabase() const {
    return _database;
}

template<typename... T>
WorkerThread* AsyncDatabase<T...>::internalThread() const {
    return _thread;
}

template <typename... T>
template <size_t tableIndex, typename FilterType>
void AsyncDatabase<T...>::read(
        unsigned offset, unsigned count,
        std::function<void(QVector<DatabaseRecord<
        typename Conversions::TypeAtT<Conversions::TypeList<T...>, tableIndex>
        >>)> cb,
        FilterType filter)
{
    auto task = [this, offset, count, filter, cb](){
        auto res = _database-> template read<tableIndex>(
                    offset, count, filter);
        cb(res);
    };
    _thread->work(task);
}

template <typename... T>
template <size_t tableIndex, typename FilterType>
void AsyncDatabase<T...>::numberOfRecords(
        std::function<void(uint)> cb,
        FilterType filter)
{
    auto task = [this,filter, cb](){
        uint res = _database->numberOfRecords<tableIndex>(filter);
//        uint res = _database->template numberOfRecords<tableIndex>(filter);
        cb(res);
    };
}

#endif // ASYNCDATABASE_H
