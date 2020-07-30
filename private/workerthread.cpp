#include "workerthread.h"

#include <QMutexLocker>
#include "DebugOutput/debugoutput.h"

/* ******************************************************************
 * Public
 * ******************************************************************
 */

WorkerThread::WorkerThread(QObject* parent)
    : QThread(parent)
    , _tasks()
    , _mutex()
    , _condition()
    , _needToAbort{false}
{

}

WorkerThread::~WorkerThread() {
//    dbg << "destruct worker thread";
    _mutex.lock();
    _needToAbort = true;
//    dbg << "wakeup thread in destructor for abort";
    _condition.wakeOne();
    _mutex.unlock();
//    dbg << "wait for destruct";
    wait();
}

void WorkerThread::work(std::function<void()> task) {
    QMutexLocker l(&_mutex);
    Q_UNUSED(l);
//    dbg << "add task";
    _tasks.enqueue(task);

    if (!isRunning()) {
//        dbg << "start thread";
        start();
    }
    else {
//        dbg << "wakeup thread";
        _condition.wakeOne();
    }
}

/* ******************************************************************
 * Protected override
 * ******************************************************************
 */

void WorkerThread::run() {
//    dbg << "run thread";
    for(;;) {
        _mutex.lock();
        if (_needToAbort) {
            _mutex.unlock();
//            dbg << "exit from run";
            return;
        }
        auto currentTask = _tasks.dequeue();
        _mutex.unlock();

        currentTask();

        _mutex.lock();
        if (_tasks.isEmpty() && !_needToAbort) {
//            dbg << "no tasks, thread going to sleep";
            _condition.wait(&_mutex);
        }
        _mutex.unlock();
    }

}
