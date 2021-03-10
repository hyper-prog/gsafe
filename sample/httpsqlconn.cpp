/*
   HttpSqlConnection - Sample code for HSqlBuilder

   (C) 2021 Peter Deak  (hyper80@gmail.com)

        License: GPL
*/
#include <QtCore>
#include <QtSql>
#include <QtGui>

#include <datalib.h>
#include <dconsole.h>
#include <builder.h>

#include "httpsqlconn.h"

QString HttpSqlConnection::server_baseurl;
QString HttpSqlConnection::server_resource_name;
QString HttpSqlConnection::server_fast_id;

#define REQUEST_TIMEOUT 10

HttpSqlConnection::HttpSqlConnection()
{
    lastErrorMessagePrivate = "";
}

HttpSqlConnection::~HttpSqlConnection()
{
}

bool HttpSqlConnection::configure(QString serverBaseUrl,QString resourceName,QString fastId)
{
    server_baseurl = serverBaseUrl;
    server_resource_name = resourceName;
    server_fast_id = fastId;
    return false;
}

QString HttpSqlConnection::mapToJsonPart(QMap<QString,QString> m)
{
    int i;
    QString jsp="{";
    QMapIterator<QString,QString> it(m);
    for(i=0;it.hasNext();)
    {
        it.next();
        jsp.append(i==0?"":",");
        QString k = it.key();
        QString v =it.value();
        jsp.append(QString("\"%1\": \"%2\"")
                  .arg(k)
                  .arg(v));
        ++i;
    }
    jsp.append("}");
    return jsp;
}

QString HttpSqlConnection::strlistToJsonArray(QStringList sl)
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

HPlainDataMatrix * HttpSqlConnection::sendReqAll(HSqlBuilder b,bool *ok)
{
    QString rd;
    b.setJsonExecutionMode(TableReturn);
    rd = QString("{\"reqId\": \"query_uni\", \"query_spec\": %1}")
                .arg(b.json_string());
    QString payload = sendRawRequest(rd);
    return parseAnswerTable(payload,b.query_field_list(),ok);
}

QString HttpSqlConnection::sendReqSingle(HSqlBuilder b,bool *ok)
{
    QString rd;
    b.setJsonExecutionMode(SingleReturn);
    rd = QString("{\"reqId\": \"query_uni\", \"query_spec\": %1}")
                .arg(b.json_string());
    QString payload = sendRawRequest(rd);
    return parseAnswerSingle(payload,ok);
}

void HttpSqlConnection::sendReqNone(HSqlBuilder b,bool *ok)
{
    QString rd;
    b.setJsonExecutionMode(NoReturn);
    rd = QString("{\"reqId\": \"query_uni\", \"query_spec\": %1}")
                .arg(b.json_string());
    QString payload = sendRawRequest(rd);
    parseAnswerNone(payload,ok);
}

HPlainDataMatrix *HttpSqlConnection::parseAnswerTable(QString payload,QStringList fields,bool *ok)
{
    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    QJsonObject jo = answer.object();
    if(!answer.isNull() && jo.value("status") == "Ok")
    {
        if(ok != NULL)
            *ok = false;
        HPlainDataMatrix *result = new HPlainDataMatrix(fields.count());
        if(!jo.value("array").isArray())
             return NULL;
        QJsonArray rowarray = jo.value("array").toArray();
        int i,ra_c;
        ra_c = rowarray.count();
        QList<QVariant> row;
        for(i=0;i<ra_c;++i)
        {
            row.clear();
            if(rowarray.at(i).isObject())
            {
                QJsonObject recordObj = rowarray.at(i).toObject();
                int ii,aac=fields.count();
                for(ii=0;ii<aac;++ii)
                    row.push_back(recordObj.value(fields.at(ii)).toString());
            }
            result->addRow(row);
        }
        result->setHeader(fields);
        if(ok != NULL)
            *ok = true;
        return result;
    }
    lastErrorMessagePrivate = QString("Error in parseAnswerTable.");
    ssdebug(lastErrorMessagePrivate,DCONSOLE_TYPE_TXTALT_RED);
    if(ok != NULL)
        *ok = false;
    return NULL;
}

QString HttpSqlConnection::parseAnswerSingle(QString payload,bool *ok)
{
    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    QJsonObject jo = answer.object();
    if(!answer.isNull() && jo.value("status") == "Ok")
    {
        if(ok != NULL)
            *ok = false;
        if(jo.value("return").isNull())
            return QString();
        if(ok != NULL)
            *ok = true;
        return jo.value("return").toString();
    }
    lastErrorMessagePrivate = QString("Error in parseAnswerSingle.");
    ssdebug(lastErrorMessagePrivate,DCONSOLE_TYPE_TXTALT_RED);
    if(ok != NULL)
        *ok = false;
    return QString();
}

void HttpSqlConnection::parseAnswerNone(QString payload,bool *ok)
{
    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    QJsonObject jo = answer.object();
    if(!answer.isNull() && jo.value("status") == "Ok")
    {
        if(ok != NULL)
            *ok = true;
        return;
    }
    lastErrorMessagePrivate = QString("Error in parseAnswerNone.");
    ssdebug(lastErrorMessagePrivate,DCONSOLE_TYPE_TXTALT_RED);
    ssdebug(payload,DCONSOLE_TYPE_TXTALT_CYAN);
    if(ok != NULL)
        *ok = false;
}

bool HttpSqlConnection::sendCheckFieldsExistsInTable(QString tablename,QStringList fields)
{
    QString rd;
    rd = QString("{\"reqId\": \"check_fields_exists\",\"tablename\": \"%1\",\"fields\": %2}")
                .arg(tablename).arg(strlistToJsonArray(fields));
    QString payload = sendRawRequest(rd);
    QJsonParseError jpe;
    QJsonDocument answer = QJsonDocument::fromJson(payload.toUtf8(),&jpe);
    QJsonObject jo = answer.object();
    if(!answer.isNull() && jo.value("status") == "Ok")
        return true;
    return false;
}

QByteArray HttpSqlConnection::processRequest(QString data)
{
    return data.toUtf8();
}

QString HttpSqlConnection::processResponse(QByteArray response)
{
    return QString::fromUtf8(response);
}

QString HttpSqlConnection::sendRawRequest(QString data)
{
    QNetworkAccessManager networkManager;
    lastErrorMessagePrivate = "";
    QUrl url;
    sdebug(QString("URL => %1").arg(server_baseurl + "/httpsqlconn/" + server_resource_name + "/<FastID>" ));
    url.setUrl(server_baseurl + "/httpsqlconn/" + server_resource_name +"/"+server_fast_id);
    QNetworkRequest req;
    req.setUrl(url);
    req.setRawHeader("Content-Type","application/json");
    req.setRawHeader("Accept","application/json");

    QSslConfiguration conf = req.sslConfiguration();
    conf.setPeerVerifyMode(QSslSocket::VerifyPeer);
    req.setSslConfiguration(conf);

    QByteArray sendraw = processRequest(data);

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
    if(restAnswerReceived(restNetworkActionReply) != 0)
        return QString();
    return last_payload;
}

int HttpSqlConnection::sslErrorHandler(const QList<QSslError>& errors)
{
    int n=0;
    QList<QSslError>::const_iterator i;
    for(i=errors.constBegin();i != errors.constEnd();++i)
        sdebug(QString("SSL Error %1: %2<br/>").arg(n++).arg(i->errorString()));
    return 0;
}

int HttpSqlConnection::restAnswerReceived(QNetworkReply* restNetworkActionReply)
{
    QString last_result;
    if(restNetworkActionReply->isFinished() && !(last_result = restNetworkActionReply->readAll()).isEmpty() )
    {
        int rest_received_errorcode;
        if((rest_received_errorcode = restNetworkActionReply->error()) != QNetworkReply::NoError)
        {
            sdebug(QString("The processing of REST request is failed.\nReceived error code: %1")
                   .arg(rest_received_errorcode));
            return 1;
        }
        last_payload = processResponse(last_result.toUtf8());
    }
    else
    {
        sdebug("The REST request does not return any data.");
        return 1;
    }
    return 0;
}
