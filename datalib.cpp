/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2014 Peter Deak  (hyper80@gmail.com)

    License: GPLv2  http://www.gnu.org/licenses/gpl-2.0.html

    datalib.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore>
#include <QtSql>

#include "datalib.h"
#include "dconsole.h"

#define COL_NONSQL 0
#define COL_SHOW   1
#define COL_KEY    2
#define COL_SERIAL 3

QString errorLocal="Error";

HRefreshAgent* HRefreshAgent::theone = NULL;

void setErrorLocal(QString s)
{
    errorLocal = s;
}

void error(QString s)
{
    dconsole_popup(errorLocal+" !",s);
}

int getIndexOf(QStringList l,QString s)
{
    int pos=0;
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
    {
        if(*it == s)
            return pos;
        ++pos;
    }
    return 0;
}

QStringList * deep_copy_stringlist(QStringList *target,QStringList *source)
{
    target->clear();
    QStringList::Iterator i = source->begin();
    while(i != source->end())
        target->push_back(*i);
    return target;
}

QString convNationalToHtmlCodes(QString input)
{
    QString r = input;
/*
    r.replace("á","&aacute;",Qt::CaseSensitive);
    r.replace("é","&eacute;",Qt::CaseSensitive);
    r.replace("í","&iacute;",Qt::CaseSensitive);
    r.replace("ó","&oacute;",Qt::CaseSensitive);
    r.replace("ö","&ouml;",Qt::CaseSensitive);
    r.replace("õ","&otilde;",Qt::CaseSensitive);
    r.replace("ú","&uacute;",Qt::CaseSensitive);
    r.replace("ü","&uuml;",Qt::CaseSensitive);
    r.replace("û","&utilde;",Qt::CaseSensitive);

    r.replace("Á","&Aacute;",Qt::CaseSensitive);
    r.replace("É","&Eacute;",Qt::CaseSensitive);
    r.replace("Í","&Iacute;",Qt::CaseSensitive);
    r.replace("Ó","&Oacute;",Qt::CaseSensitive);
    r.replace("Ö","&Ouml;",Qt::CaseSensitive);
    r.replace("Õ","&Otilde;",Qt::CaseSensitive);
    r.replace("Ú","&Uacute;",Qt::CaseSensitive);
    r.replace("Ü","&Uuml;",Qt::CaseSensitive);
    r.replace("Û","&Utilde;",Qt::CaseSensitive);
*/
    r.replace("á","&#0225;",Qt::CaseSensitive);
    r.replace("é","&#0233;",Qt::CaseSensitive);
    r.replace("í","&#0237;",Qt::CaseSensitive);
    r.replace("ó","&#0243;",Qt::CaseSensitive);
    r.replace("ö","&#0246;",Qt::CaseSensitive);
    r.replace("õ","&#0337;",Qt::CaseSensitive);
    r.replace("ú","&#0250;",Qt::CaseSensitive);
    r.replace("ü","&#0252;",Qt::CaseSensitive);
    r.replace("û","&#0369;",Qt::CaseSensitive);

    r.replace("Á","&#0193;",Qt::CaseSensitive);
    r.replace("É","&#0201;",Qt::CaseSensitive);
    r.replace("Í","&#0205;",Qt::CaseSensitive);
    r.replace("Ó","&#0211;",Qt::CaseSensitive);
    r.replace("Ö","&#0214;",Qt::CaseSensitive);
    r.replace("Õ","&#0336;",Qt::CaseSensitive);
    r.replace("Ú","&#0218;",Qt::CaseSensitive);
    r.replace("Ü","&#0220;",Qt::CaseSensitive);
    r.replace("Û","&#0368;",Qt::CaseSensitive);
    return r;
}

int my_dtoa(double v,char *buffer,int bufflen,int min,int max,int group)
{
    int digitnum;
    int i,forlength;
    int length=0; //the currnt filled length of the buffer

    char digit;
    char *str = buffer;

    unsigned long int i_ip,i_fp,idigit_value;
    double ip,fp;

    bufflen -= 2; //decrease bufflen value, to avoid decreasing in every if

    if(isnan(v))
    {
        if(bufflen < 4)
            return 1;
        strcpy(str,"NaN");
        return 0;
    }
    if(isinf(v))
    {
        if(bufflen < 4)
            return 1;
        strcpy(str,"Inf");
        return 0;
    }

    //split the number to integer and fractional part.
    fp = fabs(modf(v,&ip));
    ip = fabs(ip);
    if(fp != 0.0)
    {
        fp *= pow(10.0,max);
        fp = floor(fp + 0.5);
    }
    i_ip=ip;
    i_fp=fp;

    //If the original (rounded) number is negative put the sign to front
    v *= pow(10.0,max);
    v = floor(v + 0.5);
    if (v < 0)
    {
        *(str++) = '-';
        ++length;
        v = -v;
    }

    //Generate integer part (from i_ip)
    idigit_value = 1;
    digitnum = 1;
    while(idigit_value*10 <= i_ip)
    {
        idigit_value *= 10;
        ++digitnum;
    }
    forlength=0;
    while(idigit_value >= 1)
    {
        //put grouping space if set
        if(group && forlength != 0 && digitnum % 3 == 0)
        {
            *(str++) = ' ';
            ++length;
            if(length >= bufflen)
            {
                *(str) = '\0';
                return 1;
            }
        }

        digit = static_cast<char>((i_ip - i_ip%idigit_value) / idigit_value);
        i_ip = i_ip%idigit_value;

        *(str++) = '0' + digit%10;
        ++length;
        --digitnum;
        ++forlength;
        idigit_value /= 10;

        if(length >= bufflen)
        {
            *(str) = '\0';
            return 1;
        }
    }

    //Generate fractional part (from i_fp)
    digitnum=0;
    if( i_fp > 0 )
    {
        *(str++) = '.';
        ++length;

        idigit_value = 1;
        for(i=0;i<max-1;++i)
            idigit_value *= 10;

        while (idigit_value >= 1)
        {
            if(group && digitnum && digitnum%3 == 0)
            {
                *(str++) = ' ';
                ++length;
                if(length >= bufflen)
                {
                    *(str) = '\0';
                    return 1;
                }
            }

            digit = static_cast<char>((i_fp - i_fp%idigit_value) / idigit_value);
            i_fp = i_fp%idigit_value;

            *(str++) = '0' + digit%10;
            ++length;
            ++digitnum;
            idigit_value /= 10;

            if(length >= bufflen)
            {
                *(str) = '\0';
                return 1;
            }

            if(digitnum >= min && i_fp == 0)
                break;
        }
    }
    else
    {   //the original number was an integer, so we fill the minimal fractional part with zeros
        if(min > 0)
        {
            *(str++) = '.';
            ++length;
            for(digitnum=0;digitnum<min;)
            {
                if(group && digitnum && digitnum%3 == 0)
                {
                    *(str++) = ' ';
                    ++length;
                    if(length >= bufflen)
                    {
                        *(str) = '\0';
                        return 1;
                    }
                }
                *(str++) = '0';
                ++length;
                ++digitnum;
                if(length >= bufflen)
                {
                    *(str) = '\0';
                    return 1;
                }
            }
        }
    }
    *str = '\0';
    return 0;
}

QString doubleToQString(double val,int min,int max,int group)
{

    //return QVariant(QString("%1%2").arg(v.toDouble(),0,'f',2).arg(tailstr.isEmpty() ? "" : (" "+tailstr)));
    //return QVariant(QString("").sprintf("%.2f",v.toDouble()) + (tailstr.isEmpty() ? "" : (" "+tailstr)));

    QString v;
    char buffer[128];
    my_dtoa(val,buffer,128,min,max,group);
    v = buffer;
    return v;
}

double dRound(double val,int mode)
{
    double intpart = floor(val);
    if(mode == ROUND_FLOOR)
        return intpart;
    if( (val-intpart) >= 0.5 )
        return (intpart + 1.0);
    return intpart;
}

/////////////////////////////////////////////////////////////////////////////////
////// HXmlWriter members ///////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HXmlWriter::HXmlWriter(void)
{
    indent_pos    = 0;
    output_stream = NULL;
    node_stack    = new QStack<QString>();
    codec_name    = "";
    tc            = NULL;
    no_recode     = true;
}

HXmlWriter::~HXmlWriter(void)
{
    output_stream = NULL;
    delete node_stack;
}

QTextStream * HXmlWriter::getStream(void)
{
    return output_stream;
}

void HXmlWriter::setStream(QTextStream *oo)
{
    output_stream = oo;
}

void HXmlWriter::putHead(QString enc,bool nullhead)
{
    QString visible_enc=enc;

    if(!enc.isEmpty())
    {
        if(enc == "CP1250") visible_enc="Windows-1250";
        if(enc == "UTF-8")  visible_enc="UTF-8";

        if(!nullhead)
        {
            (*output_stream) << "<?xml version=\"1.0\" encoding=\"" + visible_enc + "\"?>\r\n";
        }
        codec_name = enc;
        tc = QTextCodec::codecForName(codec_name.toLocal8Bit().constData());
        no_recode     = false;
    }

}

void HXmlWriter::eatData(QString d)
{
    if(no_recode)
        (*output_stream) << d;
    else
        (*output_stream) << recodeData(d);
}

void HXmlWriter::beginNode(QString n,QString parameters)
{
    if(!parameters.isEmpty())
        parameters.prepend(" ");
    eatData( QString().fill('\t',indent_pos++) + "<" + n + parameters + ">\r\n" );
    node_stack->push(n);
}

void HXmlWriter::endNode(void)
{
    eatData( QString().fill('\t',--indent_pos) + "</" + node_stack->pop() + ">\r\n" );
}

void HXmlWriter::putCData(QString n,QString data,QString parameters)
{
    if(!parameters.isEmpty())
        parameters.prepend(" ");

    eatData( QString().fill('\t',indent_pos) + "<" + n + parameters + ">" + data + "</" + n + ">\r\n");
}

void HXmlWriter::putEmptyNode(QString n,QString parameters)
{
    if(!parameters.isEmpty())
        parameters.prepend(" ");

    eatData( QString().fill('\t',indent_pos) + "<" + n + parameters +"/>\r\n" );
}

void HXmlWriter::putRawData(QString data)
{
    eatData( QString().fill('\t',indent_pos) + recodeDataNoconvert(data) );
}

QByteArray HXmlWriter::recodeData(QString d)
{
    return recodeDataNoconvert(d.replace("&","&amp;"));
}

QByteArray HXmlWriter::recodeDataNoconvert(QString d)
{
    QByteArray rv;

    if(no_recode)
        rv = d.toLocal8Bit();
    else
        rv = tc->fromUnicode(d);
    return rv;
}

/////////////////////////////////////////////////////////////////////////////////
////// HNotifyAgent members /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HRefreshAgent::HRefreshAgent(void)
{
    if(theone != NULL)
    {
        error("Critical Error: The \"HRefreshAgent\" object must be only one instance!");
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
        error("Critical Error: The \"HRefreshAgent\"  class is uninitialized!");
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
        error("Critical Error: The \"HRefreshAgentNetserver\" object must be only one instance!");
        return;
    }
    sdebug("[HRefreshAgentNetserver] Initializing...");
    theone = this;

    //create a tcpserver
    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any,REFRESHAGENT_TCPPORT))
    {
         error(QString("Unable to start tcpserver: %1.")
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

    inout << "ConnectOk" <<endl;
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

    inout << "<" << name << ">" << endl;
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
        error("Critical Error: The \"HRefreshAgentNetclient\" object must be only one instance!");
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

    inout << "<" << name << ">" << endl;
    QList<QTcpSocket *>::iterator  is;

    if(socket->state() == QAbstractSocket::ConnectedState)
        socket->write(block);

    inNotify = false;
    return 0;
}

#endif

/////////////////////////////////////////////////////////////////////////////////
////// HBase members ////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HBase::HBase(void)
{
    whoami = "HBase";
}

HBase::~HBase(void)
{

}

QString HBase::getWhoami(void)
{
    return whoami;
}

/////////////////////////////////////////////////////////////////////////////////
////// HSqlInterface members ////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HSqlSingleInterface::HSqlSingleInterface(void)
{

}

HSqlSingleInterface_SqliteWin::HSqlSingleInterface_SqliteWin(void)
{
    hsqli_currentModeName   = "QtSqlite_Win";
    hsqli_usequerysize      = false;
    hsqli_booleantypename   = "BOOLEAN";
    hsqli_truevalue         = "1";
    hsqli_falsevalue        = "0";
    hsqli_hkeytype_varchar_int = false;
    hsqli_varcharhkeylength = 15;
    hsqli_hkeyconstraint    = "PRIMARY KEY AUTOINCREMENT";
    hsqli_need_create_sequence_before=false;
    hsqli_sequence_creation_string="";
    hsqli_charhash_varcharlength  = 5;
    hsqli_smalltext_varcharlength = 256;
    hsqli_largetext_sizemustdefine = false;
    hsqli_largetext_varcharlength = 2048;
    hsqli_numbertypename    = "INTEGER";
    hsqli_floattypename     = "REAL";
    hsqli_hastimestamptype  = false;
    hsqli_hasdatetype       = false;
    hsqli_timestamptypename = "VARCHAR"; //varchar - can be converted to date by functions
    hsqli_datetypename      = "VARCHAR"; //varchar - can be converted to date by functions
    hsqli_timestampnowvalue = "datetime(current_timestamp,\'localtime\')";
    hsqli_sqlchooseconstraint = false;
    hsqli_appendsqlerrormsg = false;
    hsqli_hackconvert_stdsqlconcatenation_to_local = 0;
}

HSqlSingleInterface_SqliteLin::HSqlSingleInterface_SqliteLin(void)
:HSqlSingleInterface()
{
    hsqli_currentModeName   = "QtSqlite_Lin";
    hsqli_usequerysize      = false;
    hsqli_booleantypename   = "BOOLEAN";
    hsqli_truevalue         = "1";
    hsqli_falsevalue        = "0";
    hsqli_hkeytype_varchar_int = false;
    hsqli_varcharhkeylength = 15;
    hsqli_hkeyconstraint    = "PRIMARY KEY AUTOINCREMENT";
    hsqli_need_create_sequence_before=false;
    hsqli_sequence_creation_string="";
    hsqli_charhash_varcharlength  = 5;
    hsqli_smalltext_varcharlength = 256;
    hsqli_largetext_sizemustdefine = false;
    hsqli_largetext_varcharlength = 2048;
    hsqli_numbertypename    = "INTEGER";
    hsqli_floattypename     = "REAL";
    hsqli_hastimestamptype  = false;
    hsqli_hasdatetype       = false;
    hsqli_timestamptypename = "VARCHAR"; //varchar - can be converted to date by functions
    hsqli_datetypename      = "VARCHAR"; //varchar - can be converted to date by functions
    hsqli_timestampnowvalue = "datetime(current_timestamp,\'localtime\')";
    hsqli_sqlchooseconstraint = false;
    hsqli_appendsqlerrormsg = false;
    hsqli_hackconvert_stdsqlconcatenation_to_local = 0;
}

HSqlSingleInterface_PsqlOdbcWin::HSqlSingleInterface_PsqlOdbcWin(void)
:HSqlSingleInterface()
{
    hsqli_currentModeName   = "PostgreSQL_WinOdbc";
    hsqli_usequerysize      = true;
    hsqli_booleantypename   = "BOOLEAN";
    hsqli_truevalue         = "TRUE";
    hsqli_falsevalue        = "FALSE";
    hsqli_hkeytype_varchar_int = true;
    hsqli_varcharhkeylength = 15;
    hsqli_hkeyconstraint    = "PRIMARY KEY DEFAULT(nextval(\'_SEQNAME_\'))";
    hsqli_need_create_sequence_before=true;
    hsqli_sequence_creation_string="CREATE SEQUENCE _SEQNAME_ INCREMENT 1 START 1";
    hsqli_charhash_varcharlength  = 5;
    hsqli_smalltext_varcharlength = 512;
    hsqli_largetext_sizemustdefine = false;
    hsqli_largetext_varcharlength = 4096;
    hsqli_numbertypename    = "NUMERIC";
    hsqli_floattypename     = "NUMERIC";
    hsqli_hastimestamptype  = true;
    hsqli_hasdatetype       = true;
    hsqli_timestamptypename = "TIMESTAMP";
    hsqli_datetypename      = "DATE";
    hsqli_timestampnowvalue = "now()";
    hsqli_sqlchooseconstraint = true;
    hsqli_appendsqlerrormsg = false;
    hsqli_hackconvert_stdsqlconcatenation_to_local = 0;
}

HSqlSingleInterface_PostgresqlLin::HSqlSingleInterface_PostgresqlLin(void)
:HSqlSingleInterface()
{
    hsqli_currentModeName   = "PostgreSQL_LinPsql";
    hsqli_usequerysize      = false;
    hsqli_booleantypename   = "BOOLEAN";
    hsqli_truevalue         = "TRUE";
    hsqli_falsevalue        = "FALSE";
    hsqli_hkeytype_varchar_int = true;
    hsqli_varcharhkeylength = 15;
    hsqli_hkeyconstraint    = "PRIMARY KEY DEFAULT(nextval(\'_SEQNAME_\'))";
    hsqli_need_create_sequence_before=true;
    hsqli_sequence_creation_string="CREATE SEQUENCE _SEQNAME_ INCREMENT 1 START 1";
    hsqli_charhash_varcharlength  = 5;
    hsqli_smalltext_varcharlength = 512;
    hsqli_largetext_sizemustdefine = false;
    hsqli_largetext_varcharlength = 4096;
    hsqli_numbertypename    = "NUMERIC";
    hsqli_floattypename     = "NUMERIC";
    hsqli_hastimestamptype  = true;
    hsqli_hasdatetype       = true;
    hsqli_timestamptypename = "TIMESTAMP";
    hsqli_datetypename      = "DATE";
    hsqli_timestampnowvalue = "now()";
    hsqli_sqlchooseconstraint = true;
    hsqli_appendsqlerrormsg = false;
    hsqli_hackconvert_stdsqlconcatenation_to_local = 0;
}

HSqlSingleInterface_MysqlOdbcWin::HSqlSingleInterface_MysqlOdbcWin(void)
:HSqlSingleInterface()
{
    hsqli_currentModeName   = "MYSQL_WinOdbc";
    hsqli_usequerysize      = true;
    hsqli_booleantypename   = "TINYINT(1)";
    hsqli_truevalue         = "1";
    hsqli_falsevalue        = "0";
    hsqli_hkeytype_varchar_int = false;
    hsqli_varcharhkeylength = 15;
    hsqli_hkeyconstraint    = "PRIMARY KEY NOT NULL AUTO_INCREMENT";
    hsqli_need_create_sequence_before=false;
    hsqli_sequence_creation_string="";
    hsqli_charhash_varcharlength  = 5;
    hsqli_smalltext_varcharlength = 512;
    hsqli_largetext_sizemustdefine = true;
    hsqli_largetext_varcharlength = 4096;
    hsqli_numbertypename    = "INTEGER";
    hsqli_floattypename     = "DOUBLE";
    hsqli_hastimestamptype  = true;
    hsqli_hasdatetype       = true;
    hsqli_timestamptypename = "TIMESTAMP";
    hsqli_datetypename      = "DATE";
    hsqli_timestampnowvalue = "now()";
    hsqli_sqlchooseconstraint = true;
    hsqli_appendsqlerrormsg = true;
    hsqli_hackconvert_stdsqlconcatenation_to_local = 1;
}

HSqlSingleInterface_MysqlLin::HSqlSingleInterface_MysqlLin(void)
:HSqlSingleInterface()
{
    hsqli_currentModeName   = "MYSQL_LinQt";
    hsqli_usequerysize      = false;
    hsqli_booleantypename   = "TINYINT(1)";
    hsqli_truevalue         = "1";
    hsqli_falsevalue        = "0";
    hsqli_hkeytype_varchar_int = false;
    hsqli_varcharhkeylength = 15;
    hsqli_hkeyconstraint    = "PRIMARY KEY NOT NULL AUTO_INCREMENT";
    hsqli_need_create_sequence_before=false;
    hsqli_sequence_creation_string="";
    hsqli_charhash_varcharlength  = 5;
    hsqli_smalltext_varcharlength = 512;
    hsqli_largetext_sizemustdefine = true;
    hsqli_largetext_varcharlength = 4096;
    hsqli_numbertypename    = "INTEGER";
    hsqli_floattypename     = "DOUBLE";
    hsqli_hastimestamptype  = true;
    hsqli_hasdatetype       = true;
    hsqli_timestamptypename = "TIMESTAMP";
    hsqli_datetypename      = "DATE";
    hsqli_timestampnowvalue = "now()";
    hsqli_sqlchooseconstraint = true;
    hsqli_appendsqlerrormsg = true;
    hsqli_hackconvert_stdsqlconcatenation_to_local = 1;
}

/* Miscrosoft MSSQL Express scheme. Not tested:
HSqlSingleInterface_MssqlWin::HSqlSingleInterface_MssqlWin(void)
:HSqlSingleInterface()
{
    hsqli_currentModeName   = "MSSQL_WinOdbc";
    hsqli_usequerysize      = false;
    hsqli_booleantypename   = "BIT";
    hsqli_truevalue         = "1";
    hsqli_falsevalue        = "0";
    hsqli_hkeytype_varchar_int = false;
    hsqli_varcharhkeylength = 15;
    hsqli_hkeyconstraint    = "INT IDENTITY PRIMARY KEY";
    hsqli_need_create_sequence_before=false;
    hsqli_sequence_creation_string="";
    hsqli_charhash_varcharlength  = 5;
    hsqli_smalltext_varcharlength = 512;
    hsqli_largetext_sizemustdefine = true;
    hsqli_largetext_varcharlength = 4096;
    hsqli_numbertypename    = "INT";
    hsqli_floattypename     = "DOUBLE";
    hsqli_hastimestamptype  = true;
    hsqli_hasdatetype       = true;
    hsqli_timestamptypename = "TIMESTAMP";
    hsqli_datetypename      = "DATE";
    hsqli_timestampnowvalue = "CURRENT_TIMESTAMP";
    hsqli_sqlchooseconstraint = false;
    hsqli_appendsqlerrormsg = true;
    hsqli_hackconvert_stdsqlconcatenation_to_local = 2;
}
*/

bool    HSqlInterface::isset = false;
int     HSqlInterface::currentDefaultInterface = 0;
int     HSqlInterface::defaultDefaultInterface = -1;
HSqlSingleInterface * HSqlInterface::interfaces[MAX_SQL_INTERFACES];

HSqlInterface::HSqlInterface()
{
    whoami = "HSqlInterface";
    if(!isset)
    {
        error("HSqlInterface: Define a sql interface model with HSqlInterface::setSqlMode() !");
        QCoreApplication::quit();
    }

    myinterface = currentDefaultInterface;
}

HSqlInterface::~HSqlInterface()
{
}

void HSqlInterface::setSqlMode(QString modename)
{
    int i,newdefmode;
    if(!isset) //first run ever
    {
        for(i=0;i<MAX_SQL_INTERFACES;++i)
            interfaces[i] = NULL;
        interfaces[0] = new HSqlSingleInterface_SqliteWin();
        interfaces[1] = new HSqlSingleInterface_SqliteLin();
        interfaces[2] = new HSqlSingleInterface_PsqlOdbcWin();
        interfaces[3] = new HSqlSingleInterface_PostgresqlLin();
        interfaces[4] = new HSqlSingleInterface_MysqlOdbcWin();
        interfaces[5] = new HSqlSingleInterface_MysqlLin();
        isset = true;
    }

    newdefmode = -1;
    for(i=0;i<MAX_SQL_INTERFACES;++i)
        if(interfaces[i] != NULL && interfaces[i]->hsqli_currentModeName == modename)
            newdefmode = i;

    if(newdefmode == -1)
    {
        error("HSqlInterface::setSqlMode : Unknown sql mode!");
        QCoreApplication::quit();
        return;
    }

    if(defaultDefaultInterface == -1)
        defaultDefaultInterface = newdefmode;

    currentDefaultInterface = newdefmode;
}

void HSqlInterface::setSqlModeBackToFirst(void)
{
    if(defaultDefaultInterface != -1)
        currentDefaultInterface = defaultDefaultInterface;
}

HSqlSingleInterface *HSqlInterface::currentDefaultSqlInterface(void)
{
    return interfaces[currentDefaultInterface];

}

HSqlSingleInterface *HSqlInterface::getSqlInterface(QString interfacename)
{
    int i;
    for(i=0;i<MAX_SQL_INTERFACES;++i)
        if(interfaces[i] != NULL && interfaces[i]->hsqli_currentModeName == interfacename)
            return interfaces[i];
    return NULL;
}

QString HSqlInterface::sqlConcatenateToLocal(QString string)
{
    if(interfaces[myinterface]->hsqli_hackconvert_stdsqlconcatenation_to_local == 0)
        return string;

    if(interfaces[myinterface]->hsqli_hackconvert_stdsqlconcatenation_to_local == 1)
    {
        QString newconc,inner;
        inner = string;
        inner.replace("||",",");
        newconc = "CONCAT("+inner+")";
        return newconc;
    }
    if(interfaces[myinterface]->hsqli_hackconvert_stdsqlconcatenation_to_local == 2)
    {
        QString newconc;
        newconc = string;
        newconc.replace("||","+");
        return newconc;
    }
    return "Error, uncnown string concatenation method!";
}

QString HSqlInterface::sqlConcatenateToLocal(QStringList fields)
{
    QString result;
    QString sep="";
    QStringList::iterator i;

    switch(interfaces[myinterface]->hsqli_hackconvert_stdsqlconcatenation_to_local)
    {
        case 0:  sep = "||";  break;
        case 1:  sep = ",";   break;
        case 2:  sep = "+";   break;
    }

    int fieldnum=0;
    for(i=fields.begin();i!=fields.end();++i)
    {
        result += (fieldnum == 0 ? "" : sep) + *i;
        ++fieldnum;
    }

    if(interfaces[myinterface]->hsqli_hackconvert_stdsqlconcatenation_to_local == 1)
        result = "CONCAT("+result+")";

    return result;
}

/////////////////////////////////////////////////////////////////////////////////
////// HSqlHandler members //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int     HSqlHandler::trans = 0;
bool    HSqlHandler::globalnew_nodefault = false;
QString HSqlHandler::globalnew_current_database = "";

void HSqlHandler::setCustomSqlDatabase(QString databaseName,QString sqlinterfacename)
{
    globalnew_nodefault         = true;
    globalnew_current_database  = databaseName;
    if(!sqlinterfacename.isEmpty())
        HSqlInterface::setSqlMode(sqlinterfacename);
}

void HSqlHandler::setDefaultSqlDatabase(void)
{
    globalnew_nodefault         = false;
    globalnew_current_database  = "";
    HSqlInterface::setSqlModeBackToFirst();
}

HSqlHandler::HSqlHandler(void)
{
    whoami = "HSqlHandler";

    query_error_occured = false;
    nodefault           = globalnew_nodefault;
    current_database    = globalnew_current_database;
}

HSqlHandler::~HSqlHandler(void)
{
    if(trans != 0) //nem ok
    {
        emit errorSignal("Error: I have some uncommitted/rollback-ed transaction!");
    }
}

void HSqlHandler::cloneSqlSettingsFrom(HSqlHandler *other)
{
    cloneSqlInterfaceSettingsFrom(other);

    nodefault           = other->nodefault;
    current_database    = other->current_database;
}

QSqlDatabase HSqlHandler::workDB(void)
{
    if(!nodefault)
        return QSqlDatabase::database();

    QSqlDatabase db = QSqlDatabase::database(current_database);
    if(!db.isValid() || !db.isOpen())
    {
        query_error_occured = true;
        sdebug(QString("Cannot load the given database by name \"%1\"!").arg(current_database));
        emit errorSignal(QString("Cannot load the given database by name \"%1\"!").arg(current_database));
        return QSqlDatabase::database();
    }
    return db;
}

QSqlQuery * HSqlHandler::allocateQSqlQuery(void)
{
    return new QSqlQuery(workDB());
}

void HSqlHandler::transaction(void)
{
    sdebug(QString("Begin transaction: %1").arg(trans));
    trans++;
    QSqlDatabase db = workDB();
    if(db.transaction()) return; // OK!

    emit errorSignal("Cannot begin the SQL transaction!");
}

void HSqlHandler::commit(void)
{

    if(trans<=0)
        emit errorSignal("BIG Warning: Called commit without begin a transaction!");
    trans--;
    sdebug(QString("Commit transaction: %1").arg(trans));

    QSqlDatabase db = workDB();
    if(db.commit()) return; // OK!

    emit errorSignal("Cannot COMMIT the SQL transaction!");

}

void HSqlHandler::rollback(void)
{

    if(trans<=0)
        emit errorSignal("BIG Warning: Called rollback without begin a transaction!");
    trans--;
    sdebug(QString("Rollback transaction: %1").arg(trans));

    QSqlDatabase db = workDB();
    if(db.rollback()) return; // OK!

    emit errorSignal("Cannot ROLLBACK the SQL transaction!");
}


bool HSqlHandler::submit0ResultQuery(QString q,QString err,bool tdisabled)
{
    query_error_occured = false;

    if(!tdisabled)
        transaction();

    QSqlQuery *qi = allocateQSqlQuery();

    sqldebug(q);
    qi->prepare(q);
    qi->exec();

    if(qi->lastError().type() != QSqlError::NoError)
        {
            if(!tdisabled)
                rollback();

            QString errortext_new = err;
            if(myInterface()->hsqli_appendsqlerrormsg)
            {
                errortext_new += " (" + qi->lastError().text() + ") Sql was:"+q;
            }
            delete qi;
            emit errorSignal(errortext_new);
            query_error_occured = true;
            return true;
        }
    if(!tdisabled)
        commit();

    delete qi;
    return false;
}

QVariant HSqlHandler::submit1ResultQuery(QString q,QString err,bool tdisabled)
{
    QVariant v;

    query_error_occured = false;
    v="";
    if(!tdisabled)
        transaction();

    QSqlQuery *qi = allocateQSqlQuery();

    sqldebug(q);
    qi->prepare(q);
    qi->exec();

    if( (myInterface()->hsqli_usequerysize && qi->numRowsAffected() != 1) ||
        ! qi->next() ||
        qi->lastError().type() != QSqlError::NoError   )
        {
            if(!tdisabled)
                rollback();

            QString errortext_new = err;
            if(myInterface()->hsqli_appendsqlerrormsg)
            {
                errortext_new += " (" + qi->lastError().text() + ")";
            }
            delete qi;
            emit errorSignal(errortext_new);
            query_error_occured = true;
            return v;
        }

    v = qi->value(0);

    if(!tdisabled)
        commit();

    delete qi;
    return v;
}

HPlainDataMatrix* HSqlHandler::submitNResultQuery(int N,QString q,QString err,bool tdisabled)
{
    int i;
    QList<QVariant> list;
    HPlainDataMatrix* dm=NULL;

    dm = new HPlainDataMatrix(N);

    query_error_occured = false;
    if(!tdisabled)
        transaction();

    QSqlQuery *qi = allocateQSqlQuery();

    sqldebug(q);
    qi->prepare(q);
    qi->exec();
    if((myInterface()->hsqli_usequerysize && qi->numRowsAffected() < 0) ||
        qi->lastError().type() != QSqlError::NoError   )
        {

            if(!tdisabled)
                rollback();

            QString errortext_new = err;
            if(myInterface()->hsqli_appendsqlerrormsg)
            {
                errortext_new += " (" + qi->lastError().text() + ")";
            }
            delete qi;
            emit errorSignal(errortext_new);
            query_error_occured = true;
            delete dm;
            return NULL;
        }

    while(qi->next())
    {
        list.clear();
        for(i=0 ; i < qi->record().count() ; ++i)
            list.push_back( qi->value(i) );
        dm->addRow(list);
    }

    if(!tdisabled)
        commit();

    delete qi;
    return dm;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HPlainDataMatrix::HPlainDataMatrix(int col)
{
    int i;
    whoami = "HPlainDataMatrix";
    col_count = col;

    title = "";
    exp_title = "";
    htmlcss = "";
    hheader = new QString[col_count];

    printCellWrap = new bool[col_count];
    for(i = 0 ; i < col_count ; ++i)
        printCellWrap[i] = false;

    printMaxCellWidth = new int[col_count];
    for(i = 0 ; i < col_count ; ++i)
        printMaxCellWidth[i] = 0;

    keyfield = -2; //default. The HDispPlainDataMatrix can set the key
}

HPlainDataMatrix::~HPlainDataMatrix(void)
{

    delete[] hheader;
    while( !data.isEmpty())
    {
        delete[] data.first();
        data.pop_front();
    }
}

void HPlainDataMatrix::clearData()
{
    int i;

    //Clear data fields
    while( !data.isEmpty())
    {
        delete[] data.first();
        data.pop_front();
    }
    data.clear();
    //Clear control data
    control.clear();

    //Reset iterators
    iter = data.begin() ;
    iter_ctrl = control.begin();

    //Reset other...
    for(i = 0 ; i < col_count ; ++i)
        printCellWrap[i] = false;
    for(i = 0 ; i < col_count ; ++i)
        printMaxCellWidth[i] = 0;

    keyfield = -2;
}

void HPlainDataMatrix::clearAll()
{
    int i;

    title = "";
    exp_title = "";
    htmlcss = "";

    for(i = 0 ; i < col_count ; ++i)
        hheader[i] = "";

    clearData();

}

int HPlainDataMatrix::columnCount()
{
    return col_count;
}

int  HPlainDataMatrix::rowCount()
{
    return data.count();
}

void HPlainDataMatrix::setHeaderCell(int col,QString strdata)
{
    if(col > col_count)
        return;
    hheader[col] = strdata;
}

void HPlainDataMatrix::setHeader(QList<QString> strlistdata)
{
    int i;
    for(i = 0; i < col_count;++i)
    {
        if(i < strlistdata.count())
            hheader[i] = strlistdata.at(i);
        else
            hheader[i] = "";
    }
}

void HPlainDataMatrix::setHeader(QString d1,QString d2,QString d3
                                ,QString d4,QString d5,QString d6
                                ,QString d7,QString d8,QString d9
                                ,QString d10,QString d11,QString d12
                                ,QString d13,QString d14,QString d15)
{
    int i;
    QString s;
    for(i = 0; i < col_count;++i)
    {
        switch(i)
        {
            case 0: s = d1; break;
            case 1: s = d2; break;
            case 2: s = d3; break;
            case 3: s = d4; break;
            case 4: s = d5; break;
            case 5: s = d6; break;
            case 6: s = d7; break;
            case 7: s = d8; break;
            case 8: s = d9; break;
            case 9: s = d10; break;
            case 10: s = d11; break;
            case 11: s = d12; break;
            case 12: s = d13; break;
            case 13: s = d14; break;
            case 14: s = d15; break;
            default: s = "NOTSUPPORTED"; break;
        }
        hheader[i] = s;
    }
}



QString HPlainDataMatrix::getHeaderItem(int col)
{
    return hheader[col];
}

QList<QString> HPlainDataMatrix::getHeader(void)
{
    int i;
    QList<QString> list;
    list.clear();
    for(i = 0; i < col_count;++i)
    {
        list.push_back(hheader[i]);
    }
    return list;
}

void HPlainDataMatrix::addRow(QList<QVariant> listdata,QString ctrl)
{
    int i;
    QVariant *row;

    row = new QVariant[col_count];
    for(i = 0; i < col_count;++i)
    {
        if(i < listdata.count())
            row[i] = listdata.at(i);
        else
            row[i] = QVariant("");
    }
    data.push_back(row);
    control.push_back(ctrl);
}

void HPlainDataMatrix::appendHPainDataMatrix(HPlainDataMatrix *tail)
{
    if(col_count != tail->columnCount())
        return;
    if(tail->rowCount() > 0)
    {
        tail->firstRow();
        do
            addRow(  tail->currentRow(),tail->currentRowControl() );
        while(tail->nextRow());
    }
    return;
}

void HPlainDataMatrix::addRowStr(QList<QString> strlistdata,QString ctrl)
{
    int i;
    QVariant *row;

    row = new QVariant[col_count];
    for(i = 0; i < col_count;++i)
    {
        if(i < strlistdata.count())
            row[i] = QVariant(strlistdata.at(i));
        else
            row[i] = QVariant("");
    }
    data.push_back(row);
    control.push_back(ctrl);
}

void HPlainDataMatrix::addRowStrCTRL(QString ctrl,QString d1,QString d2,QString d3
           ,QString d4,QString d5,QString d6
           ,QString d7,QString d8,QString d9
           ,QString d10,QString d11,QString d12
           ,QString d13,QString d14,QString d15)
{
    int i;
    QVariant *row;
    QString s;

    row = new QVariant[col_count];
    for(i = 0; i < col_count;++i)
    {
        switch(i)
        {
            case 0: s = d1; break;
            case 1: s = d2; break;
            case 2: s = d3; break;
            case 3: s = d4; break;
            case 4: s = d5; break;
            case 5: s = d6; break;
            case 6: s = d7; break;
            case 7: s = d8; break;
            case 8: s = d9; break;
            case 9: s = d10; break;
            case 10: s = d11; break;
            case 11: s = d12; break;
            case 12: s = d13; break;
            case 13: s = d14; break;
            case 14: s = d15; break;
            default: s = "NOTSUPPORTED"; break;
        }
        row[i] = QVariant(s);
    }
    data.push_back(row);
    control.push_back(ctrl);
}

QVariant HPlainDataMatrix::getCell(int row,int col)
{
    if(row < 0 || col < 0)
        return QVariant("ERROR");
    if(row > data.count() || col > col_count)
        return QVariant("ERROR");

    return (data.at(row))[col];
}


QString HPlainDataMatrix::getCellStr(int row,int col)
{
    if(row < 0 || col < 0)
        return "ERROR";
    if(row > data.count() || col > col_count)
        return "ERROR";

    return (data.at(row))[col].toString();
}

QString HPlainDataMatrix::getColumn(int col,QString separator)
{
    bool first;
    QString r;

    r="";
    if(data.count() <= 0)
        return r;
    firstRow();
    first = true;
    do
    {
        if(!first)
            r.append(separator);
        else
            first = false;
        r.append(currentRowStr()[col]);
    }
    while(nextRow());
    return r;
}


void HPlainDataMatrix::setCell(int row,int col,QVariant vdata)
{
    if(row < 0 || col < 0)
        return;
    if(row > data.count() || col > col_count)
        return;

    (data.at(row))[col] = vdata;
}

void HPlainDataMatrix::setRowControl(int row,QString ctrl)
{
    if(row < 0 )
        return;
    if(row > data.count())
        return;

    control[row] = ctrl;
}

void HPlainDataMatrix::setCellStr(int row,int col,QString strdata)
{
    if(row < 0 || col < 0)
        return;
    if(row > data.count() || col > col_count)
        return;

    (data.at(row))[col] = QVariant(strdata);
}

QList<QVariant> HPlainDataMatrix::getRow(int row)
{
    int i;
    QList<QVariant> list;
    list.clear();
    QVariant *rowdata=NULL;
    rowdata = data.at(row);
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i]);
        }
    return list;
}

QList<QString> HPlainDataMatrix::getRowStr(int row)
{
    int i;
    QList<QString> list;
    list.clear();
    QVariant *rowdata=NULL;
    rowdata = data.at(row);
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i].toString());
        }
    return list;
}

QString HPlainDataMatrix::getRowStr(int row,QString separator)
{
    int i;
    QString list="";
    QVariant *rowdata=NULL;
    rowdata = data.at(row);
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.append(rowdata[i].toString() + separator);
        }
    return list;
}

void HPlainDataMatrix::firstRow(void)
{
    iter = data.begin();
    iter_ctrl = control.begin();
}

bool HPlainDataMatrix::nextRow(void)
{
    if(iter == data.end())
        return false;

    ++iter;
    ++iter_ctrl;

    if(iter == data.end())
        return false;

    return true;
}

bool HPlainDataMatrix::isEnded(void)
{
    if(iter == data.end() || iter_ctrl == control.end())
        return true;
    return false;
}

void HPlainDataMatrix::removeCurrentRow(void)
{
    if(iter == data.end() || iter_ctrl == control.end())
        return;

    QVariant *to_delete = *iter;
    iter = data.erase(iter);
    iter_ctrl = control.erase(iter_ctrl);
    delete [] to_delete;

}

QList<QVariant> HPlainDataMatrix::currentRow(void)
{
    int i;
    QList<QVariant> list;
    list.clear();
    QVariant *rowdata=NULL;
    rowdata = *iter;
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i]);
        }
    return list;
}

QString HPlainDataMatrix::currentRowControl(void)
{
    return *iter_ctrl;
}

QList<QString> HPlainDataMatrix::currentRowStr(void)
{
    int i;
    QList<QString> list;
    list.clear();
    QVariant *rowdata=NULL;
    rowdata = *iter;
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i].toString());
        }
    return list;
}

QString HPlainDataMatrix::currentRowStr(QString separator)
{
    int i;
    QString list="";
    QVariant *rowdata=NULL;
    rowdata = *iter;
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.append(rowdata[i].toString() + separator);
        }
    return list;
}

void HPlainDataMatrix::replaceTextInColumn(int col,QString find,QString replace,bool parts)
{
    QList<QVariant *>::iterator iter;
    QVariant *in=NULL;

    iter = data.begin();
    while(iter != data.end())
    {
        in = *iter;
        if(parts)
        {

            if(in[col].toString().contains(find))
            {
                QString str;

                str = in[col].toString();
                str.replace(find,replace);
                in[col].setValue(str);
            }
        }
        else
        {
            if(in[col].toString() == find)
                in[col].setValue(replace);
        }
        ++iter;
    }
}

void HPlainDataMatrix::replaceSameUnderCellToEmpty(int col)
{
    QString laststr;
    QList<QVariant *>::iterator iter;
    QVariant *in=NULL;

    laststr = "__NOTHING_WAS_BEFORE__";
    iter = data.begin();
    while(iter != data.end())
    {
        in = *iter;
        if(in[col].toString() == laststr)
            in[col].setValue(QString(""));
        else
            laststr = in[col].toString();
        ++iter;
    }
}

void HPlainDataMatrix::setAppendPrependColumn(int col,QString prepend,QString append)
{
    QString str;
    QList<QVariant *>::iterator iter;
    QVariant *in=NULL;

    iter = data.begin();
    while(iter != data.end())
    {
        in = *iter;
        str = in[col].toString();
        str.prepend(prepend);
        str.append(append);
        in[col].setValue(str);
        ++iter;
    }

}

int HPlainDataMatrix::sumCoulmnAsInt(int col)
{
    int sum;
    QList<QVariant *>::iterator iter;
    QVariant *in=NULL;

    iter = data.begin();
    sum = 0;
    while(iter != data.end())
    {
        in = *iter;
        sum += in[col].toInt();

        ++iter;
    }
    return sum;
}

double HPlainDataMatrix::sumCoulmnAsDouble(int col)
{
    double sum;
    QList<QVariant *>::iterator iter;
    QVariant *in=NULL;

    iter = data.begin();
    sum = 0;
    while(iter != data.end())
    {
        in = *iter;
        sum += in[col].toDouble();
        ++iter;
    }
    return sum;
}

QString HPlainDataMatrix::concatenateCoulmn(int col,QString separator)
{
    QString sum;
    QList<QVariant *>::iterator iter;
    QVariant *in=NULL;

    iter = data.begin();
    sum = "";
    while(iter != data.end())
    {
        in = *iter;
        if(!sum.isEmpty())
            sum += separator;
        sum += in[col].toString();
        ++iter;
    }
    return sum;
}

void HPlainDataMatrix::removeHTMLTags(bool force_all)
{
    int i;
    QString str,str2;
    QList<QVariant *>::iterator iter;
    QList<QString>::iterator ctrl_iter;
    QVariant *in=NULL;

    //iter = data.begin();

    iter = data.begin();
    ctrl_iter = control.begin();
    while(iter != data.end())
    {
        if(ctrl_iter == control.end())
            error("HPlainDataMatrix::removeHTMLTags: Control and data count mismatch!");

        in = *iter;
        if(force_all || (*ctrl_iter).contains("optionalhtmltags") )
        {
            for(i=0;i<col_count;++i)
            {
                str = in[i].toString();

                str2 = str.replace(QRegExp("<[^<>]*>"),"");
                sdebug("str:"+str+","+str2);
                in[i].setValue(str2);
            }
        }
        ++iter;
        ++ctrl_iter;
    }
}

QString HPlainDataMatrix::getContentAsHtml(QString params)
{
    int all_width=0,border=0,pad=4,space=1;
    QString out;
    QString fullwidth;
    QTextStream textstream(&out);
    HXmlWriter xmlw;
    QList<QString> lst;
    int i;

    out = "";
    xmlw.setStream(&textstream);
    xmlw.putHead("",true);
    if(params.contains("html"))
        xmlw.beginNode("html");
    if(params.contains("css") && !htmlcss.isEmpty())
    {
        xmlw.beginNode("head");
        xmlw.putCData("style",htmlcss,"type=\"text/css\"");
        xmlw.endNode();
    }

    if(params.contains("center"))
        xmlw.beginNode("center");
    if(!params.contains("notitle"))
        xmlw.putCData("h2",getTitle());

    QRegExp reg1("width=(\\d+)");
    if(params.contains(reg1))
        all_width = (reg1.capturedTexts()[1]).toInt();
    QRegExp reg2("border=(\\d+)");
    if(params.contains(reg2))
        border = (reg2.capturedTexts()[1]).toInt();
    QRegExp reg3("pad=(\\d+)");
    if(params.contains(reg3))
        pad = (reg3.capturedTexts()[1]).toInt();
    QRegExp reg4("space=(\\d+)");
    if(params.contains(reg4))
        space = (reg4.capturedTexts()[1]).toInt();

    fullwidth = "";
    if(params.contains("fullwidth"))
        fullwidth = " width=\"100%\" ";

    if(!params.contains("notable"))
        xmlw.beginNode("table",QString("cellpadding=%1 cellspacing=%2 border=%3 %4")
                                .arg(pad)
                                .arg(space)
                                .arg(border)
                                .arg(fullwidth));
    if(rowCount() > 0)
    {
        if(!params.contains("noheader"))
        {
            xmlw.beginNode("tr");
            for(i = 0 ; i < col_count ; ++i)
                xmlw.putCData("th",
                              hheader[i],
                              all_width == 0 ?
                                        "align=\"center\" " :
                                        QString("align=\"center\" width=%1").arg(all_width)); //width=x

            xmlw.endNode();
        }

        firstRow();
        QString rowparameter;
        bool fcc_mode;
        bool contains_cellalign;
        bool contains_addcellparam;
        QString precell,postcell;
        QString cellparameters;


        int expanding_num=0;
        int from[15];
        int to[15];
        int inExpand;

        do
        {
            expanding_num = 1;
            from[0] = -1;
            to[0] = -1;

            precell="";
            postcell="";
            fcc_mode=false;
            rowparameter = "";
            contains_cellalign = false;
            contains_addcellparam = false;
            if(currentRowControl().contains("backgroundColor"))
            {
                QRegExp regc("backgroundColor=(......)");
                if(currentRowControl().contains(regc))
                {
                    rowparameter += " bgcolor=\"#"+regc.capturedTexts()[1] + "\"";
                }
            }
            if(currentRowControl().contains("1cellexpandcenter"))
                fcc_mode = true;
            if(currentRowControl().contains("precell"))
            {
                QRegExp regc("precell=([^;]+);");
                if(currentRowControl().contains(regc))
                {
                    precell = regc.capturedTexts()[1];
                }
            }
            if(currentRowControl().contains("postcell"))
            {
                QRegExp regc("postcell=([^;]+);");
                if(currentRowControl().contains(regc))
                {
                    postcell = regc.capturedTexts()[1];
                }
            }

            if(currentRowControl().contains("expandCol"))
            {
                int fpos;
                int posinstr=0;
                QRegExp regc("expandCol=(\\d+)-(\\d+)");
                while((fpos = regc.indexIn(currentRowControl(),posinstr)) != -1)
                {
                    posinstr = fpos + regc.matchedLength();
                    from[expanding_num] = regc.capturedTexts()[1].toInt();
                    to[expanding_num]   = regc.capturedTexts()[2].toInt();

                    ++expanding_num;
                    if(expanding_num > 11)
                    {
                        error("HPlainDataMatrix::getContentAsHtml: Too much expandCol parameter!");
                        break;
                    }
                }
            }

            if(currentRowControl().contains("alignCellRight") || currentRowControl().contains("alignCellCenter"))
                contains_cellalign = true;

            if(currentRowControl().contains("addCellParam"))
                contains_addcellparam = true;


            xmlw.beginNode("tr",rowparameter);
            lst = currentRowStr();
            inExpand=0;
            for(i = 0 ; i < col_count ; ++i)
            {
                int ii;

                if(fcc_mode && i == 0)
                {
                    xmlw.putCData("td",precell+lst[i]+postcell,QString("colspan=%1 align=\"center\"").arg(col_count) );
                    break;
                }

                cellparameters = "";

                if(all_width != 0)
                    cellparameters += QString(" width=%1").arg(all_width);

                for(ii=0;ii < expanding_num;++ii)
                {
                    if(from[ii] == i && ((to[ii]-from[ii]) > 0))
                    {
                        cellparameters += QString(" colspan=%1").arg(1+to[ii]-from[ii]);
                        inExpand=ii;
                    }
                }
                if(from[inExpand] >= 0 && i > from[inExpand] && i <= to[inExpand])
                {
                    continue;
                }

                if(contains_cellalign)
                {
                    QRegExp regc1(QString("alignCellRight=%1\\s").arg(i));
                    if(currentRowControl().contains(regc1))
                        cellparameters += " align=\"right\"";

                    QRegExp regc2(QString("alignCellCenter=%1\\s").arg(i));
                    if(currentRowControl().contains(regc2))
                        cellparameters += "align=\"center\"";
                }

                if(contains_addcellparam)
                {
                    QRegExp regc(QString("addCellParam=%1:([^;]+);").arg(i));
                    if(currentRowControl().contains(regc))
                        cellparameters += " " + regc.capturedTexts()[1];
                }

                xmlw.putCData("td",precell+lst[i]+postcell,cellparameters);
            }
            xmlw.endNode();
        }
        while(nextRow());
    }
    if(!params.contains("notable"))
        xmlw.endNode();
    if(params.contains("center"))
         xmlw.endNode();
    if(params.contains("html"))
         xmlw.endNode();

    return out;
}

int HPlainDataMatrix::actLine(QString key)
{
    emit actionOnRecord(key);
    return 0;
}

int HPlainDataMatrix::alternateActLine(QString key)
{
    emit alternateActionOnRecord(key);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HDataField_List::HDataField_List(void)
{
    next = NULL;
    nextall = NULL;
    data = NULL;
}

HDataField_List::~HDataField_List(void)
{
    if(next != NULL)
        delete next;
    if(data != NULL)
        delete data;
    data = NULL;
    next = NULL;
    nextall = NULL;
}

void HDataField_List::addElement(HDataField *d)
{
    if(data == NULL)
      { data = d; }
    else
      {
        if(next == NULL)
        {
           if(nextall != NULL)
           {
               emit errorSignal(
                        QString("Cannot insert a new element to the table,"
                                " because It already has some connected field!\n"
                                "Insert all own field first, than the connect the tables!"));
               return;
           }
           next=new HDataField_List();
        }
        next->addElement(d);
        nextall=next;
      }
}

void HDataField_List::connectElement(HDataField_List *c)
{
  if(nextall != NULL)
  {
      nextall->connectElement(c);
  }
  else
  {
      nextall=c;
  }
}

void HDataField_List::disconnectElement(void)
{
  if(next == NULL && nextall == NULL) //end of all
  {
      return;
  }
  else if(next == NULL && nextall != NULL) //disconnect
  {
      nextall = NULL;
      return;
  }
  else
  {
      next->disconnectElement();
  }
}

void HDataField_List::clearElements(void)
{
    if(next != NULL)
        delete next;
    if(data != NULL)
        delete data;
     data = NULL;
     next = NULL;
     nextall = NULL;
}

/////////////////////////////////////////////////////////////////////////////////
////// HTableBase ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HTableBase::HTableBase(QString t)
{
    whoami    = "HTableBase";

    errstr    = "";
    sqlTable  = t;
    field     = 0;
    table     = NULL;

    conn_part      = "";
    imc_thiskey    = "";
    imc_thatkey    = "";
    imc_thattable  = NULL;

    tabletitle = "";
    extrafeatures = false;
    gui_showtabletitle = false;
    connectedTableBases = new QStack<HTableBase *>();
    depend = new QStringList();
    firstField();
}

HTableBase::HTableBase(HTableBase* t)
{
    conn_part  = "";
    imc_thiskey    = "";
    imc_thatkey    = "";
    imc_thattable  = NULL;
    connectedTableBases = new QStack<HTableBase *>();

    table = NULL;
    run = NULL;
    depend = new QStringList();

    deepcopy_from_htablebase(t);
}

void HTableBase::deepcopy_from_htablebase(HTableBase *x)
{
    HDataField *df;

    cloneSqlSettingsFrom(x);

    whoami     = x->whoami;
    tabletitle = x->tabletitle;
    field      = x->field;
    sqlTable   = x->sqlTable;

    gui_showtabletitle = x->gui_showtabletitle;
    extrafeatures = x->extrafeatures;
    errstr        = x->errstr;

    clearConnections();
    freeTable();

    deep_copy_stringlist(depend,x->depend);

    x->firstField();
    while((df = x->nextField()))
    {
        addField(df->get_clone());
    }
    firstField();
}

HTableBase::~HTableBase(void)
{
    sdebug("-->Dieing...HtableBase:"+sqlTable+"   __astala vista baby!");
    freeTable();
    delete connectedTableBases;
}

HDataField *HTableBase::fieldBySqlName(QString sqln,bool all,QString containerSqlTable)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL && tmp->data->sqlSelectHead() == sqln &&
            ( !all || tmp->data->conn_sqlTable == containerSqlTable  //connected tables things...
            )
          )
        {
            return tmp->data;
        }

        if(all)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;
    }
    return NULL;
}

HDataField *HTableBase::fieldByIndex(int i,bool all)
{
    int ii=0;
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(!tmp->data->isSQLField())
        {
            if(all)
                tmp=tmp->nextall;
            else
                tmp=tmp->next;
            continue;
        }

        if(i == ii)
        {
            return tmp->data;
        }
        if(all)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;
        ++ii;
    }
    return NULL;
}

HDataField *HTableBase::staticFieldByTitle(QString title)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL &&
            tmp->data->getWhoami() == "HStatic" &&
            (title.isEmpty() || tmp->data->getTitleText() == title))
        {
            return tmp->data;
        }
        tmp=tmp->next;
    }
    return NULL;

}

void HTableBase::addDependTableName(QString n)
{
    if(!n.isEmpty())
        depend->push_back(n);
}

void HTableBase::clearAndFreeConnections(void)
{
    HTableBase *tmp;
    clearConnections();

    while(!connectedTableBases->isEmpty())
    {
        tmp = connectedTableBases->pop();
        tmp->clearConnections();
        delete tmp;
    }

}

void HTableBase::clearConnections(void)
{
    HDataField_List *run=NULL;

    if(table != NULL)
        table->disconnectElement();

    conn_part = "";

    imc_thiskey    = "";
    imc_thatkey    = "";
    imc_thattable  = NULL;
    connectedTableBases->clear();

    for(run=table;run != NULL;run=run->next)
        run->data->connected = false;

}

void HTableBase::connectTable(HTableBase *ct,int type,QString connconstraint)
{
    HDataField_List *crun=NULL,*krun=NULL;

    crun = table;
    while(crun != NULL)
    {
        if(crun->data != NULL)
            if(crun->data->getWhoami() == "HConnect") //if i found connected
                if(((HConnect *)(crun->data))->getConnectedTable() == ct->sqlTableName()) //points to the targetted table
                {
                    krun = ct->table;
                    while(krun != NULL)
                    {
                        if(krun->data != NULL)
                            if(krun->data->getWhoami() == "HKey") //and found a key field in the others
                            {
                                //got the connection!
                                connectTable(ct,crun->data->sqlSelectHead(),
                                                krun->data->sqlSelectHead(),
                                                type,connconstraint    );
                                return;

                            }

                        krun = krun->next;
                    }

                }

        crun=crun->next;
    }

    //Back again, if the dependency is other side
    crun = ct->table;
    while(crun != NULL)
    {
        if(crun->data != NULL)
            if(crun->data->getWhoami() == "HConnect") //found a connect field
                if(((HConnect *)(crun->data))->getConnectedTable() == sqlTableName()) //points to the targetted table
                {
                    krun = table;
                    while(krun != NULL)
                    {
                        if(krun->data != NULL)
                            if(krun->data->getWhoami() == "HKey") //and found a key field in the others
                            {
                                //got the connection!
                                connectTable(ct,krun->data->sqlSelectHead(),
                                                crun->data->sqlSelectHead(),
                                                type,connconstraint   );
                                return;

                            }

                        krun = krun->next;
                    }
                }

        crun=crun->next;
    }

    emit errorSignal("Automatic join fail: Can't find all necessary elements/part ");
}

void HTableBase::connectTable(HTableBase *ct,QString thiskey,QString thatkey,int type,QString connconstraint)
{
    HDataField_List *run=NULL;

    sdebug("*** HTableBase::connectTable ***");
    if(type == INNER_JOIN || type == LEFT_OUTER_JOIN)
    {
        conn_part.append(QString(" %1 %2 ON %3.%4 = %5.%6%7 %8")
                                        .arg(QString((type == INNER_JOIN ? "INNER JOIN" : ""))+
                                             QString((type == LEFT_OUTER_JOIN ? "LEFT OUTER JOIN" : "")))
                                        .arg(ct->sqlTableName())
                                        .arg(this->sqlTableName())
                                        .arg(thiskey)
                                        .arg(ct->sqlTableName())
                                        .arg(thatkey)
                                        .arg(connconstraint.isEmpty() ? "" : (" AND " +  connconstraint + " "))
                                        .arg(ct->conn_part)
                                    );


        table->connectElement(ct->table);
        connectedTableBases->push(ct);

        //sets IMC data
        ct->imc_thattable = this;
        ct->imc_thatkey   = thiskey;
        ct->imc_thiskey   = thatkey;

    }
    else
    {
        emit errorSignal(QString("Unknown connection type: %1").arg(type));
        return;
    }

    for(run=ct->table;run != NULL;run=run->next)
        run->data->connected = true;

    sdebug("*** HTableBase::connectTable *** END");
}

void HTableBase::freeTable(void)
{
    if(table != NULL) delete table;
    table = NULL;
    conn_part = "";
}

void HTableBase::addField(HDataField *i)
{
    if(table == NULL)
    {
        table = new HDataField_List();
        connect( table,SIGNAL(errorSignal(QString )),this,SLOT(errorPassSlot(QString )) );
    }
    table->addElement(i);
    i->conn_sqlTable = sqlTable;
    i->cloneSqlSettingsFrom(this);
    connect(i,SIGNAL(errorSignal(QString )),this,SLOT(errorPassSlot(QString )));
    connect(i,SIGNAL(dataChanged()),SIGNAL(dataChanged()) );
}

QString HTableBase::sqlTableName(void)
{
    return sqlTable;
}

QString HTableBase::sqlSelect(QString tail,bool all,bool keypart)
{
  bool keyfound=false;
  int i=0;
  QString ret,keyname="",keyvalue="";

  HDataField_List *tmp=table;
  ret = "SELECT ";
  for(;tmp != NULL;)
  {
      if(tmp->data->isSQLField()) //it is intrested by sql database
      {
          if(tmp->data->getWhoami() == "HSmallText" && tmp->data->sqlSelectHead() == "NON_SQL_LIST_SERIAL")
          {
              if(all)
                    tmp=tmp->nextall;
                else
                    tmp=tmp->next;
              continue;
          }

          if(tmp->data->getWhoami() == "HKey" && !keyfound)
          {
              keyfound = true;
              keyname  = sqlTable+"."+tmp->data->sqlSelectHead();
              keyvalue = tmp->data->getSQLValue();
          }
          if(i != 0) ret.append(",");

          if(!tmp->data->isEditable() && !tmp->data->getFunction().isEmpty())
          {
            ret.append(tmp->data->getFunction() + "("+( all ? (tmp->data->conn_sqlTable+".") : "" ) +
                                            tmp->data->sqlSelectHead()+")");
          }
          else if(!tmp->data->isEditable() && !tmp->data->getSubselect().isEmpty())
          {
            ret.append("(" + tmp->data->getSubselect() + ")");
          }
          else
          {
            ret.append(( all ? (tmp->data->conn_sqlTable+".") : "" ) +
                                            tmp->data->sqlSelectHead());
          }
          i++;
      }
      if(all)
          tmp=tmp->nextall;
      else
          tmp=tmp->next;
  }

  ret.append(" FROM " + sqlTable + " " + (all ? ( conn_part+ " ") : "") + "WHERE ");

  if(keypart && keyname != "" && keyvalue != "")
  {
      ret.append(" "+keyname+"="+keyvalue+" ");
  }
  else
  {
      ret.append(QString("%1 ").arg(myInterface()->hsqli_truevalue));
  }

  ret.append(tail);
  return ret;
}

QVariant HTableBase::operator [](int i)
{
    int idx=0;
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(idx == i)
        {
            if(tmp->data == NULL) return QVariant("");
            else                  return tmp->data->sqlInsertHead();
        }
        tmp=tmp->next;
        ++idx;
    }
    return QVariant("");
}

void HTableBase::setColor(int r,int g,int b)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            tmp->data->setColor(r,g,b);
        tmp=tmp->next;
    }
}

void HTableBase::setEditable(void)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            tmp->data->setEditable();
        tmp=tmp->next;
    }
}

void HTableBase::setAllNOSet(void)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            tmp->data->notSet();
        tmp=tmp->next;
    }
}

void HTableBase::setAllYESSet(void)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            tmp->data->doSet();
        tmp=tmp->next;
    }
}

int HTableBase::fieldCount(bool withconnect)
{
    int c = 0;
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        ++c;

        if(withconnect)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;
    }
    return c;
}

bool HTableBase::isChanged(bool all)
{
    sdebug("*** HTableBase::isChanged ***");
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            if(tmp->data->isSet())
                return true;

        if(all)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;
    }
    sdebug("*** HTableBase::isChanged *** END");
    return false;
}

QString HTableBase::validate(bool all)
{
    sdebug("*** HTableBase::validate ***");
    QString c,cc;

    c="";
    cc="";
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            if( !((c = tmp->data->validate()).isEmpty()) )
            {
#ifdef VALIDATE_HALT_ON_FIRST_ERROR
                return c;
#else
                cc += c + "\n";
#endif
            }

        if(all)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;

    }
    return cc;
}


void HTableBase::setReadonly(void)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            tmp->data->setReadonly();
        tmp=tmp->next;
    }
}

void HTableBase::setHide(void)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            tmp->data->setHide();
        tmp=tmp->next;
    }
}

void HTableBase::setShow(void)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
            tmp->data->setShow();
        tmp=tmp->next;
    }
}

QStringList HTableBase::getHeads(int what)
{
    QStringList l;

    l.clear();
    //1 - title  2 - short title
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL && tmp->data->isShow() && ( what == 1 || tmp->data->isSQLField() ))
        {
            l.push_back( (what == 1 ? tmp->data->getExplainText() : tmp->data->getTitleText()) );
        }
        tmp=tmp->nextall;
    }
    return l;
}

void HTableBase::firstField(void)
{
    run = NULL;
}


HDataField *HTableBase::nextField(void)
{
    if(run==NULL) //first
    {
         run=table;
         if(run == NULL) return NULL; //empty
         return run->data;
    }

    run = run->next;
    if(run == NULL)
         return NULL;
    return run->data;
 }

HDataField *HTableBase::nextFieldAll(void)
{
    if(run==NULL) //first
    {
         run=table;
         if(run == NULL) return NULL; //empty
         return run->data;
    }

    run = run->nextall;
    if(run == NULL)
         return NULL;
    return run->data;
 }

void HTableBase::dataUpdated(void)
{
    sdebug("*** HTableBase::dataUpdated ***");
    emit dataUpdatedSignal();
    sdebug("*** HTableBase::dataUpdated *** END");
}

int HTableBase::errorPassSlot(QString err)
{
    emit errorSignal(err);
    return 0;
}

QString HTableBase::sqlCreateString(QString switches)
{
    bool first;
    QString str;
    HDataField *f;

    str = "";
    if(myInterface()->hsqli_need_create_sequence_before)
    {
        str += myInterface()->hsqli_sequence_creation_string + "; \n";
    }

    str += QString("CREATE TABLE %1 \n (").arg(sqlTable);
    first = true;
    firstField();
    while((f=nextField()) != NULL)
    {
        if(f->isSQLField())
        {
            if(first)
                first = false;
            else
                str += ", ";
            str += "\n\t " + f->sqlCreateStringPart(switches) + " ";
        }
    }
    str += "\n ); \n";

    if(myInterface()->hsqli_need_create_sequence_before)
        str.replace("_SEQNAME_",sqlTable+"_pkey_sequence",Qt::CaseSensitive);

    return str;
}

void HTableBase::setShowAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->setShow();
}

void HTableBase::setHideAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
         if(df->hasMark(mark))
            df->setHide();

}

void HTableBase::setEditableAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->setEditable();

}

void HTableBase::setReadolnyAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->setReadonly();
}

void HTableBase::setColorAllMarked(QString mark,int r,int g,int b)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->setColor(r,g,b);
}

void HTableBase::returnToDefaultAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->returnToDefault();
}

void HTableBase::setGuiEnabledAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->enableControlSlot();
}

void HTableBase::setGuiDisabledAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->disableControlSlot();
}

void HTableBase::blockReInitializeBase(QString hide_marker,QString ro_marker)
{
    setShow();
    setEditable();
    setHideAllMarked(hide_marker);
    setReadolnyAllMarked(ro_marker);
    removeSqlCooseButtons();
}

void HTableBase::removeSqlCooseButtons(void)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->getWhoami() == "HSqlChoose")
        {
            ((HSqlChoose *)df)->removeToolButton();
        }
}

void HTableBase::useRole(QString name)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        df->useRole(name);
}

void HTableBase::delRole(QString name)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        df->delRole(name);
}

void HTableBase::clearRoles(void)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        df->clearRoles();
}

void HTableBase::sqlSynced(HDataChangeLogger *dclog,bool forcechange,bool forcelog)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        df->sqlSynced(dclog,forcechange,forcelog);
}

void HTableBase::clearSqlSynced(void)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        df->clearSqlSynced();
}

bool HTableBase::isReallyChanged(void)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->isReallyChanged())
            return true;
    return false;
}

void HTableBase::resetLogging(void)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        df->resetLogging();
}

void HTableBase::setLogging(void)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        df->setLogging();
}

void HTableBase::setLoggingAllMarked(QString mark)
{
    HDataField *df=NULL;

    firstField();
    while((df = nextFieldAll()))
        if(df->hasMark(mark))
            df->setLogging();
}

/////////////////////////////////////////////////////////////////////////////////
////// HTable ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HTable::HTable(QString t)
: HTableBase(t)
{
    whoami = "HTable";
    ro_mask = false;
    reread_key = false;
    insertedKey = "";
    dclog = NULL;
}

HTable::~HTable(void)
{
    sdebug("*** HTable::~HTable ***");
    freeTable();
    sdebug("*** HTable::~HTable *** END");
}

HTable::HTable(HTable *t)
: HTableBase("")
{
    reread_key = false;
    insertedKey = "";
    deepcopy_from_htable(t);
}

void HTable::deepcopy_from_htable(HTable *x)
{
    deepcopy_from_htablebase(x);

    ro_mask = x->ro_mask;
    dclog = x->dclog;
}

void HTable::setSqlFieldValue(QString sqln,QVariant v,bool sdisable)
{
    HDataField  *f=NULL;

    f = fieldBySqlName(sqln);
    if(f == NULL)
        return;
    f->setValue(v,sdisable);
    if(!sdisable)
        dataUpdated();
}

QVariant HTable::getSqlFieldValue(QString sqln)
{
    HDataField  *f=NULL;

    f = fieldBySqlName(sqln);
    if(f == NULL)
        return QVariant("");

    return f->getValue();
}

QString HTable::sqlUpdate(bool all)
{
  sdebug("*** HTable::sqlUpdate ***");
  bool begin=false,modified=false,first=false;
  int i=0;

  QString ret="",part="",keyname="",keyvalue="";

  HDataField_List *tmp=table;
  for(;tmp != NULL;tmp=tmp->nextall)
  {
      if(i==0 && !begin) //before the first field
      {
            part=QString("UPDATE %1 SET ").arg(tmp->data->conn_sqlTable);
            begin = true;
            first = true;
            modified=false;
      }

      if(tmp->data->isSQLField()) //field is intrested in sql database
      {
          if(tmp->data->getWhoami() == "HKey")
          {
              keyname  = tmp->data->sqlSelectHead();
              keyvalue = tmp->data->getSQLValue();
              continue;
          }
          if(tmp->data->getWhoami() == "HConnect")
              continue;

          if( (tmp->data->isEditable() &&                //if the field is editable
               tmp->data->isSet()         ) |           // and the value is set (set status is ture)
              (tmp->data->getWhoami() == "HTimestamp" &&     // Or this is a timestam WHICH HAVE
              ((HTimestamp *)tmp->data)->is_checkenabled())  //     chech/control function (always have to update)
            )
          {
                /*It's true, that the timastamps always have to be updated, but I don't enabled the SET/MODIFIED status
                  because if only the timestamp is updated it's unnecessary to update the table.
                  The update is only necessary if any other field is updated except the timestamp
                  that case the modifies=true is set.     */
                if(tmp->data->getWhoami() != "HTimestamp")
                    modified = true;

                if(!first) //dont need ; on the first element
                {
                    part.append(",");
                }
                else
                {
                    first = false;
                }
                part.append(tmp->data->sqlUpdate());
          }
          i++;
       }
      if(tmp->next == NULL) //after the last field in the table (without connections)
        {
            i=0;
            if(keyname == "" || keyvalue == "")
            {
                sdebug("*** HTable::sqlUpdate *** END with empty string");
                return QString("");
            }

            part.append(" WHERE "+keyname+"="+keyvalue+"; \n");
            if(modified)
                ret.append(part);
            if(!all)
            {
                sdebug("*** HTable::sqlUpdate *** ("+ret+")");
                return ret;
            }
            begin = false;
        }
  }
  sdebug("*** HTable::sqlUpdate *** ("+ret+")");
  return ret;
}

QVariant HTable::operator [](int i)
{
    int idx=0;
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(idx == i)
        {
            if(tmp->data == NULL) return QVariant("");
            else                  return tmp->data->getValue();
        }
        tmp=tmp->next;
        ++idx;
    }
    return QVariant("");
}


QString HTable::getKey(bool head_val,QString intable)
{
    QString in_table;
    if(intable.isEmpty())
        in_table = sqlTable;
    else
        in_table = intable;

    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL && tmp->data->getWhoami() == "HKey" && in_table == tmp->data->conn_sqlTable)
        {
            if(head_val)
            {
                return tmp->data->sqlSelectHead();
            }
            else
            {
                return tmp->data->getValue().toString();
            }
        }
        tmp=tmp->nextall;
    }
    return QString("");
}

int HTable::setKey(QString key)
{
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL && tmp->data->getWhoami() == "HKey")
        {
            tmp->data->setValue(key);
            return 0;
        }
        tmp=tmp->next;
    }
    return 1;
}

int HTable::updateWithKey(QString key,bool all,bool update,bool tdisabled)
{
    int idx;
    HDataField_List *tmp=table;
    QString sqlq;

    sdebug("*** HTable::updateWithKey *** ");
    if(!key.isEmpty())
         if(setKey(key))
              return 1;

    /*Have to step through on the data fields and examine if any field have any sql dependency.
      for example if an sqlchoose field found it is necessary to read de depended table
      because it can contains a new value which is are not in the table on the last read */
    HDataField_List *refresh_tmp=table;
    while(refresh_tmp != NULL)
    {
        if(refresh_tmp->data->getWhoami() == "HSqlChoose")
            ((HSqlChoose *)refresh_tmp->data)->refreshSqlValues(false,tdisabled);
        refresh_tmp=refresh_tmp->next;
    }
    //That's it.

    if(!tdisabled)
        transaction();

    if(all)
        sqlq=sqlSelectWkAll();
    else
        sqlq=sqlSelectWk();

    sqlq.append(";");
    QSqlQuery *q = allocateQSqlQuery();
    sqldebug(sqlq);
    q->prepare(sqlq);
    q->exec();

    if((myInterface()->hsqli_usequerysize && q->numRowsAffected() != 1)  ||
        ! q->next()                                       ||
       q->lastError().type() != QSqlError::NoError)
    {
        if(!tdisabled)
            rollback();
        errstr=("The SELECT returned with !=1 record! (1)\n>>"+((q->lastError()).text()));
        errstr+="\nQuery was: \""+sqlq+"\"\n(Sql name of the table:"+sqlTable+")\n";
        delete q;
        emit errorSignal(errstr);
        return 1;
    }

    idx=0;
    tmp=table;

    while(tmp != NULL)
    {
        if(tmp->data != NULL && tmp->data->isSQLField())
        {
            /* In the following if I examine if it is a refresh of a timestamp field with check controll option.
                In that case it's help to prevent the concurent writing. I tells to the node that i refreshed the value of the timestamp now .
                If i found a different timestamp during the save i will know that somody updated the record in the database.
                That case it have to prevent the store and sign an error    */
            if(tmp->data->getWhoami() == "HTimestamp" && ((HTimestamp *)tmp->data)->is_checkenabled() )
                ((HTimestamp *)tmp->data)->read_from_sql();

            tmp->data->setValue(q->value(idx),true);
            tmp->data->notSet();

            idx++;
        }
        if(all)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;
    }
    delete q;
    if(!tdisabled)
        commit();

    emit dataChanged();
    if(update)
        dataUpdated();

    sqlSynced();
    sdebug("*** HTable::updateWithKey *** END");
    return 0;
}

int HTable::returnToDefault(bool all)
{
    sdebug("*** HTable::returnToDefault ***");
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL)
        {
            QString dv;
            tmp->data->notSet();
            dv = tmp->data->getDefaultValue();
            tmp->data->setDefval(dv);
            tmp->data->resetValidateStatus();
        }
        if(all)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;
    }
    sdebug("*** HTable::returnToDefault *** END");

    dataUpdated();
    return 0;
}

int HTable::saveRecord(bool all,bool tdisabled)
{
    HDataField_List *tmp=table;
    QString ts_new="empty";
    QString ts_old="empty";

    sdebug("*** HTable::saveRecord *** ");
    if(!((errstr=validate(all)).isEmpty()))
    {
        emit errorSignal(errstr);
        sdebug("*** HTable::saveRecord *** ERROR1");
        return 1;
    }

    if(!tdisabled)
        transaction();

    tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data != NULL &&
           tmp->data->isSQLField() &&                      // sql related field
           tmp->data->getWhoami() == "HTimestamp" &&       // timestamp AND
           ((HTimestamp *)tmp->data)->is_checkenabled() && // have check/controll option AND
           ((HTimestamp *)tmp->data)->is_sqlreaded()       // the value is already readed
           )
        {
            QString sqlq=QString("SELECT %1 FROM %2 WHERE %3=\'%4\';")
                                            .arg(tmp->data->sqlSelectHead())
                                            .arg(tmp->data->conn_sqlTable)
                                            .arg(getKey(HEAD,tmp->data->conn_sqlTable))
                                            .arg(getKey(VALUE,tmp->data->conn_sqlTable));

            ts_old = tmp->data->getValue().toString(); //queries the stored timestamp valee

            //queryes the current value of the timestamp from the database
            QSqlQuery *q = allocateQSqlQuery();
            sqldebug(sqlq);
            q->prepare(sqlq);
            q->exec();
            if((myInterface()->hsqli_usequerysize && q->numRowsAffected() != 1 ) ||
                ! q->next()                                       ||
                q->lastError().type() != QSqlError::NoError)
                {

                    if(!tdisabled)
                        rollback();
                    errstr=("The SELECT returned with !=1 record! (TimeStamp Check- GetTimeStamp)\n>>"+
                                                    ((q->lastError()).text()));
                    delete q;
                    emit errorSignal(errstr);
                    sdebug(errstr);
                    sdebug("*** HTable::saveRecord *** ERROR2");
                    return 1;
                }
            ts_new = q->value(0).toString();

            //compare the two value
            if(ts_old != ts_new)
                {
                    if(!tdisabled)
                        rollback();
                    delete q;

                    errstr=QString("The data of the record is out of date in table \"%1 : %2\"!"
                                   "\nUpdate aborted! \n"
                                   "(The record has utomatically re-loaded try again the operation!)")
                                     .arg(tmp->data->conn_sqlTable)
                                     .arg(tmp->data->sqlSelectHead());

                    emit errorSignal(errstr);
                    sdebug(errstr);
                    updateWithKey("",all);
                    sdebug("*** HTable::saveRecord *** ERROR3");
                    return 1;
                }
            delete q;
        }
        if(all)
            tmp=tmp->nextall;
        else
            tmp=tmp->next;
    }

    //I queried the timestamp and checked. Let's do the update!

    QString sqlu="";

    sqlu = sqlUpdate(all);

    if(sqlu.isEmpty())
    {
        if(!tdisabled)
            rollback();
        sdebug("There is no relevant changes:emtpty update generated, exiting.");
        return 0;
    }

    QSqlQuery *qu = allocateQSqlQuery();

    sqldebug(sqlu);
    qu->prepare(sqlu);
    qu->exec();

    if(qu->lastError().type() != QSqlError::NoError)
        {
            if(!tdisabled)
                rollback();
            errstr=("An error occured during the update! :-(\n>>"+((qu->lastError()).text()));
            sdebug("SQLERROR: "+qu->lastError().text());
            delete qu;
            emit errorSignal(errstr);
            sdebug("*** HTable::saveRecord *** ERROR4");
            return 1;
        }
    if(!tdisabled)
        commit();

    delete qu;

    //logger begin
    if(dclog != NULL)
    {
        dclog->setTablename(sqlTableName());
        dclog->setChangedKey(getKey(false));
    }
    sqlSynced(dclog);
    if(dclog != NULL)
    {
        dclog->setTablename("EMPTY");
        dclog->setChangedKey("");
    }
    //logger end

    updateWithKey("",all,true,tdisabled);

    //logger write out
    if(dclog != NULL)
        dclog->writeOut(tdisabled);

    sdebug("*** HTable::saveRecord *** END");
    emit dataModifiedSignal();
    return 0;
}

int HTable::insertRecord(bool insertkey,bool disable_tr,bool resetafterinsert)
{
    QString sqli="";

    sdebug("*** HTable::insertRecord ***");

    if(!((errstr=validate(false)).isEmpty()))
    {
        emit errorSignal(errstr);
        return 1;
    }

    if(!disable_tr)
        transaction();

    sqli = sqlInsert(insertkey);
    QSqlQuery *qi = allocateQSqlQuery();

    sqldebug(sqli);
    qi->prepare(sqli);
    qi->exec();

    if(qi->lastError().type() != QSqlError::NoError)
        {
            if(!disable_tr)
                rollback();

            errstr=("An error occured during the insert!\n>>"+((qi->lastError()).text()));
            delete qi;
            emit errorSignal(errstr);
            return 1;
        }

    if(reread_key)
    {
        reread_key = false;

        insertedKey = getKeyValueAfterInsert();
        if(query_error_occured)
        {
            query_error_occured = false;
            if(!disable_tr)
                rollback();

            insertedKey = "";
            emit errorSignal("Error in HTable::getKeyValueAfterInsert !");
            delete qi;
            return 1;
        }
    }

    if(!disable_tr)
        commit();

    delete qi;
    setKey("");
    if(resetafterinsert)
    {
        returnToDefault();
        dataUpdated();
        emit dataModifiedSignal();
    }
    sdebug("*** HTable::insertRecord *** END");
    return 0;
}

int HTable::insertRecordAll(bool disable_tr)
{
    QString sqli="";

    sdebug("*** HTable::insertRecordAll ***");

    if(!((errstr=validate(true)).isEmpty()))
    {
        emit errorSignal(errstr);
        return 1;
    }

    if(!disable_tr)
        transaction();

    sqli = sqlInsertAll();

    sqldebug(sqli);

    QSqlQuery *qi = allocateQSqlQuery();
    qi->prepare(sqli);
    qi->exec();

    if(qi->lastError().type() != QSqlError::NoError)
        {
            if(!disable_tr)
                rollback();

            errstr=("An error occured during the insert!\n>>"+((qi->lastError()).text()));
            delete qi;
            emit errorSignal(errstr);
            return 1;
        }

    if(!disable_tr)
        commit();

    delete qi;
    setKey("");
    returnToDefault();
    dataUpdated();
    emit dataModifiedSignal();
    sdebug("*** HTable::insertRecordAll *** END");
    return 0;
}

QString HTable::sqlInsert(bool insertkey)
{
  int i=0;
  QString ret;

  HDataField_List *tmp=table;
  ret = "INSERT INTO "+sqlTable+"(";
  for(;tmp != NULL;tmp=tmp->next)
  {
      if(!insertkey && (tmp->data->getWhoami() == "HKey")) continue;
      if(tmp->data->isSQLField()) //field is intrested in sql database
      {
          if(i != 0) ret.append(",");
          ret.append(tmp->data->sqlInsertHead());
          i++;
      }
  }
  ret.append(") VALUES(");
  tmp=table;
  i=0;
  for(;tmp != NULL;tmp=tmp->next)
  {
      if(!insertkey && (tmp->data->getWhoami() == "HKey")) continue;
      if(tmp->data->isSQLField()) //field is intrested in sql database
      {
          if(i != 0) ret.append(",");
          ret.append(tmp->data->sqlInsertValue());
          i++;
      }
  }
  ret.append(")");
  return ret;
}

QString HTable::sqlInsertAll(void)
{
  int i=0;
  QString insert;
  QString dep;
  QString tname;
  HDataField_List *run,*mainrun=table;

  QStringList *inserts    =new QStringList();
  QStringList *dependency =new QStringList();
  QStringList *tablenames =new QStringList();
  QList<bool> *done  =new QList<bool>();

  insert = "";
  inserts->clear();
  dependency->clear();
  tablenames->clear();
  while(mainrun != NULL)
  {
      i=0;
      insert = "";
      run = mainrun;
      if(run->data == NULL)
      {
          emit errorSignal("Internal error!");
          sdebug("ERROR : Internal error!");
          return "";
      }
      tname = run->data->conn_sqlTable;
      dep   = "NULL";


      insert.append("INSERT INTO "+run->data->conn_sqlTable+"(");
      for(;run != NULL;run=run->next)
          {
              if(run->data == NULL)
              {
                emit errorSignal("Internal error!");
                sdebug("ERROR : Internal error!");
                return "";
              }
              if(run->data->getWhoami() == "HKey")
              {
                        //The key is generated. We just ignore is
                        continue;
              }

              if(run->data->isSQLField()) //sql related
              {
                  if(i != 0) insert.append(",");
                  insert.append(run->data->sqlInsertHead());
                  i++;
              }
          }
        insert.append(") VALUES(");

        i=0;
        for(run=mainrun;run != NULL;run=run->next)
        {
            if(run->data->getWhoami() == "HKey")
            {
                //The key is generated. We just ignore is
                continue;
            }
            if(run->data->getWhoami() == "HConnect")
            {
                if(i != 0) insert.append(",");
                insert.append(specifyKey(((HConnect *)run->data)->getConnectedTable()));
                i++;
                dep = ((HConnect *)run->data)->getConnectedTable();
                continue;
            }

            if(run->data->isSQLField()) //sql related
            {
                if(i != 0) insert.append(",");
                insert.append(run->data->sqlInsertValue());
                i++;
            }
        }

        insert.append(")");

        //Store the current insert with info's.
        inserts->push_back(insert);
        tablenames->push_back(tname);
        dependency->push_back(dep);
        done->push_back(false);

        //Step to the next table
        while(mainrun->next != NULL)
            mainrun=mainrun->next;
        mainrun=mainrun->nextall;
  }

  insert = "";

  ///////////////////////////////////
  // Dependency check, and sorting //

  int ttl,j;
  bool finish=false;

  sdebug("Start dependency analysing...");

  //sdebug("DEP:"+dependency->join("  \n"));
  //sdebug("NAMES:"+tablenames->join("  \n"));

  //Collect the independent tables
  for(i=0;i<(int)dependency->size();++i)
      if( (*dependency)[i] == "NULL")
      {
          (*done)[i] = true;
          insert.append((*inserts)[i] + "; \n");
      }

  ttl=done->size()*done->size();
  do
  {
      sdebug(QString("next-round ttl:%1").arg(ttl));
      //Look if we have unfinished table
      finish = true;
      for(i=0;i<(int)done->size();++i)
          if(!((*done)[i]))
              finish = false;

    //We try todo a new one
    if(!finish)
    {
          for(i=0;i<(int)dependency->size();++i)
              if(!((*done)[i])) //Not finished yet
                  for(j=0;j<(int)dependency->size();++j)
                      if((*done)[j]) //Finished
                          if( (*dependency)[i] == (*tablenames)[j] )
                          {

                               (*done)[i] = true;
                               insert.append((*inserts)[i] + "; \n");
                          }
    }

    if((ttl--) == 0)
    {
        emit errorSignal("INSERT: dependency error!");
        sdebug("ERROR : INSERT: dependency error!");
        return "";
    }
  }
  while(!finish);

  delete inserts;
  delete dependency;
  delete tablenames;
  delete done;

  return insert;
}

QString HTable::getKeyValueAfterInsert(void)
{
    sdebug("*** HTable::getKeyValueAfterInsert ***");
    QString result,query,keyname="",timestampname="";

    HDataField_List *tmp=table;
    for(;tmp != NULL;tmp=tmp->nextall)
    {
        if(tmp->data->isSQLField()) //sql related
        {
            if(tmp->data->getWhoami() == "HKey")
                keyname = tmp->data->sqlSelectHead();
            if(tmp->data->getWhoami() == "HTimestamp" && ((HTimestamp *)tmp->data)->is_checkenabled())
                timestampname = tmp->data->sqlSelectHead();
        }
    }

    if(timestampname.isEmpty())
    {
        timestampname = keyname;
    }

    query = QString("SELECT %1 FROM %2 ORDER BY %4 DESC LIMIT 1")
                .arg(keyname)
                .arg(sqlTable)
                .arg(timestampname);

    result = submit1ResultQuery(query,"Error in HTable::getKeyValueAfterInsert! Cannot get the keyvalue of the inserted record",
                        true).toString();

    sdebug("*** HTable::getKeyValueAfterInsert *** END");
    return result;
}


QString HTable::specifyKey(QString tablename)
{
    QString ret,keyname,spec,ts;
    HDataField_List *run=table;

    while(run != NULL)
    {
        if(run->data != NULL && run->data->conn_sqlTable == tablename)
            break;
        run = run->nextall;
    }
    if(run == NULL)
    {
        emit errorSignal("Cannot connect tables: Cant find table"+tablename);
        return "NULL";
    }

    keyname = "";
    spec="";
    ts = "";

    for(;run != NULL;run = run->next)
    {
        if(run->data != NULL && run->data->getWhoami() == "HKey")
        {
            keyname = run->data->sqlSelectHead();
            continue;
        }
        if(run->data != NULL && run->data->getWhoami() == "HTimestamp")
        {
            ts = run->data->sqlSelectHead();
            continue;
        }

        if(run->data != NULL && run->data->isSQLField())
        {
            if(!spec.isEmpty())
                spec.append(" AND ");
            spec.append(run->data->sqlSelectHead() + "="+run->data->getSQLValue());
        }
    }

    ret=QString("( SELECT %1 FROM %2 WHERE %3 %4 LIMIT 1)")
                    .arg(keyname)
                    .arg(tablename)
                    .arg(spec)
                    .arg( (ts.isEmpty() ? "" : (" ORDER BY "+ts+" DESC")) );

    return ret;
}

void HTable::blockReInitialize(QString hide_marker,QString ro_marker)
{
    returnToDefault();
    blockReInitializeBase(hide_marker,ro_marker);
}

/////////////////////////////////////////////////////////////////////////////////
////// HFloatTables ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HFloatTables::HFloatTables(HTable *baserec)
{
    sdebug("*** HFloatTables::HFloatTables ***");
    if(baserec != NULL)
    {
        baserecord = baserec;
        sqlTable = baserecord->sqlTableName();
        cloneSqlSettingsFrom(baserec);
    }
    else
    {
        emit errorSignal("Error: NULL baserecord passed...");
        sdebug("Error in HFloatTables::HFloatTables : NULL baserecord passed...");
        baserecord = new HTable("null");
        sqlTable = baserecord->sqlTableName();
    }

    sts  = new QList<int>();
    tbls = new QList<HTable *>();

    siter=sts->begin();
    titer=tbls->begin();
    fiter=true;
    sdebug("*** HFloatTables::HFloatTables *** END");
}

int HFloatTables::clearMem(void)
{
    HTable *t;
    QList<HTable *>::iterator i;
    i = tbls->begin();
    while(i != tbls->end())
    {
        t = *(i++);
        if(t != NULL)
            delete t;
    }

    sts->clear();
    tbls->clear();
    siter=sts->begin();
    titer=tbls->begin();
    fiter=true;
    return 0;
}

HFloatTables::~HFloatTables(void)
{
    sdebug("*** HFloatTables::~HFloatTables ***");
    clearMem();
    delete sts;
    delete tbls;
    delete baserecord;
    sdebug("*** HFloatTables::~HFloatTables *** END");
}

void HFloatTables::first(void)
{
    fiter=true;
    siter=sts->begin();
    titer=tbls->begin();
}

HTable *HFloatTables::nextRecord(void)
{
    HTable *t;

    t = NULL;

    if(fiter)
    {
        siter=sts->begin();
        titer=tbls->begin();
        fiter = false;
    }

    while(titer != tbls->end())
    {
        if(*siter == FLOATTABLES_STS_ALIVE || *siter == FLOATTABLES_STS_NEW)
            if(*titer != NULL)
            {
                t = *titer;
                ++siter;
                ++titer;
                return t;
            }
        ++siter;
        ++titer;
    }
    return t;
}

QString HFloatTables::nextKey(void)
{
    HTable *t;

    if((t = nextRecord()) == NULL)
        return "";
    return t->getKey(false);
}

void HFloatTables::setAllField(QString fname,QVariant value)
{
    HTable *t;
    first();
    while((t=nextRecord()) != NULL)
        t->fieldBySqlName(fname)->setValue(value);
}

void HFloatTables::allAliveToNew(void)
{
    QList<int>::iterator si;
    si = sts->begin();
    while(si != sts->end())
    {
        if(*si == FLOATTABLES_STS_ALIVE)
            *si = FLOATTABLES_STS_NEW;
        ++si;
    }
}

int HFloatTables::countAllStat(void)
{
    return sts->count();
}

int HFloatTables::count(void)
{
    int num;

    num=0;

    QList<int>::iterator si;
    si = sts->begin();
    while(si != sts->end())
    {
        if(*si == FLOATTABLES_STS_ALIVE || *si == FLOATTABLES_STS_NEW)
            ++num;
        ++si;
    }
    return num;
}

HTable *HFloatTables::getTByField(QString field,QString key)
{
    QList<int>::iterator si;
    QList<HTable *>::iterator ti;

    ti = tbls->begin();
    si = sts->begin();
    while(ti != tbls->end())
    {
        if(*si == FLOATTABLES_STS_ALIVE || *si == FLOATTABLES_STS_NEW)
            if( ((*ti)->fieldBySqlName(field))->getValue().toString() == key )
                return (*ti);
        ++ti;
        ++si;
    }
    return NULL;
}

HTable *HFloatTables::getTByKey(QString key)
{
    QList<int>::iterator si;
    QList<HTable *>::iterator ti;

    ti = tbls->begin();
    si = sts->begin();
    while(ti != tbls->end())
    {
        if(*si == FLOATTABLES_STS_ALIVE || *si == FLOATTABLES_STS_NEW)
            if( (*ti)->getKey(false) == key)
                return (*ti);
        ++ti;
        ++si;
    }
    return NULL;
}

HTable *HFloatTables::getTByNum(int idx)
{
    int i;
    QList<int>::iterator si;
    QList<HTable *>::iterator ti;

    i=0;
    ti = tbls->begin();
    si = sts->begin();
    while(ti != tbls->end())
    {
        if(*si == FLOATTABLES_STS_ALIVE || *si == FLOATTABLES_STS_NEW)
        {
            if( i == idx )
                return (*ti);
            ++i;
        }
        ++ti;
        ++si;
    }
    return NULL;
}

HTable *HFloatTables::createRecord(void)
{
    HTable *newrec;

    newrec=new HTable(baserecord);
    newrec->cloneSqlSettingsFrom(this);
    tbls->push_back(newrec);
    sts->push_back(FLOATTABLES_STS_NEW);

    return newrec;
}

HTable *HFloatTables::createRecordAsUndef(void)
{
    HTable *newrec;

    newrec=new HTable(baserecord);
    newrec->cloneSqlSettingsFrom(this);
    tbls->push_back(newrec);
    sts->push_back(FLOATTABLES_STS_UNDEF);

    return newrec;
}

void HFloatTables::markedRecordToNew(HTable *r)
{
    sdebug("*** HFloatTables::markedRecordToNew ***");
    QList<int>::iterator si;
    QList<HTable *>::iterator ti;

    ti = tbls->begin();
    si = sts->begin();
    while(ti != tbls->end())
    {
        if(*ti == r)
            if( *si == FLOATTABLES_STS_UNDEF || *si == FLOATTABLES_STS_ALIVE)
            {
                *si = FLOATTABLES_STS_NEW;
                sdebug("WHOAMI:"+(*ti)->getWhoami());
                sdebug("*** HFloatTables::markedRecordToNew *** END.OK");
                return;
            }
        ++ti;
        ++si;
    }
    sdebug("*** HFloatTables::markedRecordToNew *** END.NOTFOUND");
}

void HFloatTables::removeUndef(void)
{
    sdebug("*** HFloatTables::removeUndef ***");
    QList<int>::iterator si;
    QList<HTable *>::iterator ti;

    ti = tbls->begin();
    si = sts->begin();
    while(ti != tbls->end())
    {
        if(*si == FLOATTABLES_STS_UNDEF)
        {
            *ti = NULL;
            *si = FLOATTABLES_STS_CANCELLED;
        }
        ++ti;
        ++si;
    }
    sdebug("*** HFloatTables::removeUndef *** END");
}

int HFloatTables::deleteRecord(HTable *d)
{
    QList<int>::iterator si;
    QList<HTable *>::iterator ti;

    ti = tbls->begin();
    si = sts->begin();
    while(ti != tbls->end())
    {
        if(*si == FLOATTABLES_STS_ALIVE || *si == FLOATTABLES_STS_NEW)
        {
            if( d == *ti )
            {
                if(*si == FLOATTABLES_STS_ALIVE)
                    *si = FLOATTABLES_STS_DELETED;
                if(*si == FLOATTABLES_STS_NEW)
                    *si = FLOATTABLES_STS_CANCELLED;
                return 0;
            }
        }
        ++ti;
        ++si;
    }
    return 1;
}

int HFloatTables::deleteRecord(int idx)
{
    return deleteRecord(getTByNum(idx));
}

int HFloatTables::readDBTable(QString filter,bool tdisabled)
{
    sdebug("*** HFloatTables::readDBTable ***");
    HTable *t;
    QString qs;
    QStringList keys;
    QSqlQuery *q;

    if(!tdisabled)
        transaction();

    qs = QString("SELECT %1 FROM %2 WHERE %3 %4;")
        .arg(baserecord->getKey(true))
        .arg(sqlTable)
        .arg(myInterface()->hsqli_truevalue)
        .arg(filter);

    sqldebug(qs);
    q = allocateQSqlQuery();
    q->prepare(qs);
    q->exec(qs);
    if(q->lastError().type() != QSqlError::NoError)
    {
        if(!tdisabled)
            rollback();

        delete q;
        emit errorSignal("Error: Cannot exec the query in HFloatTables::readDBTable!");
        return 1;
    }

    keys.clear();
    while(q->next())
        keys.push_back(q->value(0).toString());

    clearMem();

    QStringList::iterator i;
    i = keys.begin();
    while(i != keys.end())
    {
        t = new HTable(baserecord);

        t->setKey(*i);
        tbls->push_back(t);
        sts->push_back(FLOATTABLES_STS_ALIVE);
        if(t->updateWithKeyAll(*i,true,true))
        {
            if(!tdisabled)
                rollback();
            delete q;
            emit errorSignal("Error: Cannot read a record: "+t->errstr);
            return 1;
        }
        ++i;
    }

    if(!tdisabled)
        commit();

    sdebug("*** HFloatTables::readDBTable ***END");
    return 0;
}

int HFloatTables::saveDBTable(bool tdisabled)
{
    sdebug("*** HFloatTables::saveDBTable ***");
    QString k;
    QList<int>::iterator si;
    QList<HTable *>::iterator ti;

    if(!tdisabled)
        transaction();

    ti = tbls->begin();
    si = sts->begin();
    while(ti != tbls->end())
    {
        switch(*si)
        {
            case FLOATTABLES_STS_UNDEF:
                break;
            case FLOATTABLES_STS_CANCELLED:
                break;
            case FLOATTABLES_STS_ALIVE:
                if((*ti)->isChanged())
                    if((*ti)->saveRecord(false,true))
                    {
                        if(!tdisabled)
                            rollback();
                        emit errorSignal("Cannot update a HFloatTable record(Status:ALIVE): "+ (*ti)->errstr );
                        return 1;
                    }
                break;
            case FLOATTABLES_STS_NEW:
                (*ti)->rereadNextInsertedKey();
                if((*ti)->insertRecord(false,true))
                {
                    if(!tdisabled)
                        rollback();
                    emit errorSignal("Cannot insert a HFloatTable record(Status:NEW): "+ (*ti)->errstr );
                    return 1;
                }
                k = (*ti)->getInsertedKey();
                //READ
                if((*ti)->updateWithKeyAll(k,true,true))
                {
                    if(!tdisabled)
                        rollback();
                    emit errorSignal("Cannot reread an inserted new record: "+ (*ti)->errstr );
                    return 1;
                }

                *si = FLOATTABLES_STS_ALIVE;
                break;
            case FLOATTABLES_STS_DELETED:
                if(submit0ResultQuery(QString("DELETE FROM %1 WHERE %2=\'%3\';")
                                                .arg(sqlTable)
                                                .arg((*ti)->getKey(true))
                                                .arg((*ti)->getKey(false)),"Err",true))
                {
                    if(!tdisabled)
                        rollback();
                    emit errorSignal("Cannot delete erased record!");
                    return 1;
                }
                delete *ti;
                *ti=NULL;
                *si=FLOATTABLES_STS_CANCELLED;
                break;

        }
        ++ti;
        ++si;
    }

    if(!tdisabled)
        commit();

    sdebug("*** HFloatTables::saveDBTable *** END");
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
////// HList ////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HList::HList(QString t)
: HTableBase(t)
{
    sdebug("*** HList::HList ***");
    soft_current_key = "";
    whoami = "HList";
    active_key = "";
    keys   = new QStringList();
    values = new QList<QStringList *> ();

    last_filter = "";
    last_all = true;

    if(HRefreshAgent::getNotifyAgent() != NULL)
        connect(HRefreshAgent::getNotifyAgent(),SIGNAL(getnotify(QString)),
                this,SLOT(captureNotify(QString)));

    query_valiter = values->begin();
    query_keyiter = keys->begin();

    sdebug("*** HList::HList *** END");
}

HList::~HList(void)
{
    sdebug("*** HList::~HList ***");
    freeTable();
    if(keys != NULL)
        delete keys;
    if(values != NULL)
    {
        for(QList<QStringList *>::iterator it = values->begin();it != values->end();++it)
            delete *it;
        delete values;
    }
    sdebug("*** HList::~HList *** END");
}

HList::HList(HList *t)
: HTableBase("")
{
    soft_current_key = "";
    whoami = "HList";
    active_key = "";
    keys   = new QStringList();
    values = new QList<QStringList *> ();
    last_filter = "";
    last_all = true;

    deepcopy_from_hlist(t);

    if(HRefreshAgent::getNotifyAgent() != NULL)
        connect(HRefreshAgent::getNotifyAgent(),SIGNAL(getnotify(QString)),
                this,SLOT(captureNotify(QString)));
}



void HList::deepcopy_from_hlist(HList *x)
{
    deepcopy_from_htablebase(x);
    whoami = x->whoami;
    clearList();
    active_key = x->active_key;
    soft_current_key = x->soft_current_key;
    last_filter = x->last_filter;
    last_all = x->last_all;


    for(QList<QStringList *>::iterator it = x->values->begin();it != x->values->end();++it)
    {   values->push_back(deep_copy_stringlist(new QStringList(),*it)); }

    deep_copy_stringlist(keys,x->keys);
 }


int HList::captureNotify(QString tblname)
{
    sdebug("*** HList::captureNotify ***");
    QStringList::iterator i;

    sdebug(QString(" %1 == %2").arg(tblname).arg(sqlTable));
    if(tblname == sqlTable)
        return reReadList();
    i = depend->begin();
    while(i != depend->end())
    {
        if(*i == tblname)
            return reReadList();
        ++i;
    }
    return 0;
}

int HList::clearList(void)
{
    sdebug("*** HList::clearList ***");
    for(QList<QStringList *>::iterator it = values->begin();it != values->end();++it)
        delete (*it);

    values->clear();
    keys->clear();
    active_key = "";
    sdebug("*** HList::clearList *** END");
    return 0;
}

int HList::actLine(void)
{
    sdebug("*** HList::actLine ***");
    emit actionOnRecord(active_key);
    sdebug("*** HList::actLine *** END");
    return 0;
}

int HList::alternateActLine(void)
{
    sdebug("*** HList::alternateActLine ***");
    emit alternateActionOnRecord(active_key);
    sdebug("*** HList::alternateActLine *** END");
    return 0;
}

QString HList::getSqlFieldValue(QString sqln)
{
    int idx=0;
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data->isSQLField() && tmp->data->isShow())
        {
            if(tmp->data->sqlSelectHead() == sqln)
            {
                QStringList *row;

                row  = (*values)[getIndexOf(*keys,active_key)];
                return (*row)[idx];
            }
            ++idx;
        }
        tmp=tmp->nextall;
    }
    return QString("");
}

int HList::readList(QString filter,bool all)
{
    int serial;
    sdebug("*** HList::readList ***");

    last_filter = filter;
    last_all = all;

    QStringList *row_values;
    QString row_key;

#ifdef PROGRESS_POSSIBILITY
    emit startWorking();
#endif

    for(QList<QStringList *>::iterator it = values->begin();it != values->end();++it)
        delete *it;
    values->clear();
    keys->clear();

    /*Have to step through on the data fields and examine if any field have any sql dependency.
      for example if an sqlchoose field found it is necessary to read de depended table
      because it can contains a new value which is are not in the table on the last read */
    HDataField_List *refresh_tmp=table;
    while(refresh_tmp != NULL)
    {
        if(refresh_tmp->data->getWhoami() == "HSqlChoose")
            ((HSqlChoose *)refresh_tmp->data)->refreshSqlValues(false);
        refresh_tmp=refresh_tmp->next;
    }

    //That's it
#ifdef PROGRESS_POSSIBILITY
    emit doWorking();
#endif

    int idx;
    HDataField_List *tmp=NULL;
    QString sqlq;

    transaction();

    if(all)
        sqlq=sqlSelectAll(filter);
    else
        sqlq=sqlSelect(filter);

    sqlq.append(";");

    //Make the copnnection and send the command and check the status...
    QSqlQuery *q = allocateQSqlQuery();
    sqldebug(sqlq);
    q->prepare(sqlq);
    q->exec();

    if(q->lastError().type() != QSqlError::NoError)
    {
            #ifdef PROGRESS_POSSIBILITY
            emit endWorking();
            #endif

            rollback();
            errstr=("An error occured during the reading of list!\n>>"+((q->lastError()).text()));
            delete q;
            emit errorSignal(errstr);
            sdebug("*** HList::readList *** END");
            return 1;
    }

    serial = 1;
    while(q->next()) //record cycle
    {

        #ifdef PROGRESS_POSSIBILITY
            emit doWorking();
        #endif

        idx=0;
        tmp=table;
        row_key="";
        row_values=new QStringList();
        //row_values->clear();
        for(;tmp != NULL;) //field cycle
        {
            if(tmp->data != NULL && tmp->data->isSQLField())
            {
                if(tmp->data->getWhoami() == "HSmallText" && tmp->data->sqlSelectHead() == "NON_SQL_LIST_SERIAL")
                {
                    row_values->push_back( QString("%1").arg(serial++) );
                    if(all)
                        tmp=tmp->nextall;
                    else
                        tmp=tmp->next;
                    continue;
                }

                if(tmp->data->getWhoami() == "HKey" && row_key.isEmpty())
                {
                    row_key = q->value(idx).toString();
                }
                if(tmp->data->isShow())
                {
                    row_values->push_back( tmp->data->dbValueToDispValue(q->value(idx)).toString() );
                }

                idx++;
            }

            if(all)
                tmp=tmp->nextall;
            else
                tmp=tmp->next;
        }

        values->push_back(row_values);
        keys->push_back(row_key);
        row_values = NULL;
    }
    sdebug(QString("Fetched %1 values %2 keys.")
                        .arg(values->size())
                        .arg(keys->size()));

    delete q;
    commit();

    #ifdef PROGRESS_POSSIBILITY
    emit doWorking();
    #endif

    dataUpdated();

    #ifdef PROGRESS_POSSIBILITY
    emit endWorking();
    #endif

    sdebug("*** HList::readList *** END");
    return 0;
}

QString HList::operator [](int i)
{
    int idx=0;
    HDataField_List *tmp=table;
    while(tmp != NULL)
    {
        if(tmp->data->isSQLField() && tmp->data->isShow())
        {
            if(idx == i)
            {
                if(tmp->data == NULL) return QString("");
                else                  return tmp->data->getTitleText();
            }
            ++idx;
        }
        tmp=tmp->nextall;
    }
    return QString("");

}

QString HList::activeKey(void)
{
    return active_key;
}

void HList::setActiveKey(QString k)
{
    sdebug("*** HList::setActiveKey    to   "+k);
    active_key = k;
}

void HList::queryToFirst(void)
{
    query_valiter = values->begin();
    query_keyiter = keys->begin();
}

bool HList::queryNextRecord()
{
    ++query_valiter;
    ++query_keyiter;
    if(query_valiter == values->end() ||
       query_keyiter == keys->end())
       return false;
    return true;
}

QString HList::queryNextKey()
{
    if(queryNextRecord())
        return queryCurrKey();
    return QString();

}

QStringList* HList::queryNextValues()
{
    if(queryNextRecord())
        return queryCurrValues();
    return NULL;

}

QString HList::queryCurrKey()
{
    if(query_keyiter != keys->end())
        return (*query_keyiter);
    return QString();
}

QStringList* HList::queryCurrValues()
{
    if(query_valiter != values->end())
        return (*query_valiter);
    return NULL;
}


/////////////////////////////////////////////////////////////////////////////////
////// HFloatTablesAsHList //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HFloatTablesAsHList::HFloatTablesAsHList(HFloatTables *memt,HList *templ,bool clone_struct_from_memt)
: HList(memt->sqlTableName())
{
    sdebug("*** HFloatTablesAsHList::HFloatTablesAsHList ***");

    mem = memt;
    cloneSqlSettingsFrom(mem);

    if(templ != NULL)
        deepcopy_from_hlist(templ);
    if(templ == NULL && clone_struct_from_memt)
    {
        deepcopy_from_htablebase(memt->baserecord);
        whoami = "HList";
    }

    sdebug("*** HFloatTablesAsHList::HFloatTablesAsHList *** END");
}

HFloatTablesAsHList::~HFloatTablesAsHList(void)
{
    sdebug("*** HFloatTablesAsHList::~HFloatTablesAsHList ***");

    sdebug("*** HFloatTablesAsHList::~HFloatTablesAsHList *** END");

}

HTable *HFloatTablesAsHList::getActiveTable(void)
{
    int idx;
    sdebug("active_key:"+active_key);
    sdebug("soft_current_key:"+soft_current_key);

    idx = getIndexOf(*keys,soft_current_key);
    return mem->getTByNum(idx);
}

void HFloatTablesAsHList::refreshValuesKeys(bool updSignal)
{
    QList<HTable *>::iterator ti;
    QList<int>::iterator si;

    int serial;

    QStringList *row_values;

    //deleting
    for(QList<QStringList *>::iterator it = values->begin();it != values->end();++it)
        delete *it;
    values->clear();
    keys->clear();

    int idx;
    HDataField_List *tmp=NULL;

    serial = 1;
    ti = mem->tbls->begin();
    si = mem->sts->begin();

    while(ti != mem->tbls->end())
    {
        if(*si == FLOATTABLES_STS_ALIVE || *si == FLOATTABLES_STS_NEW )
        {
            idx=0;
            tmp=table;
            row_values=new QStringList();
            for(;tmp != NULL;) //field cycle
            {
                if(tmp->data != NULL && tmp->data->isSQLField())
                {
                    if(tmp->data->getWhoami() == "HSmallText" && tmp->data->sqlSelectHead() == "NON_SQL_LIST_SERIAL")
                    {
                        row_values->push_back( QString("%1").arg(serial) );
                        tmp=tmp->next;
                        continue;
                    }

                    if(tmp->data->isShow())
                    {
                        row_values->push_back(
                            tmp->data->dbValueToDispValue(
                                    ((*ti)->fieldBySqlName(tmp->data->sqlSelectHead()))->getValue()
                                                          ).toString()
                                             );
                    }

                    idx++;
                }
                tmp=tmp->next;
            }


            keys->push_back(QString("temp_%1").arg(serial));
            values->push_back(row_values);

            row_values = NULL;
            ++serial;
        }
        ++ti;
        ++si;
    }

    /*
    sdebug(QString("NUM k.%1 v.%2").arg(keys->size()).arg(values->size()));
    QStringList::iterator i1,i3;
    QList<QStringList *>::iterator i2;
    i1=keys->begin();
    for(;i1 != keys->end();i1++)
        sdebug("KEY: "+ *i1);

    i2 = values->begin();
    for(;i2 != values->end();i2++)
    {
        i3 = (*i2)->begin();
        for(;i3 != (*i2)->end();i3++)
            sdebug("VAL:"+*i3);
    }
    */

    if(updSignal)
        dataUpdated();

    sdebug(QString("Converted %1 values %2 keys.").arg(values->size()).arg(keys->size()));
}

QString HFloatTablesAsHList::getSqlFieldValue(QString sqln)
{
    Q_UNUSED(sqln);
    // !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
    // This function disabled here
    // There can be keys in the memory which are non existing in the sql/HFloatTable. So we can't search of its!
    // !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
    return "";
}


int HFloatTablesAsHList::clearList(void)
{
    // !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
    // This function disabled here
    // You should erase the data in HFloatTable!
    // !-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!-!
    refreshValuesKeys();
    return 0;
}

int HFloatTablesAsHList::readList(QString filter,bool all)
{
    last_filter = filter;
    last_all = all;

#ifdef PROGRESS_POSSIBILITY
    emit startWorking();
#endif

    mem->readDBTable(filter);

    refreshValuesKeys();

#ifdef PROGRESS_POSSIBILITY
    emit endWorking();
#endif

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
////// HDataFieldRole ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HDataFieldRole::HDataFieldRole(void)
{
    name = "";
    mod  = "";
    val  = "";
}

HDataFieldRole::HDataFieldRole(QString name, QString mod, QString val)
{
    this->name = name;
    this->mod  = mod;
    this->val  = val;
}

HDataFieldRole::~HDataFieldRole(void)
{
}

void HDataFieldRole::addShowMod(void)
{
    if(mod.contains("hide;",Qt::CaseInsensitive))
        mod.replace("hide;","",Qt::CaseInsensitive);
    if(!mod.contains("show;",Qt::CaseInsensitive))
        mod.prepend("show;");
}

void HDataFieldRole::addHideMod(void)
{
    if(mod.contains("show;",Qt::CaseInsensitive))
        mod.replace("show;","",Qt::CaseInsensitive);
    if(!mod.contains("hide;",Qt::CaseInsensitive))
        mod.prepend("hide;");
}

void HDataFieldRole::addEditableMod(void)
{
    if(mod.contains("readonly;",Qt::CaseInsensitive))
        mod.replace("readonly;","",Qt::CaseInsensitive);
    if(!mod.contains("editable;",Qt::CaseInsensitive))
        mod.prepend("editable;");
}
void HDataFieldRole::addReadonlyMod(void)
{
    if(mod.contains("editable;",Qt::CaseInsensitive))
        mod.replace("editable;","",Qt::CaseInsensitive);
    if(!mod.contains("readonly;",Qt::CaseInsensitive))
        mod.prepend("readonly;");
}

void HDataFieldRole::addGuienabledMod(void)
{
    if(mod.contains("guidisabled;",Qt::CaseInsensitive))
        mod.replace("guidisabled;","",Qt::CaseInsensitive);
    if(!mod.contains("guienabled;",Qt::CaseInsensitive))
        mod.prepend("guienabled;");
}

void HDataFieldRole::addGuidisabledMod(void)
{
    if(mod.contains("guienabled;",Qt::CaseInsensitive))
        mod.replace("guienabled;","",Qt::CaseInsensitive);
    if(!mod.contains("guidisabled;",Qt::CaseInsensitive))
        mod.prepend("guidisabled;");
}

void HDataFieldRole::addResetMod(void)
{
    if(mod.contains("setval;",Qt::CaseInsensitive))
        mod.replace("setval;","",Qt::CaseInsensitive);
    if(!mod.contains("reset;",Qt::CaseInsensitive))
        mod.prepend("reset;");
}

void HDataFieldRole::addSetvalMod(void)
{
    if(mod.contains("reset;",Qt::CaseInsensitive))
        mod.replace("reset;","",Qt::CaseInsensitive);
    if(!mod.contains("setval;",Qt::CaseInsensitive))
        mod.prepend("setval;");
}

bool HDataFieldRole::hasShowMod(void)
{
    if(mod.contains("show;",Qt::CaseInsensitive))
        return true;
    return false;
}

bool HDataFieldRole::hasHideMod(void)
{
    if(mod.contains("hide;",Qt::CaseInsensitive))
        return true;
    return false;
}

bool HDataFieldRole::hasEditableMod(void)
{
    if(mod.contains("editable;",Qt::CaseInsensitive))
        return true;
    return false;
}

bool HDataFieldRole::hasReadonlyMod(void)
{
    if(mod.contains("readonly;",Qt::CaseInsensitive))
        return true;
    return false;
}

bool HDataFieldRole::hasGuienabledMod(void)
{
    if(mod.contains("guienabled;",Qt::CaseInsensitive))
        return true;
    return false;
}

bool HDataFieldRole::hasGuidisabledMod(void)
{
    if(mod.contains("guidisabled;",Qt::CaseInsensitive))
        return true;
    return false;
}

bool HDataFieldRole::hasResetMod(void)
{
    if(mod.contains("reset;",Qt::CaseInsensitive))
        return true;
    return false;
}

bool HDataFieldRole::hasSetvalMod(void)
{
    if(mod.contains("setval;",Qt::CaseInsensitive))
        return true;
    return false;
}

/////////////////////////////////////////////////////////////////////////////////
////// HDataField ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HDataField::HDataField(QString _sqlColumn,QString _explain,QString tit,
                       QString _tailstr,bool _editable)
{
    whoami="HDataField";
    set = 0;
    sqlColumn = _sqlColumn;
    explain   = _explain;
    tailstr   = _tailstr;
    title     = tit;
    editable  = _editable;
    color_r   = 180;
    color_g   = 180;
    color_b   = 180;
    sql       = true;
    show      = true;
    guienabled= true;
    connected = false;
    meldwithnext = false;
    nomiddlestretch = false;
    editboxmaxwidth = 0;
    conn_sqlTable = "";
    defval    = "";
    valid     = "";
    notvalid.clear();

    lastvalidate_result = false;
    permanentvalidate_result = false;
    manual_validate_error = "";

    spec_disp_sort = "";

    wcellw_print = 0;
    wcell_print  = false;

    function = "";
    derived   = "";

    markers.clear();
    roles.clear();

    logged          = false;
    sync_valueInSql = false;
    valueInSql  = "NOT_VALID";
}

HDataField::HDataField(HDataField *t)
{
    deepcopy_from_hdatafield(t);
}

void HDataField::deepcopy_from_hdatafield(HDataField *x)
{
    cloneSqlSettingsFrom(x);

    whoami    = x->whoami;
    set       = x->set;
    sqlColumn = x->sqlColumn;
    explain   = x->explain;
    tailstr   = x->tailstr;
    title     = x->title;
    editable  = x->editable;
    color_r   = x->color_r;
    color_g   = x->color_g;
    color_b   = x->color_b;
    sql       = x->sql;
    show      = x->show;
    guienabled= x->guienabled;

    defval    = x->defval;
    valid     = x->valid;
    notvalid  = x->notvalid;

    wcellw_print = x->wcellw_print;
    wcell_print  = x->wcell_print;

    meldwithnext = x->meldwithnext;
    nomiddlestretch = x->nomiddlestretch;
    editboxmaxwidth = x->editboxmaxwidth;

    function = x->function;

    derived = x->derived;

    spec_disp_sort = x->spec_disp_sort;

    markers   = x->markers;
    lastvalidate_result = x->lastvalidate_result;
    permanentvalidate_result = x->permanentvalidate_result;
    manual_validate_error = x->manual_validate_error;

    //The connection data is not copied
    connected = false;
    conn_sqlTable = "";

    roles.clear();
    roles = QList<HDataFieldRole>(roles);

    logged          = x->logged;
    sync_valueInSql = x->sync_valueInSql;
    valueInSql      = x->valueInSql;
}

HDataField* HDataField::get_clone(void)
{
    return new HDataField(this);
}

HDataField::~HDataField()
{
    //empty
}

void HDataField::setValidator(QString v)
{
    valid = v;
}

void HDataField::setNotValidValues(QStringList nvl)
{
    notvalid.clear();
    notvalid = nvl;
}

void HDataField::addNotValidValue(QString v)
{
    notvalid.push_back(v);
}

void HDataField::clearNotValidValues(void)
{
    notvalid.clear();
}

void HDataField::setMaulallyValidateError(QString message)
{
    lastvalidate_result = true;
    permanentvalidate_result = lastvalidate_result;
    manual_validate_error = message;
}


void HDataField::setFunction(QString func)
{
       if(acceptFunction())
       {
            sdebug("HDataField::setFunction - Function accepted");
            function = func;
       }
       else
       {
           sdebug(QString("Warning: HDataField::setFunction - Function definitions rejected! (Cannot set function on \"%1\")")
                    .arg(getWhoami()));
       }
}

void HDataField::setExplainText(QString expl)
{
    explain = expl;
    emit updateStaticDisplayElementsSignal();
}

void HDataField::setTitleText(QString titl)
{
    title = titl;
    emit updateStaticDisplayElementsSignal();
}

void HDataField::setTailText(QString tail)
{
    tailstr = tail;
    emit updateStaticDisplayElementsSignal();
}

void HDataField::setSubselect(QString subs)
{
       if(acceptSubselect())
       {
            sdebug("HDataField::setSubselect - Subselect accepted");
            derived = subs;
       }
       else
       {
           sdebug(QString("Warning: HDataField::setSubselect - Subselect definitions rejected! (Cannot set subselect on \"%1\")")
                    .arg(getWhoami()));
       }
}

QString HDataField::validate()
{
    QString d;
    QRegExp re(valid);

    d = "";
    if(!valid.isEmpty())
    {
        //Regex check
        if( !re.isValid() )
            d = "Regular Expression is invalid!";

        if( ! re.exactMatch(getValue().toString()) )
            d = ("ERROR: The \""+explain+"\" field contains not valid data: \""+getValue().toString()+"\" ! ");
    }

    //Notvalid values check
    if(d.isEmpty())
    {
        QStringList::iterator iter=notvalid.begin();
        while(iter != notvalid.end())
        {
            if(getValue().toString() == *iter )
                d = "ERROR: The \""+explain+"\" field contains a NOTVALID data: \""+getValue().toString()+"\" ! ";
            ++iter;
        }
    }

    if(d.isEmpty() && !manual_validate_error.isEmpty())
    {
        d = manual_validate_error;
    }
    manual_validate_error = "";

    if(d.isEmpty())
    {
        d = lowerValidate();
    }

    if(d.isEmpty())
        lastvalidate_result = false;
    else
        lastvalidate_result = true;

    permanentvalidate_result = lastvalidate_result;
    return d;
}


QVariant HDataField::dbValueToDispValue(QVariant v)
{
    return v; //default action (will be redefined in chield classes)
}

int HDataField::returnToDefault(void)
{
    QString dv;
    notSet();
    dv = getDefaultValue();
    setDefval(dv);
    resetValidateStatus();

    emit dataChanged();
    return 0;
}

QVariant HDataField::defvalParser(QString s)
{
    QString sqlq;
    QVariant v;

    defval = s; //save the default value
    QRegExp rx;
    rx.setPattern("^DEFVAL_SQL\\((.*)\\)$");
    if(rx.indexIn(s) == -1 || rx.cap(1).isEmpty()) //no match
    {
        v=s;
        return v;
    }

    sqlq = rx.cap(1);
    transaction();
    QSqlQuery *q = allocateQSqlQuery();
    sqldebug(sqlq);
    q->prepare(sqlq);
    q->exec();

    if((myInterface()->hsqli_usequerysize && q->numRowsAffected() != 1 ) ||
        ! q->next()                                       ||
        q->lastError().type() != QSqlError::NoError)
    {
        QString errstr;
        rollback();
        errstr=("The SELECT returned with !=1 record! (1)\n>>"+((q->lastError()).text()));
        delete q;
        v="";
        emit errorSignal(errstr);
        return v;
    }


    v=q->value(0);
    commit();
    delete q;
    value_changed();
    return v;
}

 void HDataField::value_changed()
 {
     resetValidateStatus();
 }

void HDataField::setData(QString _sqlColumn,QString _explain,QString tit,
                         QString _tailstr,bool _editable)
{
    sqlColumn = _sqlColumn;
    title     = tit;
    explain   = _explain;
    tailstr   = _tailstr;
    editable  = _editable;
}

void HDataField::setColor(int r,int g,int b)
{
    color_r   = r;
    color_g   = g;
    color_b   = b;
}

int HDataField::setValue(QVariant v,bool sdisabled)
{
    Q_UNUSED(v);
    //I don't have any type yet...
    if(!sdisabled)
        emit dataChanged();
    set = 1;
    value_changed();
    return -1;
}

int HDataField::setDefval(QVariant v)
{
    Q_UNUSED(v);
    //I don't have any type yet...
    return -1;
}

QString HDataField::getSQLValue (void)
{
    return QString("");
}

QString HDataField::getSQLValueAsString(void)
{
    return QString("");
}

QVariant HDataField::getValue (void)
{
    return QVariant(0);
}

QString HDataField::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HDataField::sqlInsertValue(void)
{
    return QString("NULL");
}

QString HDataField::sqlUpdate(void)
{
    return QString("%1=NULL").arg(sqlColumn);
}

QString HDataField::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

void HDataField::setEditable(void)
{
    editable = true;
}

void HDataField::setReadonly(void)
{
    editable = false;
}

QString HDataField::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    return sqlColumn + "\t TYPE";
}

void HDataField::addMark(QString mark)
{
    if(!mark.isEmpty())
        markers.push_back(mark);
}

void HDataField::addMark(QStringList pmarkers)
{
    if(!pmarkers.isEmpty())
        markers += pmarkers;
}

bool HDataField::hasMark(QString mark)
{
    if(markers.contains(mark) > 0)
        return true;
    return false;
}

void HDataField::clearMarkers(void)
{
    markers.clear();
}

QStringList HDataField::allMarker(void)
{
    return markers;
}

void HDataField::addRole(HDataFieldRole role)
{
    roles.push_back(role);
}

void HDataField::delRole(QString name)
{
    QList<HDataFieldRole>::iterator i = roles.begin();
    while(i != roles.end())
    {
        if(i->name == name)
        {
            roles.erase(i);
            delRole(name); //del other occurences...
            return;
        }
        ++i;
    }
}

void HDataField::clearRoles(void)
{
    roles.clear();
}

void HDataField::useRole(QString name)
{
    sdebug("*** HDataField::useRole ***");
    HDataFieldRole role;
    if(name.isEmpty())
        return;

    QList<HDataFieldRole>::iterator i = roles.begin();
    while(i != roles.end())
    {
        if(i->name == name)
        {
            role = *i;

            if(role.hasShowMod())
                setShow();
            if(role.hasHideMod())
                setHide();

            if(role.hasEditableMod())
                setEditable();
            if(role.hasReadonlyMod())
                setReadonly();

            if(role.hasGuienabledMod())
            {
                setGuienabled();
                enableControlSlot();
            }
            if(role.hasGuidisabledMod())
            {
                setGuidisabled();
                disableControlSlot();
            }

            if(role.hasResetMod())
                returnToDefault();
            if(role.hasSetvalMod())
            {
                notSet();
                setDefval(role.val);
                resetValidateStatus();
                emit dataChanged();
            }

            return;
        }
        ++i;
    }
}

void HDataField::sqlSynced(HDataChangeLogger *dclog,bool forcechange,bool forcelog)
{
    if( dclog != NULL && (sync_valueInSql || forcechange) && (logged || forcelog) )
        if(valueInSql != getSQLValueAsString())
        {
            /*Log*/
            sdebug(QString("DEBUG DataChangeLog: %1 field: \"%2\" to \"%3\" ---")
                   .arg(sqlInsertHead())
                   .arg(valueInSql)
                   .arg(getSQLValueAsString()));

            dclog->addChange(sqlInsertHead(),valueInSql,getSQLValueAsString());
        }

    sync_valueInSql = true;
    valueInSql = getSQLValueAsString();
}

void HDataField::clearSqlSynced(void)
{
    sync_valueInSql = false;
    valueInSql = "CLEARED";
}

bool HDataField::isReallyChanged(void)
{

    if(sync_valueInSql && valueInSql != getSQLValueAsString())
        return true;
    return false;
}

/////////////////////////////////////////////////////////////////////////////////
////// HKey /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HKey::HKey(QString sqlc,QString ex,QString tit,QString def)
    : HDataField(sqlc,ex,tit,"",false)
{
    whoami="HKey";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = false;
    sql       = true;
}

HKey::~HKey(void)
{
    //empty
}

HKey::HKey(HKey *t)
: HDataField(t)
{
    deepcopy_from(t);
}

void HKey::deepcopy_from(HKey *x)
{
    value = x->value;
}

HDataField* HKey::get_clone(void)
{
    return new HKey(this);
}

int HKey::setValue(QVariant v,bool sdisabled)
{
    set = 1;
    value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HKey::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toString();

    return 0;
}

QVariant HKey::getValue (void)
{
    return QVariant(value);
}

QString HKey::getSQLValue (void)
{
    if(!myInterface()->hsqli_hkeytype_varchar_int)
        return QString("%1").arg(value);

    return QString("\'%1\'").arg(value);
}

QString HKey::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HKey::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HKey::sqlInsertValue(void)
{
    bool ok;
    QString r;
    if(myInterface()->hsqli_hkeytype_varchar_int)
        return QString("\'%1\'").arg(value);
    r = QString("%1").arg(value.toInt(&ok));
    if(!ok)
        return QString::null;
    return r;

}

QString HKey::sqlUpdate(void)
{
    return QString::null; //We don't modify the key. (autogenerated)
}

QString HKey::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HKey::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    QString type;

    if(myInterface()->hsqli_hkeytype_varchar_int)
        type = QString("VARCHAR(%1)").arg(myInterface()->hsqli_varcharhkeylength);
    else
        type = myInterface()->hsqli_numbertypename;

    return (QString("%1\t %2 %3")
                .arg(sqlColumn)
                .arg(type)
                .arg(myInterface()->hsqli_hkeyconstraint));
}

/////////////////////////////////////////////////////////////////////////////////
////// HConnect /////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HConnect::HConnect(QString sqlc,QString connected_tablep)
    : HDataField(sqlc,"","","",true)
{
    whoami="HConnect";
    connected_table = connected_tablep;
    set = 0;
    setDefval("");
    editable  = false; //Not editable
    show      = false; //Hided
    sql       = true;  //It's an sql field
}

HConnect::~HConnect(void)
{
    //empty
}

HConnect::HConnect(HConnect *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HConnect::deepcopy_from(HConnect *x)
{
    connected_table = x->connected_table;
    value = x->value;
}

HDataField* HConnect::get_clone(void)
{
    return new HConnect(this);
}

int HConnect::setValue (QVariant v,bool sdisabled)
{
    set = 1;
    value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HConnect::setDefval(QVariant v)
{
    Q_UNUSED(v);

    if(!set) //if the value was set I dont set the default.
        value = "";
    return 0;
}

QVariant HConnect::getValue (void)
{
    return QVariant(value);
}

QString HConnect::getSQLValue (void)
{
    return QString("\'%1\'").arg(value);
}

QString HConnect::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HConnect::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HConnect::sqlInsertValue(void)
{
    if(value.isEmpty())
        return QString("NULL");
    return QString("\'%1\'").arg(value);
}

QString HConnect::sqlUpdate(void)
{
    return QString::null;
}

 QString HConnect::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HConnect::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);
    QString type;

    if(myInterface()->hsqli_hkeytype_varchar_int)
        type = QString("VARCHAR(%1)").arg(myInterface()->hsqli_varcharhkeylength);
    else
        type = myInterface()->hsqli_numbertypename;

    return (QString("%1\t %2")
                .arg(sqlColumn)
                .arg(type));
}
/////////////////////////////////////////////////////////////////////////////////
////// HSmallText ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HSmallText::HSmallText(QString sqlc,QString ex,QString tit,QString t,QString def)
    : HDataField(sqlc,ex,tit,t,true)
{
    whoami="HSmallText";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = true;
    sql       = true;
    initial_selected = false;
}

HSmallText::~HSmallText(void)
{
    //empty
}

HSmallText::HSmallText(HSmallText *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HSmallText::deepcopy_from(HSmallText *x)
{
    value = x->value;
    initial_selected = x->initial_selected;
}

HDataField* HSmallText::get_clone(void)
{
    return new HSmallText(this);
}

int HSmallText::setValue (QVariant v,bool sdisabled)
{
    set = 1;
    value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

QVariant HSmallText::dbValueToDispValue(QVariant v)
{
    return QVariant( v.toString() + (tailstr.isEmpty() ? "" : (" " + tailstr))  );
}

int HSmallText::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toString();
    return 0;
}

QVariant HSmallText::getValue (void)
{
    return QVariant(value);
}

QString HSmallText::getSQLValue (void)
{
    return QString("\'%1\'").arg(value);
}

QString HSmallText::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HSmallText::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

 QString HSmallText::sqlInsertValue(void)
{
    return QString("\'%1\'").arg(value);
}

 QString HSmallText::sqlUpdate(void)
{
    return QString("%1=\'%2\'")
                         .arg(sqlColumn)
                         .arg(value);
}

 QString HSmallText::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HSmallText::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);
    return sqlColumn + QString("\t VARCHAR(%1)").arg(myInterface()->hsqli_smalltext_varcharlength);
}

/////////////////////////////////////////////////////////////////////////////////
////// HLargeText ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HLargeText::HLargeText(QString sqlc,QString ex,QString tit,QString def)
    : HDataField(sqlc,ex,tit,"",true)
{
    whoami="HLargeText";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = true;
    sql       = true;
}

HLargeText::~HLargeText(void)
{
    //empty
}

HLargeText::HLargeText(HLargeText *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HLargeText::deepcopy_from(HLargeText *x)
{
    value = x->value;
}

HDataField* HLargeText::get_clone(void)
{
    return new HLargeText(this);
}

int HLargeText::setValue (QVariant v,bool sdisabled)
{
    set = 1;
    value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HLargeText::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toString();
    return 0;
}

QVariant HLargeText::getValue (void)
{
    return QVariant(value);
}

QString HLargeText::getSQLValue (void)
{
    return QString("\'%1\'").arg(value);
}

QString HLargeText::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HLargeText::sqlInsertHead(void)
{
    return QString("%1")
                            .arg(sqlColumn);
}

 QString HLargeText::sqlInsertValue(void)
{
    return QString("\'%1\'")
                            .arg(value);
}

 QString HLargeText::sqlUpdate(void)
{
    return QString("%1=\'%2\'")
                            .arg(sqlColumn)
                            .arg(value);
}

 QString HLargeText::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HLargeText::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    if(myInterface()->hsqli_largetext_sizemustdefine)
        return sqlColumn + QString("\t VARCHAR(%1)").arg(myInterface()->hsqli_largetext_varcharlength);

    return sqlColumn + "\t VARCHAR";
}
/////////////////////////////////////////////////////////////////////////////////
////// HCharHash ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HCharHash::HCharHash(QString sqlc,QString ex,QString tit,QString t,QString def,
                  QStringList k,QStringList v)
    : HDataField(sqlc,ex,tit,t,true)
{
    whoami="HCharHash";
    defval = def;
    set = 0;
    setDefval(def);
    keys      = k;
    values    = v;
    editable  = true;
    sql       = true;
}

HCharHash::~HCharHash(void)
{
    //empty
}


HCharHash::HCharHash(HCharHash *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HCharHash::deepcopy_from(HCharHash *x)
{
    value  = x->value;
    keys   = x->keys;
    values = x->values;
}

HDataField* HCharHash::get_clone(void)
{
    return new HCharHash(this);
}

QVariant HCharHash::dbValueToDispValue(QVariant v)
{
    return QVariant(values[getIndexOf(keys,v.toString())]);
}

QString HCharHash::getKeyOfValue(QString s)
{
    return keys[getIndexOf(values,s)];
}

QString HCharHash::getValueOfKey(QString s)
{
    return values[getIndexOf(keys,s)];
}

int HCharHash::setValue(QVariant v,bool sdisabled)
{
    set = 1;
    value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HCharHash::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toString();
    return 0;
}

QVariant HCharHash::getValue (void)
{
    return QVariant(value);
}

QString HCharHash::getSQLValue (void)
{
    return QString("\'%1\'").arg(value);
}

QString HCharHash::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HCharHash::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HCharHash::sqlInsertValue(void)
{
    return QString("\'%1\'").arg(value);
}

QString HCharHash::sqlUpdate(void)
{
    return QString("%1=\'%2\'")
                             .arg(sqlColumn)
                             .arg(value);
}

QString HCharHash::sqlSelectHead(void)
{
    return QString("%1")
                     .arg(sqlColumn);
}

QString HCharHash::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    return sqlColumn + QString("\t VARCHAR(%1)").arg(myInterface()->hsqli_charhash_varcharlength);
}

/////////////////////////////////////////////////////////////////////////////////
////// HNumHash ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HNumHash::HNumHash(QString sqlc,QString ex,QString tit,QString t,QString def,
                  QStringList k,QStringList v)
    : HDataField(sqlc,ex,tit,t,true)
{
    whoami="HNumHash";
    defval = def;
    set = 0;
    setDefval(def);
    keys      = k;
    values    = v;
    editable  = true;
    sql       = true;
}

HNumHash::~HNumHash(void)
{
    //empty
}

HNumHash::HNumHash(HNumHash *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HNumHash::deepcopy_from(HNumHash *x)
{
    value  = x->value;
    keys   = x->keys;
    values = x->values;
}

HDataField* HNumHash::get_clone(void)
{
    return new HNumHash(this);
}

QVariant HNumHash::dbValueToDispValue(QVariant v)
{
    return QVariant(values[getIndexOf(keys,v.toString())]);
}

QString HNumHash::getKeyOfValue(QString s)
{
    return keys[getIndexOf(values,s)];
}

QString HNumHash::getValueOfKey(QString s)
{
    return values[getIndexOf(keys,s)];
}

int HNumHash::setValue (QVariant v,bool sdisabled)
{
    set = 1;
    value = (int)(v.toString().toDouble());
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HNumHash::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toInt();
    return 0;
}

QVariant HNumHash::getValue (void)
{
    return QVariant(value);
}

QString HNumHash::getSQLValue (void)
{
    return QString("%1").arg(value);
}

QString HNumHash::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HNumHash::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HNumHash::sqlInsertValue(void)
{
    return QString("%1").arg(value);
}

QString HNumHash::sqlUpdate(void)
{
    return QString("%1=%2")
                             .arg(sqlColumn)
                             .arg(value);
}

QString HNumHash::sqlSelectHead(void)
{
    return QString("%1")
                     .arg(sqlColumn);
}

QString HNumHash::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    if(myInterface()->hsqli_numbertypename == "NUMERIC")
        return sqlColumn + "\t NUMERIC(5,0)";

    return sqlColumn + QString("\t %1").arg(myInterface()->hsqli_numbertypename);
}
/////////////////////////////////////////////////////////////////////////////////
////// HNumber //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HNumber::HNumber(QString sqlc,QString ex,QString tit,QString t,QString def)
    : HDataField(sqlc,ex,tit,t,true)
{
    whoami="HNumber";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = true;
    sql       = true;
    minimum = -10000000;
    maximum = 10000000;
}

HNumber::~HNumber(void)
{
    //empty
}

HNumber::HNumber(HNumber *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HNumber::deepcopy_from(HNumber *x)
{
    value = x->value;
    minimum = x->minimum;
    maximum = x->maximum;
}

HDataField* HNumber::get_clone(void)
{
    return new HNumber(this);
}

QVariant HNumber::dbValueToDispValue(QVariant v)
{
    return QVariant(QString("%1%2").arg(v.toInt()).arg(tailstr.isEmpty() ? "" : (" "+tailstr)));
}

QString HNumber::lowerValidate(void)
{
    if(value < minimum)
        return QString("%1:Value lower than minimum!").arg(sqlColumn);
    if(value > maximum)
        return QString("%1:Value higher than maximum!").arg(sqlColumn);
    return QString("");
}

int HNumber::setValue (QVariant v,bool sdisabled)
{
    set = 1;

    value = v.toInt();

    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HNumber::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toInt();
    return 0;
}

QVariant HNumber::getValue (void)
{
    return QVariant(value);
}

QString HNumber::getSQLValue (void)
{
    return QString("%1").arg(value);
}

QString HNumber::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HNumber::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HNumber::sqlInsertValue(void)
{
    return QString("%1").arg(value);
}

QString HNumber::sqlUpdate(void)
{
    return QString("%1=%2").arg(sqlColumn).arg(value);
}

QString HNumber::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HNumber::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    if(myInterface()->hsqli_numbertypename == "NUMERIC")
    {
        int i =(int)( ::log10((double)maximum));
        return sqlColumn + QString("\t NUMERIC(%1,0)").arg(i+1);
    }

    return sqlColumn + QString("\t %1").arg(myInterface()->hsqli_numbertypename);
}

/////////////////////////////////////////////////////////////////////////////////
////// HStatic //////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HStatic::HStatic(QString ex,QString tit,QString def)
    : HDataField("",ex,tit,"",false)
{
    whoami="HStatic";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = false;
    sql       = false;
}

HStatic::~HStatic(void)
{
    //empty
}

HStatic::HStatic(HStatic *t)
: HDataField(t)
{
    deepcopy_from(t);
}

void HStatic::deepcopy_from(HStatic *x)
{
    value = x->value;
}

HDataField* HStatic::get_clone(void)
{
    return new HStatic(this);
}

int HStatic::setValue (QVariant v,bool sdisabled)
{
    value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    return 0;
}

QVariant HStatic::getValue (void)
{
    return QVariant(value);
}

QString HStatic::getSQLValue (void)
{
    return QString("");
}

QString HStatic::getSQLValueAsString(void)
{
    return QString("NULL");
}

int HStatic::setDefval(QVariant v)
{
    value = defvalParser( v.toString() ).toString();
    return 0;
}

QString HStatic::sqlInsertHead(void)
{
    return QString("");
}

QString HStatic::sqlInsertValue(void)
{
    return QString("");
}

QString HStatic::sqlUpdate(void)
{
    return QString("");
}

QString HStatic::sqlSelectHead(void)
{
    return QString("");
}

QString HStatic::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    return "";
}
/////////////////////////////////////////////////////////////////////////////////
////// HCheck ///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HCheck::HCheck(QString sqlc,QString ex,QString tit,QString t,QString def,QString ts,QString fs)
 : HDataField(sqlc,ex,tit,t,true)
{
    whoami="HCheck";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = true;
    sql       = true;
    trues = ts;
    falses = fs;
}

HCheck::~HCheck(void)
{
    //empty
}

HCheck::HCheck(HCheck *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HCheck::deepcopy_from(HCheck *x)
{
    value = x->value;
    trues = x->trues;
    falses= x->falses;
}

HDataField * HCheck::get_clone(void)
{
    return new HCheck(this);
}

QVariant HCheck::dbValueToDispValue(QVariant v)
{
    return QVariant( v.toBool() ? trues : falses );
}

int HCheck::setValue (QVariant v,bool sdisabled)
{
    set = 1;
    value = v.toBool();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;

}

int HCheck::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toBool();
    return 0;
}

QVariant HCheck::getValue (void)
{
    return QVariant(value);
}

QString HCheck::getSQLValue (void)
{
    return QString(value ? myInterface()->hsqli_truevalue : myInterface()->hsqli_falsevalue);
}

QString HCheck::getSQLValueAsString(void)
{
    return QString(value ? "TRUE" : "FALSE");
}

QString HCheck::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HCheck::sqlInsertValue(void)
{
    return QString(value ? myInterface()->hsqli_truevalue : myInterface()->hsqli_falsevalue);
}

QString HCheck::sqlUpdate(void)
{
    return QString("%1=%2").arg(sqlColumn).arg(value ? myInterface()->hsqli_truevalue : myInterface()->hsqli_falsevalue);
}

QString HCheck::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HCheck::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    return sqlColumn + "\t " + myInterface()->hsqli_booleantypename;
}
/////////////////////////////////////////////////////////////////////////////////
////// HFloating ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HFloating::HFloating(QString sqlc,QString ex,QString tit,QString t,QString def)
    : HDataField(sqlc,ex,tit,t,true)
{
    whoami="HFloating";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = true;
    sql       = true;
    minimum = -10000000;
    maximum = 10000000;
    dMminDf = 0;
    dMmaxDf = 4;
    dMgroup = 0;
}

HFloating::~HFloating(void)
{
    //empty
}

HFloating::HFloating(HFloating *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HFloating::deepcopy_from(HFloating *x)
{
    value = x->value;
    minimum = x->minimum;
    maximum = x->maximum;
    dMminDf = x->dMminDf;
    dMmaxDf = x->dMmaxDf;
    dMgroup = x->dMgroup;
}

HDataField* HFloating::get_clone(void)
{
    return new HFloating(this);
}

QVariant HFloating::dbValueToDispValue(QVariant v)
{
    return QVariant( doubleToQString(v.toDouble(),dMminDf,dMmaxDf,dMgroup)
                       +
                     (tailstr.isEmpty() ? "" : (" "+tailstr))
                   );

}

QString HFloating::getValueAsString(void)
{
    return doubleToQString(value,dMminDf,dMmaxDf,dMgroup);
}

QString HFloating::lowerValidate(void)
{
    if(value < minimum)
        return QString("%1: Value lower than minimum!").arg(sqlColumn);
    if(value > maximum)
        return QString("%1: Value higher than maximum!").arg(sqlColumn);
    return QString("");
}

int HFloating::setValue (QVariant v,bool sdisabled)
{
    set = 1;

    value = v.toDouble();

    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HFloating::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toDouble();
    return 0;
}

QVariant HFloating::getValue (void)
{
    return QVariant(value);
}

QString HFloating::getSQLValue (void)
{
    return QString("%1").arg(value,0,'f');
}

QString HFloating::getSQLValueAsString(void)
{
    return QString("%1").arg(value,0,'f');
}

QString HFloating::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HFloating::sqlInsertValue(void)
{
    return QString("%1").arg(value,0,'f');
}

QString HFloating::sqlUpdate(void)
{
    return QString("%1=%2").arg(sqlColumn).arg(value,0,'f');
}

QString HFloating::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HFloating::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    if(myInterface()->hsqli_floattypename == "NUMERIC")
    {
        int i =(int)( ::log10(maximum));
        return sqlColumn + QString("\t NUMERIC(%1,3)").arg(i+4);
    }

    return sqlColumn + QString("\t %1").arg(myInterface()->hsqli_floattypename);
}

/////////////////////////////////////////////////////////////////////////////////
////// HDate ////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

HDate::HDate(QString sqlc,QString ex,QString tit,QString def,bool u_a,bool unk)
    : HDataField(sqlc,ex,tit,"",true)
{
    whoami="HDate";
    defval = def;
    set = 0;

    u_alive   = u_a;
    unknown   = unk;

    if(def.toUpper() == "NOW" )
        value   = QDate::currentDate();
    else if(u_a && def.toUpper() == "UNKNOWN")
    {
        unknown = true;
        value = QDate(2000,1,1);
    }
    else if(u_a && def.toUpper() == "UNKNOWN_AND_NOW")
    {
        unknown = true;
        value   = QDate::currentDate();
    }
    else
    {
        QString parsedDv;
        parsedDv = defvalParser(def).toString();
        if(parsedDv.isEmpty())
            value = QDate(2000,1,1);
        else
            value = QDate::fromString(parsedDv,Qt::ISODate);
    }

    editable  = true;
    sql       = true;
}

HDate::~HDate(void)
{
    //empty
}

HDate::HDate(HDate *t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HDate::deepcopy_from(HDate *x)
{
    value = x->value;
    u_alive = x->u_alive;
    unknown = x->unknown;
}

HDataField* HDate::get_clone(void)
{
    return new HDate(this);
}

void HDate::value_is_unknown(void)
{
    if(!unknown)
        set = 1;

    unknown=true;
}
void HDate::value_is_known(void)
{
    if(unknown)
        set = 1;

    if(value.toString().isEmpty() ||
       value.toString() == "NULL" ||
       value.isNull() ||
       !value.isValid() )
    {
        value = QDate(2000,1,1);
    }

    unknown=false;
}

int HDate::setValue (QVariant v,bool sdisabled)
{
    set = 1;
    if(v.toString().isEmpty() ||
       v.toString() == "NULL" )
    {
        if(u_alive)
            unknown = true;
        else
            value = QDate(2000,1,1);
        return 0;
    }
    unknown = false;

    if(myInterface()->hsqli_hasdatetype)
        value = v.toDate();
    else
    {
        value = QDate::fromString(v.toString(),"yyyy-MM-dd"); //"yyyy-M-d"
        if(!value.isValid())
            value = QDate::fromString(v.toString(),"yyyy-M-d");
    }

    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HDate::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
    {

        if(v.toString().toUpper() == "NOW" )
        {
            unknown = false;
            value   = QDate::currentDate();
        }
        else if(u_alive && v.toString().toUpper() == "UNKNOWN" )
        {
            unknown = true;
            value = QDate(2000,1,1);
        }
        else if(u_alive && v.toString().toUpper() == "UNKNOWN_AND_NOW")
        {
            unknown = true;
            value   = QDate::currentDate();
        }
        else
        {
            value   = QDate::fromString(
                                        defvalParser(v.toString()).toString(),Qt::ISODate
                                       );
        }
    }
    return 0;
}

QVariant HDate::getValue (void)
{
    if(u_alive && unknown)
    {
        return QVariant("");
    }
    if(value.toString().isEmpty() ||
       value.toString() == "NULL" ||
       value.isNull() ||
       !value.isValid() )
    {
        return QVariant(QDate(2000,1,1));
    }
    return QVariant(value);
}

QVariant HDate::getKnownValue(void)
{
    if(value.toString().isEmpty() ||
       value.toString() == "NULL" ||
       value.isNull() ||
       !value.isValid() )
    {
        return QVariant(QDate(2000,1,1));
    }
    return QVariant(value);
}

QString HDate::getSQLValue (void)
{
    if(u_alive && unknown)
        return QString("NULL");

    if(!myInterface()->hsqli_hasdatetype)
        return QString("\'%1\'")
                        .arg(value.toString("yyyy-MM-dd"));

    return QString("\'%1-%2-%3\'")
                        .arg(value.year())
                        .arg(value.month())
                        .arg(value.day());
}

QString HDate::getSQLValueAsString(void)
{
    return QString("%1")
                    .arg(value.toString("yyyy-MM-dd"));
}

QString HDate::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HDate::sqlInsertValue(void)
{
    return getSQLValue();
}

QString HDate::sqlUpdate(void)
{
    return QString("%1=%2")
                    .arg(sqlColumn)
                    .arg(getSQLValue());
}

QString HDate::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HDate::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    if(!myInterface()->hsqli_hasdatetype)
            return sqlColumn + QString("\t %1").arg("VARCHAR(32)");

    return sqlColumn + QString("\t %1").arg(myInterface()->hsqli_datetypename);
}
/////////////////////////////////////////////////////////////////////////////////
////// HTimestamp ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HTimestamp::HTimestamp(QString sqlc,QString ex,QString tit,QString def,bool c)
    : HDataField(sqlc,ex,tit,"",true)
{
    /*
    if(!myInterface()->hsqli_hastimestamptype)
    {
        QString errorstr= "Error!: Dont define timestamp type, currently unsupported by database (HSqlInterface";
        sdebug(errorstr);
        emit errorSignal(errorstr);
        error(errorstr);
        return;
    }
    */
    whoami="HTimestamp";
    defval = def;
    set = 0;
    setDefval(def);
    editable  = true;
    sql       = true;
    check     = c;
    readed    = false;
}

HTimestamp::~HTimestamp(void)
{
    //empty
}

HTimestamp::HTimestamp(HTimestamp *t)
:HDataField(t)
{
    if(!myInterface()->hsqli_hastimestamptype)
    {
        QString errorstr= "Error!: Dont define timestamp type, currently unsupported by database (HSqlInterface";
        sdebug(errorstr);
        emit errorSignal(errorstr);
        error(errorstr);
        return;
    }

    deepcopy_from(t);
}

void HTimestamp::deepcopy_from(HTimestamp *x)
{
    value = x->value;
    check = x->check;
    readed = x->readed;
}

HDataField* HTimestamp::get_clone(void)
{
    return new HTimestamp(this);
}

QVariant HTimestamp::dbValueToDispValue(QVariant v)
{
    QString s = v.toString();
    return QVariant( s.replace(QChar('T'),QChar(' ')) );
}

int HTimestamp::setValue (QVariant v,bool sdisabled)
{
    set = 1;
    value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

int HTimestamp::setDefval(QVariant v)
{
    if(!set) //if the value was set I dont set the default.
        value = defvalParser( v.toString() ).toString();
    return 0;
}

QVariant HTimestamp::getValue (void)
{
    return QVariant(value);
}

QString HTimestamp::getSQLValue (void)
{
    return QString("\'%1\'")
                        .arg(value);
}

QString HTimestamp::getSQLValueAsString(void)
{
    return QString("%1")
                        .arg(value);
}

QString HTimestamp::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HTimestamp::sqlInsertValue(void)
{
    return myInterface()->hsqli_timestampnowvalue;
}

QString HTimestamp::sqlUpdate(void)
{
    return QString("%1=%2")
                    .arg(sqlColumn).arg(myInterface()->hsqli_timestampnowvalue);
}

QString HTimestamp::sqlSelectHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HTimestamp::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    return sqlColumn + QString("\t %1").arg(myInterface()->hsqli_timestamptypename);
}
/////////////////////////////////////////////////////////////////////////////////
////// HSqlChoose ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
int nullhash(const char *c)
{
    Q_UNUSED(c);

    return 0;
}

int charcodehash(const char *c)
{
    return ((int)(c[0]));
}

int dropchar(const char *c)
{
    int i,j,l=(int)strlen(c);
    if(l<=0)
        return 0;
    char *m=new char[l+1];

    for(i=0,j=0;i<l;++i)
        if(isdigit((unsigned char)c[i]))
            m[j++]=c[i];
    m[j]='\0';
    if(m[0]=='\0')
        i = 0;
    else
        i = atoi(m);

    delete[] m;
    return i;
}

HSqlChoose::HSqlChoose(QString sqlc,QString ex,QString tit,QString def,
                       QString ct,QString kf,QString sf,QString sfh,QString fp,QString op,QString inthash,bool easyf)
    : HDataField(sqlc,ex,tit,"",true)
{
    whoami="HSqlChoose";
    defval = def;

    set = 0;

    easyform = easyf;
    readed_reftable = false;

    connected_table = ct;
    key_field       = kf;
    showed_field    = sf;
    showed_field_head = sfh;
    filter_part     = fp;
    order_part      = op;

    editable  = true;
    sql       = true;

    keys.clear();   //cache
    values.clear(); //cache
    pairs   = NULL;
    maxhash = 0;

    hash = dropchar; //set the default hash f.
    setInernalHashFunction(inthash);

    //I used to read this but now I deferred the reading (readed_reftable)
    //refreshSqlValues(false); // fills the keys and values list.

    setDefval(defval);

    toolbrec = NULL;
    btext = "";
    progress_on_read = false;
    popupdialog_title = "";

    toolbarbutton_down_text = "";

    extrafunc_receiver[0] = NULL;   extrafunc_text[0] = "";
    extrafunc_receiver[1] = NULL;   extrafunc_text[1] = "";
    extrafunc_receiver[2] = NULL;   extrafunc_text[2] = "";
    extrafunc_receiver[3] = NULL;   extrafunc_text[3] = "";
    extrafunc_receiver[4] = NULL;   extrafunc_text[4] = "";
}

HSqlChoose::~HSqlChoose(void)
{
    if(pairs != NULL)
    delete[] pairs;
}

HSqlChoose::HSqlChoose(HSqlChoose* t)
:HDataField(t)
{
    deepcopy_from(t);
}

void HSqlChoose::deepcopy_from(HSqlChoose *x)
{
    value = x->value;
    connected_table = x->connected_table;
    key_field = x->key_field;
    showed_field = x->showed_field;
    showed_field_head = x->showed_field_head;
    filter_part = x->filter_part;
    order_part = x->order_part;
    sortc = x->sortc;

    progress_on_read = x->progress_on_read;
    toolbrec = x->toolbrec;
    btext = x->btext;

    extrafunc_receiver[0] = x->extrafunc_receiver[0];   extrafunc_text[0] = x->extrafunc_text[0];
    extrafunc_receiver[1] = x->extrafunc_receiver[1];   extrafunc_text[1] = x->extrafunc_text[1];
    extrafunc_receiver[2] = x->extrafunc_receiver[2];   extrafunc_text[2] = x->extrafunc_text[2];
    extrafunc_receiver[3] = x->extrafunc_receiver[3];   extrafunc_text[3] = x->extrafunc_text[3];
    extrafunc_receiver[4] = x->extrafunc_receiver[4];   extrafunc_text[4] = x->extrafunc_text[4];

    easyform = x->easyform;


    keys.clear();
    values.clear();
    sepvalues.clear();
    maxhash = x->maxhash;
    popupdialog_title = x->popupdialog_title;

    hash = x->hash;

    pairs = NULL;
    readed_reftable = false;
}

HDataField* HSqlChoose::get_clone(void)
{
    return new HSqlChoose(this);
}

int HSqlChoose::refreshSqlValues(bool emitsignal,bool tdisabled)
{
    sdebug("*** HSqlChoose::refreshSqlValues ***");
    QString showed_field_mod;
    int i=0;
    QString sqlq="";

    keys.clear();
    values.clear();
    sepvalues.clear();
    if(!tdisabled)
        transaction();
    showed_field_mod = sqlConcatenateToLocal(showed_field);
    sqlq = QString("SELECT %1,%2 FROM %3 WHERE %4 %5 %6;")
                        .arg(key_field)
                        .arg(showed_field_mod)
                        .arg(connected_table)
                        .arg(myInterface()->hsqli_truevalue)
                        .arg( filter_part.isEmpty() ? "" : (QString(" AND ") +filter_part) )
                        .arg(order_part);

    QSqlQuery *q = allocateQSqlQuery();
    sqldebug(sqlq);


    q->prepare(sqlq);
    q->exec();

    if(q->lastError().type() != QSqlError::NoError)
    {
        QString errstr;
        if(!tdisabled)
            rollback();
        errstr=("The SELECT returned with <=0 record! (1)\n>>"+((q->lastError()).text()));
        delete q;
        emit errorSignal(errstr);
        return 1;
    }

    #ifdef PROGRESS_POSSIBILITY
    emit startWorking();
    #endif

    while(q->next())
    {
        keys  .push_back(q->value(0).toString());
        sepvalues.push_back(q->value(1).toString());
        values.push_back(q->value(1).toString().replace("|"," "));

        #ifdef PROGRESS_POSSIBILITY
        emit doWorking();
        #endif
    }
    if(!tdisabled)
        commit();
    delete q;
    sdebug("Finished query!");

    //DEBUG
    sdebug(QString("Building hash cache %1<-%2.%3 ...")
                    .arg(sqlSelectHead()).arg(connected_table).arg(key_field));
    if(hash == dropchar) sdebug(" (Using dropchar)");
    else if(hash == nullhash) sdebug(" (Using nullchar)");
    else if(hash == charcodehash) sdebug(" (Using charcodehash)");
    else sdebug(" (Using custom hash function)");
    //END DEBUG CODE

    //freeing previous cache
    if(pairs != NULL)
        delete[] pairs;

    maxhash =(int)( keys.size() * HASHMUL);
    pairs = new KVPair[ maxhash ];
    for(i=0;i<maxhash;++i)
    {
        pairs[i].busy = false;
        pairs[i].key   = "";
        pairs[i].value = "";
    }

    QStringList::Iterator ik = keys.begin();
    QStringList::Iterator iv = values.begin();
    while(ik != keys.end())
    {
        //skipping busy spaces..
          //toAscii replaced to toLocal8Bit when ported to Qt5
        for(i=(*hash)((*ik).toLocal8Bit().constData())%maxhash;pairs[i].busy;i = (i + 1) % maxhash);
        //fill it:
        pairs[i].busy = true;
        pairs[i].key   = *ik;
        pairs[i].value = *iv;

        ++ik;
        ++iv;

        #ifdef PROGRESS_POSSIBILITY
        emit doWorking();
        #endif
    }
    sdebug("...done.");

    #ifdef PROGRESS_POSSIBILITY
    emit endWorking();
    #endif

    readed_reftable = true;

    if(emitsignal)
        dataUpdatedSignal();
    return 0;
}

QVariant HSqlChoose::dbValueToDispValue(QVariant v)
{
    //sdebug("*** HSqlChoose::dbValueToDispValue ***");
    int i=0,end=0;
    QString vs;
    vs = v.toString();

    if(v.toString() == "NULL" || v.toString().isEmpty())
        return QVariant(QString(""));

    if(!readed_reftable)
        refreshSqlValues(false);

    if(pairs == NULL)
        return QVariant(values[getIndexOf(keys,vs)]);

        //toAscii replaced to toLocal8Bit when ported to Qt5
    end = i = (*hash)(vs.toLocal8Bit().constData())%maxhash;
    for( ; ((i+1) != end) ; i = (i + 1) % maxhash)
        if(pairs[i].key == vs)
            return QVariant(pairs[i].value);

    return QVariant(QString(""));
}

void HSqlChoose::setFilterPart(QString fp)
{
    readed_reftable = false;
    filter_part = fp;
}

QStringList& HSqlChoose::getKeys  (void)
{
    if(!readed_reftable)
        refreshSqlValues(false);
    return keys;
}

QStringList& HSqlChoose::getValues(void)
{
    if(!readed_reftable)
        refreshSqlValues(false);
    return values;
}

QStringList& HSqlChoose::getSeparatedValues(void)
{
    if(!readed_reftable)
        refreshSqlValues(false);
    return sepvalues;
}

int HSqlChoose::setDefval(QVariant v)
{
    sdebug("*** HSqlChoose::setDefval ***");
    QString calculated_defval;

    if(!set)//if the value was set I dont set the default.
    {
        calculated_defval = defvalParser( v.toString() ).toString();

        // If the HSqlChoose in easyform mode the default value must be valid in the referenced table,
        // because the combobox can't show "Not valid"/"Not set" value.
        // Otherwise the HSqlChoose in normal mode (popup selection) the button can show a "-" sign which means that the
        // value is not set or invalid. (That case the sql field contains NULL)
        if(easyform)
        {
            if(!readed_reftable)
                refreshSqlValues(false);

            QStringList::iterator it = keys.begin();
            while(it != keys.end())
            {
                if(*it == calculated_defval)
                {
                    //I found the calculated default field value between the keys,
                    //so I will set the current to this value.
                    value = calculated_defval;
                    sdebug("*** HSqlChoose::setDefval *** END");
                    return 0;
                }
                ++it;
            }

            //If we reach that code, it's mean that we didn't found the calculated value between the keys
            //so I will set the first to current.
            if(keys.empty())
                value = "";
            else
                value = keys[0];
        }
        else
        {
            value = calculated_defval;
        }

    }
    sdebug("*** HSqlChoose::setDefval *** END");
    return 0;
}

int HSqlChoose::setValue (QVariant v,bool sdisabled)
{
    sdebug("*** HSqlChoose::setValue ***");
    set = 1;
    if(v.toString().isEmpty())
        value = "NULL";
    else
        value = v.toString();
    if(!sdisabled)
        emit dataChanged();
    value_changed();
    return 0;
}

QVariant HSqlChoose::getValue (void)
{
    return QVariant(value);
}

QString HSqlChoose::getSQLValue (void)
{
    if(value == "NULL")
        return QString("NULL");

    return QString("\'%1\'").arg(value);
}

QString HSqlChoose::getSQLValueAsString(void)
{
    return QString("%1").arg(value);
}

QString HSqlChoose::sqlInsertHead(void)
{
    return QString("%1").arg(sqlColumn);
}

QString HSqlChoose::sqlInsertValue(void)
{
    if(value == "NULL")
        return QString("NULL");
    return QString("\'%1\'").arg(value);
}

QString HSqlChoose::sqlUpdate(void)
{
    return QString("%1=%2")
                             .arg(sqlColumn)
                             .arg((value == "NULL" || value == NULL ? "NULL" : ("\'"+value+"\'")));
}

QString HSqlChoose::sqlSelectHead(void)
{
    return QString("%1")
                     .arg(sqlColumn);
}

void HSqlChoose::enableEasyForm(void)
{
    easyform = true;
    setDefval(defval);
}

void HSqlChoose::disableEasyForm(void)
{
    easyform = false;
    setDefval(defval);

}

void HSqlChoose::addToolButton(const QObject* receiver,QString text)
{
    toolbrec = receiver;
    btext = text;
}

void HSqlChoose::removeToolButton(void)
{
    toolbrec = NULL;
    btext = "";
}

void HSqlChoose::addExtraFunction(int pos,const QObject* receiver,QString text)
{
    extrafunc_receiver[pos] = receiver;
    extrafunc_text[pos] = text;
}

void HSqlChoose::removeExtraFunction(void)
{
    extrafunc_receiver[0] = NULL;   extrafunc_text[0] = "";
    extrafunc_receiver[1] = NULL;   extrafunc_text[1] = "";
    extrafunc_receiver[2] = NULL;   extrafunc_text[2] = "";
    extrafunc_receiver[3] = NULL;   extrafunc_text[3] = "";
    extrafunc_receiver[4] = NULL;   extrafunc_text[4] = "";
}

QString HSqlChoose::sqlCreateStringPart(QString switches)
{
    Q_UNUSED(switches);

    if(!myInterface()->hsqli_sqlchooseconstraint)
        return sqlColumn + QString("\t VARCHAR(%1) ").arg(myInterface()->hsqli_varcharhkeylength);

    return sqlColumn + QString("\t VARCHAR(%1) REFERENCES %2 MATCH FULL")
                                        .arg(myInterface()->hsqli_varcharhkeylength)
                                        .arg(connected_table);
}

void HSqlChoose::setInernalHashFunction(QString hashname)
{
    sdebug("*** HSqlChoose::setInernalHashFunction ***");

    if(hashname == "dropchar")
    {
        hash = dropchar;
        sdebug("Hash function set to: DROPCHAR");
    }
    if(hashname == "charcodehash")
    {
        hash = charcodehash;
        sdebug("Hash function set to: CHARCODEHASH");
    }

    if(hashname == "nullhash")
    {
        hash = nullhash;
        sdebug("Hash function set to: NULLHASH");
    }

    sdebug("*** HSqlChoose::setInernalHashFunction *** END");
}

// ///////////////////////////////////////////////////////////////////////////////
// / Classes of HDataChangeLoggerdata fields (and related) ///////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDataChangeLogger::HDataChangeLogger(HTable *logtable,QString startpos,QString startuser)
{
    sdebug("*** HDataChangeLogger::HDataChangeLogger ***");
    whoami="HDataChangeLogger";
    QString name,type;
    curr_pos = startpos;
    curr_user = startuser;
    this->logtable = NULL;

    name = "dclkey"; type = "HKey";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "pos"; type = "HSmallText";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "cuser"; type = "HSmallText";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "sqltablename"; type = "HSmallText";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "changedkey"; type = "HSmallText";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "sqlfieldname"; type = "HSmallText";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "oldvalue"; type = "HSmallText";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "newvalue"; type = "HSmallText";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }
    name = "changetime"; type = "HTimestamp";
    if( logtable->fieldBySqlName(name) == NULL || logtable->fieldBySqlName(name)->getWhoami() != type)
    {
        error(QString("HDataChangeLogger::HDataChangeLogger:"
              " Error, the \"logtable\" must contain a \"%1\" named field with \"%2\" type!").arg(name).arg(type));
        return;
    }

    this->logtable = logtable;
    sdebug("*** HDataChangeLogger::HDataChangeLogger ***END");
}

HDataChangeLogger::~HDataChangeLogger(void)
{
    sdebug("*** HDataChangeLogger::~HDataChangeLogger ***");
    if(changes.count() > 0)
    {
        sdebug("*** HDataChangeLogger::~HDataChangeLogger : WARNING! Unsaved data in the logger!");
    }
    changes.clear();
    sdebug("*** HDataChangeLogger::~HDataChangeLogger ***END");
}

void HDataChangeLogger::addChange(QString fieldname,QString oldval,QString newval)
{
    struct HOneDataChange odc;

    odc.pos = curr_pos;
    odc.user = curr_user;
    odc.tablename = curr_table;
    odc.changedkey = curr_ckey;
    odc.fieldname = fieldname;
    odc.oldvalue = oldval;
    odc.newvalue = newval;

    changes.push_back(odc);
}

void HDataChangeLogger::writeOut(bool tdisabled)
{
    int c,i;

    c = changes.count();
    for(i=0;i<c;++i)
    {
        logtable->returnToDefault();
        logtable->setSqlFieldValue("dclkey","");
        logtable->setSqlFieldValue("pos"         ,QVariant(changes[i].pos));
        logtable->setSqlFieldValue("cuser"       ,QVariant(changes[i].user));
        logtable->setSqlFieldValue("sqltablename",QVariant(changes[i].tablename));
        logtable->setSqlFieldValue("changedkey"  ,QVariant(changes[i].changedkey));
        logtable->setSqlFieldValue("sqlfieldname",QVariant(changes[i].fieldname));
        logtable->setSqlFieldValue("oldvalue"    ,QVariant(changes[i].oldvalue));
        logtable->setSqlFieldValue("newvalue"    ,QVariant(changes[i].newvalue));
        logtable->setSqlFieldValue("changetime"  ,QVariant(""));

        if(logtable->insertRecord(false,tdisabled))
        {
            query_error_occured = true;
            emit errorSignal("Error: Can't write the logger table!");
            return;
        }
    }
    logtable->returnToDefault();
    changes.clear();
}

// ///////////////////////////////////////////////////////////////////////////////
// / Classes of HDynTable (and related) //////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDynTableElement::HDynTableElement(int type,QString name,QString sqlname,QString rowName,QString colName,QString labels)
{
    if(type != HDYNELEMENT_STRING && type != HDYNELEMENT_DOUBLE)
        type = HDYNELEMENT_STRING;

    this->type    = type;
    this->name    = name;
    this->rowName = rowName;
    this->colName = colName;
    this->sqlname = sqlname;
    this->labels.clear();
    this->labels = labels.split("|",QString::SkipEmptyParts);

    textual_value = "";
    double_value  = 0.0;
}

int HDynTableElement::getType(void)
{
    return type;
}

bool HDynTableElement::isNumeric(void)
{
    if(type == HDYNELEMENT_DOUBLE)
        return true;
    return false;
}
bool HDynTableElement::isTextual(void)
{
    return !isNumeric();
}

void HDynTableElement::setDoubleValue(double d)
{
    if(type == HDYNELEMENT_STRING)
        textual_value = QString("%1").arg(d);

    if(type == HDYNELEMENT_DOUBLE)
        double_value = d;
}

void HDynTableElement::setTextualValue(QString s)
{
    if(type == HDYNELEMENT_STRING)
        textual_value = s;

    if(type == HDYNELEMENT_DOUBLE)
    {
        double d;
        bool ok;
        d = s.toDouble(&ok);
        if(ok)
            double_value = d;
    }
}

double HDynTableElement::getDoubleValue(void)
{
    if(type == HDYNELEMENT_STRING)
    {
        double d;
        bool ok;
        d = textual_value.toDouble(&ok);
        if(ok)
            return d;
    }

    if(type == HDYNELEMENT_DOUBLE)
         return double_value;

    return 0.0;
}

QString HDynTableElement::getTextualValue(void)
{
    if(type == HDYNELEMENT_STRING)
        return textual_value;

    if(type == HDYNELEMENT_DOUBLE)
        return QString("%1").arg(double_value);

    return "";
}

void HDynTableElement::addLabel(QString l)
{
    if(!l.isEmpty())
        labels.push_back(l);
}

bool HDynTableElement::hasLabel(QString l)
{
    if(l.isEmpty())
        return true;
    QList<QString>::iterator i=labels.begin();
    while(i != labels.end())
    {
        if(*i == l)
            return true;
        ++i;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////

HDynTable::HDynTable(void)
{
    whoami = "HDynTable";
    elements.clear();
}

HDynTable::HDynTable(const HDynTable& t)
{
    whoami = "HDynTable";
    elements.clear();

    QList<HDynTableElement>::const_iterator i = t.elements.begin();
    while(i != t.elements.end())
    {
        elements.push_back(HDynTableElement(*i));
        ++i;
    }
}

HDynTable::~HDynTable(void)
{
    elements.clear();
}

HDynTable& HDynTable::operator=(HDynTable s)
{
    elements.clear();

    QList<HDynTableElement>::const_iterator i = s.elements.begin();
    while(i != s.elements.end())
    {
        elements.push_back(HDynTableElement(*i));
        ++i;
    }
    return *this;
}

void HDynTable::add(HDynTableElement e)
{
    elements.push_back(e);
}

QString HDynTable::dumpElements(void)
{
    QString s="";
    bool first=true;
    firstElement();
    while(!isEnded())
    {
        s.append(QString("%1%2=%3")
                 .arg(first ? "" : ",")
                 .arg(currentElementName())
                 .arg(currentElementValueString()));
        nextElement();
        first = false;
    }
    return s;
}

QString HDynTable::getElementSqlName(QString name)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return i->sqlname;
        ++i;
    }
    return "";
}

void HDynTable::setElementValue(QString name,double value)
{

    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            i->setDoubleValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::setElementValue(QString name,QString value)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            i->setTextualValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

double HDynTable::getElementValueDouble(QString name,bool *found)
{
    if(found != NULL)
        *found = false;

    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
        {
            if(found != NULL)
                *found = true;
            return i->getDoubleValue();
        }
        ++i;
    }
    return 0.0;
 }

QString HDynTable::getElementValueString(QString name,bool *found)
{
    if(found != NULL)
        *found = false;

    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
        {
            if(found != NULL)
                *found = true;
            return i->getTextualValue();
        }
        ++i;
    }
    return "";
}

void HDynTable::setElementValueByRowColName(QString r,QString c,double value)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
            i->setDoubleValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::setElementValueByRowColName(QString r,QString c,QString value)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
            i->setTextualValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

double HDynTable::getElementValueDoubleByRowColName(QString r,QString c,bool *found)
{
    if(found != NULL)
        *found = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
        {
            if(found != NULL)
                *found = true;
            return i->getDoubleValue();
        }
        ++i;
    }
    return 0.0;
}

QString HDynTable::getElementValueStringByRowColName(QString r,QString c,bool *found)
{
    if(found != NULL)
        *found = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
        {
            if(found != NULL)
                *found = true;
            return i->getTextualValue();
        }
        ++i;
    }
    return "";
}

void HDynTable::firstElement(void)
{
    internal_iterator = elements.begin();

}

bool HDynTable::isEnded(void)
{
    if(internal_iterator == elements.end())
        return true;
    return false;
}

void HDynTable::nextElement(void)
{
    ++internal_iterator;
}

QString HDynTable::currentElementName(void)
{
    return internal_iterator->name;
}

QString HDynTable::currentElementRowName(void)
{
    return internal_iterator->rowName;
}

QString HDynTable::currentElementColumnName(void)
{
    return internal_iterator->colName;
}

QString HDynTable::currentElementSqlName(void)
{
    return internal_iterator->sqlname;
}

QStringList HDynTable::currentElementLabels(void)
{
    return internal_iterator->labels;
}

bool HDynTable::currentElementIsNumeric(void)
{
    return internal_iterator->isNumeric();
}

double HDynTable::currentElementValueDouble(void)
{
    return internal_iterator->getDoubleValue();
}

QString HDynTable::currentElementValueString(void)
{
    return internal_iterator->getTextualValue();
}

void HDynTable::setCurrentElementValue(double d)
{
    internal_iterator->setDoubleValue(d);
    emit dataChangedNongui();
}

void HDynTable::setCurrentElementValue(QString s)
{
    internal_iterator->setTextualValue(s);
    emit dataChangedNongui();
}

void HDynTable::emptyall(QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo))
        {
            if(i->isNumeric())
                i->setDoubleValue(0.0);
            else
                i->setTextualValue("");
        }
        ++i;
    }
    emit dataChangedNongui();

}

void HDynTable::zeroall(QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo))
            if(i->isNumeric())
                i->setDoubleValue(0.0);

        ++i;
    }
    emit dataChangedNongui();
}

double HDynTable::max(QString limitTo)
{
    double max = 0;
    bool set = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric() && !set)
        {
            max = i->getDoubleValue();
            set = true;
        }
        if(i->hasLabel(limitTo) && i->isNumeric() && i->getDoubleValue() > max)
            max = i->getDoubleValue();
        ++i;
    }
    return max;
}

double HDynTable::min(QString limitTo)
{
    double min = 0;
    bool set = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric() && !set)
        {
            min = i->getDoubleValue();
            set = true;
        }
        if(i->hasLabel(limitTo) && i->isNumeric() && i->getDoubleValue() < min)
            min = i->getDoubleValue();
        ++i;
    }
    return min;
}

double HDynTable::sum(QString limitTo)
{
    double sum = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            sum += i->getDoubleValue();
        ++i;
    }
    return sum;
}

void HDynTable::sustraction(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() - d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::sustraction(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() - dt.getElementValueDouble(i->name) );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::addition(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() + d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::addition(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() + dt.getElementValueDouble(i->name) );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::multiplicate(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() * d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::multiplicate(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() * dt.getElementValueDouble(i->name) );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::divide(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            if(d != 0.0)
                i->setDoubleValue( i->getDoubleValue() / d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::divide(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
        {
            double d;
            d = dt.getElementValueDouble(i->name);
            if(d != 0.0)
                i->setDoubleValue( i->getDoubleValue() / d );
        }
        ++i;
    }
    emit dataChangedNongui();
}

int HDynTable::countElements(QString limitTo)
{
    int count = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo))
            ++count;
        ++i;
    }
    return count;
}

int HDynTable::indexByElementName(QString name)
{
    int idx=0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return idx;

        ++idx;
        ++i;
    }
    return -1;
}

int HDynTable::indexByElementSqlName(QString sqlname)
{
    int idx=0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->sqlname == sqlname)
            return idx;

        ++idx;
        ++i;
    }
    return -1;
}

QString HDynTable::indexedElementName(int index)
{
    int run = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(run == index)
            return i->name;

        ++run;
        ++i;
    }
    return "";
}

QString HDynTable::indexedElementRowName(int index)
{
    int run = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(run == index)
            return i->rowName;

        ++run;
        ++i;
    }
    return "";
}

QString HDynTable::indexedElementColName(int index)
{
    int run = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(run == index)
            return i->colName;

        ++run;
        ++i;
    }
    return "";
}

QString HDynTable::indexedElementSqlName(int index)
{
    return getElementSqlName( indexedElementName(index) );
}

double HDynTable::getIndexedValueDouble(int index,bool *found)
{
    return getElementValueDouble( indexedElementName(index) , found );
}

QString HDynTable::getIndexedValueString(int index,bool *found)
{
    return getElementValueString( indexedElementName(index) , found );
}

void HDynTable::setIndexedElementValue(int index,double d)
{
    setElementValue( indexedElementName(index) , d );
}

void HDynTable::setIndexedElementValue(int index,QString s)
{
    setElementValue( indexedElementName(index) , s );
}

bool HDynTable::elementHasLabel(QString name,QString label,bool *found)
{
    if(found != NULL)
        *found = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
        {
            if(found != NULL)
                *found = true;
            return i->hasLabel(label);
        }
        ++i;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////

void HDynTableSqlConnector::insertSql(QString additionaldefpart,QString additionalvaluepart)
{
    if(dynt == NULL)
        return;

    QString dpart="",vpart="";

    int idx = 0;
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        if(idx != 0)
        {
            dpart.append(",");
            vpart.append(",");
        }

        dpart.append( dynt->currentElementSqlName() );
        vpart.append( QString("\'%1\'").arg(dynt->currentElementValueString()) );
        ++idx;
        dynt->nextElement();
    }

    if(!additionaldefpart.isEmpty())
    {
        if(!dpart.isEmpty())
            dpart.append(",");
        dpart.append(additionaldefpart);
    }

    if(!additionalvaluepart.isEmpty())
    {
        if(!vpart.isEmpty())
            vpart.append(",");
        vpart.append(additionalvaluepart);
    }

    QString query = QString("INSERT INTO %1(%2) VALUES(%3);")
                        .arg(tablename)
                        .arg(dpart)
                        .arg(vpart);
    submit0ResultQuery(query,errorText,tdisabled);
}

int HDynTableSqlConnector::readSql(void)
{
    if(dynt == NULL)
        return 1;

    sdebug("*** HDynTableSqlConnector::readSql ***");
    int idx = 0;
    QString query="SELECT ";
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        if(idx != 0)
            query.append(",");
        query.append(dynt->currentElementSqlName());
        ++idx;
        dynt->nextElement();
    }
    query.append(QString(" FROM %1%2%3;")
                        .arg(tablename)
                        .arg(filterpart.isEmpty() ? "" : " WHERE ")
                        .arg(filterpart));

    HPlainDataMatrix *r = submitNResultQuery(idx,query,errorText,tdisabled);
    if(!errorStatus() && r->rowCount() > 0)
    {
        idx = 0;
        dynt->firstElement();
        while(!dynt->isEnded())
        {
            dynt->setCurrentElementValue(r->getCellStr(0,idx));

            ++idx;
            dynt->nextElement();
        }
        emit justReaded();
    }

    delete r;
    sdebug("*** HDynTableSqlConnector::readSql ***END");
    return 0;
}

int HDynTableSqlConnector::updateSql(void)
{
    if(dynt == NULL)
        return 1;

    sdebug("*** HDynTableSqlConnector::updateSql ***");
    int idx = 0;
    QString query = QString("UPDATE %1 SET ").arg(tablename);
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        if(idx != 0)
            query.append(",");
        query.append( dynt->currentElementSqlName() );
        query.append( "=" );
        query.append( QString("\'%1\'").arg(dynt->currentElementValueString()) );
        ++idx;
        dynt->nextElement();
    }
    query.append(QString("%1%2;")
                        .arg(filterpart.isEmpty() ? "" : " WHERE ")
                        .arg(filterpart));

    submit0ResultQuery(query,errorText,tdisabled);
    if(!errorStatus())
        emit justUpdated();
    sdebug("*** HDynTableSqlConnector::updateSql *** END");
    return 0;
}

QString HDynTableSqlConnector::sqlCreateString(QString switches)
{
    Q_UNUSED(switches);
    QString cs;

    cs = QString("CREATE TABLE %1 (").arg(tablename);
    int idx = 0;
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        if(idx != 0)
            cs.append(",\n\t");
        else
            cs.append("\n\t");
        cs.append( dynt->currentElementSqlName() );
        cs.append(" ");
        cs.append( dynt->currentElementIsNumeric() ?
                       (currentDefaultSqlInterface()->hsqli_floattypename) :
                        "VARCHAR");

        ++idx;
        dynt->nextElement();
    }
    cs.append("\n);");
    return cs;
}

//End of gSAFE datalib.cpp
