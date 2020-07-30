#ifndef TASKEDLISTMODEL_H
#define TASKEDLISTMODEL_H

#include <functional>

#include <QAbstractListModel>
#include <QQueue>
#include <QMutex>

class TaskedListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit TaskedListModel(QObject* parent = nullptr);

    void addGuiTask(std::function<void()> task);

public slots:
    void processGuiTasks();

signals:
    void needProcessGuiTasks();

private:
    QQueue<std::function<void()>> _tasks;
    QMutex _mutex;

};

#endif // TASKEDLISTMODEL_H
