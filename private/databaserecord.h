#ifndef DATABASERECORD_H
#define DATABASERECORD_H

#include <tuple>
#include <utility>
#include <type_traits>
#include <qglobal.h>

#include "../TupleConversions/structconversions.h"

template <typename T>
struct DatabaseRecord : public T {
    DatabaseRecord()
        : T()
        , rowId{0}
    {}
    DatabaseRecord(qint64 rowId, T t)
        : T(t)
        , rowId{rowId}
    {}
    template <typename... Args>
    DatabaseRecord(qint64 id, Args ...args)
        : T{args...}
        , rowId{id}
    {}

    bool operator ==(const DatabaseRecord<T>& other) {
        return rowId == other.rowId;
    }

    qint64 rowId;
};

template <typename T>
struct WriteToDatabaseResult {
    DatabaseRecord<T> record;
    uint recordsBeforeAdd;
    bool needAddToView;
};


namespace std {
template <typename T>
struct tuple_size<DatabaseRecord<T>>
        : integral_constant<size_t, ::StructConversions::Detail::to_tuple_size<T>::value+1 >{};

template < typename T>
struct tuple_element<0, DatabaseRecord<T>> {
    using type = qint64;
};
template <size_t I, typename T>
struct tuple_element<I, DatabaseRecord<T>> {
    using tupleType = typename ::StructConversions::StructExtractor<T>::TupleType;
    using type = tuple_element_t<I-1, tupleType>;
};
} // namespace std

template <size_t I, typename T>
auto get(DatabaseRecord<T> cs) {
    if constexpr (I == 0) {
        return cs.rowId;
    }
    else {
        const T& t = cs;
        auto tu = TupleConversions::makeTuple(t);
        return std::get<I-1>(tu);
    }
}


namespace StructConversions::Detail
{

template<typename T>
struct to_tuple_size<DatabaseRecord<T>> : std::integral_constant<
        std::size_t, 1+to_tuple_size<T>::value
        >{};

} //namespace StructConversions::Detail
#endif // DATABASERECORD_H
