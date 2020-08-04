#ifndef DATABASE_DETAIL_H
#define DATABASE_DETAIL_H

/**
 * @file
 * @brief Detail namespace database functions
 */

#include <tuple>
#include <type_traits>

#include <QString>
#include <QDateTime>
#include <QVariant>

#include <QSqlQuery>
#include <QSqlRecord>

#include "QtTupleConversions/conversions.h"
#include "QtTupleConversions/structconversions.h"
#include "QtTupleConversions/typelist.h"

namespace DatabaseDetail {

template <typename T>
/**
 * @brief SQL type of T
 * @return String that corresponds type T
 */
constexpr QString sqlTypeName() {
    if constexpr (std::is_same<T, int>{}) {
        return "INTEGER";
    }
    else if constexpr (std::is_same<T, double>{}) {
        return "REAL";
    }
    else if constexpr (std::is_same<T, qint64>{}) {
        return "INTEGER";
    }
    else if constexpr (std::is_same<T, bool>{}) {
        return "BOOLEAN";
    }
    else if constexpr (std::is_same<T, QString>{}) {
        return "STRING";
    }
    else if constexpr (std::is_same<T, QDateTime>{}) {
        return "INTEGER";
    }
    else {
        // TODO static_assert
        return "Unknown";
    }
}

template <size_t tableIndex>
inline QString tableName() {
    return QString("table") + QString::number(tableIndex);
}

template <typename T, typename U>
struct Selector;

template <typename Tuple, size_t... Is>
struct Selector<Tuple, std::index_sequence<Is...>> {
    // remove references from tuple type
    using TupleType = typename std::decay<Tuple>::type;

    static constexpr QString createTableArgsQuery() {
        QString res;        
        ((
        res += ((Is != 0) ? ", _" : "_"),
        res += QString::number(Is),
        res += Conversions::typeName<
                     typename std::tuple_element<
                         Is,
                         TupleType
                         >::type
                      >(),
        res += " ",
        res += sqlTypeName<
                typename std::tuple_element<
                    Is,
                    TupleType
                    >::type
                 >()
        ),...);
        return res;
    }

    static constexpr QStringList tableColumnNames() {
        QStringList list;
        ((
        list.append(
                QString('_')
                + QString::number(Is)
                + Conversions::typeName<
                     typename std::tuple_element<
                         Is,
                         TupleType
                         >::type
                      >())
        ),...);
        return list;
    }

    static constexpr QStringList tableColumnTypes() {
        QStringList list;
        ((
        list.append(sqlTypeName<
                typename std::tuple_element<
                    Is,
                    TupleType
                    >::type
                >()
                )
        ),...);
        return list;
    }
};

// FIXME add rvalue reference "&&" to Tuple type
//template <typename Tuple, size_t...Is>
//QVariantList tupleToStoredVariantList(Tuple/*&&*/ t, std::index_sequence<Is...>) {
//    QVariantList vl;
//    ((
//    vl.append(QVariant(Conversions::toStoredDataValue(std::get<Is>(t))))
//    ), ...);
//    return vl;
//}

//template <typename Data>
//static constexpr QVariantList structToStoredVariantList(const Data& d) {
//    auto t = TupleConversions::makeTuple(d);
//    return DatabaseDetail::tupleToStoredVariantList(
//                t,
//                std::make_index_sequence<
//                    StructConversions::StructExtractor<Data>::size
//                    >{}
//                );
//}

template <typename Tuple/*... Args*/>
struct ArgsSelector;

template <typename... Args>
struct ArgsSelector<std::tuple<Args...>> {

    using TupleType = std::tuple<Args...>;
    using IndexSequence = typename std::index_sequence_for<Args...>;

    static constexpr QString createTableIndexQuery() {
        return Selector<
                TupleType,
                IndexSequence
        >::createTableArgsQuery();
    }

    static constexpr QStringList tableColumnNames() {
        return Selector<
                TupleType,
                IndexSequence
        >::tableColumnNames();
    }

    static constexpr QStringList tableColumnTypes() {
        return Selector<
                TupleType,
                IndexSequence
        >::tableColumnTypes();
    }

};

//template <typename T, typename... Args>
//T returnType(T(*)(Args...));

// FIXME try to not to create object
//template <typename T,
//          /*typename = std::enable_if_t<
//              !std::is_same<JsonObject, T>{}
//              >*/
//          typename = std::enable_if_t<
//              !TupleConversions::IsTuple<T>::value
//              >
//          >
//constexpr QString createTableQuery() {
//    //T object;
//    using StructConversions::TupleMaker;
//    //using FunType = std::invoke_result<TupleMaker(T)>::type;
//    //using Tuple = decltype(returnType<FunType, T>(&StructConversions::makeTuple));
//    using Tuple = decltype(declval<TupleMaker<T>>().makeTuple(T));
//    //using Tuple = decltype(TupleMaker<T>().makeTuple(T));
//    return ArgsSelector<Tuple>::createTableIndexQuery();
//}






template <typename T>
constexpr QStringList columnNames() {
    using TupleType = typename StructConversions::StructExtractor<T>::TupleType;
    return ArgsSelector<TupleType>::tableColumnNames();
}

template <typename T, size_t index>
constexpr QString columnName() {
    //T object;
    //auto t = StructConversions::makeTuple(object);
    //using TupleType = std::decay_t<decltype(t)>;
    //using TupleType = typename StructExtractor<T>::TupleType;
    //return ArgsSelector<TupleType>::tableColumnNames().at(index);
    return columnNames<T>().at(index);
}

template <typename T, size_t index>
constexpr QString columnType() {
    using TupleType = typename StructConversions::StructExtractor<T>::TupleType;
    return ArgsSelector<TupleType>::tableColumnTypes().at(index);
}

template <typename T>
constexpr QString createTableQueryPart() {
    using TupleType = typename StructConversions::StructExtractor<T>::TupleType;
    const size_t size = StructConversions::StructExtractor<T>::size;//std::tuple_size_v<TupleType>;

    auto names = ArgsSelector<TupleType>::tableColumnNames();
    auto types = ArgsSelector<TupleType>::tableColumnTypes();

    QString res;
    for (auto i = 0UL; i < size; ++i) {
        res += (i != 0 ? ", " : "");
        res += names.at(i);
        res += " ";
        res += types.at(i);
    }
    return res;
}

/**
 * @brief createTableQuery<T, tableindex>
 * @param T - Type stored in table
 * @param table index - index of table in database
 * @return Query string to create table which stores
 * T instances into [index] table
 */
template <typename T, size_t tableIndex>
constexpr QString createTableQuery() {
    QString res = "CREATE TABLE ";
    res += DatabaseDetail::tableName<tableIndex>();
    res += "(";
    res += DatabaseDetail::createTableQueryPart<T>();
    res += ");";
    return res;
}

/**
 * @brief addRecordQuery<T, tableindex>
 * @param T - Type stored in table
 * @param table index - index of table in database
 * @return Query string to add T instance into [index] table
 */
template <typename T, size_t tableIndex>
constexpr QString addRecordQuery() {
    QString res = "INSERT INTO ";
    res += DatabaseDetail::tableName<tableIndex>();
    auto namesList = DatabaseDetail::columnNames<T>();
    res += "(";
    res += namesList.join(",");
    res += ")";
    res += " VALUES(";
    for (auto i = 0; i < namesList.size(); ++i) {
        res += (i != 0 ? ",?" : "?");
    }
    res += ")";
    return res;
}

/**
 * @brief addRecordQuery<T, tableindex>
 * @param T - Type stored in table
 * @param table index - index of table in database
 * @return Query string to add T instance into [index] table
 */
template <typename T, size_t tableIndex>
constexpr QString updateRecordQuery(size_t rowId) {
    QString res = "UPDATE ";
    res += DatabaseDetail::tableName<tableIndex>();
    res += " SET ";
    auto namesList = DatabaseDetail::columnNames<T>();
    res += namesList.join("=?,");
    res += "=? WHERE _rowid_=";
    res += QString::number(rowId);
    return res;
}

template <typename Tuple, typename Sequence>
struct QueryFillerFromTuple;

template<typename Tuple, size_t... Is>
struct QueryFillerFromTuple<Tuple, std::index_sequence<Is...>> {
    static void populateSqlQuery(QSqlQuery& query, const Tuple& tuple) {
        ((
        query.addBindValue(Conversions::toStoredDataValue(std::get<Is>(tuple)))
        ), ...);
    }
};

// TODO make constexpr
template <typename T>
/*constexpr*/ void populateSqlQuery(QSqlQuery& query, const T& val) {
    auto t = TupleConversions::makeTuple(val);
    const size_t size = std::tuple_size_v<decltype(t)>;

    DatabaseDetail::QueryFillerFromTuple<
            decltype(t), std::make_index_sequence<size>
            >::populateSqlQuery(query, t);
}



//struct QueryFiller {

//}




template <typename Type, typename Sequence>
struct RecordExtractor;

template <typename Type, size_t... Is>
struct RecordExtractor<Type, std::index_sequence<Is...>> {
    static auto extract(const QSqlRecord& record) {
        auto t = std::make_tuple(extractField<Is>(record.value(Is)) ...);
        return t;
    }

private:

    template <size_t index>
    static auto extractField(const QVariant& var) {
        using TupleType = typename StructConversions::StructExtractor<Type>::TupleType;
        using FieldType = typename std::decay_t<std::tuple_element_t<index, TupleType>>;
        return Conversions::fromStoredVariant<FieldType>(var);
    }
};

template<typename T>
auto extractRecord(const QSqlRecord& record) {
    //using TupleType = typename StructConversions::StructExtractor<T>::TupleType;
    const size_t tupleSize = StructConversions::StructExtractor<T>::size;
    auto t = RecordExtractor<T, std::make_index_sequence<tupleSize>>::extract(record);
    static_assert(std::tuple_size_v<decltype(t)> == tupleSize, "not same size");
    return t;
}

// TODO Change without rowid
// TODO make constexpr
//constexpr
template <size_t tableIndex>
inline QString removeHalfRecordsQuery() {
    QString query;
    query += "DELETE FROM ";
    query += DatabaseDetail::tableName<tableIndex>();
    query += " WHERE _rowid_ <= "
             "(SELECT MAX(_rowid_) FROM ";
    query += DatabaseDetail::tableName<tableIndex>();
    query += ")/2";
    return query;
}

// TODO make constexpr
//constexpr
template <size_t tableIndex>
inline QString cleanQuery() {
    QString query;
    query += "DELETE FROM ";
    query += DatabaseDetail::tableName<tableIndex>();
    return query;
}

// TODO make constexpr
//constexpr
template <size_t tableIndex>
inline QString maxRowIdQuery() {
    QString queryString = "SELECT MAX(_rowid_) AS maxId FROM ";
    queryString += DatabaseDetail::tableName<tableIndex>();
    return queryString;
}

// TODO make constexpr
//constexpr
template <size_t tableIndex>
inline QString countQuery() {
    QString query = "SELECT count(*) AS maxId FROM ";
    query += DatabaseDetail::tableName<tableIndex>();
    return query;
}

// TODO make constexpr
//constexpr
template <size_t tableIndex>
inline QString readQuery(uint offset, uint count, QString filterQuery) {
    QString res = "SELECT _rowid_, * FROM ";
    res += DatabaseDetail::tableName<tableIndex>();
    res += filterQuery;
    if (count != 0u) {
        res += " LIMIT ";
        res += QString::number(count);
    }
    if (offset != 0u) {
        res += " OFFSET ";
        res += QString::number(offset);
    }
    return res;
}

template <typename List>
struct OneLength;

template <typename... T>
struct OneLength<Conversions::TypeList<T...>> {
    using List = Conversions::TypeList<T...>;
    static constexpr bool value = (List::length == 1u);
};

//template <typename List>
//inline constexpr bool OneLengthV;

template <typename List>
inline constexpr bool OneLengthV = OneLength<List>::value;

//template <typename... T>
//inline constexpr bool OneLengthV<> = OneLength<Conversions::TypeList<T...>>::value;

//template <typename T>
//struct OneLength {
//    static constexpr bool value = true;
//};

//template <typename...>
//struct OneLength {
//    static constexpr bool value = false;
//};


} // namespace DatabaseDetail

#endif // DATABASE_DETAIL_H
