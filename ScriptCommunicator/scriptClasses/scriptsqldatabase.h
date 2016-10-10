#ifndef SCRIPTSQLDATABASE_H
#define SCRIPTSQLDATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QStringList>
#include <QSqlField>
#include <QSqlIndex>
#include <QScriptValue>
#include <QScriptable>
#include <QScriptEngine>
#include <QSqlField>
#include <QSqlResult>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlDriver>
#include <QScriptValueIterator>
#include "scriptHelper.h"
#include "scriptObject.h"

class ScriptSqlIndex;


class  ScriptSqlField : public QObject, public ScriptObject
{
    Q_OBJECT
    friend class ScriptSqlIndex;
    friend class ScriptSqlRecord;

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:

    ScriptSqlField(const QSqlField& field) : QObject(0), m_field(field){}

    ScriptSqlField& operator=(const ScriptSqlField& other)
    {
        m_field = other.m_field;
        return *this;
    }

    ~ScriptSqlField(){}

    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSqlField.api");
    }

    Q_INVOKABLE void setValue(QVariant value)
    {
        if(value.type() == QVariant::List){value = ScriptHelper::variantListToByteArray(value.toList());}
        m_field.setValue(value);
    }
    Q_INVOKABLE QVariant value()
    {
        QVariant val = m_field.value();
        if(val.type() == QVariant::ByteArray){val = ScriptHelper::byteArrayToVariantList(val.toByteArray());}
        return val;
    }
    Q_INVOKABLE void setName(QString name){m_field.setName(name);}
    Q_INVOKABLE QString name(){return m_field.name();}
    Q_INVOKABLE bool isNull(){return m_field.isNull();}
    Q_INVOKABLE void setReadOnly(bool readOnly){m_field.setReadOnly(readOnly);}
    Q_INVOKABLE bool isReadOnly(){return m_field.isReadOnly();}
    Q_INVOKABLE void clear(){m_field.clear();}
    Q_INVOKABLE /*QVariant::Type*/quint32 type(){return (quint32)m_field.type();}
    Q_INVOKABLE bool isAutoValue(){return m_field.isAutoValue();}

    Q_INVOKABLE void setType(QVariant::Type type){m_field.setType(type);}
    Q_INVOKABLE void setRequiredStatus(quint32 status){m_field.setRequiredStatus((QSqlField::RequiredStatus)status);}
    Q_INVOKABLE void setRequired(bool required){m_field.setRequired(required);}
    Q_INVOKABLE void setLength(int fieldLength){m_field.setLength(fieldLength);}
    Q_INVOKABLE void setPrecision(int precision){m_field.setPrecision(precision);}
    Q_INVOKABLE void setDefaultValue(QVariant value)
    {
        if(value.type() == QVariant::List){value = ScriptHelper::variantListToByteArray(value.toList());}
        m_field.setDefaultValue(value);
    }
    Q_INVOKABLE void setSqlType(int type){m_field.setSqlType(type);}
    Q_INVOKABLE void setGenerated(bool gen){m_field.setGenerated(gen);}
    Q_INVOKABLE void setAutoValue(bool autoVal){m_field.setAutoValue(autoVal);}

    Q_INVOKABLE /*QSqlField::RequiredStatus*/quint32 requiredStatus(){return (quint32)m_field.requiredStatus();}
    Q_INVOKABLE int length(){return m_field.length();}
    Q_INVOKABLE int precision(){return m_field.precision();}
    Q_INVOKABLE QVariant defaultValue()
    {
        QVariant val = m_field.defaultValue();
        if(val.type() == QVariant::ByteArray){val = ScriptHelper::byteArrayToVariantList(val.toByteArray());}
        return val;
    }
    Q_INVOKABLE int typeID(){return m_field.typeID();}
    Q_INVOKABLE bool isGenerated(){return m_field.isGenerated();}
    Q_INVOKABLE bool isValid(){return m_field.isValid();}

private:
    QSqlField m_field;
};

class  ScriptSqlRecord : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptSqlRecord(const QSqlRecord& other) : QObject(0), m_record(other){}

    ScriptSqlRecord& operator=(const ScriptSqlRecord& other)
    {
        m_record = other.m_record;
        return *this;
    }

    ~ScriptSqlRecord(){}

    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSqlRecord.api");
    }

    Q_INVOKABLE QVariant value(int i)
    {
        QVariant val = m_record.value(i);
        if(val.type() == QVariant::ByteArray){val =ScriptHelper:: byteArrayToVariantList(val.toByteArray());}
        return val;
        return val;
    }
    Q_INVOKABLE QVariant value(QString name)
    {
        QVariant val = m_record.value(name);
        if(val.type() == QVariant::ByteArray){val = ScriptHelper::byteArrayToVariantList(val.toByteArray());}
        return val;
    }
    Q_INVOKABLE void setValue(int i, QVariant val)
    {
        if(val.type() == QVariant::List){val = ScriptHelper::variantListToByteArray(val.toList());}
        m_record.setValue(i, val);
    }
    Q_INVOKABLE void setValue(QString name, QVariant val)
    {
        if(val.type() == QVariant::List){val = ScriptHelper::variantListToByteArray(val.toList());}
        m_record.setValue(name, val);
    }

    Q_INVOKABLE void setNull(int i){m_record.setNull(i);}
    Q_INVOKABLE void setNull(QString name){m_record.setNull(name);}
    Q_INVOKABLE bool isNull(int i) {return m_record.isNull(i);}
    Q_INVOKABLE bool isNull(QString name){return m_record.isNull(name);}

    Q_INVOKABLE int indexOf(QString name){return m_record.indexOf(name);}
    Q_INVOKABLE QString fieldName(int i) {return m_record.fieldName(i);}

    Q_INVOKABLE QScriptValue field(int i)
    {
        ScriptSqlField* field = new ScriptSqlField(m_record.field(i));
        return engine()->newQObject(field, QScriptEngine::ScriptOwnership);
    }

    Q_INVOKABLE QScriptValue field(QString name)
    {
        ScriptSqlField* field = new ScriptSqlField(m_record.field(name));
        return engine()->newQObject(field, QScriptEngine::ScriptOwnership);
    }


    Q_INVOKABLE bool isGenerated(int i){return m_record.isGenerated(i);}
    Q_INVOKABLE bool isGenerated(QString name){return m_record.isGenerated(name);}
    Q_INVOKABLE void setGenerated(QString name, bool generated){m_record.setGenerated(name, generated);}
    Q_INVOKABLE void setGenerated(int i, bool generated){m_record.setGenerated(i, generated);}

    Q_INVOKABLE void append(ScriptSqlField* field){m_record.append(field->m_field);}
    Q_INVOKABLE void replace(int pos, ScriptSqlField* field){m_record.replace(pos, field->m_field);}
    Q_INVOKABLE void insert(int pos, ScriptSqlField* field){m_record.insert(pos, field->m_field);}
    Q_INVOKABLE void remove(int pos){m_record.remove(pos);}

    Q_INVOKABLE bool isEmpty(){return m_record.isEmpty();}
    Q_INVOKABLE bool contains(QString name){return m_record.contains(name);}
    Q_INVOKABLE void clear(){m_record.clear();}
    Q_INVOKABLE void clearValues(){m_record.clearValues();}
    Q_INVOKABLE int count(){return m_record.count();}
    Q_INVOKABLE QScriptValue keyValues(ScriptSqlRecord* keyFields)
    {
        ScriptSqlRecord* record = new ScriptSqlRecord(m_record.keyValues(keyFields->m_record));
        return engine()->newQObject(record, QScriptEngine::ScriptOwnership);
    }

private:
    QSqlRecord m_record;
};

class  ScriptSqlIndex : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptSqlIndex(QSqlIndex index) : QObject(0),m_index(index){ }
    ~ScriptSqlIndex(){}

    ScriptSqlIndex &operator=(const ScriptSqlIndex &other)
    {
        m_index = other.m_index;
        return *this;
    }

    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSqlIndex.api");
    }

    Q_INVOKABLE void setCursorName(QString cursorName){m_index.setCursorName(cursorName);}
    Q_INVOKABLE QString cursorName(void){ return m_index.cursorName(); }
    Q_INVOKABLE void setName(QString name){m_index.setName(name);}
    Q_INVOKABLE QString name(void) { return m_index.name(); }

    Q_INVOKABLE void append(ScriptSqlField* field){m_index.append(field->m_field);}
    Q_INVOKABLE void append(ScriptSqlField* field, bool desc){m_index.append(field->m_field,desc);}

    Q_INVOKABLE bool isDescending(int i){return m_index.isDescending(i);}
    Q_INVOKABLE void setDescending(int i, bool desc){m_index.setDescending(i, desc);}

private:
    QSqlIndex m_index;
};


class  ScriptSqlError : public QObject, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:

    ScriptSqlError(const QSqlError& other) : QObject(0), m_error(other) {}
    ScriptSqlError& operator=(const ScriptSqlError& other)
    {
        m_error = other.m_error;
        return *this;
    }

    ~ScriptSqlError(){}

    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSqlError.api");
    }

    Q_INVOKABLE QString driverText(void){return m_error.driverText();}
    Q_INVOKABLE QString databaseText(void){return m_error.databaseText();}
    Q_INVOKABLE /*QSqlError::ErrorType*/ quint32 type(void){return (quint32)m_error.type();}
    Q_INVOKABLE QString nativeErrorCode(void){return m_error.nativeErrorCode();}
    Q_INVOKABLE QString text(void){return m_error.text();}
    Q_INVOKABLE bool isValid(void){return m_error.isValid();}
private:

    QSqlError m_error;
};


class  ScriptSqlQuery : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:

    ScriptSqlQuery(const QSqlQuery& other) : QObject(0), m_query(other){}
    ScriptSqlQuery& operator=(const ScriptSqlQuery& other)
    {
        m_query = other.m_query;
        return *this;
    }

    ~ScriptSqlQuery(){}

    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSqlQuery.api");
    }

    Q_INVOKABLE bool isValid(){return m_query.isValid();}
    Q_INVOKABLE bool isActive(){return m_query.isActive();}
    Q_INVOKABLE bool isNull(int field){return m_query.isNull(field);}
    Q_INVOKABLE bool isNull(QString name){return m_query.isNull(name);}
    Q_INVOKABLE int at(){return m_query.at();}
    Q_INVOKABLE QString lastQuery(){return m_query.lastQuery();}
    Q_INVOKABLE int numRowsAffected(){return m_query.numRowsAffected();}
    Q_INVOKABLE QScriptValue lastError()
    {
        ScriptSqlError* error = new ScriptSqlError(m_query.lastError());
        return engine()->newQObject(error, QScriptEngine::ScriptOwnership);
    }

    Q_INVOKABLE bool isSelect(){return m_query.isSelect();}
    Q_INVOKABLE int size(){return m_query.size();}
    //const QSqlDriver* driver() const;
    //const QSqlResult* result() const;
    Q_INVOKABLE bool isForwardOnly(){return m_query.isForwardOnly();}
    Q_INVOKABLE QScriptValue record()
    {
        ScriptSqlRecord* record = new ScriptSqlRecord(m_query.record());
        return engine()->newQObject(record, QScriptEngine::ScriptOwnership);
    }

    Q_INVOKABLE void setForwardOnly(bool forward){m_query.setForwardOnly(forward);}
    Q_INVOKABLE bool exec(QString query){return m_query.exec(query);}

    Q_INVOKABLE QVariant value(int i)
    {
        QVariant val = m_query.value(i);
        if(val.type() == QVariant::ByteArray){val = ScriptHelper::byteArrayToVariantList(val.toByteArray());}
        return val;
    }
    Q_INVOKABLE QVariant value(QString name)
    {
        QVariant val = m_query.value(name);
        if(val.type() == QVariant::ByteArray){val = ScriptHelper::byteArrayToVariantList(val.toByteArray());}
        return val;
    }

    Q_INVOKABLE void setNumericalPrecisionPolicy(quint32 precisionPolicy){m_query.setNumericalPrecisionPolicy((QSql::NumericalPrecisionPolicy)precisionPolicy);}
    Q_INVOKABLE /*QSql::NumericalPrecisionPolicy*/quint32 numericalPrecisionPolicy(){return (quint32)m_query.numericalPrecisionPolicy();}

    Q_INVOKABLE bool seek(int i, bool relative = false){return m_query.seek(i,relative);}
    Q_INVOKABLE bool next(){return m_query.next();}
    Q_INVOKABLE bool previous(){return m_query.previous();}
    Q_INVOKABLE bool first(){return m_query.first();}
    Q_INVOKABLE bool last(){return m_query.last();}

    Q_INVOKABLE void clear(){m_query.clear();}

    Q_INVOKABLE bool exec(){return m_query.exec();}
    Q_INVOKABLE bool execBatch(quint32 mode = (quint32)QSqlQuery::ValuesAsRows){return m_query.execBatch((QSqlQuery::BatchExecutionMode)mode);}
    Q_INVOKABLE bool prepare(QString query){return m_query.prepare(query);}

    Q_INVOKABLE void bindValue(QString placeholder, QVariant val, quint32 type = (quint32)QSql::In)
    {
        if(val.type() == QVariant::List){val = ScriptHelper::variantListToByteArray(val.toList());}
        m_query.bindValue(placeholder, val, (QSql::ParamType)type);
    }
    Q_INVOKABLE void bindValue(int pos, QVariant val, quint32 type = (quint32)QSql::In)
    {
        if(val.type() == QVariant::List){val = ScriptHelper::variantListToByteArray(val.toList());}
        m_query.bindValue(pos, val, (QSql::ParamType)type);
    }

    Q_INVOKABLE void addBindValue(QVariant val, quint32 type = (quint32)QSql::In)
    {
        if(val.type() == QVariant::List){val = ScriptHelper::variantListToByteArray(val.toList());}
        m_query.addBindValue(val, (QSql::ParamType)type);
    }
    Q_INVOKABLE QVariant boundValue(QString placeholder)
    {
        QVariant val = m_query.boundValue(placeholder);
        if(val.type() == QVariant::ByteArray){val = ScriptHelper::byteArrayToVariantList(val.toByteArray());}
        return val;
    }
    Q_INVOKABLE QVariant boundValue(int pos)
    {
        QVariant val = m_query.boundValue(pos);
        if(val.type() == QVariant::ByteArray){val =ScriptHelper:: byteArrayToVariantList(val.toByteArray());}
        return val;
    }
    Q_INVOKABLE ScriptMap boundValues(){return ScriptMap(m_query.boundValues());}
    Q_INVOKABLE QString executedQuery(){return m_query.executedQuery();}
    Q_INVOKABLE QVariant lastInsertId(){return m_query.lastInsertId();}
    Q_INVOKABLE void finish(){m_query.finish();}
    Q_INVOKABLE bool nextResult(){return m_query.next();}

private:
    QSqlQuery m_query;
};


class ScriptSql;

class ScriptSqlDatabase : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT
    friend class ScriptSql;

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    explicit ScriptSqlDatabase(QObject *parent = 0) : QObject(parent), m_database(){}

    ScriptSqlDatabase(QSqlDatabase db, QObject *parent = 0) : QObject(parent), m_database(db){}

    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("ScriptSqlDatabase.api");
    }

    Q_INVOKABLE bool open(){return m_database.open();}
    Q_INVOKABLE bool open(QString user, QString password){return m_database.open(user, password);}
    Q_INVOKABLE void close(){return m_database.close();}
    Q_INVOKABLE bool isOpen(){return m_database.isOpen();}
    Q_INVOKABLE bool isOpenError(){return m_database.isOpenError();}
    Q_INVOKABLE QStringList tables(quint32 type = QSql::TableType::Tables){return m_database.tables((QSql::TableType)type);}
    Q_INVOKABLE QScriptValue primaryIndex(QString tablename)
    {
        ScriptSqlIndex* index = new ScriptSqlIndex(m_database.primaryIndex(tablename));
        return engine()->newQObject(index, QScriptEngine::ScriptOwnership);
    }

    Q_INVOKABLE QScriptValue record(QString tablename)
    {
        ScriptSqlRecord* record = new ScriptSqlRecord(m_database.record(tablename));
        return engine()->newQObject(record, QScriptEngine::ScriptOwnership);
    }

    Q_INVOKABLE QScriptValue exec(QString query = QString())
    {
        ScriptSqlQuery* obj = new ScriptSqlQuery(m_database.exec(query));
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }

    Q_INVOKABLE QScriptValue lastError()
    {
        ScriptSqlError* obj = new ScriptSqlError(m_database.lastError());
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }

    Q_INVOKABLE bool isValid(){return m_database.isValid();}

    Q_INVOKABLE bool transaction(){return m_database.transaction();}
    Q_INVOKABLE bool commit(){return m_database.commit();}
    Q_INVOKABLE bool rollback(){return m_database.rollback();}

    Q_INVOKABLE void setDatabaseName(QString name){m_database.setDatabaseName(name);}
    Q_INVOKABLE void setUserName(QString name){m_database.setUserName(name);}
    Q_INVOKABLE void setPassword(QString password){m_database.setPassword(password);}
    Q_INVOKABLE void setHostName(QString host){m_database.setHostName(host);}
    Q_INVOKABLE void setPort(int p){m_database.setPort(p);}
    Q_INVOKABLE void setConnectOptions(QString options = QString()){m_database.setConnectOptions(options);}
    Q_INVOKABLE QString databaseName(){return m_database.databaseName();}
    Q_INVOKABLE QString userName(){return m_database.userName();}
    Q_INVOKABLE QString password(){return m_database.password();}
    Q_INVOKABLE QString hostName(){return m_database.hostName();}
    Q_INVOKABLE QString driverName(){return m_database.driverName();}
    Q_INVOKABLE int port(){return m_database.port();}
    Q_INVOKABLE QString connectOptions(){return m_database.connectOptions();}
    Q_INVOKABLE QString connectionName(){return m_database.connectionName();}
    Q_INVOKABLE void setNumericalPrecisionPolicy(quint32 precisionPolicy){m_database.setNumericalPrecisionPolicy((QSql::NumericalPrecisionPolicy)precisionPolicy);}
    Q_INVOKABLE /*QSql::NumericalPrecisionPolicy*/quint32 numericalPrecisionPolicy(){return (quint32)m_database.numericalPrecisionPolicy();}

    //static QSqlDatabase addDatabase(QSqlDriver* driver,
      //                           const QString& connectionName = QLatin1String(defaultConnection));

    //static void registerSqlDriver(const QString &name, QSqlDriverCreatorBase *creator);


    //QSqlDriver* driver() const;

private:
    QSqlDatabase m_database;

};

class ScriptSql : public QObject, protected QScriptable, public ScriptObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)
public:
    ScriptSql() : QObject(0){}

    ///Registers all (for this class) necessary meta types.
    void registerScriptMetaTypes(QScriptEngine* scriptEngine)
    {
        qRegisterMetaType<ScriptSqlField*>("ScriptSqlField*");
        qRegisterMetaType<ScriptSqlRecord*>("ScriptSqlRecord*");
        qRegisterMetaType<ScriptSqlDatabase*>("ScriptSqlDatabase*");

         scriptEngine->globalObject().setProperty("scriptSql", scriptEngine->newQObject(this));
    }

    virtual QString getPublicScriptElements(void)
    {
        return MainWindow::parseApiFile("scriptSql.api");
    }

    /**
     * Adds a database to the list of database connections using the driver type and the connection name
     * connectionName. If there already exists a database connection called connectionName, that connection
     * is removed.
     * The database connection is referred to by connectionName. The newly added database connection is
     * returned.If type is not available or could not be loaded, isValid() returns false.
     * If connectionName is not specified, the new connection becomes the default connection for the
     * application, and subsequent calls to database() without the connection name argument will return the
     * default connection. If a connectionName is provided here, use database(connectionName) to retrieve the
     * connection.
     * Warning: If you add a connection with the same name as an existing connection, the new connection
     * replaces the old one. If you call this function more than once without specifying connectionName, the
     * default connection will be the one replaced.
     */
    Q_INVOKABLE QScriptValue addDatabase(QString type,
                                 QString connectionName = QLatin1String(QSqlDatabase::defaultConnection))
    {
        ScriptSqlDatabase* obj = new ScriptSqlDatabase(QSqlDatabase::addDatabase(type, connectionName));
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }

    /**
     * Clones the database connection other and stores it as connectionName. All the settings from the
     * original database, e.g. databaseName(), hostName(), etc., are copied across. Does nothing if other
     * is an invalid database. Returns the newly created database connection.
     * Note: The new connection has not been opened. Before using the new connection, you must call open().
     */
    Q_INVOKABLE QScriptValue cloneDatabase(ScriptSqlDatabase* other, QString connectionName)
    {
        ScriptSqlDatabase* obj = new ScriptSqlDatabase(QSqlDatabase::cloneDatabase(other->m_database, connectionName));
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }
    /**
     * Returns the database connection called connectionName. The database connection must have been
     * previously added with addDatabase(). If open is true (the default) and the database connection is not
     *  already open it is opened now. If no connectionName is specified the default connection is used.
     * If connectionName does not exist in the list of databases, an invalid connection is returned.
     */
    Q_INVOKABLE QScriptValue database(QString connectionName = QLatin1String(QSqlDatabase::defaultConnection),
                                 bool open = true)
    {
        ScriptSqlDatabase* obj = new ScriptSqlDatabase(QSqlDatabase::database(connectionName, open));
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }
    /**
     * Removes the database connection connectionName from the list of database connections.
     * Warning: There should be no open queries on the database connection when this function is called,
     * otherwise a resource leak will occur.
     */
    Q_INVOKABLE void removeDatabase(QString connectionName){QSqlDatabase::removeDatabase(connectionName);}
    /**
     * Returns true if the list of database connections contains connectionName; otherwise returns false.
     */
    Q_INVOKABLE bool contains(QString connectionName = QLatin1String(QSqlDatabase::defaultConnection)){return QSqlDatabase::contains(connectionName);}

    ///Returns a list containing the names of all connections.
    Q_INVOKABLE QStringList connectionNames(){return QSqlDatabase::connectionNames();}

    ///Returns a list of all the available database drivers.
    Q_INVOKABLE QStringList drivers(){return QSqlDatabase::drivers();}

	///Returns true if a driver called name is available; otherwise returns false.
    Q_INVOKABLE bool isDriverAvailable(QString name){return QSqlDatabase::isDriverAvailable(name);}

    ///Creates a ScriptSqlQuery object using the SQL query and the current database.
    Q_INVOKABLE QScriptValue createQuery(ScriptSqlDatabase* dataBase, QString query = QString())
    {
        ScriptSqlQuery* obj = new ScriptSqlQuery(QSqlQuery(query, dataBase->m_database));
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }
    ///Creates a ScriptSqlField object.
    Q_INVOKABLE QScriptValue createField(QString fieldName = QString(),
                                         quint32 type = (quint32)QVariant::Invalid)
    {
        ScriptSqlField* obj = new ScriptSqlField(QSqlField(fieldName, (QVariant::Type)type));
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }
    ///Creates a ScriptSqlRecord object.
    Q_INVOKABLE QScriptValue createRecord(void)
    {
        ScriptSqlRecord* obj = new ScriptSqlRecord(QSqlRecord());
        return engine()->newQObject(obj, QScriptEngine::ScriptOwnership);
    }

};

#endif // SCRIPTSQLDATABASE_H
