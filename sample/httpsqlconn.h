/*
  HttpSqlConnection - Sample code for HSqlBuilder

  (C) 2021 Péter Deák  (hyper80@gmail.com)

     License: GPL
*/
#ifndef HTTPSQLCONN_HEADER
#define HTTPSQLCONN_HEADER

#include <QtCore>

#include <datalib.h>
#include <builder.h>

class HttpSqlConnection : public QObject
{
    Q_OBJECT

public:
    HttpSqlConnection();
    virtual ~HttpSqlConnection();

    void sendReqNone(HSqlBuilder b,bool *ok = NULL);
    QVariant sendReqSingle(HSqlBuilder b,bool *ok = NULL);
    HPlainDataMatrix * sendReqAll(HSqlBuilder b,bool *ok = NULL);
    bool sendCheckFieldsExistsInTable(QString tablename,QStringList fields);

    static bool configure(QString serverBaseUrl,QString resourceName,QString fastId);

    QString sendRawRequest(QString data);
    QString lastErrorMessage(void) { return lastErrorMessagePrivate; };

public slots:
    int sslErrorHandler(const QList<QSslError>& errors);

protected:
    QString last_payload;

    HPlainDataMatrix *parseAnswerTable(QString payload,QStringList fields,bool *ok);
    QVariant parseAnswerSingle(QString payload,bool *ok);
    void parseAnswerNone(QString payload,bool *ok);

    QString mapToJsonPart(QMap<QString,QString> m);
    QString strlistToJsonArray(QStringList sl);
    int restAnswerReceived(QNetworkReply* restNetworkActionReply);
    QByteArray processRequest(QString data);
    QString processResponse(QByteArray response);

private:
    QString lastErrorMessagePrivate;

    static QString server_baseurl;
    static QString server_resource_name;
    static QString server_fast_id;
};

#endif // HTTPSQLCONN_HEADER
