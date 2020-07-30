#ifndef FILTER_H
#define FILTER_H

#include <type_traits>
#include <utility>

#include <QString>

#include "TupleConversions/structconversions.h"
#include "TupleConversions/conversions.h"
#include "TupleConversions/typelist.h"
#include "database_detail.h"
#include "eventdatabaserecord.h"

namespace FilterDetail {
struct RowId {
    size_t id;
};
struct Blank {};

struct ActivationId {
    qint64 id;
};
template <size_t i>
struct Column {
    static const size_t index = i;
};

template <typename T>
struct IsColumn : std::false_type {};
template <std::size_t N>
struct IsColumn<Column<N>> : std::true_type {};

enum class ComparisonType {
    None,
    Equal,
    Greater,
    Less,
    GreaterOrEqual,
    LessOrEqual
};

template <typename T>
constexpr bool compare(T l, T r, ComparisonType type) {
    switch (type) {
    case FilterDetail::ComparisonType::Equal:
        return l == r;
    case FilterDetail::ComparisonType::Greater:
        return l > r;
    case FilterDetail::ComparisonType::Less:
        return l < r;
    case FilterDetail::ComparisonType::GreaterOrEqual:
        return l >= r;
    case FilterDetail::ComparisonType::LessOrEqual:
        return l <= r;
    case FilterDetail::ComparisonType::None:
        return true;
    }
}

} // namespace FilterDetail

template <typename Struct, typename Type>
class Filter
{
public:
    using List = typename StructConversions::StructExtractor<Struct>::ArgumentsList;

    explicit constexpr Filter();

    constexpr QString query() const;


    constexpr Filter<Struct, Type>& value(Type t);

    bool operator ==(const Filter<Struct, Type>& other) {
        return query() == other.query();
    }
    bool operator !=(const Filter<Struct, Type>& other) {
        return !operator ==(other);
    }

    constexpr Filter<Struct, Type>& equal();
    constexpr Filter<Struct, Type>& greater();
    constexpr Filter<Struct, Type>& less();
    constexpr Filter<Struct, Type>& greaterOrEqual();
    constexpr Filter<Struct, Type>& lessOrEqual();

    constexpr bool tryPass(Type t);
    constexpr bool tryPass(Struct s);

private:
    QString _query;
    FilterDetail::ComparisonType _comparisonType;
    Type _value;
    bool _valueSet;
    //const size_t _argIndex;
};


template <typename Struct, typename Type>
constexpr Filter<Struct, Type>::Filter()
    : _query()
    , _comparisonType{ FilterDetail::ComparisonType::None }
    , _valueSet{ false }
    //, _argIndex{Conversions::ListIndexV<Type, List>}
{
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        _query += " WHERE ";
        if constexpr (std::is_same<Type, FilterDetail::RowId>{}) {
            _query += "_rowid_";
        }
        else if constexpr (FilterDetail::IsColumn<Type>{}) {
            _query += DatabaseDetail::columnName<
                    Struct,
                    Type::index
                    >();
        }
        else if constexpr (std::is_same<Type, FilterDetail::ActivationId>{}) {
            using EventRecord = EventDatabaseRecord<FilterDetail::ActivationId>;
            _query += DatabaseDetail::columnName<
                    EventRecord,
                    0
                    >();
        }
        else if constexpr (std::is_same<Type, qint64>{}) {
            using List = typename StructConversions::StructExtractor<Struct>::ArgumentsList::Tail;
    //        constexpr bool hasType = Conversions::TypeChecker<
    //            Type, List
    //            >::hasType;
    //        static_assert(hasType, "Struct dont have type");
            constexpr auto index = Conversions::ListIndex<
                    Type, List
                    >::value;
            _query += DatabaseDetail::columnName<
                    Struct,
                    size_t(index+1)
                    >();
        }
        else {
            using List = typename StructConversions::StructExtractor<Struct>::ArgumentsList;
    //        constexpr bool hasType = Conversions::TypeChecker<
    //            Type, List
    //            >::hasType;
    //        static_assert(hasType, "Struct dont have type");
            constexpr auto index = Conversions::ListIndex<
                    Type, List
                    >::value;
            _query += DatabaseDetail::columnName<
                    Struct,
                    size_t(index)
                    >();
        }
    }
}

template <typename Struct, typename Type>
constexpr QString Filter<Struct, Type>::query() const {
    if (_comparisonType == FilterDetail::ComparisonType::None || !_valueSet) {
        return "";
    }
    return _query; // TODO generate query here instead of storing it
}

template<typename Struct, typename Type>
constexpr Filter<Struct, Type>& Filter<Struct, Type>::value(Type t) {
    if (_comparisonType == FilterDetail::ComparisonType::None || _valueSet) {
        return *this;
    }
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        _value = t;
        if constexpr (std::is_same<Type, FilterDetail::RowId>{}) {
            _query += QString::number(t.id);
        }
        else if constexpr (FilterDetail::IsColumn<Type>{}) {
            // TODO
        }
        else if constexpr (std::is_same<Type, FilterDetail::ActivationId>{}) {
            _query += QString::number(t.id);
        }
        else {
            _query += Conversions::toStoredDataValueString(t);
        }
        _valueSet = true;
    }
    return *this;
}

template<typename Struct, typename Type>
constexpr Filter<Struct, Type>& Filter<Struct, Type>::equal() {
    if (_valueSet) {
        return *this;
    }
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        _query += " = ";
        _comparisonType = FilterDetail::ComparisonType::Equal;
    }
    return *this;
}

template<typename Struct, typename Type>
constexpr Filter<Struct, Type>& Filter<Struct, Type>::greater() {
    if (_valueSet || _comparisonType != FilterDetail::ComparisonType::None) {
        return *this;
    }
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        _query += " > ";
        _comparisonType = FilterDetail::ComparisonType::Greater;
    }
    return *this;
}

template<typename Struct, typename Type>
constexpr Filter<Struct, Type>& Filter<Struct, Type>::less() {
    if (_valueSet || _comparisonType != FilterDetail::ComparisonType::None) {
        return *this;
    }
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        _query += " < ";
        _comparisonType = FilterDetail::ComparisonType::Less;
    }
    return *this;
}

template<typename Struct, typename Type>
constexpr Filter<Struct, Type>& Filter<Struct, Type>::greaterOrEqual() {
    if (_valueSet || _comparisonType != FilterDetail::ComparisonType::None) {
        return *this;
    }
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        _query += " >= ";
        _comparisonType = FilterDetail::ComparisonType::GreaterOrEqual;
    }
    return *this;
}

template<typename Struct, typename Type>
constexpr Filter<Struct, Type>& Filter<Struct, Type>::lessOrEqual() {
    if (_valueSet || _comparisonType != FilterDetail::ComparisonType::None) {
        return *this;
    }
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        _query += " <= ";
        _comparisonType = FilterDetail::ComparisonType::LessOrEqual;
    }
    return *this;
}


template<typename Struct, typename Type>
constexpr bool Filter<Struct, Type>::tryPass(Type t) {
    static_assert(!FilterDetail::IsColumn<Type>{});
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        if (!_valueSet || _comparisonType == FilterDetail::ComparisonType::None) {
            return false;
        }
        if constexpr (std::is_same<Type, FilterDetail::RowId>{}) {
            switch (_comparisonType) {
            case FilterDetail::ComparisonType::Equal:
                return t.id == _value.id;
            case FilterDetail::ComparisonType::Greater:
                return t.id > _value.id;
            case FilterDetail::ComparisonType::Less:
                return t.id < _value.id;
            case FilterDetail::ComparisonType::GreaterOrEqual:
                return t.id >= _value.id;
            case FilterDetail::ComparisonType::LessOrEqual:
                return t.id <= _value.id;
            }
        }
        else if constexpr (std::is_same<Type, FilterDetail::ActivationId>{}) {
            switch (_comparisonType) {
            case FilterDetail::ComparisonType::Equal:
                return t.id == _value.id;
            case FilterDetail::ComparisonType::Greater:
                return t.id > _value.id;
            case FilterDetail::ComparisonType::Less:
                return t.id < _value.id;
            case FilterDetail::ComparisonType::GreaterOrEqual:
                return t.id >= _value.id;
            case FilterDetail::ComparisonType::LessOrEqual:
                return t.id <= _value.id;
            }
        }
        else {
            switch (_comparisonType) {
            case FilterDetail::ComparisonType::Equal:
                return t == _value;
            case FilterDetail::ComparisonType::Greater:
                return t > _value;
            case FilterDetail::ComparisonType::Less:
                return t < _value;
            case FilterDetail::ComparisonType::GreaterOrEqual:
                return t >= _value;
            case FilterDetail::ComparisonType::LessOrEqual:
                return t <= _value;
            }
        }
//        _valueSet = true;
        return false;
    }
    return true;

}

template<typename Struct, typename Type>
constexpr bool Filter<Struct, Type>::tryPass(Struct s) {
    static_assert(!std::is_same<Type, FilterDetail::RowId>{});
    static_assert(
                std::disjunction_v<
                    std::negation<std::is_same<Type, FilterDetail::ActivationId>>,
                    IsEventDatabaseRecord<Struct>
                    >
                );
    if constexpr (!(std::is_same<Type, FilterDetail::Blank>{})) {
        if (!_valueSet || _comparisonType == FilterDetail::ComparisonType::None) {
            return false;
        }
        if constexpr (FilterDetail::IsColumn<Type>{}) {
            auto t = TupleConversions::makeTuple(s);
            auto index = Type::index;
            auto val = std::get<index>(t);

            switch (_comparisonType) {
            case FilterDetail::ComparisonType::Equal:
                return val == _value;
            case FilterDetail::ComparisonType::Greater:
                return val > _value;
            case FilterDetail::ComparisonType::Less:
                return val < _value;
            case FilterDetail::ComparisonType::GreaterOrEqual:
                return val >= _value;
            case FilterDetail::ComparisonType::LessOrEqual:
                return val <= _value;
            }
        }
        else if constexpr (std::is_same<Type, FilterDetail::ActivationId>{}) {
//            qDebug() << "check for" << s.id << "filter:" << _value.id;
            switch (_comparisonType) {
            case FilterDetail::ComparisonType::None: // FIXME delete this
            case FilterDetail::ComparisonType::Equal:
                return s.id == _value.id;
            case FilterDetail::ComparisonType::Greater:
                return s.id > _value.id;
            case FilterDetail::ComparisonType::Less:
                return s.id < _value.id;
            case FilterDetail::ComparisonType::GreaterOrEqual:
                return s.id >= _value.id;
            case FilterDetail::ComparisonType::LessOrEqual:
                return s.id <= _value.id;
            }
        }
        else {
            auto t = TupleConversions::makeTuple(s);
            using L = typename StructConversions::StructExtractor<Struct>::ArgumentsList;
            auto index = Conversions::ListIndex<Type, L>::value;
            auto val = std::get<index>(t);

            switch (_comparisonType) {
            case FilterDetail::ComparisonType::Equal:
                return val == _value;
            case FilterDetail::ComparisonType::Greater:
                return val > _value;
            case FilterDetail::ComparisonType::Less:
                return val < _value;
            case FilterDetail::ComparisonType::GreaterOrEqual:
                return val >= _value;
            case FilterDetail::ComparisonType::LessOrEqual:
                return val <= _value;
            }
        }
//        _valueSet = true;
        return false;
    }
    return true;

}

#endif // FILTER_H

