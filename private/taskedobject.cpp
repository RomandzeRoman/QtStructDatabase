#include "taskedobject.h"

#include <QCoreApplication>

TaskedObject::TaskedObject(QObject *parent)
    : QObject(parent)
    , _tasks()
    , _mutex()
{
    // object MUST be created in gui thread
    Q_ASSERT(thread()== QCoreApplication::instance()->thread());
    // For processing tasks in gui thread
    connect(this, &TaskedObject::needProcessGuiTasks,
            this, &TaskedObject::processGuiTasks,
            Qt::ConnectionType::QueuedConnection);
}

void TaskedObject::addGuiTask(std::function<void ()> task) {
    _mutex.lock();
    _tasks.enqueue(task);
    _mutex.unlock();
    emit needProcessGuiTasks();
}

void TaskedObject::processGuiTasks() {
    _mutex.lock();
    while (!_tasks.isEmpty()) {
        auto task = _tasks.dequeue();
        _mutex.unlock();
        task();
        _mutex.lock();
    }
    _mutex.unlock();
}
