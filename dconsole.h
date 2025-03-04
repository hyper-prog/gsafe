﻿/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2025 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   dconsole.h
*/

/* This module is not depend from gSAFE
 * Can be used separatedly (Just add dconsole.h/dconsole.cpp to your project)
 */

#ifndef GSAFE_DCONSOLE__HEADER_
#define GSAFE_DCONSOLE__HEADER_

#include <QtCore>

#ifndef GSAFE_DISABLE_DEBUG
#include <QtGui>
#include <QtWidgets>

#endif

/** \defgroup dconsole dconsole */
/*  @{  */

#ifndef DCONSOLE_NO_SQL
/** Puts a debug/info text as sql text.
 *  If there is no started HDebugConsole this function does nothing.
 *  @see HDebugConsole*/
void sqldebug(QString s);
#endif // DCONSOLE_NO_SQL

/** Puts a debug/info text as normal text.
 *  If there is no started HDebugConsole this function does nothing.
 *  @see HDebugConsole*/
void sdebug(QString s);

/** Puts a debug/info text as special text.
 *  If there is no started HDebugConsole this function does nothing.
 *  @see HDebugConsole*/
void ssdebug(QString s,char type);

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

#define DCONSOLE_TYPE_TEXT      0
#define DCONSOLE_TYPE_MESSAGE   1
#define DCONSOLE_TYPE_SQL       2
#define DCONSOLE_TYPE_RESULT    3
#define DCONSOLE_TYPE_CMD       4
#define DCONSOLE_TYPE_QTDEBUG   5
#define DCONSOLE_TYPE_TXTALT_A  6
#define DCONSOLE_TYPE_TXTALT_B  7
#define DCONSOLE_TYPE_TXTALT_C  8
#define DCONSOLE_TYPE_TXTALT_D  9
#define DCONSOLE_TYPE_TXTALT_E  10

#define DCONSOLE_TYPE_TXTALT_CYAN    DCONSOLE_TYPE_TXTALT_A
#define DCONSOLE_TYPE_TXTALT_CORAL   DCONSOLE_TYPE_TXTALT_B
#define DCONSOLE_TYPE_TXTALT_TEAL    DCONSOLE_TYPE_TXTALT_C
#define DCONSOLE_TYPE_TXTALT_PURPLE  DCONSOLE_TYPE_TXTALT_D
#define DCONSOLE_TYPE_TXTALT_RED     DCONSOLE_TYPE_TXTALT_E

void dconsoleMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

class QFontMetrics;
class QCloseEvent;
class HConsolePanel;
class HDebugConsolePrivate;
/** HDebugConsole is a debug/info console with a limited sql console and internal command line functionality.
 *  \image html pictures_doc/hdebugconsole.png
 *  - This DebugConsole is a console window designed to work separatedly from any other window or dialog. You can popup or close this stuff everywhere in the program code.
 *    It works 100% independent from other codes.
 *  - If you want to use this debug console without gSAFE, you only need to add dconsole.h and dconsole.cpp to your project. They doesn't require any other gSAFE parts.\n
 *  - You can use static global functions to start/stop this console dconsole() and dconsole_close()
 *  - You can send debug messages from everywhere with sdebug(QString) and sqldebug(QString) or
 *    alternatively you can send debug messages standard Qt way with qDebug. (see example below)
 *  - You can place sdebug("Debug message") and sqldebug("Sql command") functions everywhere in the program. You don't need
 *      to comment out these after the debugging. If there is no active debug console, this functions exits immediately.
 *       It means that if you don't start the console this command does nothing and don't slow down your program.
 *  - You can run SQL command from this console.
 *  - The console has a "show" command which can shows the tables of the database,
 *    and describe the table definitions of a specified SQL table.
 *  - You can register your own functions as a command with register_dconsole_command() function.
 *  - HDebugConsole can run in synchron write function. (Immediatly write every text to a log file with sync)
 *
 *
 * An example to start console, and send some debug message:
 \code
    //somewhere in the code:
    ...
    dconsole(); //start the debug console.
    ...
    sdebug("First phase passed.");
    ...
    sdebug("N-th phase passed.");
 \endcode
 *
 * The HDebugConsole grabs the Qt's qDebug messages so you can see the Qt system messages/warnngs/errors\n
 * The following messages are goes to the HDebugConsole (when opened):
 \code
    //somewhere in the code:
    ...
    qDebug() << "Start processing";
    ...
    qWarning() << "The string is:" << obj.toString();
    ...
 \endcode
 *
 * You can easely use this console in your project by adding dconsole.h and dconsole.cpp to your project file.\n
 * If you don't need the Sql functionality, so you would like to drop the QtSql dependency define the DCONSOLE_NO_SQL macro in your project file:
 \code
    SOURCES += dconsole.cpp
    HEADERS += dconsole.h

    #define this below to stay Sql independent.
    DEFINES += DCONSOLE_NO_SQL
 \endcode
 * \n\n
 *
 * Current available commands (Console "help" command output):
 \code
    Console>help
    HDebugConsole, commands:
     alldb - Show all available database connections
     clear - Clears the debug window's text
     close - Close the debug window (only)
     dbinfo - Show the current connected database information
     disable <filter> - Disable the "filter" kind of output
     enable <filter> - Enable the "filter" kind of output
     exit - Exit main program (The debugged program too)
     filters - Show available debug filters
     help - List the available commands
     run <custom> - Run the "custom" program command
     save - Save the content of debug window to debug.txt
     setdb - Sets the current database to default (not the program but console)
     setdb <dbname> - Sets the current database to "dbname"
     show - Show all table in the current database
     show <tablename> - Show the fields of table named "tablename"
     state <filter> - Show the state of the "filter" kind of output filter
     synw - Query the state or clear the content of syndebug.txt
     synw off - Disable the writing of syndebug.txt
     synw on - Enable the writing of syndebug.txt
     synw clear - Clear the content of syndebug.txt
     write <text> - Write "text" to the console
     "SQL" - Execute the SQL command
 \endcode
 *
 */
class HDebugConsole : public QWidget
{
    Q_OBJECT

public:
    /** The one pointer of the only one HDebugConsole */
    static HDebugConsole *myself;

public:
    /** Creates a HDebugConsole. Don't create directly. Use the dconsole() global function instead */
    HDebugConsole(QWidget *parent);
    /** Destructor */
    ~HDebugConsole(void);

    /** Write a text to the console. Don't use it directly. Use the sdebug() and sqldebug() instead */
    void add_text(QString s,int type);
#ifndef DCONSOLE_NO_SQL
    /** Write an sql text to the console */
    static void debug_sql(QString s);
#endif // DCONSOLE_NO_SQL
    /** Write a normal text to the console */
    static void debug_txt(QString s);
    /** Write a special typed text to the console */
    static void debug_typedtxt(QString s,char type);
    /** Popups a warning text */
    static void popup(QString title,QString str);
    /** You can disable the command excution if this function called with FALSE.
     *  @param enabled FALSE:The user can't execute any command.
     *                 TRUE(Default state):The console execute the user's commands. */
    void setCommandExecution(bool enabled = true);
    /** Returns the console panel object */
    HConsolePanel *consoleObject(void);

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    int execCommand(QString query);
    int tabPressed(QString query);
    /** You can call this slot anytime you want. If this slot can't find any toplevel widget
     *  in this program which is different from this console and visible, it closes the debugwindow. */
    int checkIfIClose();
private:

    class HDebugConsolePrivate *p;
};

class HConsoleLine;
class HConsolePanelPrivate;

/** HConsolePanel is an universal console panel which has a readonly textual history part,
 *  and an editable command line part.
 *  \image html pictures_doc/hdebugconsole.png
 *  It designed to handle a large amount of text fastly.
 *  It supports text selections, clipboard functions,
 *  basic editing functionality on command line and command history.
 *      Available key commands:
 *  - SHIFT+PgUp	Scrolls 5 line up
 *  - SHIFT+PgDown	Scrolls 5 line down
 *  - CTRL+A		Selects all text
 *  - CTRL+Ins      Copy selected text to clipboard
 *  - CTRL+C		Copy selected text to clipboard
 *  - SHIFT+Ins     Paste from clipboard to command line
 *  - CTRL+V		Paste from clipboard to command line
 *  - HOME          In case of empty command line:Scrolls to the beggining of text
 *  -               In case of none empty command line: Jump to the beginning of the current command line
 *  - END           In case of empty command line:Scrolls to the end of text
 *  -               In case of none empty command line: Jump to the end of the current command line
 *  - SHIFT+HOME	Selects all to the beginning of the line
 *  - SHIFT+END     Selects all to the end of the line
 *  - ESC           Erase the command line
 *  - Backspace     Deletes the character before the cursor
 *  - Delete 		Deletes the character after the cursor
 *  - UP            Load the Previous command
 *  - DOWN          Load the next command
 *  - LEFT          Moves Cursor to left/back
 *  - RIGHT         Moves Cursor to right/front
 *  - SHIFT+LEFT	Moves Cursor to left/back while expand the selection
 *  - SHIFT+RIGHT	Moves Cursor to right/front while expand the selection
 *  - ENTER         Send the command
 *  - SHIFT+ENTER	Starts a new line in command line (without sending the command)
 *  - CTRL+<PLUS>
 *  - CTRL+B        Sets bigger font size
 *  - CTRL+0        Sets normal font size
 *  - CTRL+<MINUS>
 *  - CTRL+S        Sets smaller font size
*/
class HConsolePanel : public QFrame
{
    Q_OBJECT

public:
    /** Creates an empty HConsolePanel */
    HConsolePanel(QWidget *parent);
    ~HConsolePanel(void);

    /** Adds a text to the readonly/view part.
     *  @param t text to add
     *  @param type A numerical type feature of the text, which can be used to show this text different color.
     *  @see setTextTypeColor */
    void addText(QString t,char type = 0);
    /** Returns the serial number of the last line */
    int  maxSerial(void);
    /** Returns the serial number of the display top line */
    int  topSerial(void);
    /** Returns the line which has the parameter passed serial number
     *  @param serial the requested line's serial
     *  @param withWraps if this parameter true, the next autowrapped lines
     *      are concatenated to the returned line
     *  @param onDisplay If this parameter true the searching is started on the current display (much faster) */
    QString lineBySerial(int serial,bool withWraps =false,bool onDisplay = true);

    /** Scrolls up the readonly/view textual part by the lineCount number line */
    void scrollUp(int lineCount = 1);
    /** Scrolls down the readonly/view textual part by the lineCount number line */
    void scrollDown(int lineCount = 1);
    /** Scrolls the readonly/view textual part to the specified serial numbered line */
    void scrollToSerial(int serial);
    /** Returns the currently selected text. */
    QString selectedText(void);
    /** Sets the tab stop count when addText replaces the tabs to spaces.
     *  The default value is 8. */
    void setTabStop(int count);

    /** Sets a margin text, which is displayed on the left margin of all readonly line (every line except command lines) */
    void setMarginText(QString margin);

    /** Sets the color of a specified console area
     *  @param section One of
            "background","selection","standardtext","cmdtext","cursor","margin","marginbg"
     *  @param color the color to set */
    void setColor(QString section,QColor color);
    /** Sets the color of the type coded text.
     *  @param type the text type to set. This type match with that type passed with addText function
     *  @param color the color to set
     *  @see addText */
    void setTextTypeColor(char type,QColor color);

    /** Sets the command line prompt */
    void setPromptString(QString prm);
    /** Returns the current active command line prompt */
    QString promptString(void);
    /** If this function called with disable:true the user entered charaters
     *  won't write back to the console.
     *  You can reenable character echoing by calling this function with disable:false.
     *  (This is the default case) */
    void setCommandLineCharacterEcho(bool disable = false);
    /** Returns the current active command line text */
    QString commandLineText(void);
    /** Moves the cursor in the command line text to left with charCount position */
    void cursorLeft(int charCount = 1,bool inSelection = false);
    /** Moves the cursor in the command line text to right with charCount position */
    void cursorRight(int charCount = 1,bool inSelection = false);
    /** Sets the command line text to t (It clears the current active) */
    void setCommandLineText(QString t,bool disableupdate=false);
    /** Returns the cursor position in command line text.
     *  It's an absolute character number from the beginning of command line. */
    int  cursorPosition(void);
    /** Sets the cursor position in command line text.
     *  It's an absolute character number from the beginning of command line. */
    void setCursorPosition(int c);
    /** Reset the command line part. Set empty command line, clears all selection, scrolling, etc..  */
    void resetCommandLine(void);
    /** Adds the parameter text to the command line by the cursor position.
     *  If the command line has active selection the text will replace that. */
    void addTextToCursor(QString text);
    /** Delete character from the command line by the cursor position.
     *  If the command line has active selection the function will delete that instead of the character.
     *  @param forwardDel If true the function deletes the character after the cursor (Delete).
            If false the function deletes the character before the cursor and steps the cursor back (Backspace). */
    void deleteFromCursor(bool forwardDel = false);

    /** Sets the font size used by HConsolePanel */
    void setFontSize(int point);
    /** Returns the font point size used by HConsolePanel */
    int  fontSize(void);
    /** Sets the font used by HConsolePanel */
    void setCustomFont(QFont f);

public slots:
    /** Adds a text to the readonly/view part. Means: addText(text,0); */
    void addNormalText(QString text);
    /** Clears all text form the readonly/view part. */
    void clearText(void);
    /** Selects all text in the readonly/view part. */
    void selectAll(void);
    /** Clears selection in the readonly/view part. */
    void clearSelection(void);
    /** Copy the current selection to the clipboard */
    void copySelection(void);
    /** Scrolls the absolute to of the console */
    void scrollTop(void);
    /** Reset scrolling of the console. Shows the last line. */
    void scrollReset(void);
    /** Paste the current cliboard text to the command line */
    void pasteToCommandLine(void);
    /** Deletes the current selected text from the command line */
    void deleteCmdLineSelection(void);

    void biggerFontSize(void);
    void smallerFontSize(void);
    void normalFontSize(void);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);

    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void wheelEvent(QWheelEvent *e);
    bool event(QEvent *e);

signals:
    /** Emitted when the user type a text to the command line and hit Enter/Return on it.
     *  The command line text will be reseted before this signal emitted */
    void commandEntered(QString commandText);
    /** Emitted when the user type a text to the command line and hit TAB it.
     *  The command line text stay untouched. */
    void tabPressed(QString commandText);

private:
    class HConsolePanelPrivate *p;
};

#endif //GSAFE_DISABLE_DEBUG

/*  @}  */

#endif
