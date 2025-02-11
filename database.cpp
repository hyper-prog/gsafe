/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2024 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   database.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "data.h"
#include "database.h"
#include "dconsole.h"
#include "builder.h"
#include "dm.h"
#include "dmext.h"

#define REQUEST_TIMEOUT 45

QString HSql::overwrittenDefaultDbName = "";
int     HSql::trans = 0;
QMap<QString,QString> HSql::specifiedDialects = QMap<QString,QString>();

QMap<QString,HRestSqlSessionData> HRestSqlDatabase::sessions = QMap<QString,HRestSqlSessionData>();
QMap<QString,HRestSqlDatabase *> HRestSqlDatabase::rest_databases = QMap<QString,HRestSqlDatabase *>();


HRestSqlDatabase::HRestSqlDatabase(HRestSqlDatabase& database)
 : QObject()
{
    databaseName   = database.databaseName;
    serverUrl      = database.serverUrl;
    resourceName   = database.resourceName;
    fastId         = database.fastId;
    peerVerify     = database.peerVerify;
    externalCaFile = database.externalCaFile;
}

HRestSqlDatabase::HRestSqlDatabase(QString name,QMap<QString,QString>& params)
 : QObject()
{
    if(!QSslSocket::supportsSsl())
    {
        QMessageBox::warning(NULL,"Warning!",
                QString("Cannot initialize OpenSSL subsystem!\n"
                        "Please install the OpenSSL %1 bit!\n\n"
                        "This Qt system is build with version:\n"
                        "%2\n\nCurrent available version:\n%3")
                                  .arg(QSysInfo::WordSize)
                                  .arg(QSslSocket::sslLibraryBuildVersionString())
                                  .arg(QSslSocket::sslLibraryVersionString()));
        return;
    }
    if(!params.contains("server_url") ||
       !params.contains("resource_name") ||
       !params.contains("fast_security_id"))
    {
        QMessageBox::warning(NULL,"Warning!",
                QString("Not fully specified REST database connection: %1\n"
                        "You have to set \"server_url\", \"resource_name\" and \"fast_security_id\" parameters to establish REST database connection!\n")
                             .arg(name));
        return;
    }


    databaseName = name;
    serverUrl    = params["server_url"];
    resourceName = params["resource_name"];
    fastId       = params["fast_security_id"];

    peerVerify   = "no";
    if(params.contains("peer_verify") &&
            (params["peer_verify"] == "yes" || params["peer_verify"] == "query"))
    {
        peerVerify = params["peer_verify"];
    }

    if(params.contains("external_ca_file"))
        externalCaFile = params["external_ca_file"];
}

HRestSqlDatabase& HRestSqlDatabase::addDatabase(QString name,QMap<QString,QString>& params)
{
    removeDatabase(name);
    HRestSqlDatabase *new_instance = new HRestSqlDatabase(name,params);
    registerDatabase(new_instance);
    return *new_instance;
}

void HRestSqlDatabase::registerDatabase(HRestSqlDatabase *new_instance)
{
    rest_databases[new_instance->databaseName] = new_instance;
}

void HRestSqlDatabase::removeDatabase(QString name)
{
    if(!rest_databases.contains(name))
        return;

    HRestSqlDatabase *to_remove = rest_databases[name];
    if(to_remove != NULL)
        delete to_remove;

    rest_databases.remove(name);
}

bool HRestSqlDatabase::contains(QString name)
{
    return rest_databases.contains(name);
}

QStringList HRestSqlDatabase::connectionNames()
{
    return rest_databases.keys();
}

HRestSqlDatabase& HRestSqlDatabase::database(QString name)
{
    if(rest_databases.contains(name))
        return *(rest_databases.value(name));
    return *(rest_databases.values()[0]);
}

HRestSqlDatabase::~HRestSqlDatabase(void)
{

}

QString HRestSqlDatabase::currentSessionUserLogin()
{
    if(sessions.contains(databaseName))
        if(sessions[databaseName].auth)
            return sessions[databaseName].login;
    return "";
}

QString HRestSqlDatabase::currentSessionUserName()
{
    if(sessions.contains(databaseName))
        if(sessions[databaseName].auth)
            return sessions[databaseName].name;
    return "";
}

int HRestSqlDatabase::sslErrorHandler(const QList<QSslError>& errors)
{
    int n=0;
    QList<QSslError>::const_iterator i;
    for(i=errors.constBegin();i != errors.constEnd();++i)
        sdebug(QString("SSL Error %1: %2<br/>").arg(n++).arg(i->errorString()));
    return 0;
}

bool HRestSqlDatabase::transaction(void)
{
    return true; //The class not support transaction through rest.
}

bool HRestSqlDatabase::commit(void)
{
    return true; //The class not support transaction through rest.
}

bool HRestSqlDatabase::rollback(void)
{
    return true; //The class not support transaction through rest.
}

QString HRestSqlDatabase::sendRequest(HSqlBuilder& request,QMap<QString,QString> toplevelExtraFields)
{
    QString data = buildDataReqMessageFromRequest(request,toplevelExtraFields);
    QString responsePayload = sendRawRequest(data);
    processSessionBlockIfExists(responsePayload);
    return responsePayload;
}

QJsonDocument HRestSqlDatabase::sendRequestJSON(HSqlBuilder& request,QMap<QString,QString> toplevelExtraFields)
{
    QJsonParseError jpe;
    QString data = buildDataReqMessageFromRequest(request,toplevelExtraFields);
    QJsonDocument answerJson = QJsonDocument::fromJson(sendRawRequest(data).toUtf8(),&jpe);
    QJsonObject jo = answerJson.object();
    if(answerJson.object().contains("session"))
    {
        QJsonObject jso = answerJson.object().value("session").toObject();
        processSessionJsonObject(jso);
    }
    return answerJson;
}

bool HRestSqlDatabase::sendFieldExistenceCheckRequest(QString tablename,QStringList fields,QMap<QString,QString> toplevelExtraFields)
{
    QString data = buildDataReqMessageFromFlExChRequest(tablename,fields,toplevelExtraFields);
    QString payload = sendRawRequest(data);

    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    QJsonObject jo = answer.object();
    if(answer.object().contains("session"))
    {
        QJsonObject jso = answer.object().value("session").toObject();
        processSessionJsonObject(jso);
    }
    if(!answer.isNull() && jo.value("status") == "Ok")
        return true;
    return false;
}

QString HRestSqlDatabase::sendCustomRequest(QString reqId,QString& request,QMap<QString,QString> toplevelExtraFields)
{
    QString data = buildDataReqMessageFromCustomRequest(reqId,request,toplevelExtraFields);
    QString responsePayload = sendRawRequest(data);
    processSessionBlockIfExists(responsePayload);
    return responsePayload;
}

void HRestSqlDatabase::processSessionBlockIfExists(QString data)
{
    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(data.toUtf8(),&jpe);
    QJsonObject jo = answer.object();
    if(answer.object().contains("session"))
    {
        QJsonObject jso = answer.object().value("session").toObject();
        processSessionJsonObject(jso);
    }
}

void HRestSqlDatabase::processSessionJsonObject(QJsonObject jso)
{
    if(jso.contains("keychange") && jso.value("keychange").toString() == "required")
    {
        if(sessions.contains(databaseName) && sessions[databaseName].auth)
        {
            sessions[databaseName].chkval = jso.value("chkval").toString();
            sdebug("*** communication keychange applied ***");
            return;
        }
    }
}

QString HRestSqlDatabase::sendLoginToCodkep(QString login,QString credential)
{
    HRestSqlSessionData sess;
    sess.auth = false;
    sess.login = "";
    sess.name = "";
    sess.apitoken = "";
    sess.chkval = "";
    sessions[databaseName] = sess;

    QMap<QString,QString> param;
    param["login"] = login;
    param["cred"] = credential;
    QString parameterJson = strmapToJsonObject(param);
    QString payload = sendCustomRequest("login_user",parameterJson);
    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    if(answer.isNull())
        return "";
    QJsonObject jo = answer.object();
    if(jo.contains("status") && jo.value("status") == "Ok")
    {
        if(jo.contains("array"))
        {
            QJsonObject ijo = jo.value("array").toObject();
            if(ijo.contains("login") && !ijo.value("login").toString().isEmpty() &&
               ijo.contains("name") && !ijo.value("name").toString().isEmpty() &&
               ijo.contains("chkval") && !ijo.value("chkval").toString().isEmpty() &&
               ijo.contains("apitoken") && !ijo.value("apitoken").toString().isEmpty())
            {
                sess.auth = true;
                sess.login = ijo.value("login").toString();
                sess.name = ijo.value("name").toString();
                sess.apitoken = ijo.value("apitoken").toString();
                sess.chkval = ijo.value("chkval").toString();
                sessions[databaseName] = sess;

                sdebug(QString("Succesful login: %1").arg(sess.login));
                return sess.login;
            }
        }
    }
    return "";
}

QString HRestSqlDatabase::sendWhoamiToCodkep()
{
    QMap<QString,QString> param;
    QString parameterJson = strmapToJsonObject(param);
    QString payload = sendCustomRequest("whoami_user",parameterJson);

    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    if(answer.isNull())
        return "";
    QJsonObject jo = answer.object();
    if(jo.contains("status") && jo.value("status") == "Ok")
    {
        if(jo.contains("array"))
        {
            QJsonObject ijo = jo.value("array").toObject();
            if(ijo.contains("auth") && !ijo.value("auth").toString().isEmpty() &&
               ijo.contains("login") && !ijo.value("login").toString().isEmpty())
            {
                if(ijo.value("auth").toString() == "yes" &&
                   ijo.value("login").toString() == sessions[databaseName].login)
                {
                    return sessions[databaseName].login;
                }
                return "";
            }
        }
    }
    return "";
}

QString HRestSqlDatabase::sendLogoutFromCodkep()
{
    QMap<QString,QString> param;
    QString parameterJson = strmapToJsonObject(param);
    QString payload = sendCustomRequest("logout_user",parameterJson);

    HRestSqlSessionData sess;
    sess.auth = false;
    sess.login = "";
    sess.name = "";
    sess.apitoken = "";
    sess.chkval = "";
    sessions[databaseName] = sess;

    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    if(answer.isNull())
        return "";
    QJsonObject jo = answer.object();
    if(jo.contains("status") && jo.value("status") == "Ok")
        return "";
    return "Error";
}

QString HRestSqlDatabase::sendRawRequest(QString& data)
{
    QNetworkAccessManager networkManager;
    QUrl url;
    sdebug(QString("URL => %1").arg(serverUrl + "/" +resourceName + "/<FastSequrityID>" ));
    url.setUrl(serverUrl + "/" + resourceName + "/" +fastId);
    QNetworkRequest req;
    req.setUrl(url);
    req.setRawHeader("Content-Type","application/octet-stream");
    req.setRawHeader("Accept","application/octet-stream");

    QSslConfiguration conf = req.sslConfiguration();
    if(peerVerify == "no"   ) conf.setPeerVerifyMode(QSslSocket::VerifyNone);
    if(peerVerify == "yes"  ) conf.setPeerVerifyMode(QSslSocket::VerifyPeer);
    if(peerVerify == "query") conf.setPeerVerifyMode(QSslSocket::QueryPeer);
    if(!externalCaFile.isEmpty())
        conf.addCaCertificates(externalCaFile);
    req.setSslConfiguration(conf);

    //ssdebug("---------- Send payload data ----------",DCONSOLE_TYPE_TXTALT_CORAL);
    //ssdebug(QString("%1").arg(data),DCONSOLE_TYPE_TXTALT_CORAL);
    //ssdebug("---------- Send payload data ----------",DCONSOLE_TYPE_TXTALT_CORAL);

    QByteArray sendraw = processRequest(data);

    //ssdebug("---------- Send raw data ----------",DCONSOLE_TYPE_TXTALT_TEAL);
    //ssdebug(QString::fromUtf8(sendraw),DCONSOLE_TYPE_TXTALT_TEAL);
    //ssdebug("---------- Send raw data ----------",DCONSOLE_TYPE_TXTALT_TEAL);

    QNetworkReply* restNetworkActionReply;
    restNetworkActionReply = networkManager.post(req,sendraw);
    if(!connect(restNetworkActionReply,SIGNAL(sslErrors(QList<QSslError>)),this,SLOT(sslErrorHandler(QList<QSslError>))))
            QMessageBox::information(NULL,"cannot connect z0","cannot connect z0");
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect( restNetworkActionReply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect( &timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(REQUEST_TIMEOUT*1000);
    loop.exec();
    return restAnswerReceived(restNetworkActionReply);
}

QString HRestSqlDatabase::restAnswerReceived(QNetworkReply* restNetworkActionReply)
{
    QString last_result;
    if(restNetworkActionReply->isFinished() && !(last_result = restNetworkActionReply->readAll()).isEmpty() )
    {
        int rest_received_errorcode;
        if((rest_received_errorcode = restNetworkActionReply->error()) != QNetworkReply::NoError)
        {
            sdebug(QString("The processing of REST request is failed.\nReceived error code: %1")
                   .arg(rest_received_errorcode));
            return QString();
        }

        //ssdebug("---------- Received raw data ----------",DCONSOLE_TYPE_TXTALT_TEAL);
        //ssdebug(last_result,DCONSOLE_TYPE_TXTALT_TEAL);
        //ssdebug("---------- Received raw data ----------",DCONSOLE_TYPE_TXTALT_TEAL);
        return processResponse(last_result.toUtf8());
    }
    else
    {
        sdebug("The REST request does not return any data.");
        return QString();
    }
    return QString();
}

QByteArray HRestSqlDatabase::processRequest(QString data)
{
    return data.toUtf8();
}

QString HRestSqlDatabase::processResponse(QByteArray response)
{
    return QString::fromUtf8(response);
}

QString HRestSqlDatabase::buildJsonToplevelExtra(QMap<QString,QString> toplevelExtraFields)
{
    QString efpart = "";
    QMap<QString,QString>::iterator iter;
    for( iter = toplevelExtraFields.begin() ; iter != toplevelExtraFields.end() ; ++iter )
        efpart.append(QString(" \"%1\": \"%2\",")
                        .arg(iter.key())
                        .arg(iter.value()));

    if(sessions.contains(databaseName) && sessions[databaseName].auth)
    {
        efpart.append(QString(" \"session\": {\"apitoken\": \"%1\", \"chkval\": \"%2\"},")
                            .arg(sessions[databaseName].apitoken)
                            .arg(sessions[databaseName].chkval));
    }
    return efpart;
}

QString HRestSqlDatabase::buildDataReqMessageFromRequest(HSqlBuilder& request,QMap<QString,QString> toplevelExtraFields)
{
    QString rmode = "table";

    if(request.getJsonExecutionMode() == NoReturn)
        rmode = "none";
    if(request.getJsonExecutionMode() == SingleReturn)
        rmode = "single";
    if(request.getJsonExecutionMode() == TableReturn)
        rmode = "table";

    return QString("{\"reqId\": \"query_uni\",%1 \"return\": \"%2\", \"query_spec\": %3}")
                    .arg(buildJsonToplevelExtra(toplevelExtraFields))
                    .arg(rmode)
                    .arg(request.json_string());
}

QString HRestSqlDatabase::buildDataReqMessageFromFlExChRequest(QString tablename,QStringList fields,QMap<QString,QString> toplevelExtraFields)
{
    return QString("{\"reqId\": \"check_fields_exists\",%1 \"tablename\": \"%2\",\"fields\": %3}")
                    .arg(buildJsonToplevelExtra(toplevelExtraFields))
                    .arg(tablename)
                    .arg(strlistToJsonArray(fields));
}

QString HRestSqlDatabase::buildDataReqMessageFromCustomRequest(QString reqId,QString& request,QMap<QString,QString> toplevelExtraFields)
{
    QString efpart = buildJsonToplevelExtra(toplevelExtraFields);

    if(request.startsWith("{") && request.endsWith("}"))
        return QString("{\"reqId\": \"%1\",%2 \"parameters\": %3}")
                        .arg(reqId)
                        .arg(efpart)
                        .arg(request);
    return QString("{\"reqId\": \"%1\",%2 \"parameters\": \"%3\"}")
                    .arg(reqId)
                    .arg(efpart)
                    .arg(request);
}

QString strlistToJsonArray(QStringList sl)
{
    int i,l;
    QString jsp="[";
    l=sl.count();
    for(i=0;i < l;++i)
    {
        jsp.append(i==0?"":",");
        jsp.append(QString("\"%1\"")
                  .arg(sl[i]));
    }
    jsp.append("]");
    return jsp;

}

QString strmapToJsonObject(QMap<QString,QString> m)
{
    int i;
    QString jsp="{";
    QMapIterator<QString,QString> it(m);
    for(i=0;it.hasNext();)
    {
        it.next();
        jsp.append(i==0 ? "" : ",");
        QString k = it.key();
        QString v =it.value();
        if(v.trimmed().startsWith("{") && v.trimmed().endsWith("}"))
        {
            jsp.append(QString("\"%1\": %2")
                  .arg(k)
                  .arg(v.trimmed()));
        }
        else
        {
            jsp.append(QString("\"%1\": \"%2\"")
                  .arg(k)
                  .arg(v));
        }
        ++i;
    }
    jsp.append("}");
    return jsp;
}

// ///////////////////////////////////////////////////////////////////// //

HSqlConnector::~HSqlConnector(void)
{

}

// ///////////////////////////////////////////////////////////////////// //

HSqlConnector_QtNativeSql::HSqlConnector_QtNativeSql(QString dbname)
{
    specifiedDialect = HSql::dialectForDatabaseName(dbname);
    if(!dbname.isEmpty())
        query = QSqlQuery(QSqlDatabase::database(dbname));

    errorStatusValue = false;
    errorStringValue = "";
}

HSqlConnector_QtNativeSql::~HSqlConnector_QtNativeSql(void)
{
    sdebug("HSqlConnector_QtNativeSql::~HSqlConnector_QtNativeSql -> delete");
}

QSqlQuery *HSqlConnector_QtNativeSql::q(void)
{
    return &query;
}

void HSqlConnector_QtNativeSql::exec(HSqlBuilder& request)
{
    errorStatusValue = false;
    errorStringValue = "";

    QString dialect = specifiedDialect;
    if(dialect.isEmpty())
        dialect = dialectFromDriverHandlerType(query.driver()->handle().typeName());
    sqldebug(QString("-- Local SQL command in %1 dialect -->").arg(dialect));
    QString sqlstring = request.local_cmd(true,false,dialect);
    sqldebug(sqlstring);
    query.prepare(sqlstring);

    QMap<QString,QVariant> ba = request.bind_array();
    QMapIterator<QString, QVariant> bi(ba);
    while(bi.hasNext())
    {
        bi.next();
        sqldebug(QString("%1 -> %2").arg(bi.key()).arg(bi.value().toString()));
        query.bindValue(bi.key(),bi.value());
    }

    query.exec();
    if(query.lastError().type() != QSqlError::NoError)
    {
        errorStatusValue = true;
        errorStringValue = query.lastError().text();
    }
}

bool HSqlConnector_QtNativeSql::errorStatus(void)
{
    return errorStatusValue;
}

QString HSqlConnector_QtNativeSql::errorMessage(void)
{
    return errorStringValue;
}

int HSqlConnector_QtNativeSql::recordCount(void)
{
    return query.numRowsAffected();
}

int HSqlConnector_QtNativeSql::columnCount(void)
{
    return query.record().count();
}

bool HSqlConnector_QtNativeSql::nextRecord(void)
{
    return query.next();
}

QList<QString> HSqlConnector_QtNativeSql::fieldNames(void)
{
    int cc = columnCount();
    QList<QString> fs;
    for(int i = 0 ; i < cc ; ++i)
        fs.push_back(query.record().fieldName(i));
    return fs;
}

HValue HSqlConnector_QtNativeSql::value(int index)
{
    if(query.record().count() < index + 1 )
        throw GSafeException("Called HSqlConnector_QtNativeSql::value with an index greater than record value count");
    return HValue(query.record().value(index));
}

HValue HSqlConnector_QtNativeSql::value(QString name)
{
    if(!query.record().contains(name))
        throw GSafeException("Called HSqlConnector_QtNativeSql::value with a not existing item name");
    return HValue(query.record().value(name));
}

HValue HSqlConnector_QtNativeSql::value(void)
{
    return HValue(value(0));
}

// ///////////////////////////////////////////////////////////////////////// //


HSqlConnector_HttpRest::HSqlConnector_HttpRest(HRestSqlDatabase& database)
 : db(database)
{
    resetRecord();
    errorStatusValue = false;
    errorStringValue = "";
}

HSqlConnector_HttpRest::~HSqlConnector_HttpRest(void)
{

}

void HSqlConnector_HttpRest::resetRecord(void)
{
    record_runner = -1;
}

void HSqlConnector_HttpRest::exec(HSqlBuilder& request)
{
    errorStatusValue = false;
    errorStringValue = "";

    resetRecord();
    parseAnswerTable(db.sendRequestJSON(request),request.query_field_list());
}

void HSqlConnector_HttpRest::parseAnswerTable(QString payload,QStringList fields)
{
    processed_rtype = 0;
    processed_fields.clear();
    processed_data.clear();

    if(payload.isEmpty())
        return;

    QJsonParseError jpe;
    QJsonDocument answer;
    answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);

    parseAnswerTable(answer,fields);
}

void HSqlConnector_HttpRest::parseAnswerTable(QJsonDocument parsedJson,QStringList fields)
{
    processed_rtype = 0;
    processed_fields.clear();
    processed_data.clear();

    QJsonObject jo = parsedJson.object();
    if(!parsedJson.isNull() && jo.value("status") == "Ok")
    {
        if(jo.value("rtype") == "none" )
        {
            processed_rtype = 1;
            processed_single = "";
            return;
        }
        if(jo.value("rtype") == "single" && !jo.value("return").isUndefined())
        {
            processed_rtype = 1;
            processed_single = jo.value("return");
            return;
        }

        if(jo.value("rtype") == "array" && jo.value("array").isArray())
        {
            processed_rtype = 2;
            QJsonArray rowarray = jo.value("array").toArray();

            int i,ra_c;
            ra_c = rowarray.count();
            QList<HValue> row;
            for(i=0;i<ra_c;++i)
            {
                row.clear();
                if(rowarray.at(i).isObject())
                {
                    QJsonObject recordObj = rowarray.at(i).toObject();
                    int ii,aac=fields.count();
                    for(ii=0;ii<aac;++ii)
                        row.push_back(HValue(recordObj.value(fields.at(ii))));
                }
                processed_data.push_back(row);
            }
            processed_fields = fields;
            return;
        }
    }
    errorStatusValue = true;
    errorStringValue =  QString("Error in parseAnswerTable.");
    ssdebug(errorStringValue,DCONSOLE_TYPE_TXTALT_RED);
}

bool HSqlConnector_HttpRest::errorStatus(void)
{
    return errorStatusValue;
}

QString HSqlConnector_HttpRest::errorMessage(void)
{
    return errorStringValue;
}

int HSqlConnector_HttpRest::recordCount(void)
{
    return processed_data.count();
}

int HSqlConnector_HttpRest::columnCount(void)
{
    return processed_fields.count();
}

bool HSqlConnector_HttpRest::nextRecord(void)
{
    if(processed_rtype == 0)
        return false;
    if(processed_rtype == 1)
    {
        if(record_runner < 0) //first run
        {
            record_runner = 0;
            return true;
        }

        if(record_runner > 0)
            return false;

        record_runner++;
        return false;
    }

    if(record_runner < 0) //first run
    {
        if(processed_data.count() > 0) //has data
        {
            record_runner = 0;
            return true;
        }
        return false;
    }

    record_runner++;
    if(record_runner < processed_data.count())
        return true;
    return false;
}

QList<QString> HSqlConnector_HttpRest::fieldNames()
{
    return processed_fields;
}

HValue HSqlConnector_HttpRest::value(int index)
{
    return processed_data.at(record_runner).at(index);
}

HValue HSqlConnector_HttpRest::value(QString name)
{
    int index = processed_fields.indexOf(name);
    return processed_data.at(record_runner).at(index);
}

HValue HSqlConnector_HttpRest::value(void)
{
    if(processed_rtype == 1)
        return processed_single;

    if(processed_data.count() > 0 && processed_fields.count() > 0)
        return HValue();
    return processed_data.at(0).at(0);
}

// ///////////////////////////////////////////////////////////////////////// //
//  HSql                                                                     //
// ///////////////////////////////////////////////////////////////////////// //

HSqlConnector* HSql::getConnector(void)
{
    if(overwrittenDbName.isEmpty() && overwrittenDefaultDbName.isEmpty())
        return new HSqlConnector_QtNativeSql();

    QString selected_database_name = overwrittenDbName;
    if(selected_database_name.isEmpty())
        selected_database_name = overwrittenDefaultDbName;

    if(HRestSqlDatabase::connectionNames().contains(selected_database_name))
        return new HSqlConnector_HttpRest(HRestSqlDatabase::database(selected_database_name));

    return new HSqlConnector_QtNativeSql(selected_database_name);
}

HSql::HSql(void)
{
    query_error_occured = false;
    overwrittenDbName = "";
}

HSql::HSql(QString databaseName)
{
    overwrittenDbName = databaseName;
}

HSql::~HSql(void)
{
    if(trans != 0) //nem ok
    {
        emit errorSignal("Error: I have some uncommitted/rollback-ed transaction!");
    }
}

HSql& HSql::db(QString databaseName)
{
    overwrittenDbName = databaseName;
    return *this;
}

void HSql::setDefaultDatabase(QString name)
{
    overwrittenDefaultDbName = name;
}

void HSql::unsetDefaultDatabase(void)
{
    overwrittenDefaultDbName = "";
}

void HSql::setDialectForDatabaseName(QString dbname,QString dialect)
{
    specifiedDialects[dbname] = dialect;
}

QString HSql::dialectForDatabaseName(QString dbname)
{
    if(specifiedDialects.contains(dbname))
        return specifiedDialects[dbname];
    return "";
}

void HSql::exec(HSqlBuilder& request,QString err,bool tdisabled)
{
    sdebug("*** HSql::exec ***");
    query_error_occured = false;
    if(!tdisabled)
        transaction();

    HSqlConnector *result = getConnector();
    request.setJsonExecutionMode(NoReturn);
    result->exec(request);
    if(result->errorStatus())
    {
        if(!tdisabled)
            rollback();

        query_error_occured = true;
        QString errTxt  = QString("%1 - %2")
                            .arg(err)
                            .arg(result->errorMessage());

        emit errorSignal(errTxt);
        sdebug("Error detected in HSql::exec, exiting...");
        sdebug(errTxt);
        sdebug("\\--> Exiting...");
        return;
    }

    if(!tdisabled)
        commit();

    delete result;
    sdebug("*** HSql::exec *** END");
}

HValue HSql::execSingle(HSqlBuilder& request,QString err,bool tdisabled)
{
    sdebug("*** HSql::execSingle ***");

    query_error_occured = false;
    if(!tdisabled)
        transaction();

    HSqlConnector *result = getConnector();
    request.setJsonExecutionMode(SingleReturn);
    result->exec(request);
    if(result->errorStatus())
    {
        if(!tdisabled)
            rollback();

        query_error_occured = true;
        QString errTxt = QString("%1 - %2")
                            .arg(err)
                            .arg(result->errorMessage());

        emit errorSignal(errTxt);
        sdebug("Error detected in HSql::execSingle, exiting...");
        sdebug(errTxt);
        sdebug("\\--> Exiting...");
        return HValue();
    }

    if(!tdisabled && !query_error_occured)
        commit();

    HValue retval = HValue();
    if(!result->nextRecord())
    {
        query_error_occured = true;
        emit errorSignal(QString("%1 - No result").arg(err));
    }
    else
    {
        retval = result->value();
    }

    delete result;
    sdebug("*** HSql::execSingle *** END");
    return retval;
}

HSqlConnector* HSql::execMultiUnsafe(HSqlBuilder& request,QString err,bool tdisabled)
{
    sdebug("*** HSql::execMultiUnsafe ***");
    query_error_occured = false;
    if(!tdisabled)
        transaction();

    HSqlConnector *result = getConnector();
    request.setJsonExecutionMode(TableReturn);
    result->exec(request);
    if(result->errorStatus())
    {
        if(!tdisabled)
            rollback();

        query_error_occured = true;
        QString errTxt = QString("%1 - %2")
                            .arg(err)
                            .arg(result->errorMessage());

        emit errorSignal(errTxt);
        sdebug("Error detected in HSql::execMultiUnsafe:");
        sdebug(errTxt);
        sdebug("\\--> Exiting...");
        return result;
    }

    if(!tdisabled)
        commit();

    sdebug("*** HSql::execMultiUnsafe *** END");
    return result;
}

QSharedPointer<HSqlConnector> HSql::execMulti(HSqlBuilder& request,QString err,bool tdisabled)
{
    return QSharedPointer<HSqlConnector>(execMultiUnsafe(request,err,tdisabled));
}

void HSql::execFillDataMtrx(HSqlBuilder& request,HDataMatrix* dm,QString err,bool tdisabled)
{
    if(dm == NULL)
        return;
    HSqlConnector* result = execMultiUnsafe(request,err,tdisabled);
    QList<HValue> recordData;

    int r = 0;
    int cc = 0;
    while(result->nextRecord())
    {
        recordData.clear();
        cc = result->columnCount();
        if(r == 0)
            dm->setHeader(result->fieldNames());
        for(int i = 0 ; i < cc; ++i)
            recordData.push_back( result->value(i) );
        dm->addRow(recordData);
        ++r;
    }
    dm->sendDataChanged();
    delete result;
}

HDataMatrix* HSql::execDataMtrxCommonUnsafe(HSqlBuilder& request,bool noe,QString err,bool tdisabled)
{
    HDataMatrix* dm = NULL;
    HSqlConnector* result = execMultiUnsafe(request,err,tdisabled);
    QList<HValue> recordData;

    int r = 0;
    int cc = 0;
    if(!noe)
        dm = new HDataMatrix();
    while(result->nextRecord())
    {
        recordData.clear();
        cc = result->columnCount();
        if(r == 0)
        {
            if(noe)
                dm = new HDataMatrix();
            dm->setHeader(result->fieldNames());
        }
        for(int i=0 ; i < cc; ++i)
            recordData.push_back( result->value(i) );
        dm->addRow(recordData);
        ++r;
    }

    delete result;
    return dm;
}

HDataMatrix* HSql::execDataMtrxUnsafe(HSqlBuilder& request,QString err,bool tdisabled)
{
    return execDataMtrxCommonUnsafe(request,false,err,tdisabled);
}

HDataMatrix* HSql::execDataMtrxNoeUnsafe(HSqlBuilder& request,QString err,bool tdisabled)
{
    return execDataMtrxCommonUnsafe(request,true,err,tdisabled);
}

QSharedPointer<HDataMatrix> HSql::execDataMtrx(HSqlBuilder& request,QString err,bool tdisabled)
{
    return QSharedPointer<HDataMatrix>(execDataMtrxUnsafe(request,err,tdisabled));
}

QSharedPointer<HDataMatrix> HSql::execDataMtrxNoe(HSqlBuilder& request,QString err,bool tdisabled)
{
    return QSharedPointer<HDataMatrix>(execDataMtrxNoeUnsafe(request,err,tdisabled));
}

// //////////////////////////////////////////////////////////////////////////////// //
void HSql::transaction(void)
{
    trans++;
    sdebug(QString("Begin transaction: %1").arg(trans));

    QSqlDatabase db = QSqlDatabase::database();

    if(!overwrittenDbName.isEmpty() || !overwrittenDefaultDbName.isEmpty())
    {
        QString selected_database_name = overwrittenDbName;
        if(selected_database_name.isEmpty())
            selected_database_name = overwrittenDefaultDbName;

        if(HRestSqlDatabase::connectionNames().contains(selected_database_name))
        {
            if(HRestSqlDatabase::database(selected_database_name).transaction())
                return;
            emit errorSignal("Cannot start the TRANSACTION! (Rest)");
        }

        db = QSqlDatabase::database(selected_database_name);
    }

    if(db.isOpen() && db.isValid())
    {
        if(db.transaction())
            return;
        emit errorSignal("Cannot start the TRANSACTION!");
        return;
    }
    sdebug("HSqlHandler transaction() : There is no active open sql database which support transactions!");
}

void HSql::commit(void)
{
    if(trans<=0)
        emit errorSignal("Warning: Called commit without begin a transaction!");

    sdebug(QString("Commit transaction: %1").arg(trans));
    trans--;

    QSqlDatabase db = QSqlDatabase::database();

    if(!overwrittenDbName.isEmpty() || !overwrittenDefaultDbName.isEmpty())
    {
        QString selected_database_name = overwrittenDbName;
        if(selected_database_name.isEmpty())
            selected_database_name = overwrittenDefaultDbName;

        if(HRestSqlDatabase::connectionNames().contains(selected_database_name))
        {
            if(HRestSqlDatabase::database(selected_database_name).commit())
                return;
            emit errorSignal("Cannot COMMIT the SQL transaction! (Rest)");
        }

        db = QSqlDatabase::database(selected_database_name);
    }

    if(db.isOpen() && db.isValid())
    {
        if(db.commit())
            return;
        emit errorSignal("Cannot COMMIT the SQL transaction!");
        return;
    }
    sdebug("HSqlHandler commit() : There is no active open sql database which support transactions!");
}

void HSql::rollback(void)
{

    if(trans<=0)
        emit errorSignal("BIG Warning: Called rollback without begin a transaction!");

    sdebug(QString("Rollback transaction: %1").arg(trans));
    trans--;

    QSqlDatabase db = QSqlDatabase::database();

    if(!overwrittenDbName.isEmpty() || !overwrittenDefaultDbName.isEmpty())
    {
        QString selected_database_name = overwrittenDbName;
        if(selected_database_name.isEmpty())
            selected_database_name = overwrittenDefaultDbName;

        if(HRestSqlDatabase::connectionNames().contains(selected_database_name))
        {
            if(HRestSqlDatabase::database(selected_database_name).rollback())
                return;
            emit errorSignal("Cannot ROLLBACK the SQL transaction! (Rest)");
        }

        db = QSqlDatabase::database(selected_database_name);
    }

    if(db.isOpen() && db.isValid())
    {
        if(db.rollback())
            return;
        emit errorSignal("Cannot ROLLBACK the SQL transaction!");
        return;
    }
    sdebug("HSqlHandler rollback() : There is no active open sql database which support transactions!");
}

QString dialectFromDriverHandlerType(QString dht)
{
    if(dht == "sqlite3*") return "sqlite";
    return "nochange";
}

//End of gSAFE database.cpp
