/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2024 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   database.h
*/

#ifndef GSAFE__DATABASE_HEADER_FILE_X_
#define GSAFE__DATABASE_HEADER_FILE_X_

#include <QtCore>
#include <QtNetwork>
#include <QtSql>

/*  @{  */

#include <data.h>

class HValue;
class HSqlBuilder;
class HSqlConnector;
class HSqlConnector_HttpRest;
class HDataMatrix;

/** The base Sql handler class.
 *  Here are the base Sql functions, to make the base operations, error handling, make transactions etc...
 *  This class can be used directly for making atomic sql operations too. \n
 \code
    ...
    HSql sql;
    auto b = sql.execDataMtrx(
                    db_query("persons")
                        .get("name")
                        .get("alias")
                        .cond_fv("id",Unquoted,"3","=")
                    );
    QString csv = b->getContentAsCSV());
    ...
 \endcode
 */
class HSql : public QObject
{
    Q_OBJECT

    public:
        HSql(void);
        HSql(QString databaseName);
        ~HSql(void);

        /** Begins an SQL transaction. Have to call this before calling a commit() or a rollback()
         *  @see commit()
         *  @see rollback()
         *  @see transaction() */
        void transaction(void);

        /** Commits the current transaction
         *  @see commit()
         *  @see rollback()
         *  @see transaction() */
        void commit(void);

        /** Rollback the current transaction
         *  @see commit()
         *  @see rollback()
         *  @see transaction() */
        void rollback(void);

        /** Set the current database by name. It only change the database on this instance */
        HSql& db(QString databaseName = "");

        static void setDefaultDatabase(QString name);
        static void unsetDefaultDatabase(void);

        /** Submit a simple SQL command which have no result (DDL/DML command)
         *  @param request The SQL request object
         *  @param err In case of error the emitted error signal will be contains this message
         *  @param tdisabled FALSE:It is submitted in a new separate transaction, TRUE:Don's start a new transaction  */
        void        exec(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);

        /** Submit a simple SQL command which returns one result (One row,one cell)
         *  @param request The SQL request object
         *  @param err In case of error the emitted error signal will be contains this message
         *  @param tdisabled FALSE:It is submitted in a new separate transaction, TRUE:Don's start a new transaction
         *  @return Returns a HValue holding the value. */
        HValue    execSingle(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);

        HSqlConnector* execMultiUnsafe(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);
        QSharedPointer<HSqlConnector> execMulti(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);
        HDataMatrix *execDataMtrxUnsafe(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);
        HDataMatrix *execDataMtrxNoeUnsafe(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);

        void execFillDataMtrx(HSqlBuilder& request,HDataMatrix* dm,QString err = "Sql error",bool tdisabled = false);

        /** Submits an SQL command which have a table result
         *  @param request The SQL request object
         *  @param err In case of error the emitted error signal will be contains this message
         *  @param tdisabled FALSE:It is submitted in a new separate transaction, TRUE:Don's start a new transaction
         *  @see HPlainDataMatrix
         *  @return The result table in a HPlainDataMatrix instance */
        QSharedPointer<HDataMatrix> execDataMtrx(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);
        /** Submits an SQL command which have a table result, it returns null on empty result (istead of empty table)
         *  @param request The SQL request object
         *  @param err In case of error the emitted error signal will be contains this message
         *  @param tdisabled FALSE:It is submitted in a new separate transaction, TRUE:Don's start a new transaction
         *  @see HPlainDataMatrix
         *  @return The result table in a HPlainDataMatrix instance */
        QSharedPointer<HDataMatrix> execDataMtrxNoe(HSqlBuilder& request,QString err = "Sql error",bool tdisabled=false);

        /** Queryes if the last SQL query is successfully executed or any error occured
         *  @return TRUE:error occured, FALSE:no error occured */
        bool errorStatus(void) { return query_error_occured; }

        static void setDialectForDatabaseName(QString dbname,QString dialect);
        static QString dialectForDatabaseName(QString dbname);

    protected:
        HDataMatrix *execDataMtrxCommonUnsafe(HSqlBuilder& request,bool noe,QString err = "Sql error",bool tdisabled=false);
        bool query_error_occured;

        /** Currently active transaction number */
        static int trans;

        HSqlConnector* getConnector(void);
        static QString overwrittenDefaultDbName;
        QString overwrittenDbName;

        static QMap<QString,QString> specifiedDialects;

    public:

    signals:
        /** If any error occured during the sql execution this signal is emitted. The "err" will be contains the error describe. */
        void errorSignal(QString err);
};

class HRestSqlSessionData {
public:
    bool auth;
    QString login,name;
    QString apitoken,chkval;
};

/** Represents a REST style database connection.
 *  When use this class as an Sql connection, all request
 *  send to a server as Json data channel. */
class HRestSqlDatabase : public QObject
{
    Q_OBJECT

public:
    HRestSqlDatabase(HRestSqlDatabase& database);

protected:
    HRestSqlDatabase(QString name,QMap<QString,QString>& params);

    QString databaseName;
    QString serverUrl;
    QString resourceName;
    QString fastId;
    QString peerVerify;
    QString externalCaFile;

private:
    static QMap<QString,HRestSqlDatabase *> rest_databases;
    static QMap<QString,HRestSqlSessionData> sessions;

public:

    static HRestSqlDatabase& addDatabase(QString name,QMap<QString,QString>& params);
    static void removeDatabase(QString name);
    static bool contains(QString name);
    static QStringList connectionNames();
    static HRestSqlDatabase& database(QString name);

    QString sendRequest(HSqlBuilder& request,QMap<QString,QString> toplevelExtraFields = QMap<QString,QString>());
    QJsonDocument sendRequestJSON(HSqlBuilder& request,QMap<QString,QString> toplevelExtraFields = QMap<QString,QString>());
    bool sendFieldExistenceCheckRequest(QString tablename,QStringList fields,QMap<QString,QString> toplevelExtraFields = QMap<QString,QString>());
    QString sendCustomRequest(QString reqId,QString& request,QMap<QString,QString> toplevelExtraFields = QMap<QString,QString>());

    QString sendRawRequest(QString& data);

    QString currentSessionUserLogin();
    QString currentSessionUserName();

    QString sendLoginToCodkep(QString login,QString credential);
    QString sendWhoamiToCodkep();
    QString sendLogoutFromCodkep();

    virtual bool transaction(void);
    virtual bool commit(void);
    virtual bool rollback(void);

public slots:
    int sslErrorHandler(const QList<QSslError>& errors);

protected:
    virtual QString buildJsonToplevelExtra(QMap<QString,QString> toplevelExtraFields);

    static void registerDatabase(HRestSqlDatabase *new_instance);

    QString restAnswerReceived(QNetworkReply* restNetworkActionReply);
    virtual QByteArray processRequest(QString data);
    virtual QString processResponse(QByteArray response);
    virtual QString buildDataReqMessageFromRequest(HSqlBuilder& request,QMap<QString,QString> toplevelExtraFields = QMap<QString,QString>());
    virtual QString buildDataReqMessageFromFlExChRequest(QString tablename,QStringList fields,QMap<QString,QString> toplevelExtraFields = QMap<QString,QString>());
    virtual QString buildDataReqMessageFromCustomRequest(QString reqId,QString& request,QMap<QString,QString> toplevelExtraFields = QMap<QString,QString>());
    virtual void processSessionBlockIfExists(QString data);
    virtual void processSessionJsonObject(QJsonObject jso);
public:
    ~HRestSqlDatabase(void);
};

/** Low level abstract class of handling an Sql operation.
 *  This class used as low level operation base class of Native Qt Sql and gSAFE REST Sql operations */
class HSqlConnector
{
    public:
        virtual ~HSqlConnector(void) = 0;

        virtual void exec(HSqlBuilder& request) = 0;
        virtual bool errorStatus(void) = 0;
        virtual QString errorMessage(void) = 0;
        virtual int recordCount(void) = 0;
        virtual int columnCount(void) = 0;
        virtual bool nextRecord(void) = 0;
        virtual QList<QString>  fieldNames() = 0;
        virtual HValue value(int index) = 0;
        virtual HValue value(QString name) = 0;
        virtual HValue value(void) = 0;
};

/** The gSAFE REST Sql implementation of HSqlConnector */
class HSqlConnector_HttpRest : public HSqlConnector
{

public:
    HRestSqlDatabase& db;

    HSqlConnector_HttpRest(HRestSqlDatabase& database);
    virtual ~HSqlConnector_HttpRest(void);

    virtual void exec(HSqlBuilder& request);
    virtual bool errorStatus(void);
    virtual QString errorMessage(void);
    virtual int recordCount(void);
    virtual int columnCount(void);
    virtual bool nextRecord(void);
    virtual QList<QString>  fieldNames();
    virtual HValue value(int index);
    virtual HValue value(QString name);
    virtual HValue value(void);

    void resetRecord(void);

protected:
    bool errorStatusValue;
    QString errorStringValue;

    int record_runner;
    int                   processed_rtype;
    HValue                processed_single;
    QList<QString>        processed_fields;
    QList<QList<HValue> > processed_data;

    void parseAnswerTable(QString payload,QStringList fields);
    void parseAnswerTable(QJsonDocument parsedJson,QStringList fields);
};

/** The Native Qt Sql implementation of HSqlConnector */
class HSqlConnector_QtNativeSql : public HSqlConnector
{
private:
    QString specifiedDialect;
    QSqlQuery query;
    bool errorStatusValue;
    QString errorStringValue;

public:
    HSqlConnector_QtNativeSql(QString dbname = QString());
    virtual ~HSqlConnector_QtNativeSql(void);

    QSqlQuery *q(void);
    virtual void exec(HSqlBuilder& request);
    virtual bool errorStatus(void);
    virtual QString errorMessage(void);
    virtual int recordCount(void);
    virtual int columnCount(void);
    virtual bool nextRecord(void);
    virtual QList<QString>  fieldNames();
    virtual HValue value(int index);
    virtual HValue value(QString name);
    virtual HValue value(void);
};

/** This functions returns the Sql dialect text accoring to the Qt native driver string */
QString dialectFromDriverHandlerType(QString dht);
QString strlistToJsonArray(QStringList sl);
QString strmapToJsonObject(QMap<QString,QString> m);

/* @} */

#endif

//End of gSAFE database.h
