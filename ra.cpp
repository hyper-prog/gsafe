/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   ra.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore>
#include <QtSql>

#include "ra.h"
#include "dconsole.h"
#include "builder.h"


HRefreshAgent* HRefreshAgent::theone = NULL;

/////////////////////////////////////////////////////////////////////////////////
////// HNotifyAgent members /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HRefreshAgent::HRefreshAgent(void)
{
    if(theone != NULL)
    {
        QMessageBox::warning(NULL,"Error","Error: The \"HRefreshAgent\" object must be only one instance!");
        return;
    }
    sdebug("HRefreshAgent initialized...");
    theone = this;
}

HRefreshAgent::~HRefreshAgent(void)
{
    theone = NULL;
}

void HRefreshAgent::notify(QString tblname)
{
    if(theone == NULL)
    {
        QMessageBox::warning(NULL,"Error","Critical Error: The \"HRefreshAgent\"  class is uninitialized!");
        return;
    }

    theone->internalNotify(tblname);
}

int HRefreshAgent::notifySlot(QString tblname)
{
    theone->internalNotify(tblname);
    return 0;
}

void HRefreshAgent::internalNotify(QString tblname)
{
    sdebug("*** HRefreshAgent::internalNotify ***");

    emit getnotify(tblname);
}

/////////////////////////////////////////////////////////////////////////////////
////// HRefreshAgentNetserver members ///////////////////////////////////////////

#ifdef MODULE_REFRESHAGENT_NETSERVER

bool HRefreshAgentNetserver::inNotify = false;
HRefreshAgentNetserver * HRefreshAgentNetserver::theone = NULL;

HRefreshAgentNetserver::HRefreshAgentNetserver(void)
{
    tcpServer = NULL;
    if(theone != NULL)
    {
        QMessageBox::warning(NULL,"Error","Error: The \"HRefreshAgentNetserver\" object must be only one instance!");
        return;
    }
    sdebug("[HRefreshAgentNetserver] Initializing...");
    theone = this;

    //create a tcpserver
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any,REFRESHAGENT_TCPPORT))
    {
        QMessageBox::warning(NULL,"Error",QString("Unable to start tcpserver: %1.")
                                             .arg(tcpServer->errorString()));
    }

    connect(tcpServer,SIGNAL(newConnection()),this,SLOT(newConn()));
    sdebug(QString("[HRefreshAgentNetserver] Listening on port %1...").arg(tcpServer->serverPort()));
    laddr = "No ip";
}

bool HRefreshAgentNetserver::isRunning(void)
{
    if(theone->tcpServer == NULL || !theone->tcpServer->isListening())
        return false;
    return true;
}

QString HRefreshAgentNetserver::serverAddress()
{
    if(theone->tcpServer != NULL && theone->tcpServer->isListening())
    {
        return theone->laddr;
    }
    return QString("Not listening");
}

int HRefreshAgentNetserver::newConn(void)
{
    sdebug("*** HRefreshAgentNetserver::newConn ***");

    QTcpSocket  *socket;
    QByteArray  block;
    QTextStream inout(&block,QIODevice::ReadWrite);

    if(!tcpServer->hasPendingConnections())
    {
        sdebug("*** HRefreshAgentNetserver::newConn *** END ERROR");
        return 1;
    }

    socket = tcpServer->nextPendingConnection();

    laddr = socket->localAddress().toString();

    inout << "ConnectOk" << Qt::endl;
    socket->write(block);
    connect(socket,SIGNAL(readyRead()),this,SLOT(request()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(clientLeaving()));

    tcps_list.push_back(socket);

    sdebug(QString("[HRefreshAgentNetserver] Client connected %1... (have %2 client)")
        .arg(socket->peerAddress().toString())
        .arg(tcps_list.size()));

    sdebug("*** HRefreshAgentNetserver::newConn *** END");
    return 0;
}

int HRefreshAgentNetserver::request(void)
{
    int num,sender;
    QString str;
    sdebug("*** HRefreshAgentNetserver::request ***");
    QTcpSocket *socket;

    QByteArray  block;
    QTextStream inout(&block,QIODevice::ReadWrite);

    QList<QTcpSocket *>::iterator   is;

    str = "";
    is  = tcps_list.begin();
    sender = -1;
    num = 0;
    while(is  != tcps_list.end())
    {
        socket = *is;
        if(socket != NULL && socket->isValid())
            if(socket->state() == QAbstractSocket::ConnectedState)
                if(socket->bytesAvailable() != 0)
                {
                    block  = socket->readAll();
                    inout >> str;
                    sdebug(QString("[HRefreshAgentNetserver] Data arrived: \"%1\" from %2 !")
                            .arg(str)
                            .arg(socket->peerAddress().toString()));
                    sender = num;
                    if(!str.isEmpty() && (str == "close" ||
                                          str == "quit"  ||
                                          str == "exit"  ||
                                          str == "disconnect"))
                    {
                        socket->flush();
                        socket->disconnectFromHost();
                        str = "";
                        return 0;
                    }
                }
        ++is;
        ++num;
    }

    if(!str.isEmpty() && str.startsWith("<") && str.endsWith(">") )
    {
        int length = str.length();
        str = str.mid(1,length-2);
        action(str,sender);
    }

    sdebug("*** HRefreshAgentNetserver::request *** END");
    return 0;
}

int HRefreshAgentNetserver::action(QString name,int sendernum)
{
    int num;
    QByteArray  block;
    QTextStream inout(&block,QIODevice::ReadWrite);

    if(inNotify)
        return 0;
    inNotify = true;

    sdebug("[HRefreshAgentNetserver] NetServer Notify: "+name);

    inout << "<" << name << ">" << Qt::endl;
    num = 0;
    QList<QTcpSocket *>::iterator  is;
    is  = tcps_list.begin();
    while(is  != tcps_list.end())
    {
        if((*is)->state() == QAbstractSocket::ConnectedState)
            if(num != sendernum)
                (*is)->write(block);
        ++is;
        ++num;
    }
    inNotify = false;
    return 0;
}

int HRefreshAgentNetserver::clientLeaving(void)
{
    QList<QTcpSocket *>::iterator   is;
    is  = tcps_list.begin();
    while(is  != tcps_list.end())
    {
        if((*is)->state() == QAbstractSocket::UnconnectedState ||
            (*is)->state() == QAbstractSocket::ClosingState         )
        {
            tcps_list.removeAll(*is);
            sdebug(QString("[HRefreshAgentNetserver] Client leaving... (have %1 client)").arg(tcps_list.size()));
            return 0;
        }
        ++is;
    }
    return 0;
}

HRefreshAgentNetserver::~HRefreshAgentNetserver(void)
{
    QList<QTcpSocket *>::iterator   is;

    is  = tcps_list.begin();
    while(is  != tcps_list.end())
    {
        (*is)->close();
        ++is;
    }
    tcps_list.clear();

    tcpServer->close();
    delete tcpServer;

    theone = NULL;
}

#endif

/////////////////////////////////////////////////////////////////////////////////
////// HRefreshAgentNetclient members ///////////////////////////////////////////

#ifdef MODULE_REFRESHAGENT_NETCLIENT

bool HRefreshAgentNetclient::inNotify = false;
HRefreshAgentNetclient * HRefreshAgentNetclient::theone = NULL;

HRefreshAgentNetclient::HRefreshAgentNetclient(QString server_ip)
{
    if(theone != NULL)
    {
        QMessageBox::warning(NULL,"Error","Error: The \"HRefreshAgentNetclient\" object must be only one instance!");
        return;
    }
    sdebug("HRefreshAgentNetclient initialized...");
    theone = this;

    //code here...
    socket = new QTcpSocket(this);
    socket->connectToHost(server_ip,REFRESHAGENT_TCPPORT,QIODevice::ReadWrite);

    connect(socket,SIGNAL(readyRead()),this,SLOT(request()));
    connect(socket,SIGNAL(disconnected()),this,SLOT(serverClosing()));
    connect(HRefreshAgent::getNotifyAgent(),SIGNAL(getnotify(QString)),this,SLOT(action(QString)));
    c_ok = false;
}

bool HRefreshAgentNetclient::isConnected(void)
{
    if(theone->socket->state() == QAbstractSocket::ConnectedState && theone->c_ok)
        return true;
    return false;
}

HRefreshAgentNetclient::~HRefreshAgentNetclient()
{
    socket->close();
    delete socket;
    c_ok = false;
    theone = NULL;
}

int HRefreshAgentNetclient::request(void)
{
    QString str;

    QByteArray  block;
    QTextStream inout(&block,QIODevice::ReadWrite);

    str = "";
    if(socket->state() == QAbstractSocket::ConnectedState && socket->bytesAvailable() != 0)
    {
        block  = socket->readAll();
        inout >> str;
        sdebug(QString("[HRefreshAgentNetclient] Data arrived: \"%1\" ").arg(str));
        if(!c_ok && str == "ConnectOk")
        {
            sdebug(QString("[HRefreshAgentNetclient] Connection established."));
            c_ok = true;
            str = "";
            emit connectionEstablished();
        }
    }

    if(c_ok && !str.isEmpty() && str.startsWith("<") && str.endsWith(">"))
    {
        int length = str.length();
        str = str.mid(1,length-2);
        sdebug(QString("[HRefreshAgentNetclient] HRefreshAgent->notify: \"%1\"").arg(str));
        inNotify = true;
        HRefreshAgent::notify(str);
        inNotify = false;
    }
    return 0;
}

int HRefreshAgentNetclient::serverClosing(void)
{
    socket->close();
    c_ok = false;
    emit connectionLost();
    sdebug("[HRefreshAgentNetclient] The server closed the connection.");
    return 0;
}

int HRefreshAgentNetclient::action(QString name)
{
    QByteArray  block;
    QTextStream inout(&block,QIODevice::ReadWrite);

    if(!c_ok)
        return 0;

    if(inNotify)
        return 0;
    inNotify = true;

    sdebug("[HRefreshAgentNetserver] Send Notify: "+name);

    inout << "<" << name << ">" << Qt::endl;

    if(socket->state() == QAbstractSocket::ConnectedState)
        socket->write(block);

    inNotify = false;
    return 0;
}

#endif

//End of gSAFE ra.cpp
