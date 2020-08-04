#ifndef EVENTDATABASEPRIVATE_H
#define EVENTDATABASEPRIVATE_H

#include <QAbstractListModel>
#include <QTimer>
#include <QReadWriteLock>

#include <functional>

#include "taskedobject.h"

class EventDatabasePrivate : public TaskedObject
{
    Q_OBJECT

    Q_PROPERTY(QAbstractListModel* activations
               READ activations
               CONSTANT)
    Q_PROPERTY(QAbstractListModel* data
               READ data
               CONSTANT)

    Q_PROPERTY(uint currentActivation READ currentActivation NOTIFY currentActivationChanged)

public:
    explicit EventDatabasePrivate(
            QAbstractListModel* activations,
            QAbstractListModel* data,
            std::function<void(uint activationIndex)> activationChangeCallback,
            QObject* parent = nullptr
            );

public slots:
    QAbstractListModel* activations();
    QAbstractListModel* data();
    uint currentActivation() const;
    void selectActivation(uint activationIndex);

signals:
    void needDenyActivationUpdate();
    void currentActivationChanged(uint activationIndex);

protected:
    void setActivations(QAbstractListModel* activations);
    void setData(QAbstractListModel* data);

    bool canUpdateActivationRecord() const;

protected slots:
    void denyUpdateActivationRecord();

private slots:
    void processCanUpdateActivation();
    void setCurrentActivation(uint newActivation);

private:
    QAbstractListModel* _activations;
    QAbstractListModel* _data;
    std::function<void(uint activationIndex)> _activationChangeCallback;
    QTimer* _canUpdateActivationRecordTimer;
    mutable QReadWriteLock _canUpdateLock;
    uint _currentActivation;
    bool _canUpdateActivationRecord;

};

#endif // EVENTDATABASEPRIVATE_H
