/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2006-2013 Peter Deak  (hyper80@gmail.com)

    License: GPLv2  http://www.gnu.org/licenses/gpl-2.0.html

    dconsole.cpp
*/

#ifndef GSAFE_DISABLE_DEBUG
#include <QtGui>

#include <QtCore>

#ifndef DCONSOLE_NO_SQL
#include <QtSql>
#endif // DCONSOLE_NO_SQL

#ifndef COMPILED_WITH_QT4X
#include <QtWidgets>
#endif //COMPILED_WITH_QT4X

#endif //GSAFE_DISABLE_DEBUG

#include "dconsole.h"

//#define FILE_DEBUG 1

#ifndef GSAFE_DISABLE_DEBUG
HDebugConsole * HDebugConsole::myself = NULL;
#endif //GSAFE_DISABLE_DEBUG

#ifndef DCONSOLE_NO_SQL
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
#endif // DCONSOLE_NO_SQL

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
    setWindowTitle("HDebugConsole");
    databasename = "";
    myself = this;

    QVBoxLayout *qvbl = new QVBoxLayout(this);
    QHBoxLayout *qhbl = new QHBoxLayout(this);
    cf = new HConsolePanel(this);

#ifndef DCONSOLE_NO_SQL
    pushSql = new QPushButton(tr("Sql"),this);
    pushSql->setCheckable(true);
    pushSql->setChecked(true);
    qhbl->addWidget(pushSql);
#endif // DCONSOLE_NO_SQL
    pushText = new QPushButton(tr("Text"),this);
    pushText->setCheckable(true);
    pushText->setChecked(true);
    pushSyncwrite = new QPushButton(tr("SyncWrite:\"syndebug.txt\""),this);
    pushSyncwrite->setCheckable(true);
    pushSyncwrite->setChecked(true);
    pushClear = new QPushButton(tr("Clear"),this);
    qhbl->addWidget(pushText);
    qhbl->addWidget(pushSyncwrite);
    qhbl->addStretch();
    qhbl->addWidget(pushClear);

    qvbl->setMargin(0);
    qvbl->setSpacing(0);
    qvbl->addLayout(qhbl);
    qvbl->addWidget(cf);

    connect(pushClear,SIGNAL(clicked()),cf,SLOT(clearText()));
    connect(cf,SIGNAL(commandEntered(QString)),this,SLOT(execCommand(QString)));
    cf->setTextTypeColor(1,QColor(255,150,150));
    cf->setTextTypeColor(2,QColor(200,200,200));
    cf->addText("START");

    resize(660,420);
    #ifdef FILE_DEBUG
    pushSyncwrite->setChecked(true);
    #endif
}

HDebugConsole::~HDebugConsole(void)
{
    myself = NULL;
}

void HDebugConsole::popup(QString title,QString str)
{
    QMessageBox::warning(NULL,title,str);
}

//#define FILE_DEBUG
void HDebugConsole::add_text(QString s,int type)
{
    if(pushSyncwrite->isChecked())
    {
        FILE *dbgf;

        dbgf = fopen("syndebug.txt","a");
        fseek(dbgf,0,SEEK_END);
        fprintf(dbgf,"\n%s\n",s.toLocal8Bit().constData());
        fclose(dbgf);
    }

#ifndef DCONSOLE_NO_SQL
    if(pushSql->isChecked() && type == 0)
    {
        cf->addText(s,1);
    }
#endif // DCONSOLE_NO_SQL

    if(pushText->isChecked() && type == 1)
    {
        cf->addText(s,0);
    }

    QApplication::processEvents();
}

void HDebugConsole::closeEvent(QCloseEvent *e)
{
    myself = NULL;
    QWidget::closeEvent(e);
}

#ifndef DCONSOLE_NO_SQL
void HDebugConsole::debug_sql(QString s)
{
    if(myself != NULL)
        myself->add_text(s,0);
}
#endif // DCONSOLE_NO_SQL

void HDebugConsole::debug_txt(QString s)
{
    if(myself != NULL)
        myself->add_text(s,1);
}

int HDebugConsole::execCommand(QString query)
{
    if(query == "")
    {
        cf->addText(".");
        return 0;
    }
    if(query == "help")
    {
        cf->addText("");
        cf->addText("exit - Exit main program (The debugged program too)");
        cf->addText("close - Close the debug window (only)");
        cf->addText("clear - Clear the debug window");
#ifndef DCONSOLE_NO_SQL
        cf->addText("alldb - Show all available database connections");
        cf->addText("dbinfo - Show the current connected database information");
        cf->addText("setdb - Set the current database to the default (not the program but console)");
        cf->addText("setdb name - Set the current database to \"name\" (not the program but console)");
#endif // DCONSOLE_NO_SQL
        cf->addText("filters - Show available debug filters");
        cf->addText("state <filter> - Show the state of the specified filter");
        cf->addText("enable <filter> - Enable the specified kind of output");
        cf->addText("disable <filter> - Disable the specified kind of output");
        cf->addText("synw - Query the state of syndebug");
        cf->addText("synw [off|on] - Set the state of syndebug");
        cf->addText("synw clear - Clear the file syndebug.txt");
        cf->addText("write <text> - Write the text to the console");
        cf->addText("save - Save the content of debug window to debug.txt");
        cf->addText("run <command> - Run (custom) a program command");
#ifndef DCONSOLE_NO_SQL
        cf->addText("\"SQL\" - Execute the SQL command");
#endif // DCONSOLE_NO_SQL
        if(user_commands.size() > 0)
        {
            cf->addText("  -- CUSTOM/PROGRAM COMMANDS --  ");
            QMultiMap<QString,HDConsoleCommandHolder *>::const_iterator i = user_commands.constBegin();
            while(i != user_commands.constEnd())
            {
                cf->addText(QString(" run %1%2%3")
                            .arg(i.key())
                            .arg( user_commands_descr.value(i.key()) == "" ? "" : " - ")
                            .arg(user_commands_descr.value(i.key())));
                ++i;
            }
        }
        cf->addText("\n");
        return 0;
    }
    if(query == "exit")
        QApplication::exit(0);
    if(query == "close")
        close();
    if(query == "clear")
    {
        cf->clearText();
        return 0;
    }
    if(query == "save")
    {
        FILE *dbgf;

        dbgf = fopen("debug.txt","w");
        fseek(dbgf,0,SEEK_SET);
        cf->selectAll();
        fprintf(dbgf,"\n%s\n",cf->selectedText().toLocal8Bit().constData());
        cf->clearSelection();
        fclose(dbgf);
        cf->addText("Saving console text to debug.txt is done.\n");
        return 0;
    }

#ifndef DCONSOLE_NO_SQL
    if(query.startsWith("dbinfo"))
    {
        QSqlDatabase db;
        if(databasename.isEmpty())
            db = QSqlDatabase::database();
        else
            db = QSqlDatabase::database(databasename);
        if(db.isOpen())
        {
            cf->addText(QString(".\nCurrent OPENED database connection is:\nConnection name=%1\nDriver=%2\nName=%3\nHost=%4\nUser=%5\n")
                .arg(db.connectionName())
                .arg(db.driverName())
                .arg(db.databaseName())
                .arg(db.hostName())
                .arg(db.userName()));
            return 0;
        }
        else
        {
            cf->addText(QString(".\nThere is no opened database!\n"));
            return 0;
        }

        return 0;
    }
    if(query == "alldb")
    {
        QStringList l = QSqlDatabase::connectionNames();
        QStringList::iterator i=l.begin();
        cf->addText(QString(".\nAvailable databases (%1) :").arg(l.count()));
        while(i != l.end())
        {
            cf->addText("  " + *i);
            ++i;
        }
        return 0;
    }
    if(query.startsWith("setdb"))
    {
        QString c;

        c = query.mid(6);
        if(c.isEmpty())
            c=""; //default will be used
        cf->addText(QString("Setting database to default (in console) \"%1\"...").arg(c));
        databasename = c;
        cf->addText("");
        return 0;
    }
#endif // DCONSOLE_NO_SQL

    if(query == "synw")
    {
        cf->addText(QString(".\nSyncronized backup file writing: %1.\n").arg(pushSyncwrite->isChecked() ? "on" : "off"));
        return 0;
    }
    if(query == "synw on")
    {
        pushSyncwrite->setChecked(true);
        cf->addText(QString(".\nEnabled syncronized backup file writing (syndebug.txt).\n"));
        return 0;
    }
    if(query == "synw off")
    {
        cf->addText(QString(".\nDisabled syncronized backup file writing.\n"));
        pushSyncwrite->setChecked(false);
        return 0;
    }
    if(query == "synw clear")
    {
        FILE *dbgf;
        dbgf = fopen("syndebug.txt","w");
        fseek(dbgf,0,SEEK_SET);
        fprintf(dbgf,"\n");
        fclose(dbgf);
        cf->addText(QString(".\nThe content of syndebug.txt has been cleared.\n"));
        return 0;
    }
    if(query.startsWith("write"))
    {
        sdebug(query.mid(6));
        return 0;
    }
    if(query.startsWith("run"))
    {

        QString c;
        QMultiMap<QString,HDConsoleCommandHolder *>::iterator i;

        c = query.mid(4);
        cf->addText(QString("Starting command \"%1\"...").arg(c));
        i = user_commands.find(c);
        if(i == user_commands.end())
        {
            cf->addText(" Error: Command not found!");
        }
        else
        {
            HDConsoleCommandHolder *dcch = *i;
            sdebug(dcch->donsole_command_interpreter(c));
            cf->addText("finished.\n");
        }
        cf->addText("");
        return 0;
    }

    if(query.startsWith("filters"))
    {
#ifndef DCONSOLE_NO_SQL
        cf->addText(".\nAvailable filters: sql text\n");
#else
        cf->addText(".\nAvailable filters: text\n");
#endif // DCONSOLE_NO_SQL
        return 0;
    }
    if(query.startsWith("enable"))
    {
#ifndef DCONSOLE_NO_SQL
        if(query == "enable sql")
        {
            pushSql->setChecked(true);
            cf->addText(".\nSQL output is enabled.");
            return 0;
        }
#endif // DCONSOLE_NO_SQL
        if(query == "enable text")
        {
            pushText->setChecked(true);
            cf->addText(".\nTEXT output is enabled.");
            return 0;
        }
        cf->addText(".\nUnknown or missing filter name. Exec \"filters\" command!\n");
        return 0;
    }
    if(query.startsWith("disable"))
    {
#ifndef DCONSOLE_NO_SQL
        if(query == "disable sql")
        {
            pushSql->setChecked(false);
            cf->addText(".\nSQL output is disabled.");
            return 0;
        }
#endif // DCONSOLE_NO_SQL
        if(query == "disable text")
        {
            pushText->setChecked(false);
            cf->addText(".\nTEXT output is disabled.");
            return 0;
        }
        cf->addText(".\nUnknown or missing filter name. Exec \"filters\" command!\n");
        return 0;
    }
    if(query.startsWith("state"))
    {
#ifndef DCONSOLE_NO_SQL
        if(query == "state sql")
        {
            cf->addText(QString(".\nSQL output is %1.").arg(pushSql->isChecked() ? "enabled" : "disabled"));
            return 0;
        }
#endif // DCONSOLE_NO_SQL
        if(query == "state text")
        {
            cf->addText(QString(".\nTEXT output is %1.").arg(pushText->isChecked() ? "enabled" : "disabled"));
            return 0;
        }
        cf->addText(".\nUnknown or missing filter name. Exec \"filters\" command!\n");
        return 0;
    }

    //end

#ifndef DCONSOLE_NO_SQL
    int i,cn;
    QVariant v;

    cn=0;
    cf->addText("\nExec Console sumbitted query...\n");
    cf->addText("\""+query+"\"\n",1);

    QSqlDatabase db;
    if(databasename.isEmpty())
        db = QSqlDatabase::database();
    else
        db = QSqlDatabase::database(databasename);

    db.transaction();
    QSqlQuery q(db);
    q.exec(query);
    QString result;

    if(q.lastError().type() == QSqlError::NoError)
    {
        cf->addText("Succesfull executed. Dumping data:\n");

        while(q.next())
        {
            if(!q.record().isEmpty())
            {
                cn = q.record().count();
                result.append("\n");
                for(i=0;i<cn;++i)
                {
                    v = q.value(i);
                    if(v.isValid())
                    {
                        if(i != 0)
                            result.append(" | ");
                        result.append(v.toString());
                    }
                }
            }
        }
        cf->addText(result,2);
        cf->addText(QString("\n\n (%1 rows affected/%2 size)").arg(q.numRowsAffected()).arg(q.size()));
        cf->addText("\n End query: \""+query+"\"");
        db.commit();
    }
    else
    {
        cf->addText(QString("\nError: %1").arg(q.lastError().text()));
        db.rollback();
    }
    #else
    cf->addText(QString("Unknown command: %1").arg(query));
    #endif // DCONSOLE_NO_SQL
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

/* *****************************************************************************
 * HConsolePanel and related classes                                           *
 * *****************************************************************************/
class HConsoleLine //Holds a console line in memory. Only used by HConsoleLine(Private)
{
    friend class HConsolePanel;
    friend class HConsolePanelPrivate;

private:
    HConsoleLine();
    ~HConsoleLine();
    HConsoleLine(QString& l,HConsoleLine *current = NULL);

    QString line;
    char type;
    short int width;
    int serial;
    bool autole;

    HConsoleLine *next;
    HConsoleLine *prev;
};

class HConsolePanelPrivate //pimpl class of HConsolePanel.
{
    friend class HConsolePanel;

private:
    HConsolePanelPrivate(HConsolePanel *ppointer) { pp = ppointer; }

private:
    void fitConsole(void);
    void calcCmdLnTop(void);
    void calcScrollBar();

    int calcStringWidth(QString s);
    inline int calcStringBreakPos(QString s);
    void stringReplaceTabToSpace(QString &t);

    HConsoleLine* lineObjectByDisplayPos(int inLine,char& type);
    void getLCfromXY(int x,int y,int& l,int& c,int& scroll,char& t);

    void addTextPart(QString& t,bool hardend,char type);
    void addLine(QString& t,bool hardend,char type);

    void addCommandLineTextPart(QString t,bool hardend);
    void addCommandLineLine(QString t,bool hardend);
    void setCommandLineLineBySerial(int serial,QString newtext);
    QString commandLineLineBySerial(int serial);
    HConsoleLine* commandLineObjectBySerial(int serial);
    QString cmdLineTextSkipsel(int& new_cursor_pos);

    void paintRows(QPainter *p);
    void paintBar(QPainter *p);

    void hoverHandler(int x,int y);

private:
    HConsolePanel *pp;

    HConsoleLine *first,*top,*viewtop,*last;
    HConsoleLine *cfirst,*ctop,*clast;

    QColor bgColor,selectionColor,fontColor,cmdLineColor,cursorColor,marginColor,marginBgColor;
    QMap<char,QColor> typeColors;
    QList<QString> history;
    QList<QString>::iterator history_it;
    QFontMetrics *fm;
    QString promptStr;
    QString marginStr;

    int oldWindowWidth;
    int marginXl,marginXlfix,marginXr;
    bool fastfix;
    int fontsize;
    int lineCapacity;
    int lineHeight;
    int letterWidth;
    int lineBreakPosition;
    int maxLineLength;
    int maxCharWidth;
    int selectionHeight;
    int lhascdiffhalf;
    int tabstop;
    int addedSpaces;

    int cursorPos[2];
    int origCursorPos2;
    char mousePressedArea;
    bool holdMouseButton;
    int mouseOldX,mouseOldY,mouseOldCursor;

    int sbar_length;
    int sbar_maxlength;
    int sbar_minlength;
    int sbar_begin,sbar_obegin;

    bool selection;
    int selectionRange[6]; //row,col->row,col original-row-col
    bool cselection;
    int cselectionRange[6]; //row,col->row,col original-row-col
};

HConsoleLine::HConsoleLine()
{
    line = "";
    next = NULL;
    prev = NULL;
    type = 0;
    serial = 0;
    autole = true;
}

HConsoleLine::HConsoleLine(QString& l,HConsoleLine *current)
{
    line = l;
    next = NULL;
    prev = current;
    serial = 0;
    autole = true;
    if(prev != NULL)
    {
        prev->next = this;
        serial = prev->serial + 1;
    }
    type = 0;
}

HConsoleLine::~HConsoleLine()
{
}

HConsolePanel::HConsolePanel(QWidget *parent) : QFrame(parent)
{
    QString s;
    setFocusPolicy(Qt::StrongFocus);
    setFocus();


    QFont f = font();
    f.setFamily("Monospace");
    f.setStyleHint(QFont::TypeWriter);
    f.setPixelSize(14);
    f.setBold(true);
    setFont(f);

    setMinimumSize(200,150);
    setCursor(Qt::IBeamCursor);

    p = new HConsolePanelPrivate(this);
    p->fm = new QFontMetrics(font(),this);

    p->fontsize = 14;
    p->bgColor = Qt::black;
    p->selectionColor = Qt::blue;
    p->fontColor = Qt::yellow;
    p->cmdLineColor = Qt::green;
    p->marginColor = QColor(240,240,240);
    p->marginBgColor = Qt::black;
    p->cursorColor = Qt::yellow;

    p->last = p->top = p->first = NULL;
    s = "";
    p->cfirst = new HConsoleLine(s);
    p->ctop = p->clast = p->cfirst;
    p->cursorPos[0] = p->cfirst->serial;
    p->cursorPos[1] = 0;
    p->promptStr = "Console>";
    p->marginStr = "";
    p->marginXl = p->marginXlfix = 5;
    p->marginXr = 10;

    if(QFontInfo(font()).fixedPitch())
        p->fastfix = true;

    p->oldWindowWidth = 0;
    p->sbar_minlength = 10;
    p->fitConsole();

    p->history.clear();
    p->history_it = p->history.end();
    p->mousePressedArea = 0;
    p->tabstop = 8;

    p->holdMouseButton = false;
    p->mouseOldCursor = 0;
    setMouseTracking(true);
}

HConsolePanel::~HConsolePanel(void)
{
    delete p;
}

int HConsolePanelPrivate::calcStringWidth(QString s)
{
    if(fastfix)
        return s.length() * letterWidth;
    return fm->width(s);
}

int HConsolePanelPrivate::calcStringBreakPos(QString s)
{
    if(fastfix)
        return lineBreakPosition;
    int ll = s.length();
    while(fm->width(s.left(ll)) > maxLineLength)
        --ll;
    return ll;
}

void HConsolePanel::setMarginText(QString margin)
{
     p->marginStr = margin;
     p->fitConsole();
}

void HConsolePanelPrivate::fitConsole(void)
{
    QString str;
    int cursorLines = 0;
    QString cursortxt;
    int displayLines = 0;

    cursortxt = pp->commandLineText();

    selection = false;
    cselection = false;
    viewtop = NULL;
    maxCharWidth = fm->maxWidth();
    lineHeight = fm->height();
    letterWidth = fm->averageCharWidth();
    lhascdiffhalf = (int) floor((lineHeight - fm->ascent()) / 2);
    selectionHeight = fm->ascent() + lhascdiffhalf;
    lineCapacity = (int) ((pp->height()-(2+1+fm->descent())) / lineHeight);
    marginXl = marginXlfix + calcStringWidth(marginStr);
    maxLineLength = pp->width() - (4 + marginXr + marginXl);
    lineBreakPosition = (int)floor( maxLineLength / letterWidth );
    mousePressedArea = 0;
    sbar_maxlength = pp->height() - 12;

    //Do automation line break/merege if the horisontal space changed
    if(oldWindowWidth != pp->width())
    {
        int newSerial = 0;
        int textwidth = 0;
        QString textline = "";
        HConsoleLine *rbegin,*rend,*r=first;

        oldWindowWidth = pp->width();
        while(r != NULL)
        {
            bool merged_basline,splitted;
            textline = r->line;
            textwidth = r->width;
            rbegin = r;

            //Merged the autowrapped rows into textline
            merged_basline = false;
            while(r->autole && r->next != NULL)
            {
                r = r->next;
                textline += r->line;
                textwidth += r->width;
                merged_basline = true;
            }  //r on the last line which added to the textline

            //Split line if too long for the current window width
            splitted = false;
            while(!textline.isEmpty() && textwidth > maxLineLength)
            {
                int ll = calcStringBreakPos(textline);

                str = textline.left(ll);
                HConsoleLine *rnew = new HConsoleLine(str);
                rnew->width = calcStringWidth(rnew->line);
                rnew->type = rbegin->type;
                rnew->autole = true;
                rnew->serial = newSerial++;

                rnew->prev = rbegin->prev;
                rnew->next = rbegin;
                if(rbegin->prev != NULL)
                    rbegin->prev->next = rnew;
                else
                    first = rnew;
                rbegin->prev = rnew;

                textline = textline.mid(ll,-1);
                textwidth = calcStringWidth(textline);
                splitted = true;
            }

            if(splitted || merged_basline)
            {
                rbegin->line   = textline;
                rbegin->width  = textwidth;
                rbegin->autole = false;
                rbegin->serial = newSerial++;
            }

            if(merged_basline) //Delete old lineparts
            {
                rbegin->next = r->next; //skip from chain
                if(r->next != NULL)
                    r->next->prev = rbegin;

                rend = r;
                r = r->next; //Set r for the next cycle because the current r will be deleted (it was linepart)
                while(rend != NULL && rend != rbegin) //free from memory
                {
                    HConsoleLine *delme;
                    delme = rend;
                    rend = rend->prev;
                    delete delme;
                }
                last = rbegin; //This is the last examined permanent row
                continue;
            }

            if(!splitted)
                r->serial = newSerial++;
            last = r; //This is the last examined row
            r = r->next;
        }
    }

    pp->setCommandLineText(cursortxt,true);
    cursorLines = clast->serial + 1;
    top = last;
    if(top != NULL)
    {
        while(top->prev != NULL && displayLines < (lineCapacity-(cursorLines+1)))
        {
            top = top->prev;
            ++displayLines;
        }
        ++displayLines; //because the upper cycle doesn't count the *top line.
    }

    calcScrollBar();
    calcCmdLnTop();
}

void HConsolePanelPrivate::calcScrollBar()
{
    if(last == NULL || last->serial + 1 < lineCapacity)
    {
        sbar_begin = 0;
        sbar_length = sbar_maxlength;
    }
    else
    {
        sbar_length = int((double(lineCapacity) / double(last->serial + 1)) * double(sbar_maxlength));
        if(sbar_length < sbar_minlength)
            sbar_length = sbar_minlength;
        sbar_begin = int((double((viewtop != NULL ? viewtop->serial : top->serial)) / double(top->serial)) * double(sbar_maxlength-sbar_length));
    }
}

void HConsolePanelPrivate::calcCmdLnTop(void)
{
    HConsoleLine *cr = clast;
    int ll = 0;
    int displayLines = 0;
    if(last != NULL)
        displayLines = last->serial - top->serial + 1;
    while(cr->prev != NULL)
    {
        if(displayLines + ll++ >= lineCapacity && cursorPos[0] >= cr->serial)
            break;
        cr = cr->prev;
    }
    ctop = cr;
}

void HConsolePanel::paintEvent(QPaintEvent *e)
{
    QPainter *painter = new QPainter(this);
    p->paintRows(painter);
    p->paintBar(painter);
    painter->end();
    delete painter;
}

void HConsolePanelPrivate::paintRows(QPainter *p)
{
    int x=0,y=2;
    char cmdline;
    int l;
    bool mSelSign;
    int *mSelRange;

    p->fillRect(0,0,pp->width(),pp->height(),bgColor);
    x = marginXl;
    y += lineHeight;

    HConsoleLine* r;
    l = 0;
    for(cmdline = 0;cmdline < 2;++cmdline)
    {
        if(!cmdline)
        {
            r = viewtop == NULL ? top : viewtop;
            mSelSign = selection;
            mSelRange = selectionRange;
        }
        else
        {
            r = ctop;
            mSelSign = cselection;
            mSelRange = cselectionRange;
        }

        if(cmdline || viewtop != NULL || lineCapacity > clast->serial + 1)
            while(r != NULL)
            {
                if(mSelSign && mSelRange[0] == r->serial && mSelRange[2] == r->serial) //selection begin-end
                {
                    int ss,ee;
                    ss=mSelRange[1];
                    if(ss < 0)
                        ss = 0;
                    ee = mSelRange[3];
                    if(ee > r->line.length())
                        ee = r->line.length();

                    p->fillRect(x+calcStringWidth((cmdline && r == cfirst ? promptStr : QString()) + r->line.left(ss)),
                                y-selectionHeight,
                                calcStringWidth(r->line.left(ee).mid(ss,-1)),
                                selectionHeight+lhascdiffhalf,
                                selectionColor);

                }
                else if(mSelSign && mSelRange[0] == r->serial) //selection begin
                {
                    int ss = mSelRange[1];
                    if(ss < 0)
                        ss = 0;

                    p->fillRect(x+calcStringWidth((cmdline && r == cfirst ? promptStr : QString()) + r->line.left(ss)),
                                y-selectionHeight,
                                calcStringWidth(r->line) - calcStringWidth((r->line).left(ss)),
                                selectionHeight+lhascdiffhalf,selectionColor);
                }
                else if(mSelSign && mSelRange[0] < r->serial && mSelRange[2] > r->serial) //selection in
                {
                    p->fillRect(x,y-selectionHeight,
                                calcStringWidth(r->line),selectionHeight+lhascdiffhalf,selectionColor);
                }
                else if(mSelSign && mSelRange[2] == r->serial) //selection end
                {
                    int ee = mSelRange[3];
                    if(ee > r->line.length())
                        ee = r->line.length();

                    p->fillRect(x,y-selectionHeight,
                                calcStringWidth(r->line.left(ee)),selectionHeight+lhascdiffhalf,selectionColor);
                }

                p->setPen(marginColor);
                if(!cmdline)
                {
                    if(!marginStr.isEmpty())
                    {
                        QString s = marginStr;
                        s.replace(QRegExp("%[0]*s"),QString("%1").arg(r->serial+1));
                        p->fillRect(0,y-selectionHeight,marginXl-marginXlfix,lineHeight+lhascdiffhalf,marginBgColor);
                        p->drawText(0,y,s);
                    }
                    if(r->type == 0)
                        p->setPen(fontColor);
                    else
                        p->setPen(typeColors.find(r->type) == typeColors.end() ? fontColor : typeColors[r->type] );
                    p->drawText(x,y,r->line);
                }
                else
                {
                    if(cursorPos[0] == r->serial && pp->hasFocus())
                    {
                        int xpos;
                        p->setPen(cursorColor);
                        xpos = x+calcStringWidth(r->line.left(cursorPos[1]))+calcStringWidth(r == cfirst ? promptStr : QString(""));
                        p->drawLine(xpos,y-selectionHeight,xpos,y+lhascdiffhalf);
                        p->drawLine(xpos+1,y-selectionHeight,xpos+1,y+lhascdiffhalf);
                    }
                    p->setPen(cmdLineColor);
                    p->drawText(x,y,(r == cfirst ? promptStr : QString("")) + r->line);
                }

                y += lineHeight;
                r = r->next;

                if(++l >= lineCapacity)
                    return;
            }
    }
}

void HConsolePanelPrivate::paintBar(QPainter *p)
{
    p->setPen(QColor(150,150,150));
    p->setBrush(QBrush(QColor(50,50,50),Qt::SolidPattern));
    p->drawRoundedRect(marginXl+maxLineLength+2,4,marginXr,pp->height()-8,5,5);
    p->setBrush(QBrush(QColor(150,150,150),Qt::SolidPattern));
    p->drawRoundedRect(marginXl+maxLineLength+4,6+sbar_begin,marginXr-4,sbar_length,3,3);
    p->setBrush(Qt::NoBrush);
}

void HConsolePanel::setColor(QString section,QColor color)
{
    if(section == "background")
        p->bgColor = color;
    if(section == "selection")
        p->selectionColor = color;
    if(section == "standardtext")
        p->fontColor = color;
    if(section == "cmdtext")
        p->cmdLineColor = color;
    if(section == "margin")
        p->marginColor = color;
    if(section == "cursor")
        p->cursorColor = color;
    if(section == "marginbg")
        p->marginBgColor = color;
}

void HConsolePanel::setTextTypeColor(char type,QColor color)
{
    p->typeColors[type] = color;
}

void HConsolePanel::addNormalText(QString text)
{
    addText(text);
}

void HConsolePanel::setTabStop(int count)
{
    p->tabstop = count;
}

void HConsolePanel::addText(QString t,char type)
{
    p->addedSpaces = 0;
    if(t.contains("\n"))
    {
        QStringList lines = t.split("\n");
        QList<QString>::iterator i=lines.begin();
        for(;i!=lines.end();++i)
            p->addTextPart(*i,true,type);
    }
    else
        p->addTextPart(t,true,type);
    p->calcScrollBar();
    update();
}

void HConsolePanelPrivate::stringReplaceTabToSpace(QString &t)
{
    int fl,tp;
    while((tp = t.indexOf("\t")) != -1)
    {
        fl = int( ceil(double(tp)/double(tabstop)) * tabstop) - tp;
        if(fl == 0)
            fl = tabstop;
        addedSpaces += fl - 1;
        t.replace(tp,1,QString().fill(' ',fl));
    }
}

void HConsolePanelPrivate::addTextPart(QString& t,bool hardend,char type)
{
    stringReplaceTabToSpace(t);
    if(maxLineLength < calcStringWidth(t))
    {
        QString fitted,leaved;
        int s = calcStringBreakPos(t);
        fitted = t.left(s);
        leaved = t.mid(s,-1);
        addLine(fitted,false,type);
        addTextPart(leaved,hardend,type);
        return;
    }
    addLine(t,hardend,type);
}

void HConsolePanelPrivate::addLine(QString& t,bool hardend,char type)
{
    viewtop = NULL;
    last = new HConsoleLine(t,last);
    if(first == NULL)
        first = top = last;
    last->width = calcStringWidth(t);
    last->type = type;
    if(hardend)
        last->autole = false;

    if(last->serial - top->serial + 1 >= lineCapacity)
        top = top->next;
}

void HConsolePanel::clearText(void)
{
    HConsoleLine *d,*r = p->first;
    while(r != NULL)
    {
        d = r;
        r = r->next;
        delete d;
    }
    p->top = p->first = p->last = NULL;
    p->fitConsole();
    update();
}

void HConsolePanel::resizeEvent(QResizeEvent *e)
{
    p->fitConsole();
    QFrame::resizeEvent(e);
}

void HConsolePanel::scrollUp(int lineCount)
{
    for(;lineCount > 0;--lineCount)
    {
        if(p->viewtop == NULL)
        {
            if(p->top != NULL)
                p->viewtop = p->top->prev;
        }
        else
        {
            if(p->viewtop->prev != NULL)
                p->viewtop = p->viewtop->prev;
            if(p->viewtop == p->top)
                p->viewtop = NULL;
        }
    }
    p->calcScrollBar();
    update();
}

void HConsolePanel::scrollDown(int lineCount)
{
    for(;lineCount > 0;--lineCount)
    {
        if(p->viewtop != NULL)
        {
            if(p->viewtop->next == p->top)
                p->viewtop = NULL;
            else
                p->viewtop = p->viewtop->next;
            if(p->viewtop == p->top)
                p->viewtop = NULL;
        }
    }
    p->calcScrollBar();
    update();
}

void HConsolePanel::scrollTop(void)
{
    p->viewtop = p->first;
    p->calcScrollBar();
    update();
}

void HConsolePanel::scrollReset(void)
{
    p->viewtop = NULL;
    p->calcScrollBar();
    update();
}

void HConsolePanel::scrollToSerial(int serial)
{
    p->viewtop = NULL;
    HConsoleLine *r = p->first;
    while(r != NULL && r->serial < p->top->serial)
    {
        if(r->serial == serial)
        {
            p->viewtop = r;
            break;
        }
        r = r->next;
    }
    p->calcScrollBar();
    update();
}

int HConsolePanel::maxSerial(void)
{
    if(p->last == NULL)
        return -1;
    return p->last->serial;
}

int HConsolePanel::topSerial(void)
{
    if(p->top == NULL)
        return -1;
    return p->viewtop == NULL ? p->top->serial : p->viewtop->serial;
}

QString HConsolePanel::lineBySerial(int serial,bool onDisplay)
{
    HConsoleLine* r = p->first;
    if(onDisplay)
        r = p->viewtop == NULL ? p->top : p->viewtop;
    if(serial == -1)
        return QString();
    while(r != NULL)
    {
        if(r->serial == serial)
            return r->line;
        r = r->next;
    }
    return QString();
}

void HConsolePanel::selectAll(void)
{
    if(p->first == NULL)
        return;
    p->selectionRange[0] = p->first->serial;
    p->selectionRange[1] = 0;
    p->selectionRange[2] = p->last->serial;
    p->selectionRange[3] = p->last->line.length();
    p->selection = true;
    p->cselection = false;
    update();
}

void HConsolePanel::clearSelection(void)
{
    p->selection = false;
    p->cselection = false;
}

QString HConsolePanel::selectedText(void)
{
    if(!p->selection && !p->cselection)
        return QString();

    int *mSelRange;
    QString sel="";
    HConsoleLine* r;

    if(p->cselection)
    {
        mSelRange = p->cselectionRange;
        r = p->cfirst;
    }
    if(p->selection)
    {
        mSelRange = p->selectionRange;
        r = p->first;
    }

    while(r != NULL)
    {
        if(mSelRange[0] == r->serial && mSelRange[2] == r->serial) //selection begin-end
        {
            int ss,ee;
            ss=mSelRange[1];
            if(ss < 0)
                ss = 0;
            ee = mSelRange[3];
            if(ee > r->line.length())
                ee = r->line.length();

            return r->line.left(ee).mid(ss,-1);
        }
        else if(mSelRange[0] == r->serial) //selection begin
        {
            int ss = mSelRange[1];
            if(ss < 0)
                ss = 0;
            sel += r->line.mid(ss,-1) + (r->autole ? "" : "\n");

        }
        else if(mSelRange[0] < r->serial && mSelRange[2] > r->serial) //selection in
        {
            sel += r->line + (r->autole ? "" : "\n");
        }
        else if(mSelRange[2] == r->serial) //selection end
        {
            int ee = mSelRange[3];
            if(ee > r->line.length())
                ee = r->line.length();

            sel += r->line.left(ee);
            return sel;
        }
        r = r->next;
    }
    return sel;
}

void HConsolePanel::copySelection(void)
{
    if(!p->selection && !p->cselection)
        return;
    QClipboard * cb = QApplication::clipboard();
    cb->setText(selectedText());
}

HConsoleLine* HConsolePanelPrivate::lineObjectByDisplayPos(int inLine,char& type)
{
    HConsoleLine* r = viewtop == NULL ? top : viewtop;
    type = 1;
    int ll = 0;

    if(viewtop != NULL || lineCapacity > clast->serial + 1)
        while(r != NULL)
        {
            if(ll == inLine)
                return r;
            r = r->next;
            ++ll;
        }
    type = 2;
    r = ctop;
    while(r != NULL)
    {
        if(ll == inLine)
            return r;
        r = r->next;
        ++ll;
    }
    type = 0;
    return NULL;
}

void HConsolePanelPrivate::getLCfromXY(int x,int y,int& l,int& c,int& scroll,char& t)
{
    int displayLines = 0;
    int dl,sl;
    HConsoleLine *lineObj;

    if(last != NULL)
         displayLines = last->serial - ( viewtop != NULL ? viewtop->serial : top->serial ) + 1;
    if(displayLines > lineCapacity)
        displayLines = lineCapacity;

    scroll = 0;
    if(x<0)
        x = 0;
    if(t == 0 && x > marginXl + maxLineLength )
    {
        t = 0;
        l = 0; c = 0;
        scroll = 0;
        return;
    }
    if(y<0)
    {
        y = 0;
        if(top->serial > 0)
            scroll = -1;
    }

    dl = (int) floor((y-2)/lineHeight);
    if(t == 1 && dl>displayLines-1)
    {
        dl = displayLines-1;
        if(viewtop != NULL && viewtop->serial < top->serial)
            scroll = 1;
    }
    if((t == 0 || t == 2) && dl > displayLines+clast->serial)
    {
        dl = displayLines+clast->serial;
        if(cfirst != ctop)
            scroll = 1;
    }
    lineObj = lineObjectByDisplayPos(dl,t);
    if(lineObj == NULL)
    {
        l = 0; c = 0; t = 0;
        return;
    }
    l = lineObj->serial;
    if(lineObj->line.isEmpty())
    {
        c = 0;
        return;
    }
    QString prefix="";
    if(lineObj == cfirst)
        prefix = promptStr;
    sl = lineObj->line.length();
    for( c=1 ; calcStringWidth(prefix + lineObj->line.left(c)) - calcStringWidth(lineObj->line.at(c-1))/2 < x - marginXl ; ++c )
        if(sl <= c)
        {
            c = sl;
            return;
        }
    --c; //because the selection ponint placewd _before_ the numberred character.
    return;
}

void HConsolePanel::mousePressEvent(QMouseEvent *e)
{
    if(e->button() != Qt::LeftButton)
        return;

    int l,c,scroll;
    char type;
    type = 0;
    p->getLCfromXY(e->x(),e->y(),l,c,scroll,type);
    p->mousePressedArea = type;
    p->holdMouseButton = true;
    if(type == 0)
    {
        p->mouseOldX = e->x();
        p->mouseOldY = e->y();
        p->sbar_obegin = p->sbar_begin;
        return;
    }
    if(type == 2 && scroll == 0)
    {
        p->cursorPos[0] = l;
        p->cursorPos[1] = c;
        p->origCursorPos2 = p->cursorPos[1];
    }
    if(type == 1)
    {
        p->selectionRange[4] = p->selectionRange[0] = l; //fromRow
        p->selectionRange[5] = p->selectionRange[1] = c; //fromRow
    }
    if(type == 2)
    {
        p->cselectionRange[4] = p->cselectionRange[0] = l; //fromRow
        p->cselectionRange[5] = p->cselectionRange[1] = c; //fromRow
    }
    p->selection = false;
    p->cselection = false;
    update();
}

void HConsolePanel::mouseReleaseEvent(QMouseEvent *e)
{
    p->holdMouseButton = false;
    p->mousePressedArea = 0;

    if(e->button() == Qt::RightButton)
    {
        QMenu *m=new QMenu(this);
        m->addAction(tr("Scroll to top"),this,SLOT(scrollTop()));
        m->addAction(tr("Scroll to bottom"),this,SLOT(scrollReset()));
        m->addSeparator();
        m->addAction(tr("Select all"),this,SLOT(selectAll()));
        m->addAction(tr("Clear selection"),this,SLOT(clearSelection()));
        m->addSeparator();
        m->addAction(tr("Copy"),this,SLOT(copySelection()));
        m->addAction(tr("Paste to command line"),this,SLOT(pasteToCommandLine()));
        m->addSeparator();
        m->addAction(tr("Clear all text"),this,SLOT(clearText()));

        m->exec(mapToGlobal(e->pos()));
        delete m;
    }
}

void HConsolePanelPrivate::hoverHandler(int x,int y)
{
    if(x < 0 && x > pp->width())
    {
        mouseOldCursor = 0;
        return;
    }
    if(x > 0 && x < marginXl+maxLineLength && mouseOldCursor != 1)
    {
        pp->setCursor(Qt::IBeamCursor);
        mouseOldCursor = 1;
        return;
    }
    if(x > marginXl+maxLineLength && x < pp->width() && mouseOldCursor != 2)
    {
        pp->setCursor(Qt::PointingHandCursor);
        mouseOldCursor = 2;
        return;
    }
}

void HConsolePanel::mouseMoveEvent(QMouseEvent *e)
{
    if(!p->holdMouseButton)
    {
        p->hoverHandler(e->x(),e->y());
        return;
    }

    int l,c,s;
    char type;
    if(p->mousePressedArea == 0)
    {
        if(p->mouseOldX > p->marginXl + p->maxLineLength + 2 &&
           p->mouseOldX < p->marginXl + p->maxLineLength + 2 + p->marginXr &&
           p->mouseOldY > 2 &&
           p->mouseOldY < height() - 2)
        {
            int nb = p->sbar_obegin + (e->y() - p->mouseOldY);
            if(nb < 0)
                nb = 0;
            if(nb > p->sbar_maxlength)
                nb = p->sbar_maxlength;
            scrollToSerial(int((double(nb)/ double(p->sbar_maxlength-p->sbar_length))*double(p->top->serial)));
        }
        return;
    }

    type = p->mousePressedArea;
    p->getLCfromXY(e->x(),e->y(),l,c,s,type);
    if(s == -1)
    {
        scrollUp(1);
        p->getLCfromXY(e->x(),e->y(),l,c,s,type);
    }
    if(s == 1)
    {
        scrollDown(1);
        p->getLCfromXY(e->x(),e->y(),l,c,s,type);
    }

    if(type == 0)
        return;

    int *mSelRange;
    bool sel;
    if(type == 1)
    {
        sel = p->selection;
        mSelRange = p->selectionRange;
    }
    if(type == 2)
    {
        sel = p->cselection;
        mSelRange = p->cselectionRange;
    }

    if(!sel || mSelRange[2] != l || mSelRange[3] != c)
    {
        if(mSelRange[4] < l || (mSelRange[4] == l && mSelRange[5] < c))
        {
            mSelRange[2] = l;
            mSelRange[3] = c;
            mSelRange[0] = mSelRange[4];
            mSelRange[1] = mSelRange[5];
        }
        else
        {
            mSelRange[0] = l;
            mSelRange[1] = c;
            mSelRange[2] = mSelRange[4];
            mSelRange[3] = mSelRange[5];
        }

        p->selection = type == 1 ? true : false;
        p->cselection = type == 2 ? true : false;
        update();
    }
}

void HConsolePanel::mouseDoubleClickEvent(QMouseEvent *e)
{
    if(e->button() != Qt::LeftButton)
        return;

    int l,c,i,s;
    int *mSelRange;
    char type;
    type = 0;
    p->getLCfromXY(e->x(),e->y(),l,c,s,type);
    if(type == 0 || s != 0)
    {
        p->selection = false;
        p->cselection = false;
        update();
        return;
    }

    QString line;
    if(type == 1)
    {
            line = lineBySerial(l);
            mSelRange = p->selectionRange;
    }
    if(type == 2)
    {
        line = p->commandLineLineBySerial(l);
        mSelRange = p->cselectionRange;
    }

    if(c >= line.length() || line.at(c).isSpace())
    {
        p->selection = false;
        p->cselection = false;
        update();
        return;
    }

    mSelRange[0] = l;
    mSelRange[2] = l;
    mSelRange[1] = 0;

    for(i=c;i>0;--i)
        if(line.at(i).isSpace())
        {
            mSelRange[1] = i+1;
            break;
        }

    mSelRange[3] = line.length();
    for(i=c;i<line.length();++i)
        if(line.at(i).isSpace())
        {
            mSelRange[3] = i;
            break;
        }

    p->selection = type == 1 ? true : false;
    p->cselection = type == 2 ? true : false;
    update();
}

HConsoleLine* HConsolePanelPrivate::commandLineObjectBySerial(int serial)
{
    HConsoleLine* r = cfirst;
    if(serial == -1)
        return NULL;
    while(r != NULL)
    {
        if(r->serial == serial)
            return r;
        r = r->next;
    }
    return NULL;
}

QString HConsolePanelPrivate::commandLineLineBySerial(int serial)
{
    HConsoleLine* r = commandLineObjectBySerial(serial);
    if(r == NULL)
        return QString();
    return r->line;
}

void HConsolePanelPrivate::setCommandLineLineBySerial(int serial,QString newtext)
{
    HConsoleLine* r = cfirst;
    if(serial == -1)
        return;
    while(r != NULL)
    {
        if(r->serial == serial)
        {
            r->line = newtext;
            return;
        }
        r = r->next;
    }
}

void HConsolePanel::resetCommandLine(void)
{
    p->cfirst->line = "";
    HConsoleLine *todel=NULL,*cr = p->cfirst->next;
    while(cr != NULL)
    {
        todel = cr;
        cr = cr->next;
        delete todel;
    }
    p->cfirst->next = NULL; //we just erased above
    p->ctop = p->clast = p->cfirst;
    p->cfirst->autole = true;
    p->cursorPos[0] = p->cfirst->serial;
    p->cursorPos[1] = 0;
    p->origCursorPos2 = p->cursorPos[1];
    p->calcCmdLnTop();
}

void HConsolePanel::setCommandLineText(QString t,bool disableupdate)
{
    resetCommandLine();
    p->viewtop = NULL;
    p->calcScrollBar();
    if(!t.isEmpty())
    {
        if(t.contains("\n"))
        {
            QStringList lines = t.split("\n");
            QList<QString>::iterator i=lines.begin();
            for(;i!=lines.end();++i)
                p->addCommandLineTextPart(*i,true);
        }
        else
            p->addCommandLineTextPart(t,true);
    }
    p->cursorPos[0] = p->clast->serial;
    p->cursorPos[1] = p->clast->line.length();
    p->origCursorPos2 = p->cursorPos[1];
    if(!disableupdate)
    {
        p->fitConsole();
        update();
    }
}

void HConsolePanelPrivate::addCommandLineTextPart(QString t,bool hardend)
{
    stringReplaceTabToSpace(t);
    if(maxLineLength < calcStringWidth( (cfirst == clast ? promptStr : QString()) + t ))
    {
        int s = calcStringBreakPos( (cfirst == clast ? promptStr : QString()) + t );
        s -= (cfirst == clast ? promptStr.length() : 0);
        addCommandLineLine(t.left(s),false);
        addCommandLineTextPart(t.mid(s,-1),hardend);
        return;
    }
    addCommandLineLine(t,hardend);
}

void HConsolePanelPrivate::addCommandLineLine(QString t,bool hardend)
{
    if(cfirst->next == NULL && cfirst == clast && cfirst->line.isEmpty())
    {
        cfirst->line = t;
        cfirst->autole = !hardend;
    }
    else
    {
        clast = new HConsoleLine(t,clast);
        if(hardend)
            clast->autole = false;
    }
}

QString HConsolePanel::commandLineText(void)
{
    QString c = QString();
    HConsoleLine* cr = p->cfirst;
    while(cr != NULL)
    {
        c += cr->line + ((cr->next != NULL && !cr->autole) ? "\n" : "");
        cr = cr->next;
    }
    return c;
}

int HConsolePanel::cursorPosition(void)
{
    int cp=0;
    HConsoleLine* cr = p->cfirst;
    while(cr != NULL)
    {
        if(p->cursorPos[0] > cr->serial)
            cp += ( cr->line.length() + (cr->autole ? 0 : 1) );
        if(p->cursorPos[0] == cr->serial)
            return cp + p->cursorPos[1];
        cr = cr->next;
    }
    return -1;
}

void HConsolePanel::setCursorPosition(int c)
{
    if(c == -1)
        return;
    HConsoleLine* cr = p->cfirst;
    while(cr != NULL)
    {
        if(c > cr->line.length())
            c -= ( cr->line.length() + (cr->autole ? 0 : 1) );
        else
        {
            p->cursorPos[0] = cr->serial;
            p->cursorPos[1] = c;
            p->origCursorPos2 = p->cursorPos[1];
            return;
        }
        cr = cr->next;
    }
    p->calcCmdLnTop();
}

void HConsolePanel::addTextToCursor(QString text)
{
    if(p->cursorPos[0] == p->clast->serial &&
       !text.contains("\n") &&
       !text.contains("\t") &&
       !p->cselection &&
       p->maxLineLength > p->calcStringWidth(
                (p->cursorPos[0] == p->cfirst->serial ? p->promptStr : QString()) +
                    p->clast->line + text )
      )
    {
        //Fast insert/add method (run when the cursor stay in the row)
        if(p->cursorPos[1] == p->clast->line.length())
            p->clast->line.append(text);
        else
            p->clast->line.insert(p->cursorPos[1],text);

        p->clast->width = p->calcStringWidth(p->clast->line);
        p->cursorPos[1] += text.length();
        p->origCursorPos2 = p->cursorPos[1];
        update();
        return;
    }

    int abs;
    QString c;
    abs = cursorPosition();
    c = p->cselection ? p->cmdLineTextSkipsel(abs) : commandLineText();
    c.insert(abs,text);
    p->addedSpaces = 0;
    setCommandLineText(c);
    setCursorPosition(abs+text.length()+p->addedSpaces);
    p->calcCmdLnTop();
    p->cselection = false;
    p->addedSpaces = 0;
    update();
}

void HConsolePanel::deleteFromCursor(bool forwardDel)
{
    if(p->cselection)
    {
        deleteCmdLineSelection();
        return;
    }

    if(p->cursorPos[0] == p->clast->serial)
    {
        if(p->cfirst == p->clast && p->clast->line.isEmpty())
            return;

        if(!forwardDel && p->cursorPos[1] > 0)
        {
            p->clast->line.remove(p->cursorPos[1]-1,1);
            p->cursorPos[1] = p->cursorPos[1] - 1;
            p->origCursorPos2 = p->cursorPos[1];
            update();
            return;
        }
        if(forwardDel)
        {
            if(p->cursorPos[1] >= p->clast->line.length())
                return;
            p->clast->line.remove(p->cursorPos[1],1);
            p->origCursorPos2 = p->cursorPos[1];
            update();
            return;
        }
    }

    int abs;
    QString c;
    c = commandLineText();
    abs = cursorPosition();

    if(!forwardDel && abs > 0)
    {
        --abs;
        c.remove(abs,1);
    }

    if(forwardDel && abs < c.length())
        c.remove(abs,1);

    setCommandLineText(c);
    setCursorPosition(abs);
    p->calcCmdLnTop();
    update();
}

void HConsolePanel::deleteCmdLineSelection(void)
{
    if(!p->cselection)
        return;

    int abs;
    QString c;
    c = p->cmdLineTextSkipsel(abs);
    setCommandLineText(c);
    setCursorPosition(abs);
    p->calcCmdLnTop();
    p->cselection = false;
    update();
}

void HConsolePanel::cursorLeft(int charCount,bool inSelection)
{
    HConsoleLine *l;

    if(!inSelection)
        p->cselection = false;

    if(inSelection && !p->cselection)
    {
        p->cselectionRange[0] = p->cursorPos[0];
        p->cselectionRange[1] = p->cursorPos[1];
        p->cselectionRange[4] = p->cursorPos[0];
        p->cselectionRange[5] = p->cursorPos[1];
    }

    for(;charCount > 0;--charCount)
    {
        if(p->cursorPos[1] > 0)
        {
            --p->cursorPos[1];
        }
        else if(p->cursorPos[0] > p->cfirst->serial)
        {
            --p->cursorPos[0];
            l = p->commandLineObjectBySerial(p->cursorPos[0]);
            p->cursorPos[1] = l == NULL ? 0 : l->line.length();
        }
    }

    p->origCursorPos2 = p->cursorPos[1];

    if(inSelection)
    {
        if(p->cselectionRange[4] < p->cursorPos[0] ||
           (p->cselectionRange[4] == p->cursorPos[0] && p->cselectionRange[5] < p->cursorPos[1]))
        {
            p->cselectionRange[2] = p->cursorPos[0];
            p->cselectionRange[3] = p->cursorPos[1];
            p->cselectionRange[0] = p->cselectionRange[4];
            p->cselectionRange[1] = p->cselectionRange[5];
        }
        else
        {
            p->cselectionRange[0] = p->cursorPos[0];
            p->cselectionRange[1] = p->cursorPos[1];
            p->cselectionRange[2] = p->cselectionRange[4];
            p->cselectionRange[3] = p->cselectionRange[5];
        }
        p->cselection = true;
        p->selection = false;
    }
    p->calcCmdLnTop();
    update();
}

void HConsolePanel::cursorRight(int charCount,bool inSelection)
{
    HConsoleLine *l;

    if(!inSelection)
        p->cselection = false;

    if(inSelection && !p->cselection)
    {
        p->cselectionRange[0] = p->cursorPos[0];
        p->cselectionRange[1] = p->cursorPos[1];
        p->cselectionRange[4] = p->cursorPos[0];
        p->cselectionRange[5] = p->cursorPos[1];
    }

    for(;charCount > 0;--charCount)
    {
        l = p->commandLineObjectBySerial(p->cursorPos[0]);
        if(l == NULL)
            return;
        if(p->cursorPos[0] == p->clast->serial)
        {
            if(p->cursorPos[1] < l->line.length())
            {
                ++p->cursorPos[1];
            }
        }
        else
        {
            if(p->cursorPos[1] < l->line.length())
            {
                ++p->cursorPos[1];
            }
            else
            {
                ++p->cursorPos[0];
                p->cursorPos[1] = 0;
            }
        }
    }

    p->origCursorPos2 = p->cursorPos[1];

    if(inSelection)
    {
        if(p->cselectionRange[4] < p->cursorPos[0] ||
           (p->cselectionRange[4] == p->cursorPos[0] && p->cselectionRange[5] < p->cursorPos[1]))
        {
            p->cselectionRange[2] = p->cursorPos[0];
            p->cselectionRange[3] = p->cursorPos[1];
            p->cselectionRange[0] = p->cselectionRange[4];
            p->cselectionRange[1] = p->cselectionRange[5];
        }
        else
        {
            p->cselectionRange[0] = p->cursorPos[0];
            p->cselectionRange[1] = p->cursorPos[1];
            p->cselectionRange[2] = p->cselectionRange[4];
            p->cselectionRange[3] = p->cselectionRange[5];
        }
        p->cselection = true;
        p->selection = false;
    }
    p->calcCmdLnTop();
    update();
}

QString HConsolePanelPrivate::cmdLineTextSkipsel(int& new_cursor_pos)
{
    if(!cselection)
        return pp->commandLineText();

    bool sb = false;
    QString c="";
    new_cursor_pos = 0;
    HConsoleLine* r = cfirst;
    while(r != NULL)
    {
        if(cselectionRange[0] == r->serial && cselectionRange[2] == r->serial) //selection begin-end
        {
            int ss,ee;
            ss=cselectionRange[1];
            if(ss < 0)
                ss = 0;
            ee = cselectionRange[3];
            if(ee > r->line.length())
                ee = r->line.length();

            c += r->line.left(ss);
            c += r->line.mid(ee,-1) + ((r->next != NULL && !r->autole) ? "\n" : "");
            new_cursor_pos += r->line.left(ss).length();
            sb = true;
        }
        else if(cselectionRange[0] == r->serial) //selection begin
        {
            int ss = cselectionRange[1];
            if(ss < 0)
                ss = 0;
            c += r->line.left(ss);
            new_cursor_pos += r->line.left(ss).length();
            sb = true;
        }
        else if(cselectionRange[0] < r->serial && cselectionRange[2] > r->serial) //selection in
        {
            ;//skipped text
        }
        else if(cselectionRange[2] == r->serial) //selection end
        {
            int ee = cselectionRange[3];
            if(ee > r->line.length())
                ee = r->line.length();

            c += r->line.mid(ee,-1) + ((r->next != NULL && !r->autole) ? "\n" : "");
        }
        else
        {
            if(!sb)
                new_cursor_pos += ( r->line.length() + (r->autole ? 0 : 1) );
            c += r->line + ((r->next != NULL && !r->autole) ? "\n" : "");
        }
        r = r->next;
    }
    return c;
}

void HConsolePanel::pasteToCommandLine(void)
{
    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();

    addTextToCursor(mimeData->text());
}

void HConsolePanel::wheelEvent(QWheelEvent *e)
{
    int l,c,scroll;
    char t = 0;
    if(p->first == NULL)
        return;
    p->getLCfromXY(e->x(),e->y(),l,c,scroll,t);
    if(t == 2 && p->cfirst != p->clast)
    {
        p->viewtop = NULL;
        if(e->delta()>0 && p->cursorPos[0] > p->cfirst->serial)
            p->cursorPos[0] -= 1;
        if(e->delta()<0 && p->cursorPos[0] < p->clast->serial)
            p->cursorPos[0] += 1;

        p->cursorPos[1] = p->origCursorPos2;
        if(p->cursorPos[1] > p->commandLineLineBySerial(p->cursorPos[0]).length())
            p->cursorPos[1] = p->commandLineLineBySerial(p->cursorPos[0]).length();
        p->calcCmdLnTop();
        update();
    }
    else
    {
        if(e->delta()<0)
            scrollDown(e->modifiers() == Qt::ShiftModifier ? 10 : 1);
        if(e->delta()>0)
            scrollUp(e->modifiers() == Qt::ShiftModifier ? 10 : 1);
    }
}

void HConsolePanel::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_PageUp && e->modifiers() == Qt::ShiftModifier)
    {
        scrollUp(5);
        return;
    }
    if(e->key() == Qt::Key_PageDown && e->modifiers() == Qt::ShiftModifier)
    {
        scrollDown(5);
        return;
    }

    if(e->key() == Qt::Key_A && e->modifiers() == Qt::ControlModifier)
    {
        selectAll();
        return;
    }

    if((e->key() == Qt::Key_Insert && e->modifiers() == Qt::ControlModifier) ||
       (e->key() == Qt::Key_C && e->modifiers() == Qt::ControlModifier))
    {
        copySelection();
        return;
    }

    if((e->key() == Qt::Key_Insert && e->modifiers() == Qt::ShiftModifier) ||
       (e->key() == Qt::Key_V && e->modifiers() == Qt::ControlModifier))
    {
        pasteToCommandLine();
        return;
    }

    if(e->key() == Qt::Key_Home)
    {
        if(p->cfirst == p->clast && p->cfirst->line.isEmpty() && p->first != p->top)
        {
            p->viewtop = p->first;
            p->calcScrollBar();
            update();
            return;
        }
        if(e->modifiers() == Qt::ShiftModifier)
        {
            p->cselectionRange[2] = p->cursorPos[0];
            p->cselectionRange[3] = p->cursorPos[1];
            p->cselectionRange[4] = p->cselectionRange[0];
            p->cselectionRange[5] = p->cselectionRange[1];
        }
        p->viewtop = NULL;
        p->selection = false;
        p->cselection = false;
        p->cursorPos[1] = 0;
        if(e->modifiers() == Qt::ShiftModifier)
        {
            p->cselectionRange[0] = p->cursorPos[0];
            p->cselectionRange[1] = p->cursorPos[1];
            if(p->cselectionRange[1] != p->cselectionRange[3])
                p->cselection = true;
        }
        p->calcScrollBar();
        update();
        return;
    }
    if(e->key() == Qt::Key_End)
    {
        if(p->cfirst == p->clast && p->cfirst->line.isEmpty() && p->viewtop != NULL)
        {
            p->viewtop = NULL;
            p->calcScrollBar();
            update();
            return;
        }
        if(e->modifiers() == Qt::ShiftModifier)
        {
            p->cselectionRange[0] = p->cursorPos[0];
            p->cselectionRange[1] = p->cursorPos[1];
            p->cselectionRange[4] = p->cselectionRange[0];
            p->cselectionRange[5] = p->cselectionRange[1];
        }
        p->viewtop = NULL;
        p->selection = false;
        p->cselection = false;
        p->cursorPos[1] = p->commandLineLineBySerial(p->cursorPos[0]).length();
        if(e->modifiers() == Qt::ShiftModifier)
        {
            p->cselectionRange[2] = p->cursorPos[0];
            p->cselectionRange[3] = p->cursorPos[1];
            if(p->cselectionRange[1] != p->cselectionRange[3])
                p->cselection = true;
        }
        p->calcScrollBar();
        update();
        return;
    }

    if(e->key() == Qt::Key_Escape)
    {
        p->viewtop = NULL;
        p->selection = false;
        p->cselection = false;
        resetCommandLine();
        p->history_it = p->history.end();
        p->fitConsole();
        update();
        return;
    }

    if(e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete)
    {
        p->viewtop = NULL;
        p->calcScrollBar();
        deleteFromCursor(e->key() == Qt::Key_Delete ? true : false);
        return;
    }

    if(e->key() == Qt::Key_Up)
    {
        p->viewtop = NULL;
        p->calcScrollBar();
        p->cselection = false;
        if(!p->history.isEmpty())
        {
            if(p->history_it == p->history.end())
            {
                p->history_it = p->history.end();
                --p->history_it;
            }
            else
            {
                if(p->history_it != p->history.begin())
                    --p->history_it;
            }
            setCommandLineText(*p->history_it);
        }
        return;
    }

    if(e->key() == Qt::Key_Down)
    {
        p->viewtop = NULL;
        p->calcScrollBar();
        p->cselection = false;
        if(!p->history.isEmpty() && p->history_it != p->history.end())
        {
            ++p->history_it;
            if(p->history_it == p->history.end())
            {
                resetCommandLine();
                update();
            }
            else
                setCommandLineText(*p->history_it);
        }
        return;
    }

    if(e->key() == Qt::Key_Left)
    {
        cursorLeft(1,e->modifiers() == Qt::ShiftModifier ? true : false);
        return;
    }

    if(e->key() == Qt::Key_Right)
    {
        cursorRight(1,e->modifiers() == Qt::ShiftModifier ? true : false);
        return;
    }

    if(e->key() == Qt::Key_Tab)
    {
        p->viewtop = NULL;
        p->calcScrollBar();
        QString send = commandLineText();
        update();
        emit tabPressed(send);
        return;
    }

    if(e->key() == Qt::Key_B &&  e->modifiers() == Qt::ControlModifier)
    {
        biggerFontSize();
        return;
    }

    if(e->key() == Qt::Key_S && e->modifiers() == Qt::ControlModifier)
    {
        smallerFontSize();
        return;
    }

    if(e->key() == Qt::Key_0 && e->modifiers() == Qt::ControlModifier)
    {
        normalFontSize();
        return;
    }

    if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)
    {
        if(e->modifiers() == Qt::ShiftModifier)
        {
            addTextToCursor("\n");
            return;
        }

        p->viewtop = NULL;
        p->calcScrollBar();
        QString send = commandLineText();
        if(!send.isEmpty() && (p->history.isEmpty() || send != p->history.last()))
        {
            p->history.push_back(send);
            p->history_it = p->history.end();
        }
        resetCommandLine();
        update();
        emit commandEntered(send);
        return;
    }

    if(e->key() == Qt::Key_Shift || e->key() == Qt::Key_Alt || e->key() == Qt::Key_Control)
        return;

    //Keypress
    if(e->modifiers() != Qt::ControlModifier && e->modifiers() != Qt::AltModifier)
    {
        p->viewtop = NULL;
        p->calcScrollBar();
        addTextToCursor(e->text());
    }
}

void HConsolePanel::setPromptString(QString prm)
{
    p->promptStr = prm;
    update();
}

QString HConsolePanel::promptString(void)
{
    return p->promptStr;
}

void HConsolePanel::setFontSize(int point)
{
    p->fontsize = point;
    QFont f = font();
    f.setPixelSize(p->fontsize);
    setFont(f);
    delete p->fm;
    p->fm = new QFontMetrics(font(),this);
    p->fitConsole();
}

int  HConsolePanel::fontSize(void)
{
    return p->fontsize;
}

void HConsolePanel::biggerFontSize(void)
{
    int s = fontSize() + 1;
    if(s > 25)
        s = 25;
    setFontSize(s);
}

void HConsolePanel::smallerFontSize(void)
{
    int s = fontSize() - 1;
    if(s < 10)
        s = 10;
    setFontSize(s);
}

void HConsolePanel::normalFontSize(void)
{
    setFontSize(14);
}

#endif //GSAFE_DISABLE_DEBUG

//end code
