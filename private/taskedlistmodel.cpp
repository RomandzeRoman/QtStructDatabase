#include "taskedlistmodel.h"

#include <QMutexLocker>
#include <QCoreApplication>

TaskedListModel::TaskedListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _tasks()
    , _mutex()
{
    // object MUST be created in gui thread
    Q_ASSERT(thread()== QCoreApplication::instance()->thread());
    // For processing tasks in gui thread
    connect(this, &TaskedListModel::needProcessGuiTasks,
            this, &TaskedListModel::processGuiTasks,
            Qt::ConnectionType::QueuedConnection);
}

void TaskedListModel::addGuiTask(std::function<void ()> task) {
    _mutex.lock();
    _tasks.enqueue(task);
    _mutex.unlock();
    emit needProcessGuiTasks();
}

void TaskedListModel::processGuiTasks() {
    _mutex.lock();
    while (!_tasks.isEmpty()) {
        auto task = _tasks.dequeue();
        _mutex.unlock();
        task();
        _mutex.lock();
    }
    _mutex.unlock();
}
