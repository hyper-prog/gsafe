/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2006-2013 Peter Deak  (hyper80@gmail.com)

    License: GPLv2  http://www.gnu.org/licenses/gpl-2.0.html

    dconsole.cpp
*/

#include <QtCore>
#include <QtSql>

#ifndef GSAFE_DISABLE_DEBUG
#include <QtGui>

#ifndef COMPILED_WITH_QT4X
#include <QtWidgets>
#endif

#endif

#include "dconsole.h"

//#define FILE_DEBUG 1

#ifndef GSAFE_DISABLE_DEBUG
HDebugConsole * HDebugConsole::myself = NULL;
#endif //GSAFE_DISABLE_DEBUG

void sqldebug(QString s)
{
#ifndef GSAFE_DISABLE_DEBUG
    if(HDebugConsole::myself == NULL)
        return;
    HDebugConsole::debug_sql(s);
#else
    Q_UNUSED(s);
#endif //GSAFE_DISABLE_DEBUG
}

void sdebug(QString s)
{
#ifndef GSAFE_DISABLE_DEBUG
    if(HDebugConsole::myself == NULL)
        return;
    HDebugConsole::debug_txt(s);
#else
    Q_UNUSED(s);
#endif //GSAFE_DISABLE_DEBUG
}

void dconsole(void)
{
#ifndef GSAFE_DISABLE_DEBUG
    if(HDebugConsole::myself == NULL)
    {
        HDebugConsole *dw=new HDebugConsole(0);
        dw->show();
    }
#endif //GSAFE_DISABLE_DEBUG
}

void dconsole_close(void)
{
#ifndef GSAFE_DISABLE_DEBUG
    if(HDebugConsole::myself != NULL)
    {
        HDebugConsole::myself->close();
        delete HDebugConsole::myself;
        HDebugConsole::myself=NULL;
    }
#endif //GSAFE_DISABLE_DEBUG
}

void dconsole_popup(QString t,QString txt)
{
    #ifndef GSAFE_DISABLE_DEBUG
    QMessageBox::warning(NULL,t,txt);
    #else
    Q_UNUSED(t);
    Q_UNUSED(txt);
    #endif //GSAFE_DISABLE_DEBUG
}


QMultiMap<QString,HDConsoleCommandHolder *> user_commands;
QMultiMap<QString,QString> user_commands_descr;

void register_dconsole_command(QString command,HDConsoleCommandHolder *interpreter,QString descr)
{
    if(!command.isEmpty() && interpreter != NULL)
    {
        user_commands.insert(command,interpreter);
        user_commands_descr.insert(command,descr);
    }
}

void unregister_dconsole_command(QString command)
{
    if(!command.isEmpty())
    {
        user_commands.remove(command);
        user_commands_descr.remove(command);
    }
}

void clear_dconsole_commands()
{
    user_commands.clear();
    user_commands_descr.clear();
}

#ifndef GSAFE_DISABLE_DEBUG

HDebugConsole::HDebugConsole(QWidget *parent)
:QWidget(parent)
{
    setupUi(this);
    databasename = "";
    connect(lineSQL,SIGNAL(returnPressed()),this,SLOT(execSql()));
    myself = this;
    debugtxt->setPlainText("START:\n");
    #ifdef FILE_DEBUG
    pushSyncwrite->setChecked(true);
    #endif
}

HDebugConsole::~HDebugConsole(void)
{
    pre = "";
    myself = NULL;
}

void HDebugConsole::popup(QString title,QString str)
{
    QMessageBox::warning(NULL,title,str);
}

//#define FILE_DEBUG
void HDebugConsole::add_text(QString s,int type)
{
    bool change;
    QString dtext="";

    change = false;

    if(pushSyncwrite->isChecked())
    {
        FILE *dbgf;

        dbgf = fopen("syndebug.txt","a");
        fseek(dbgf,0,SEEK_END);
        fprintf(dbgf,"\n%s\n",s.toLocal8Bit().constData());
        fclose(dbgf);
    }

    if(pushSql->isChecked() && type == 0)
    {
        change = true;
        //dtext.append("\n<font color=\"#FF0000\">"+s+"</font>");
        dtext.append(s);
    }
    if(pushText->isChecked() && type == 1)
    {
        change = true;
        dtext.append(s);
    }

    if(change)
    {
        debugtxt->append(dtext);
        debugtxt->moveCursor(QTextCursor::End);
    }
    //checkIfIClose();
    QApplication::processEvents();
}

void HDebugConsole::closeEvent(QCloseEvent *e)
{
    myself = NULL;
    QWidget::closeEvent(e);
}

void HDebugConsole::debug_sql(QString s)
{
    //s.prepend("<font color=\"#FF8888\">").append("</font>");
    if(myself != NULL)
        myself->add_text(s,0);
}

void HDebugConsole::debug_txt(QString s)
{
    //s.append("");
    if(myself != NULL)
        myself->add_text(s,1);
}

int HDebugConsole::execSql(void)
{
    int i,cn;
    QVariant v;
    QString query;
    QString dtext;

    dtext = debugtxt->toPlainText();
    query = lineSQL->text();

    //Special commands:
    if(query == "")
    {
        lineSQL->clear();
        sdebug(".");
        return 0;
    }
    if(query == "help")
    {
        lineSQL->clear();
        sdebug("");
        sdebug("exit - Exit main program (The debugged program too)");
        sdebug("close - Close the debug window (only)");
        sdebug("clear - Clear the debug window");
        sdebug("alldb - Show all available database connections");
        sdebug("dbinfo - Show the current connected database information");
        sdebug("setdb - Set the current database to the default (not the program but console)");
        sdebug("setdb name - Set the current database to \"name\" (not the program but console)");
        sdebug("filters - Show available debug filters");
        sdebug("state <filter> - Show the state of the specified filter");
        sdebug("enable <filter> - Enable the specified kind of output");
        sdebug("disable <filter> - Disable the specified kind of output");
        sdebug("synw - Query the state of syndebug");
        sdebug("synw [off|on] - Set the state of syndebug");
        sdebug("synw clear - Clear the file syndebug.txt");
        sdebug("write <text> - Write the text to the console");
        sdebug("save - Save the content of debug window to debug.txt");
        sdebug("pre - Load previous SQL command");
        sdebug("run <command> - Run (custom) a program command");
        sdebug("\"SQL\" - Execute the SQL command");
        if(user_commands.size() > 0)
        {
            sdebug("  -- CUSTOM/PROGRAM COMMANDS --  ");
            QMultiMap<QString,HDConsoleCommandHolder *>::const_iterator i = user_commands.constBegin();
            while(i != user_commands.constEnd())
            {
                sdebug(QString(" run %1%2%3")
                            .arg(i.key())
                            .arg( user_commands_descr.value(i.key()) == "" ? "" : " - ")
                            .arg(user_commands_descr.value(i.key())));
                ++i;
            }
        }
        sdebug("\n");
        return 0;
    }
    if(query == "exit")
        QApplication::exit(0);
    if(query == "close")
        close();
    if(query == "clear")
    {
        lineSQL->clear();
        debugtxt->clear();
        return 0;
    }
    if(query == "save")
    {
        FILE *dbgf;

        dbgf = fopen("debug.txt","w");
        fseek(dbgf,0,SEEK_SET);
        fprintf(dbgf,"\n%s\n",dtext/*.replace(QRegExp(QString("\<[^\<\>]*\>"),Qt::CaseInsensitive,QRegExp::RegExp),"")*/.toLocal8Bit().constData());
        fclose(dbgf);
        lineSQL->clear();
        sdebug("Saving console text to debug.txt is done.\n");
        return 0;
    }
    if(query.startsWith("dbinfo"))
    {
        lineSQL->clear();
        QSqlDatabase db;
        if(databasename.isEmpty())
            db = QSqlDatabase::database();
        else
            db = QSqlDatabase::database(databasename);
        if(db.isOpen())
        {
            sdebug(QString(".\nCurrent OPENED database connection is:\nConnection name=%1\nDriver=%2\nName=%3\nHost=%4\nUser=%5\n")
                .arg(db.connectionName())
                .arg(db.driverName())
                .arg(db.databaseName())
                .arg(db.hostName())
                .arg(db.userName()));
            return 0;
        }
        else
        {
            sdebug(QString(".\nThere is no opened database!\n"));
            return 0;
        }

        return 0;
    }

    if(query == "synw")
    {
        lineSQL->clear();
        sdebug(QString(".\nSyncronized backup file writing: %1.\n").arg(pushSyncwrite->isChecked() ? "on" : "off"));
        return 0;
    }
    if(query == "synw on")
    {
        lineSQL->clear();
        pushSyncwrite->setChecked(true);
        sdebug(QString(".\nEnabled syncronized backup file writing (syndebug.txt).\n"));
        return 0;
    }
    if(query == "synw off")
    {
        lineSQL->clear();
        sdebug(QString(".\nDisabled syncronized backup file writing.\n"));
        pushSyncwrite->setChecked(false);
        return 0;
    }
    if(query == "synw clear")
    {
        lineSQL->clear();

        FILE *dbgf;
        dbgf = fopen("syndebug.txt","w");
        fseek(dbgf,0,SEEK_SET);
        fprintf(dbgf,"\n");
        fclose(dbgf);
        sdebug(QString(".\nThe content of syndebug.txt has been cleared.\n"));
        return 0;
    }
    if(query.startsWith("write"))
    {
        lineSQL->clear();
        sdebug(query.mid(6));
        return 0;
    }
    if(query == "alldb")
    {
        lineSQL->clear();
        QStringList l = QSqlDatabase::connectionNames();
        QStringList::iterator i=l.begin();
        sdebug(QString(".\nAvailable databases (%1) :").arg(l.count()));
        while(i != l.end())
        {
            sdebug("  " + *i);
            ++i;
        }
        return 0;
    }
    if(query.startsWith("setdb"))
    {
        QString c;

        lineSQL->clear();
        c = query.mid(6);
        if(c.isEmpty())
            c=""; //default will be used
        sdebug(QString("Setting database to default (in console) \"%1\"...").arg(c));
        databasename = c;
        sdebug("");
        return 0;
    }
    if(query.startsWith("run"))
    {

        QString c;
        QMultiMap<QString,HDConsoleCommandHolder *>::iterator i;

        lineSQL->clear();
        c = query.mid(4);
        sdebug(QString("Starting command \"%1\"...").arg(c));
        i = user_commands.find(c);
        if(i == user_commands.end())
        {
            sdebug(" Error: Command not found!");
        }
        else
        {
            HDConsoleCommandHolder *dcch = *i;
            sdebug(dcch->donsole_command_interpreter(c));
            sdebug("finished.\n");
        }
        sdebug("");
        return 0;
    }

    if(query.startsWith("filters"))
    {
        lineSQL->clear();
        sdebug(".\nAvailable filters: sql text\n");
        return 0;
    }
    if(query.startsWith("enable"))
    {
        lineSQL->clear();
        if(query == "enable sql")
        {
            pushSql->setChecked(true);
            sdebug(".\nSQL output is enabled.");
            return 0;
        }
        if(query == "enable text")
        {
            pushText->setChecked(true);
            sdebug(".\nTEXT output is enabled.");
            return 0;
        }
        sdebug(".\nUnknown or missing filter name. Exec \"filters\" command!\n");
        return 0;
    }
    if(query.startsWith("disable"))
    {
        lineSQL->clear();
        if(query == "disable sql")
        {
            pushSql->setChecked(false);
            sdebug(".\nSQL output is disabled.");
            return 0;
        }
        if(query == "disable text")
        {
            pushText->setChecked(false);
            sdebug(".\nTEXT output is disabled.");
            return 0;
        }
        sdebug(".\nUnknown or missing filter name. Exec \"filters\" command!\n");
        return 0;
    }
    if(query.startsWith("state"))
    {
        lineSQL->clear();
        if(query == "state sql")
        {
            sdebug(QString(".\nSQL output is %1.").arg(pushSql->isChecked() ? "enabled" : "disabled"));
            return 0;
        }
        if(query == "state text")
        {
            sdebug(QString(".\nTEXT output is %1.").arg(pushText->isChecked() ? "enabled" : "disabled"));
            return 0;
        }
        sdebug(".\nUnknown or missing filter name. Exec \"filters\" command!\n");
        return 0;
    }

    if(query == "pre")
    {
        lineSQL->setText(pre);
        return 0;
    }
    //end

    cn=0;
    pre = query;
    lineSQL->clear();
    dtext.append("\nExec Console sumbitted query...\n");
    dtext.append("\""+query+"\"\n");

    QSqlDatabase db;
    if(databasename.isEmpty())
        db = QSqlDatabase::database();
    else
        db = QSqlDatabase::database(databasename);

    db.transaction();
    QSqlQuery q(db);
    q.exec(query);

    if(q.lastError().type() == QSqlError::NoError)
    {
        dtext.append("Succesfull executed. Dumping data:\n");

        while(q.next())
        {
            if(!q.record().isEmpty())
            {
                cn = q.record().count();
                dtext.append("\n");
                for(i=0;i<cn;++i)
                {
                    v = q.value(i);
                    if(v.isValid())
                    {
                        if(i != 0)
                            dtext.append(" | ");
                        dtext.append(v.toString());
                    }
                }
            }
        }
        dtext.append(QString("\n\n (%1 rows affected/%2 size)").arg(q.numRowsAffected()).arg(q.size()));
        dtext.append("\n End query: \""+query+"\"");
        db.commit();
    }
    else
    {
        dtext.append(QString("\nError: %1").arg(q.lastError().text()));
        db.rollback();
    }

    debugtxt->setPlainText(dtext);
    debugtxt->moveCursor(QTextCursor::End);
    return 0;
}

int HDebugConsole::checkIfIClose(void)
{
    bool found=false;
    QList<QWidget *>::const_iterator i = QApplication::topLevelWidgets().begin();
    while(i != QApplication::topLevelWidgets().end())
    {
        if (*i != this && (*i)->isVisible())
            found = true;
        ++i;
    }
    if(!found)
        close();
    return 0;
}

#endif //GSAFE_DISABLE_DEBUG

//end code
