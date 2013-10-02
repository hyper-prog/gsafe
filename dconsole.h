/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2006-2013 Peter Deak  (hyper80@gmail.com)

    License: GPLv2  http://www.gnu.org/licenses/gpl-2.0.html

    dconsole.h
*/

/* This module is not depend from gSAFE
 * Can be used separatedly (Just add dconsole.h/dconsole.cpp to your project)
 */


#ifndef GSAFE_DCONSOLE__HEADER_
#define GSAFE_DCONSOLE__HEADER_

#include <QtCore>

/** \defgroup dconsole dconsole */
/*  @{  */

#ifndef GSAFE_DISABLE_DEBUG

#ifdef FORMS_HAS_UI_PREFIX
#include "ui_debugwidgetbase.h"
#else
#include "debugwidgetbase.h"
#endif

#endif //GSAFE_DISABLE_DEBUG

/** Puts a debug/info text as sql text.
 *  If there is no started HDebugConsole this function does nothing.
 *  @see HDebugConsole*/
void sqldebug(QString s);
/** Puts a debug/info text as normal text.
 *  If there is no started HDebugConsole this function does nothing.
 *  @see HDebugConsole*/
void sdebug(QString s);

/** Starts/Popup the HDebugConsole.
 *  If the console already run this function does nothing.
 *  @see HDebugConsole  */
void dconsole(void);
/** Closes the DebugConsole
 *  If the console already closed this function does nothing.
 *  @see HDebugConsole*/
void dconsole_close(void);
/** Popups a warning message
 *  @see HDebugConsole*/
void dconsole_popup(QString t,QString txt);

/** HDConsoleCommandHolder is an interface class for HDebugConsole.
 *  You can register your own command with this class.
 *  You have to implement this class in your code.
 *  The command can be registered with register_dconsole_command() function. */
class HDConsoleCommandHolder
{
    public:
        /** Called on every programmer defined command call.
         *  You receive the called comman text in parameter.
         *  The return string is printed to the debug console. */
        virtual QString donsole_command_interpreter(QString commandString) = 0;
};

/** Register a new command in HDebugConsole */
void register_dconsole_command(QString command,HDConsoleCommandHolder *interpreter,QString descr="");
/** Unregister a command from HDebugConsole */
void unregister_dconsole_command(QString command);
/** Delete all command from HDebugConsole */
void clear_dconsole_commands();

#ifndef GSAFE_DISABLE_DEBUG

class QCloseEvent;
/** HDebugConsole is a debug/info console with a limited sql console and internal command handler functionality.
 *  \image html pictures_doc/hdebugconsole.png
 *  - This DebugConsole is an indepedent window from the main window. You can popup or close this window everywhere in the program code.
 *      100% independent from other codes.
 *  - You can use static global functions to start/stop this console and put texts. dconsole() dconsole_close() sdebug() sqldebug()
 *  - You can place sdebug("Debug message") and sqldebug("Sql command") functions everywhere in the program. You don't need
 *      to comment out these after the debugging! If there is no active debug console, this functions exits immediately.
        So if you don't start the console this command does nothing and don't slow down your program.
 *  - You can run SQL command from this console.
 *  - You can register some function as a command.
 *  - HDebugConsole can run in synchron write function. (Immediatly write every text to a log file with sync)
 *
 *
 \code
    //somewhere in the code:
    ...
    dconsole(); //start the debug console.
    ...
    sdebug("First phase passed.");
    ...
    sdebug("N-th phase passed.");
 \endcode   */
class HDebugConsole : public QWidget , public Ui::DebugWidgetBase
 {

    Q_OBJECT
    public:
        /** The one pointer of the only one HDebugConsole */
        static HDebugConsole *myself;

    public:
        QString databasename;
        QString pre;
        /** Creates a HDebugConsole. Don't create directly. Use the dconsole() global function instead */
        HDebugConsole(QWidget *parent);
        /** Destructor */
        ~HDebugConsole(void);
        /** Write a text to the console. Don't use it directly. Use the sdebug() and sqldebug() instead */
        void add_text(QString s,int type); //0-sql,1-txt

        /** Write an sql text to the console */
        static void debug_sql(QString s);
        /** Write a normal text to the console */
        static void debug_txt(QString s);

        /** Popups a warning text */
        static void popup(QString title,QString str);

    protected:
        void closeEvent(QCloseEvent *e);

    public slots:
        int execSql(void);
        /** You can call this slot anytime you want. If this slot can't find any toplevel widget
         *  in this program which is different from this console and visible, it closes the debugwindow. */
        int checkIfIClose();

 };
#endif //GSAFE_DISABLE_DEBUG

/*  @}  */

#endif
