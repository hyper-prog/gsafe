/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   ra.h
*/

#ifndef GSAFE__RA_HEADER_FILE_X_
#define GSAFE__RA_HEADER_FILE_X_

#include <QtCore>

/** \defgroup ra ra */
/*  @{  */

#ifdef MODULE_REFRESHAGENT_NETSERVER
#include <QtNetwork>
#endif
#ifdef MODULE_REFRESHAGENT_NETCLIENT
#include <QtNetwork>
#endif

#define REFRESHAGENT_TCPPORT          1234

/** This is the "refresh agent" of gSAFE.
 *  This class is intend to refresh all sql table data which is updated in the sql database.
 *  You can use this class by creating an instance. (But only one instance in the program)\n
 *  If this class receive a notify with an sql table name it makes to refresh all HRecordLines in program which
 *  shows that table or depended table. \n
 *  You can extend the functionaity of this class not only one program but more instance of programs
 *  on whole network by using HRefreshAgentNetserver and HRefreshAgentNetclient.
 *  @see notify()
 *  @see HList */
class HRefreshAgent : public QObject
{
    Q_OBJECT

    protected:
        static HRefreshAgent *theone;

    public slots:
        int notifySlot(QString tblname);

    public:
        /** Creates the refresh agent (No need to store the pointer of the instance) */
        HRefreshAgent(void);
        ~HRefreshAgent(void);

        /** Call this function with an sql table name to makes refresh all HList which depend from that table.
         *  That's you need to do. Everything else is automatical.
         *  @param tblname the changed tablename    */
        static void notify(QString tblname);

        /** Returns the pointer of the refresh agent instance */
        static HRefreshAgent* getNotifyAgent(void) { return theone; }


    protected:
        void internalNotify(QString tblname);

    signals:
        /** This signals is used for notify the HList-s */
        void getnotify(QString tblname);
};

#ifdef MODULE_REFRESHAGENT_NETSERVER
/** The HRefreshAgentNetclient - The server class of HRefreshAgentNetclient
 *  This class is not connected to HRefreshAgent. If the machice running this server object,
 *  you have to run a client instance too to use HRefreshAgent on the network.
 *  That case the client has to be connected to localhost.
 *  @see HRefreshAgentNetclient */
class HRefreshAgentNetserver : public QObject
{
    Q_OBJECT

    public:
        /** Creates an instance of HRefreshAgentNetserver. Will be listening on network and allow connect from evrywhere */
        HRefreshAgentNetserver(void);
        /** Destructor */
        ~HRefreshAgentNetserver(void);

        /** Returns true if the server is running, otherwise false */
        static bool isRunning(void);
        /** Returns the pointer of the HRefreshAgentNetserver instance */
        static HRefreshAgentNetserver * getHRefreshAgentNetserver(void) { return theone; }
        /** Returns the server's ip address */
        static QString serverAddress();

    protected:
        static bool inNotify;
        static HRefreshAgentNetserver *theone;

    private:
        QTcpServer *tcpServer;
        QList<QTcpSocket *> tcps_list;
        QString laddr;

    private slots:
        int newConn(void);
        int request(void);
        int clientLeaving(void);
        int action(QString name,int sendernum);

};
#endif

#ifdef MODULE_REFRESHAGENT_NETCLIENT
/** This is the class which extends the functionality of HRefreshAgent to work on network and more program instance.
 *  It works as a network client which connect to the server.
 *  (The server runs an instance of HRefreshAgentNetserver) \n
 *  You can use this class by creating an instance afrer the HRefreshAgent.
 *  The HRefreshAgent instance must be created before creating HRefreshAgentNetclient instance!
 *  This class will be automatically find and work together with HRefreshAgent.
 *  You can set the HRefreshAgentNetserver IP address or name in the constructor. */
class HRefreshAgentNetclient : public QObject
{
    Q_OBJECT

    public:
        /** Creates an instance of HRefreshAgentNetclient and connect to the server_ip address */
        HRefreshAgentNetclient(QString server_ip);
        /** Destructor */
        ~HRefreshAgentNetclient();

        /** Returns true if the client is connected to server */
        static bool isConnected(void);
        /** Returns the pointer of the HRefreshAgentNetclient instance */
        static HRefreshAgentNetclient * getHRefreshAgentNetclient(void) { return theone; }

    protected:
        static bool inNotify;
        static HRefreshAgentNetclient *theone;

    private:
        QTcpSocket *socket;
        bool c_ok;

    private slots:
        int request(void);
        int serverClosing(void);
        int action(QString name);

    signals:
        /** Activated if the network connection is lost */
        void connectionLost();
        /** Activcated if the network connection is established */
        void connectionEstablished();
};
#endif

/* @} */
#endif

//End of gSAFE ra.h
