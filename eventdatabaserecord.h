#ifndef EVENTDATABASERECORD_H
#define EVENTDATABASERECORD_H

#include <tuple>
#include <utility>
#include <type_traits>
#include <qglobal.h>
#include <QDateTime>

#include "QtTupleConversions/structconversions.h"

template <typename T>
struct EventDatabaseRecord : public T {
    EventDatabaseRecord(T t, qint64 id, QDateTime time)
        : T(t)
        , id{id}
        , time{time}
    {}
    template <typename... Args>
    EventDatabaseRecord(qint64 id, QDateTime time, Args ...args)
        : T{args...}
        , id{id}
        , time{time}
    {}
    EventDatabaseRecord()
        : T()
        , id{0}
        , time()
    {}

    qint64 id; // TODO rename to activationId
    QDateTime time;
};

namespace std {
template <typename T>
struct tuple_size<EventDatabaseRecord<T>>
        : integral_constant<size_t, ::StructConversions::Detail::to_tuple_size<T>::value+2 >{};

template <typename T>
struct tuple_element<0, EventDatabaseRecord<T>> {
    using type = qint64;
};
template < typename T>
struct tuple_element<1, EventDatabaseRecord<T>> {
    using type = QDateTime;
};
template <size_t I, typename T>
struct tuple_element<I, EventDatabaseRecord<T>> {
    using tupleType = typename ::StructConversions::StructExtractor<T>::TupleType;
    using type = tuple_element_t<I-2, tupleType>;
};
} // namespace std

template <size_t I, typename T>
auto get(EventDatabaseRecord<T> cs) {
    if constexpr (I == 0) {
        return cs.id; // TODO rename to activationId
    } else if constexpr (I == 1) {
        return cs.time;
    }
    else {
        const T& t = cs;
        auto tu = TupleConversions::makeTuple(t);
        return std::get<I-2>(tu);
    }
}

template <typename T>
struct IsEventDatabaseRecord : std::false_type {};
template <typename T>
struct IsEventDatabaseRecord<EventDatabaseRecord<T>> : std::true_type {};

namespace StructConversions::Detail
{

template<typename T>
struct to_tuple_size<::EventDatabaseRecord<T>> : std::integral_constant<
        std::size_t, 2+to_tuple_size<T>::value
        >{};

} //namespace StructConversions::Detail

#endif // EVENTDATABASERECORD_H
