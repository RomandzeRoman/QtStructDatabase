# QtStructDatabase
Qt module for store plain structs in QSLite database

## Example store structs
```
#include "QtStructDatabase/database.h"
struct Stored {
    int a;
};
Database<Stored> db("storage.db");
db.addRecord(Stored{0});
auto vec = db.read(0, 1); // returns QVector<Stored>{ Stored{0} }
```
## Example show in QML

## Stored types
- int
- double
- qint64
- bool
- QString
- QDateTime

