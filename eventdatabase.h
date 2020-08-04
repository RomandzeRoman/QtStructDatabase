#ifndef EVENTDATABASE_H
#define EVENTDATABASE_H

#include <QDateTime>
#include <QThread>

#include "QtStructDatabase/private/eventdatabaseprivate.h"
#include "QtStructDatabase/private/databasetableviewmodel.h"

#include "private/asyncdatabase.h"
#include "private/eventdatabasedetail.h"
#include "eventdatabaserecord.h"

#include "QtDebugPrint/debugoutput_disabled.h"

template <typename T>
class EventDatabase : public EventDatabasePrivate
{

public:
    explicit EventDatabase(
            const QString& databasePath,
            QStringList roles,
            DatabaseViewModelDetail::Direction activationsListDirection,
            DatabaseViewModelDetail::Direction dataListDirection,
            QObject* parent = nullptr
            );
    explicit EventDatabase(
            const QString& databasePath,
            QStringList roles,
            QObject* parent = nullptr
            );
    ~EventDatabase();

    /**
     * @brief addRecord
     * This method is thread-safe
     * @param record
     */
    void addRecord(const T& record);

    /**
     * @brief isPreviousShutdownCorrect
     * This method is NOT thread-safe
     * @return
     */
    bool isPreviousShutdownCorrect() const;

private:
    void initDatabases(
            QStringList roles,
            DatabaseViewModelDetail::Direction activationsListDirection,
            DatabaseViewModelDetail::Direction dataListDirection);
    void addActivationRecord();
    void updateActivationRecord(bool shutDownCorrect = false);
    void setActivationIndex(uint activationIndex);
    void checkIfPreviousActivationCorrect();

private:
    AsyncDatabase<
            EventDatabaseDetail::Activation,
            EventDatabaseRecord<T> >
            _database;
    qint64 _activationId;
    DatabaseTableViewModel<
            Database<
                EventDatabaseDetail::Activation,
                EventDatabaseRecord<T>>,
            0u,
            Filter<EventDatabaseDetail::Activation, FilterDetail::Blank>
            >* _activationModel;
    DatabaseTableViewModel<
            Database<
                EventDatabaseDetail::Activation,
                EventDatabaseRecord<T>>,
            1u,
            Filter<EventDatabaseRecord<T>, FilterDetail::ActivationId>
            >* _dataModel;
    bool _previousShutdownCorrect;
};

/* ******************************************************************
 * Public
 * ******************************************************************
 */

template <typename T>
EventDatabase<T>::EventDatabase(
        const QString& databasePath,
        QStringList roles,
        DatabaseViewModelDetail::Direction activationsListDirection,
        DatabaseViewModelDetail::Direction dataListDirection,
        QObject* parent
        )
    : EventDatabasePrivate(
          nullptr,
          nullptr,
          [this](int index){setActivationIndex(index);},
          parent
          )
    , _database(databasePath)
    , _activationId(QDateTime::currentDateTime().toMSecsSinceEpoch())
    , _activationModel{ nullptr }
    , _dataModel{ nullptr }
    , _previousShutdownCorrect{ false }
{
    initDatabases(roles, activationsListDirection, dataListDirection);
    checkIfPreviousActivationCorrect();

    addActivationRecord();
}


template <typename T>
EventDatabase<T>::EventDatabase(
        const QString& databasePath,
        QStringList roles,
        QObject* parent
        )
    : EventDatabase(
          databasePath,
          roles,
          DatabaseViewModelDetail::Direction::Reversed,
          DatabaseViewModelDetail::Direction::NonReversed,
          parent)
{}

template<typename T>
EventDatabase<T>::~EventDatabase() {
//    dbg << "updating activation record:";
    updateActivationRecord(true);
}

template <typename T>
void EventDatabase<T>::addRecord(const T& record) {
    Q_ASSERT(_activationModel != nullptr);
    EventDatabaseRecord<T> r(record, _activationId, QDateTime::currentDateTime());
    _dataModel->addRecord(r);
    if (canUpdateActivationRecord()) {
        denyUpdateActivationRecord();
        updateActivationRecord();
    }
}

template <typename T>
bool EventDatabase<T>::isPreviousShutdownCorrect() const {
    return _previousShutdownCorrect;
}

/* ******************************************************************
 * Private
 * ******************************************************************
 */

template<typename T>
void EventDatabase<T>::initDatabases(
        QStringList roles,
        DatabaseViewModelDetail::Direction activationsListDirection,
        DatabaseViewModelDetail::Direction dataListDirection)
{
    _activationModel = new DatabaseTableViewModel<
            Database<EventDatabaseDetail::Activation, EventDatabaseRecord<T>>, 0u,
            Filter<EventDatabaseDetail::Activation, FilterDetail::Blank> >(
                &_database,
                {"enableTime", "disableTime", "activationId", "previousShutdownCorrect"},
                activationsListDirection,
                this
                );
    setActivations(_activationModel);
    QStringList dataRoles = {"activationId", "time"};
    dataRoles.append(roles);
    _dataModel = new DatabaseTableViewModel<
            Database<EventDatabaseDetail::Activation, EventDatabaseRecord<T>>, 1u,
            Filter<EventDatabaseRecord<T>, FilterDetail::ActivationId> >(
                &_database,
                dataRoles,
                dataListDirection,
                this
                );
    setData(_dataModel);
}

template<typename T>
void EventDatabase<T>::addActivationRecord() {
    // TODO check if previous activation disabled correctly
    // - deactivation time non zero
    EventDatabaseDetail::Activation activationRecord {
        QDateTime::fromMSecsSinceEpoch(_activationId),
        QDateTime::fromMSecsSinceEpoch(_activationId),
        _activationId,
        false
    };
    Q_ASSERT(_activationModel != nullptr);
    _activationModel->addRecord(activationRecord);

    Filter<EventDatabaseRecord<T>, FilterDetail::ActivationId> dataFilter;
    dbg << "set filter by id:" << _activationId;
    dataFilter.equal().value({_activationId});
    //_dataModel->setFilter(dataFilter);
}

template<typename T>
void EventDatabase<T>::updateActivationRecord(bool shutDownCorrect) {
    if (thread() != QThread::currentThread()) {
        auto guiTask = [this, shutDownCorrect]() {
            updateActivationRecord(shutDownCorrect);
        };
        addGuiTask(guiTask);
        return;
    }
    if (_activationModel->rowCount() == 0) {
        dbg << "Activation model is empty, can update activation record";
        return;
    }
    auto record = _activationModel->recordAt(0);
    record.disableTime = QDateTime::currentDateTime();
    record.shutdownCorrect = shutDownCorrect; // false by default
    _activationModel->updateRecord(record);// TODO update async
}

template<typename T>
void EventDatabase<T>::setActivationIndex(uint activationIndex) {
    EventDatabaseDetail::Activation record = _activationModel->recordAt(activationIndex);
//    qDebug() << "set filter by id:" << record.id;
    Filter<EventDatabaseRecord<T>, FilterDetail::ActivationId> dataFilter;
    dataFilter.equal().value({record.id});
    _dataModel->setFilter(dataFilter);
}

template<typename T>
void EventDatabase<T>::checkIfPreviousActivationCorrect() {
    int activationsCount = _activationModel->rowCount(QModelIndex());
    if (activationsCount == 0) {
        _previousShutdownCorrect = true;
        return;
    }
    auto lastActivation = _activationModel->recordAt(0);
    _previousShutdownCorrect = lastActivation.shutdownCorrect;
}

#include "QtDebugPrint/undefdebug.h"

#endif // EVENTDATABASE_H
