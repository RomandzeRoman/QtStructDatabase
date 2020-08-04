#include "eventdatabaseprivate.h"
#include <QReadLocker>
#include <QWriteLocker>
#include <QThread>

/********************************************************************
 * Public
 ********************************************************************
 */

EventDatabasePrivate::EventDatabasePrivate(
        QAbstractListModel* activations,
        QAbstractListModel* data,
        std::function<void(uint activationIndex)> activationChangeCallback,
        QObject* parent)
    : TaskedObject(parent)
    , _activations{ activations }
    , _data{ data }
    , _activationChangeCallback{ activationChangeCallback }
    , _canUpdateActivationRecordTimer{ new QTimer(this) }
    , _currentActivation{ 0u }
    , _canUpdateActivationRecord{ false }
{
    _canUpdateActivationRecordTimer->setSingleShot(true);
    _canUpdateActivationRecordTimer->setInterval(60000);
    connect(_canUpdateActivationRecordTimer,
            &QTimer::timeout,
            this,
            &EventDatabasePrivate::processCanUpdateActivation);
    _canUpdateActivationRecordTimer->start();
    connect(this,
            &EventDatabasePrivate::needDenyActivationUpdate,
            _canUpdateActivationRecordTimer,
            static_cast<void (QTimer::*)()>(&QTimer::start),
            Qt::QueuedConnection);
}

QAbstractListModel* EventDatabasePrivate::activations() {
    Q_ASSERT(_activations != nullptr);
    return _activations;
}

QAbstractListModel* EventDatabasePrivate::data() {
    Q_ASSERT(_data != nullptr);
    return _data;
}

uint EventDatabasePrivate::currentActivation() const {
    return _currentActivation;
}

void EventDatabasePrivate::selectActivation(uint activationIndex) {
    setCurrentActivation(activationIndex);
    _activationChangeCallback(activationIndex);
}

/********************************************************************
 * Protected
 ********************************************************************
 */

void EventDatabasePrivate::setActivations(QAbstractListModel* activations) {
    Q_ASSERT(activations != nullptr);
    _activations = activations;
}

void EventDatabasePrivate::setData(QAbstractListModel* data) {
    Q_ASSERT(data != nullptr);
    _data = data;
}

bool EventDatabasePrivate::canUpdateActivationRecord() const {
    QReadLocker l(&_canUpdateLock);
    return _canUpdateActivationRecord;
}

void EventDatabasePrivate::denyUpdateActivationRecord() {
    _canUpdateLock.lockForWrite();
    _canUpdateActivationRecord = false;
    _canUpdateLock.unlock();
    if (QThread::currentThread() != thread()) {
        emit needDenyActivationUpdate();
        return;
    }
    _canUpdateActivationRecordTimer->start();
}

/*******************************************************************
 * Private slots
 ********************************************************************
 */

void EventDatabasePrivate::processCanUpdateActivation() {
    QWriteLocker l(&_canUpdateLock);
    _canUpdateActivationRecord = true;
}

void EventDatabasePrivate::setCurrentActivation(uint newActivation) {
    if (_currentActivation != newActivation) {
        _currentActivation = newActivation;
        emit currentActivationChanged(newActivation);
    }
}
