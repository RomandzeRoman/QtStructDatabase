#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <functional>

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>

class WorkerThread : public QThread
{
public:
    WorkerThread(QObject* parent = nullptr);
    ~WorkerThread();

    void work(std::function<void()> task);

protected:
    void run() override;

private:
    //void

private:
    QQueue<std::function<void()>> _tasks;
    QMutex _mutex;
    QWaitCondition _condition;

    bool _needToAbort;

};

#endif // WORKERTHREAD_H
