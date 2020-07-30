#ifndef EVENTDATABASEDETAIL_H
#define EVENTDATABASEDETAIL_H

#include <QDateTime>
#include <QDebug>

namespace EventDatabaseDetail {

struct Activation {
    QDateTime enableTime;
    QDateTime disableTime;
    qint64 id;
    bool shutdownCorrect;
};





//struct EventId {
//    qint64 activationId;
//    QDateTime time;
//};

} // namespace EventDatabaseDetail

inline QDebug operator<<(QDebug debug, const EventDatabaseDetail::Activation &a) {
  QDebugStateSaver saver(debug);
  debug.nospace() << "Activation(" << a.id << ')';

  return debug;
}

#endif // EVENTDATABASEDETAIL_H
