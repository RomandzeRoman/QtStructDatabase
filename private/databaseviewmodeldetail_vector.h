#ifndef DATABASEVIEWMODELDETAIL_VECTOR_H
#define DATABASEVIEWMODELDETAIL_VECTOR_H

#include <QVector>

namespace DatabaseViewModelDetail {

template <typename Data, uint reservedSize>
class Vector : public QVector<Data> {
public:
    uint size() const {
        return uint(QVector<Data>::size()) - _reservedCount;
    }
    Data at(uint i) const {
        return QVector<Data>::at(i + _reservedCount);
    }
    void clear() {
        QVector<Data>::clear();
        _reservedCount = 0u;
    }
    int indexOf(const Data& t, int from = 0) const {
        int index = QVector<Data>::indexOf(t, _reservedCount + from);
        return index == -1 ? index : index - _reservedCount;
    }
    void prepend(const Data& t) {
        if (_reservedCount != 0u) {
            --_reservedCount;
            QVector<Data>::replace(int(_reservedCount), t);
        }
        else {
            _reservedCount = reservedSize;
            QVector<Data> newVec(reservedSize);
            newVec.append(*this);
            QVector<Data>::clear();
            QVector<Data>::append(newVec);
        }
    }

    Data& operator[](int i) {
        return QVector<Data>::operator [](i + _reservedCount);
    }

    const Data& operator[](int i) const {
        return QVector<Data>::operator [](i + _reservedCount);
    }

    bool isEmpty() const {
        return uint(QVector<Data>::size()) == _reservedCount;
    }

    void reserveSpace() {
        QVector<Data> newVec(reservedSize);
        QVector<Data>::append(newVec);
        _reservedCount = reservedSize;
    }

private:
    /*mutable*/ uint _reservedCount = 0u;

};

} // namespace DatabaseViewModelDetail

#endif // DATABASEVIEWMODELDETAIL_VECTOR_H
