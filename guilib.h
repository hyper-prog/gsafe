/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2020 Peter Deak  (hyper80@gmail.com)

    License: LGPLv2.1

    guilib.h
*/

#ifndef GSAFE__GUILIB_HEADER_FILE_X_
#define GSAFE__GUILIB_HEADER_FILE_X_

#include <QtCore>
#include <QtGui>

#ifndef COMPILED_WITH_QT4X
#include <QtWidgets>
#endif

#ifdef WEBKITBASED_HTMLSHOW
#ifndef COMPILED_WITH_QT4X
#include <QtWebKitWidgets>
#else
#include <QtWebKit>
#endif
#endif

#include "datalib.h"

/** \defgroup guilib guilib */
/*  @{  */

#define EXT_EDIT_PROCESS    "externaledit.bat"

#define NEED_UPDATE 0
#define CLICK_ON_CELL 1

/** The metadata instance handling of GUI calsses (What the GUI class do with the metadata pointer on starting and destorying )
 *  - KEEP_ALL      Don't modify the data class: After creating it will show the (previous set) data of the metadata class.
 *                  On destory it leaves all data untouched too.
 *  - FULL_CLEAN    DEFAULT MODE! On start it cleans every data to the (metadata)default value.
 *                  (This way tha gui initially it shows this default data of course)
 *                  On destory it cleans every data, and set to everything to default again.
 *  - START_CLEAN   Only set the data to the default on starting. On destory the data will be untouched.
 *  - END_CLEAN     Only set the data to the default on destory. On start the data will be untouched.  */
#define KEEP_ALL     0
#define FULL_CLEAN   1 ///< \copydoc KEEP_ALL
#define START_CLEAN  2 ///< \copydoc KEEP_ALL
#define END_CLEAN    3 ///< \copydoc KEEP_ALL

/** This define tells what happend with the metadata if the gui object is deleted.
 *  -   DONT_DELETE_DATA    If the gui object is deleted, leaves the metadata object untouched.
 *                          (You need to manually delete the metadata later)
 *  -   DELETE_DATA         If the gui object is deleted it deletes the metadata object too.
 *                          It deletes the connected meta-data object too. (Connected tables, see HTableBase)
 *                          (You can't use the metadata later) */
#define DONT_DELETE_DATA 0
#define DELETE_DATA      1 ///< \copydoc DONT_DELETE_DATA

class QLabel;
class QLineEdit;
class QComboBox;
class QTextEdit;
class QCheckBox;
class QDateEdit;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QPoint;
class QKeyEvent;

class HTableBrowserElement;

/** HArrayButton is a helper class to creates an "abc" array of small QPushButton -s with labels abc letters: A B C D ...
 *  The size of buttons will be equalized automatically. The button array can be created with the abc_filt() function.
 *  When the user push a button the HArrayButton emits a signal with the pressed letter.
 *  (Can be useful to filter the result of a list to the first letter)
 *  \image  html pictures_doc/harraybuttonabc.png */
class HArrayButton : public QPushButton
{
    Q_OBJECT

    public:
        /** The size of the buttons. Automatically set, don't need to touch. */
        static int defaultSizeX,defaultSizeY;

        /** Creates one piece of array-button. Use the abc_filt() to create a whole abc array. */
        HArrayButton(QString text,QWidget *parent);
        ~HArrayButton(void);

        /** Creates an abc array of buttons.
         *  @param parent the parent QWidget descendant
         *  @param receiver Connect the "reciver" objects "member" named slot to the array buttons clicked signal.
         *      The "member" slot have to receive a QString parameter which will be the text of the pressed button.
         *  @param member see param receiver.
         *  @return Returns a layout which contains the buttons.    */
        static QLayout * abc_filt(QWidget *parent,const QObject *receiver,const char *member);

    private slots:
        int clickHandler(void);

    signals:
        void clickedValue(QString text);
};

/** HSpinBox is a similar object than QSpinBox but the user can change the value with the mouse wheel.
 *  \image  html pictures_doc/hspinbox.png  */
class HSpinBox : public QFrame
{
    Q_OBJECT

    public:
        /** Creates a HSpinBox
         *  @param parent the parent widget of the HSpinBox */
        HSpinBox(QWidget *parent);
        /** Destuctor */
        ~HSpinBox(void);

        /** Sets the minimum value of the HSpinBox  */
        void setMinValue(int v) { min = v; check(); }
        /** Sets the maximum value of the HSpinBox  */
        void setMaxValue(int v) { max = v; check(); }
        /** Returns the current value */
        int value(void)         { return val; }

    public slots:
        /** Sets the value of the HSpinBox */
        int setValue(int v);

    protected:
        int val;
        int min,max;
        QString oldtext;

        QLineEdit *le;

    protected slots:
        void check(void);
        int  leChanged(const QString& t);
        int  bUp(void);
        int  bDown(void);

    protected:
        /** The mouse wheel event handler */
        void wheelEvent(QWheelEvent *e);

    signals:
        /** Emitted when the value is changed */
        void valueChanged(int);
};

/* NOT DOCUMENTED: Helper class of HTableBrowser. Do not use directly.
 * The HTableBrowser class uses this to qsort the elements. */
class HTableBrowserElementPair
{
    public:
        HTableBrowserElementPair(HTableBrowserElement *lp,HTableBrowserElement *rp)
            { left = lp; right = rp; }

        HTableBrowserElement *left;
        HTableBrowserElement *right;
};

/** HTableBrowser is a table view class, which can show the list of data groups (they can be a records).\n
 *  This browser is designed to show/handle really big data tables without slow down.\n
 *  Shows N column and M rows.\n
 *  This viewer is a readonly viewer, you can't edit the content with it.
 *  Hovewer the list is readonly there is a cursor which marked an element to currently active.
 *  The HTableBrowser holds the data in a double linked list, memorize the beginning and the end of the displayed data part,
 *  so the browsing will be same speed, when the browser hold a dozen elements or more thousends.
 *  The elements are HTableBrowserElement objects. Every record have one key value, and N(column count) displayed value.
 *  -   The user can select an element. The event signal will pass the key value of this element.
 *  -   The user can search in the browser by typing letters. The matched letters are indicated until the search is finished.
 *      (In the example picture the user already pressed a "De" string)
 *      The search is finished if the user click to another cell, or moves the cursor to a different position.
 *  -   The user can sort the elements by clicking on the header. (Can sort by two column, first and secondary)
 *  -   You can set different colors for columns.
 *
 *  \image  html pictures_doc/htablebrowser.png
 *  \code
    HTableBrowser *htb = new HTableBrowser(parentwidget);
    htb->setHeadTexts(QString("Fruit;Color").split(";",QString::SkipEmptyParts));
    htb->addElement(new HTableBrowserElement("001" , "Apple","Red"));
    htb->addElement(new HTableBrowserElement("002" , "Pear","Green"));
    htb->addElement(new HTableBrowserElement("003" , "Strawberry","Red"));
    \endcode
 *
 *  In the background this data browser uses bi-directional linked list to store the data, and handles
 *  the viewed area separatedly from the full list.
 *  There is not matter the size of the full list when drawing/scrolling or moving in the list
 *  but the displayed/drawed size only. This drawing is also fastened by pixmap caches, so a full screen scrollig is fast enough.
 *  The sorting is achived by quick sort algorithm to be the fastest as possible.
 *
 *  @see HTableBrowserElement   */
class HTableBrowser : public QFrame
{
    Q_OBJECT

    public:
        /** Creates a HTableBrowser class
         *  @param parent the parent widget of the HTableBrowser */
        HTableBrowser(QWidget *parent);
        /** Destructor. Deletes the elements too. */
        ~HTableBrowser(void);

        /** Sets the color of the specified column.
         *  @param col the index of the column to set the color
         *  @param c the color to set */
        void setColumnColor(int col,QColor c);

    public slots:

        /** Set the query cursor to the required position
         *  - queryToStart() - Sets the cursor to the absolute first element of the table.
         *  - queryToTop() - Set the query cursor to the first displayed element
         *                   (Which is currently in the top of viewed window)
         *  - queryToCurrent() - Set the query cursor to the position of the cursor (Currently active element)
         *
         *  \code
            HTableBrowser *htb = HTableBrowser(parentwidget);
            HTableBrowserElement *e = NULL;
            ...
            htb->queryToStart();
            while((e = htb->queryNextElement()) != NULL)
            {
                do_something( e[0] );
            }
            \endcode
         *  @see queryToStart() @see queryToTop() @see queryToCurrent() @see queryNextElement() @see queryNextValues() @see queryNextKey() */
        void queryToStart(void);
        void queryToTop(void);     ///< \copydoc queryToStart()
        void queryToCurrent(void); ///< \copydoc queryToStart()

    public:

        /** Returns the HTableBrowserElement pointer in the position of the query cursor.
         *  After returning the pointer it steps to the next position.
         *  If we have already reached the end of the list NULL returned.\n
         *  You will get a direct pointer, don't delete it!
         *  @see queryToStart() @see queryToTop() @see queryToCurrent() */
        HTableBrowserElement *queryNextElement(void);
        /** Returns the displayed values (in a QStringList pointer) of the HTableBrowserElement
         *  in the position of the query cursor.
         *  After returning the values it steps to the next position.
         *  If we have already reached the end of the list NULL returned.\n
         *  You will get a new allocated object, you have to delete it!
         *  @see queryToStart() @see queryToTop() @see queryToCurrent() */
        QStringList          *queryNextValues(void);
        /** Returns the key (in a QString pointer) of the HTableBrowserElement
         *  in the position of the query cursor.
         *  After returning the key value it steps to the next position.
         *  If we have already reached the end of the list NULL returned.\n
         *  You will get a new allocated object, you have to delete it!
         *  @see queryToStart() @see queryToTop() @see queryToCurrent() */
        QString              *queryNextKey(void);

        /** Returns the number of the elements in the table (Number of rows) */
        int numberOfElements(void);

        /** Returns the HTableBrowserElement pointer in the position of the required index.
         *  If the element doesn't reachable NULL returned.\n
         *  You will get a direct pointer, don't delete it!
         *  @param n the required index. (The first element is 0)
         *  @see elementByIndex() @see valuesByIndex() @see keyByIndex() @see numberOfElements() */
        HTableBrowserElement *elementByIndex(int n);
        /** Returns the displayed values (in a QStringList pointer) of HTableBrowserElement
         *  in the position of the required index.
         *  If the element doesn't reachable NULL returned.\n
         *  You will get a new allocated object, you have to delete it!
         *  @param n the required index. (The first element is 0)
         *  @see elementByIndex() @see valuesByIndex() @see keyByIndex() @see numberOfElements()  */
        QStringList          *valuesByIndex(int n);
        /** Returns the key (in a QString pointer) of HTableBrowserElement
         *  in the position of the required index.
         *  If the element doesn't reachable NULL returned.\n
         *  You will get a new allocated object, you have to delete it!
         *  @param n the required index. (The first element is 0)
         *  @see elementByIndex() @see valuesByIndex() @see keyByIndex() @see numberOfElements()  */
        QString              *keyByIndex(int n);

        /** Returns true if the table has an element with a key passed in parameter, otherwise return false. */
        bool hasKey(QString key);

    public slots:

        /** It flushes the cell drawing cache */
        int fulshDrawingCache(void);

        /** Clears every element from the table */
        int clear();

        /** Adds a new element to the table. The new element will be placed the last position.
         *  @param e the new element. (You have to allocate the object, don't need to delete later, automatically deleted)
         *  @see HTableBrowserElement   */
        int addElement(HTableBrowserElement *e);

        /** Removes all elemet which have a same key passed in parameter.
         *  @param key the key value to delete.
         *  @return the number of deleted elements */
        int removeAllElementByKey(QString key);

        /** Sets the header texts of the table. You have to call this funtion before you insert any elements!
         *  @param h the text of the headers. The QStringList have to have same number QString as the column of table. */
        int setHeadTexts(QStringList h);

        /** Sets the current elements.
         *  @param the requested KEY value to be current. The first element which have the key value
         *      matched with the c will be the currrent.
         *  @param t (Optional) The key value of the top element of the DISPLAYED list.
         *      We can use this parameter to avoid unnecessary scrolling.
         *      The new current element always will be visible but:
         *      If we specify the currently displayed top element in parameter t(op), the browser will examine the visibility of
                the new current element. If it's possible the top element won't change. */
        int setCurrentElement(QString c,QString t="");

        /** Sets the current element by index */
        int setCurrentElement(int index);

        /** Sets the displayed top element by index */
        int setTopElement(int index);

        /** Sets the index of the column to sort the elements.
         *  The previous column will be the secondary sorting column.
         *  If you pass same column index which already set the sort order will be changed.\n
         *  Example:
         *  -   initial (no sorting), then...
         *  -   sort(2) - ascendig sort to 3. column, then...
         *  -   sort(1) - ascendig sort to 2. column first secondary 3. , then...
         *  -   sort(1) - descendig sort to 2. column first secondary 3.  */
        void sort(int keycol);

        /** Sets the cursor to the absolute first element */
        int  cursorToBegin(void);
        /** Sets the cursor to the absolute last element */
        int  cursorToEnd(void);

    public:

        int mouseRightClickPosX;
        int mouseRightClickPosY;

        /** Returns the current element. Returns NULL, if there is not current.
         *  Don't delete the object! */
        HTableBrowserElement * currentElement(void);
        /** Returns the key value of the current element */
        QString                currentKey(void);
        /** Returns the first element on the display. (Top element)
         *  Don't delete the object! */
        HTableBrowserElement * topElement(void);
        /** Returns the key of the first element on the display. */
        QString                topKey(void);

        /** Set a sort modifier for a specified column.
         *  @param col the index of the modified sorting column.
         *  @param specsortname name of the sort modifier.
         *      Now only can receive one modifier: "dropchar" which means the characters are
         *      dropped from the value and the result converted to integer, then
         *      the result is sorted. \n
         *      (For example if the sorted values are: id_1,id_10,id_14,id_2,id_21  will be  1,2,10,14,21 */
        void setSpecSort(int col,QString specsortname);

        /** Sets the color of the backround */
        void setBackgroundColor(QColor c);
        /** Sets the color of the cursor line (current line) */
        void setCursorColor(QColor c) { color_cursor = c; }
        /** Sets the color of the header line */
        void setHeaderColor(QColor c) { color_header = c; }
        /** Sets the color of the header line in cursor position */
        void setHeaderColorCursor(QColor c) { color_header_cursor = c; }

    //================================================================
    private:
        int limitCache(void);
        std::list<HTableBrowserElement *> cachedElements;
        int cacheLimit;

    private:
        QColor color_cursor;
        QColor color_header;
        QColor color_header_cursor;
        QColor color_background;

        bool asc;
        bool asc2;
        int  keycolumn;
        int  keycolumn2;
        int sortmode;    // 0-normal 1-dropchar

        void q_sort    (void);
        void q_sort_run(HTableBrowserElementPair *p);
        bool ch(bool m,HTableBrowserElement *a,HTableBrowserElement *b);

        QStack<HTableBrowserElementPair *> *q_sort_stack;

        QString lastkey;

        int oldw,oldh,old_col_scroll;

        QMutex cacheMutex;

    protected:
        QString last_right_clicked;
        int number;
        int curc;
        HTableBrowserElement *start,*end,*top,*cur,*query_cursor;
        int col_scroll;

        static bool    staticinit;
        static QPixmap *pix_up;
        static QPixmap *pix_down;
        static QPixmap *pix_asc;
        static QPixmap *pix_desc;
        static QPixmap *pix_asc2;
        static QPixmap *pix_desc2;

        QFontMetrics      *fm;
        QList<int>         colw;
        QList<QColor>      colc;
        QList<QString>     spec_sort;
        QStringList        head;
        int                rowh;
        int                plusw;
        int                showrow;
        QString            seek;
        bool               inscroll;

        QMap<int,QColor>   precolor;

    protected:
        /** Step one item up */
        bool stepUp(void);
        /** Step one item down */
        bool stepDown(void);
        /** Handle a mouse event */
        bool mouseNetClick(QMouseEvent *e,int rmode = NEED_UPDATE);

    protected:
        void paintEvent(QPaintEvent *e);
        void keyPressEvent(QKeyEvent *e);
        void mousePressEvent(QMouseEvent *e);
        void mouseReleaseEvent(QMouseEvent *e);
        void mouseDoubleClickEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);
        void wheelEvent(QWheelEvent *e);
        void resizeEvent(QResizeEvent *e);

    signals:
        /** This signal is activated if the user click on an item.
         *  The signal will pass the key value of the activated item */
        void activateItem(const QString& s);
        /** This signal is activated if the user right click on an item.
         *  The signal will pass the key value of the activated item */
        void alternateActivateItem(const QString& s);
        /** This signal is activated if the user changed the current element.
         *  The signal will pass the key value of the new current item */
        void itemChanged(const QString& s);
};

/** One item (record) of the HTableBrowser. Every row in HTableBrowser correspond to a HTableBrowserElement.
 *  When you add a new element to the browser you have to create this kind of object.
 *  @see HTableBrowser() */
class HTableBrowserElement
{
    public:
        int index; ///< Index of the element
        QString key; ///< Key value of the element
        QStringList *rows; ///< The visible data of the element

        bool owncolor; ///< If this attribute is true the element has own color
        int ocr,ocg,ocb; ///< If the owncolor is true these variables holds the color

        /** DO NOT MODIFY OR USE THIS ATTRIBUTE! IT CONTROLLS THE PIXMAP CACHE. */
        QPixmap *cachePixmap;

    public:
        /** Creates an element. You have to specify the key of the element and a QString array which will be the visible fields.
         *  @param k the key of the item
         *  @param c the QString array of the visible field values. */
        HTableBrowserElement(QString k,QStringList *c);

        /** Creates an element. You have to specify the key of the element and at least one visible value.
         *  You can specify the other visible value fields by adding more parameters.
         *  (Maximum 10 with this constructor. If you would like to more use the other constructor)
         *  @param k the key of the item */
        HTableBrowserElement(QString k,QString v1,QString v2="",QString v3="",QString v4="",QString v5="",
                             QString v6="",QString v7="",QString v8="",QString v9="");
        /** Desctructor */
        ~HTableBrowserElement(void);

        /** Returns the displayed value of the the i index column. */
        QString operator[](int i);
        /** Returns the displayed value of the the i index column. */
        QString getCVal(int i) { return (*rows)[i]; }

        /** Redefine the color of the element (Every cell in the row)
         *  If you don't call this function the cells are colored according to the column data.
         *  @return It returns the pointer of HTableBrowserElement object (this)*/
        HTableBrowserElement * setColor(int r,int g,int b);
        /** Clears the redefined color data
         *  @see setColor()
         *  @return It returns the pointer of HTableBrowserElement object (this)*/
        HTableBrowserElement * clearColor(void);

        /** Returns the number of columns. */
        int size(void);

        /** Swap this HTableBrowserElement element with the parameter passed element.
         *  The sort function of the HTableBrowser uses this function */
        void swapWith(HTableBrowserElement *x);

        HTableBrowserElement& operator=(HTableBrowserElement& e);
        HTableBrowserElement& operator=(HTableBrowserElement* e);

    public:
        HTableBrowserElement *next; ///< Pointer to the next element
        HTableBrowserElement *prev; ///< Pointer to the previous element
};

/** HPlainDataMatrix is a GUI viewer class for a HPlainDataMatrix object.
 *  Shows the data in a HTableBrowser, and put a xml output button and a print preview button above the lister.
 *  It can put an optional html output button too, if the content have html output.
 *  (The html content will be generated by HPlainDataMatrix::getContentAsHtml()
 *   the html content will be generated with the following parameters: "html center" )
 *  @see HPlainDataMatrix
 *  @see HPlainDMD */
class HDispPlainDataMatrix : public QFrame
{
    Q_OBJECT

    protected:
        QString genhtml_parameters;
        HPlainDataMatrix  *data;
        HTableBrowser     *list;
        int deletedata;     ///< Data allocation handling mode

    public:
        /** Creates a HDispPlainDataMatrix object
         *  @param parent the parent GUI widget (QWidget descendant)
         *  @param d the metadata object to show. This case it means a HPlainDataMatrix object
         *  @param htmlmode if this parameter true, the html output button will be shown.
         *      (The html content will be generated by HPlainDataMatrix::getContentAsHtml() with parameters "html center" )
         *  @param keyfield you can specify the key of the HTableBrowser elements.
         *      This is important when the user clicks on an element. That case the "activateItem" signal will be emitted which
         *      tell the key of the activated item.
         *      The HPlainDataMatrix class also have a keyfield attributa which is overrides
         *      this parameter except if it is leaved -2 (which is the default)
         *      This keyfield can be:
         *      -   keyfield = -1 a new key will be generated, which will be the index number of the rows.
         *      -   keyfield >= 0 means the keyfield indexed column will be the index.
         *  @param ddata :\copydoc DONT_DELETE_DATA      */
        HDispPlainDataMatrix(QWidget *parent,HBase *d,bool htmlmode=false,int keyfield=-1,int ddata=DONT_DELETE_DATA);
        /** Destructor */
        ~HDispPlainDataMatrix(void);

    public:
        /** Sets html generation parameters.
         *  The html content will be generated by HPlainDataMatrix::getContentAsHtml()
         *  with parameters "html center" by default. You can redefine this parameterstring with this function */
        void setGenHtmlParameters(QString p) { genhtml_parameters = p; }

        /** Sets the color of the specified column */
        void setColumnColor(int col,QColor color) { list->setColumnColor(col,color); }

    public slots:
        /** Shows the print preview dialog */
        int dialogPrint(void);
        /** Shows the xml output dialog */
        int dialogXml(void);
        /** Shows the html view dialog */
        int dialogHtmlShow(void);

        /** Activate the key "k" element */
        int slotActivateItem(const QString k);
        /** Alternate activate the key "s" element */
        int itemAlternateActivated(const QString& s);

    signals:
        /** The user activated (clicked) on the following keyed element "k" */
        void activateItem(QString k);
};

/** A dialog box class which contains a HDispPlainDataMatrix class.
 *  Use this class if you would like to show a HPlainDataMatrix in a popupped dialog. \n
 *  This dialog can save the data to Excel XML format,
 *  can generate html form of the data which can be saved or opened in an outer editor( eg OpenOffice)\n
 *  A screenshot of this dialog with a HPlainDatamatrix (This dialog will appear) :
 *  \image  html pictures_doc/hplaindmd.png
 *  If the html export is enabled the dialog will show this:\n
 *  \image  html pictures_doc/hplaindmd_extra.png
    This code below shows the way to show the dialog above.\n
    \code
    // Creates a HPlainDataMatrix object
    HPlainDataMatrix *m=new HPlainDataMatrix(3);
    m->setHeader("Header1","Header2","Header3");

    m->addRowStr("Red","Brown","Green");
    m->addRowStr("Apple","Nut","Pear");
    m->setTitle("Title");

    //Show the HPlainDataMatrix object with HPlainDMD
    HPlainDMD *d = new HPlainDMD(this,m,true);
    d->setWindowTitle("HPlainDMD");
    d->setWindowIcon(QIcon());
    d->exec();

    //Freeing memory
    delete d;
    delete m;
    \endcode
  */
class HPlainDMD : public QDialog
{
    Q_OBJECT

    private:
        HDispPlainDataMatrix * dpdm;
    public:
        /** Creates a HPlainDMD
         *  See the parameters of HDispPlainDataMatrix::HDispPlainDataMatrix() */
        HPlainDMD(QWidget *parent,HBase *d,bool htmlmode=false);
        /** Destructor */
        ~HPlainDMD(void);

        /** Returns the HDispPlainDataMatrix object working inside */
        HDispPlainDataMatrix *getHDispPlainDataMatrix(void) { return dpdm; }

};

/** HDispTable is the GUI class to view/manipulate the HTable objects.\n
 *  You have to pass the HTable in parameter, and the HDispTable will automatically build the necessary gui.
 *  This is a QFrame descendant, so you can put it to layouts, widgets, dialogs.
 *  \image  html pictures_doc/hdisptable.png
 *  @see HTable */
class HDispTable : public QFrame
{
    Q_OBJECT

    protected:

        bool ro_mask;       ///< Readonly mask
        int datamode;       ///< Data handling mode
        int deletedata;     ///< Data allocation handling mode
        HTable *data;       ///< The HTable data pointer;
        QVBoxLayout *layout;

    public:
        /** Creates a HDispTable
         *  @param parent the parent GUI widget (QWidget descendant)
         *  @param d  the metadata object to show. This case it means a HTable object
         *  @param datamodep :\copydoc KEEP_ALL
         *  @param ddata :\copydoc DONT_DELETE_DATA */
        HDispTable(QWidget *parent,HTable *d,int datamodep = FULL_CLEAN,int ddata=DONT_DELETE_DATA);
        /** Destructor */
        ~HDispTable(void);

        /** Returns the metadata object */
        HTable *getDataClass(void) { return data;};

    public slots:

        /** This slot is activated by the metadata object, when the data modified in the memory.
         *  This means, if this function is called, the gui will re-read and show the new values of the metadata class. */
        int updateDisplay(void);

        /** Shows the print preview dialog */
        int dialogPrint(void);
        /** Shows the xml output dialog */
        int dialogXml(void);

        /** Adds a stretchable part to the end of the table. Useful that case when the table have smaller height than
         *  the dialog. If no stretch field the dialog will stretch all fields height to match the sizes.
         *  With this stretch field, you can prevent the height changing of the other data fields when the dialog height is grows. */
        int addStretchToEnd(void);

    signals:
        /** This signal is used to notify the HDispDataField descendant classes.
         *  It passes the refresh request. */
        void updateDisplaySignal(void);
};

/** HDispList is the GUI class to view/manipulate the HList objects.\n
 *  You have to pass the HList in parameter, and the HDispList will automatically build the necessary gui.
 *  This class is uses a HTableBrowser to show the data.
 *  This is a QFrame descendant, so you can put it to layouts, widgets, dialogs.\n
 *  The colors of the column is came from the metadata object (From HDataField -s).
 *  \image  html pictures_doc/htablebrowser.png
 *  @see HList */
class HDispList : public QFrame
{
    Q_OBJECT

    protected:
        int datamode;   ///< Data handling mode
        int deletedata; ///< Data allocation handling mode
        HList  *data;   ///< The HList data pointer
        HTableBrowser *list; ///< The HTableBrowser object.

    public:
        /** Creates a HDispList
         *  @param parent the parent GUI widget (QWidget descendant)
         *  @param d  the metadata object to show. This case it means a HList object
         *  @param datamodep :\copydoc KEEP_ALL
         *  @param ddata :\copydoc DONT_DELETE_DATA */
        HDispList(QWidget *parent,HList *d,int datamodep = FULL_CLEAN,int ddata=DONT_DELETE_DATA);
        /** Destructor */
        ~HDispList(void);

        /** Returns the metadata object */
        HList *getDataClass(void) { return data;}
        /** Returns the HTableBrowser instance working inside */
        HTableBrowser *getBrowserObjectClass(void) { return list; }

    public slots:

        /** This slot is activated by the metadata object, when the data modified in the memory.
         *  This means, if this function is called, the gui will re-read and show the new values of the metadata class. */
        int updateDisplay(void);

        /** Shows the print preview dialog */
        int dialogPrint(void);
        /** Shows the xml output dialog */
        int dialogXml(void);

        /** Sets the cursor to the first element, and refresh the display */
        int toBegin() { list->cursorToBegin(); return updateDisplay(); }
        /** Sets the cursor to the last element, and refresh the display */
        int toEnd()   { list->cursorToEnd();   return updateDisplay(); }

        /** Sets the sorting of the HTableBrowser
         *  @see HTableBrowser::sort()  */
        int sortByColumn(int col);

        /** This slot is received the itemActivated signal of HTableBrowser.
         *  It passes down the action to HList. Don't use it, don't call directly! */
        int itemActivated(const QString& s);
        /** This slot is received the alternateActivateItem signal of HTableBrowser.
         *  It passes down the action to HList. Don't use it, don't call directly! */
        int itemAlternateActivated(const QString& s);
        /** This slot is received the itemChanged signal of HTableBrowser. Don't use it */
        int litemChanged(const QString& s);

    signals:
        /** This signal is emitted when the current item is changed. (When the cursor moves) */
        void listItemChanged(void);

    protected:
        void keyPressEvent(QKeyEvent *e);

};

// ///////////////////////////////////////////////////////////////////////////////
// / data fields /////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

/** HDispDataField is the correspond GUI class of HDataField meta-data class. This is the "universal" gui data field class.
 *  The meda-data classes will control this class automatically.\n
 *  Because you don't need to use the HDataField directly, you don't need to use neither this class directly.\n
 *  Other gui data field classes will be inherited from here. \n
 *  Every HDataField descendant have a HDispDataField descendant class.
 *  @see HDataField     */
class HDispDataField : public QFrame
{
    Q_OBJECT

    protected:
        HDataField *data;
        bool ro_mask;
        bool tbool;

        QHBoxLayout *sublayout;
    public:
        static QHBoxLayout *static_sublayout;

    public:
        HDispDataField(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispDataField(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int updateDisplaySlot(void);
        int timedUpdateSlot(void);

    public slots:
        int enableControlSlot(void) { enableControl(); return 0; }
        int disableControlSlot(void) { disableControl(); return 0; }

    protected:
        void paintEvent(QPaintEvent *e);

    private:
        int  blink;

};

/** HDispKey is the correspond GUI class of HKey meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HKey   */
class HDispKey : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel *explain,*key;

    public:
        HDispKey(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispKey(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);

};

/** HDispSmallText is the correspond GUI class of HSmallText meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HSmallText   */
class HDispSmallText : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        QLineEdit *edtext;
        QLabel    *nedtext;
        QLabel    *tailtext;

    public:
        HDispSmallText(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispSmallText(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int textUpdate(const QString& n);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispLargeText is the correspond GUI class of HLargeText meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HLargeText   */
class HDispLargeText : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel          *explain;
        QTextEdit       *edtext;
        QTextEdit       *nedtext;
    public:
        HDispLargeText(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispLargeText(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int textUpdate(void);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispCharHash is the correspond GUI class of HCharHash meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HCharHash   */
class HDispCharHash : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        QComboBox *edval;
        QLabel    *nedtext;
        QLabel    *tailtext;
    public:
        HDispCharHash(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispCharHash(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int itemSelected(int index);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispNumHash is the correspond GUI class of HNumHash meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HNumHash   */
class HDispNumHash : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        QComboBox *edval;
        QLabel    *nedtext;
        QLabel    *tailtext;
    public:
        HDispNumHash(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispNumHash(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int itemSelected(int index);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispNumber is the correspond GUI class of HNumber meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HNumber   */
class HDispNumber : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        HSpinBox  *edval;
        QLabel    *nedtext;
        QLabel    *tailtext;
    public:
        HDispNumber(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispNumber(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int valueUpdate(int value);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispStatic is the correspond GUI class of HStatic meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HStatic   */
class HDispStatic : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        QLabel    *nedtext;
    public:
        HDispStatic(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispStatic(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispCheck is the correspond GUI class of HCheck meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HCheck   */
class HDispCheck : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel *explain;
        QCheckBox *edval;
        QLabel    *nedtext;

    public:
        HDispCheck(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispCheck(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int valueUpdate(void);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispFloating is the correspond GUI class of HFloating meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HFloating   */
class HDispFloating : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        QLineEdit *edtext;
        QLabel    *nedtext;
        QLabel    *tailtext;

    public:
        HDispFloating(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispFloating(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int textUpdate(const QString& n);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispDate is the correspond GUI class of HDate meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HDate   */
class HDispDate : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel *explain;
        QDateEdit *edval;
        QCheckBox *unk;
        QLabel  *nedtext;

    public:
        HDispDate(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispDate(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

    public slots:
        int valueUnkUpdate(void);
        int valueUpdate(const QDate& d);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

/** HDispTimestamp is the correspond GUI class of HTimestamp meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HTimestamp   */
class HDispTimestamp : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        QLabel    *nedtext;

    public:
        HDispTimestamp(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispTimestamp(void);

        virtual void enableControl(void);
        virtual void disableControl(void);


    public slots:
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);
};

class SqlChooseDialog;
/** HDispSqlChoose is the correspond GUI class of HSqlChoose meta-data class.
 *  The meda-data classes will control this class automatically.
 *  @see HSqlChoose   */
class HDispSqlChoose : public HDispDataField
{
    Q_OBJECT

    protected:
        QLabel    *explain;
        QComboBox *edval;
        QLabel    *nedtext;
        QToolButton *rfbutt;
        QPushButton *pbutt;

        SqlChooseDialog *pupup_dialog;

    public:
        HDispSqlChoose(QWidget *parent,HDataField *d,bool _ro_mask=false);
        ~HDispSqlChoose(void);

        virtual void enableControl(void);
        virtual void disableControl(void);

        void setToolbarbuttonSignalOn(QString text);
        void setToolbarbuttonSignalOff();

        HSqlChoose *getHSqlChoose(void) { return ((HSqlChoose *)data); }

    public slots:
        int itemSelected(int index);
        int refresh(void);
        int updateDisplaySlot(void);
        int updateStaticGuiElementsSlot(void);

        int popupSelector(void);
        int itemSelected(const QString& s);


};

/* NOT DOCUMENTED: This is not a disp field class. This class is part of the HDispSqlChoose.
 *  If the HSqlChoose in disabled easy mode, (popup selection) this class will be the popup selection dialog.
 *  It connects to the slot of HDispSqlChoose and notify the selected record this way. */
class SqlChooseDialog : public QDialog
{
    Q_OBJECT

    private:
        QString popupped_key;

    public:
        SqlChooseDialog( HDispSqlChoose* p);
        ~SqlChooseDialog();

        HTableBrowser* table;
        QPushButton* button;
        bool disabled_recvkey;
        bool key_received;
        QString received_key;

    protected:
        HDispSqlChoose* parent;

        QVBoxLayout* SqlChooseDialogLayout;
        QHBoxLayout* layout1;

        bool have_extra_func;

    public slots:
        int nullButtonPressed(void);
        int buttonPressed(void);
        int listviewitemSelected(const QString& s);
        int updateList(void);

        /*  This slot is called when the plus toolbarbutton is pushed. (See HSqlChoose::addToolButton())
         *  This slot is emits the callToolButtonHandler signal (down here) which is can be connected to the handler slot
         *  which do the effective work. (That external work which defined by you in HSqlChoose::addToolButton() ) \n
         *  This event chain is designed because of this class need a meta-data refresh after you do the external work.
         *  That slot (which you define) can change the list of values, so the list need to be updated before the user select an item. \n
         *  That's why I need a passing middle slot to grab the event when the external work finishes.
         *  If I would directly connect to the toolbutton signal, the class could not refresh the values in the necessary time */
        int toolbarButtonClicked(void);

        int receivedASelectedKey(QString key);

        int popUpMenuActivated(const QString& key);

        /*  The extra popup menu activates this slots.
         *  Because the popup menu only can call slots without key,
         *  this slots are emits the sigToExtraFunc signals (down here) which can pass the selected key values too.
         *  That signals are connected with the target slots. */
        int popup_catch_0(void);
        int popup_catch_1(void);
        int popup_catch_2(void);
        int popup_catch_3(void);
        int popup_catch_4(void);

        int captureNotify(QString tblname);

    signals:
        void callToolButtonHandler(void);
        void itemSelected(const QString& s);    //this connected to the parents

        void sigToExtraFunc_0(QString key);
        void sigToExtraFunc_1(QString key);
        void sigToExtraFunc_2(QString key);
        void sigToExtraFunc_3(QString key);
        void sigToExtraFunc_4(QString key);
};

/////////////////////////////////////////////////////////////////////////////////////////////

/** This is a helper class of HDispPlainDataMatrix and HPlainDMD.
 *  That classes have a html view option. If the html option requested, the HShowPrintHtml class will show the generated html.\n
 *  This class carry out a dialog with a html viewer, and a Print button, a Save button, and an external editor button.
 *  The external editor will be described in EXT_EDIT_PROCESS define.
 *  @see HDispPlainDataMatrix
 *  @see HPlainDMD
 *  @see HPlainDataMatrix   */
class HShowPrintHtml : public QDialog
{
    Q_OBJECT

private:
    QString html;

#ifdef WEBKITBASED_HTMLSHOW
    QWebView *te;
#else
    QTextEdit *te;
#endif

public:
    HShowPrintHtml(QWidget *parent);
    ~HShowPrintHtml(void);

public:
    void setContent(QString str);
    void setWinTitle(QString t);

public slots:
    void slotPrint(void);
    void slotStart(void);
    void slotSave(void);
};

// ///////////////////////////////////////////////////////////////////////////////////////////
// // OTHER CLASSES //////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////////////////

/** HPleaseWaitWindow is an animated work indicator window. You can easily start this animatid window, with calling
 *  an everywhere available static function. You don't need to care about the speed of animation, it handled automatically.
 *  \image  html pictures_doc/hpww.png
 *
 \code
    HPleaseWaitWindow::start();
    while(...) //long time work
    {
        do_the_job();
        ...
        HPleaseWaitWindow::step();
    }
    HPleaseWaitWindow::end();
 \endcode */
class HPleaseWaitWindow : public QWidget
 {
  Q_OBJECT

  private:
    int sizex,sizey;
    int lastx,lasty;
    int seq;
    int refreshTime;

    HPleaseWaitWindow(int sizex_=80,int sizey_=80,int refreshTime_=80);
    ~HPleaseWaitWindow();

  public slots:
    /** @see step() */
    void progress(void);

  protected:

    QTime t;
    void mouseMoveEvent(QMouseEvent *me);
    void mousePressEvent(QMouseEvent *me);

    /** You can redefine this function if you would line to change my stupid animation... */
    void paintEvent(QPaintEvent *pe);

  public:
      /** Pointer of the current HPleaseWaitWindow */
      static HPleaseWaitWindow *pww;

      /** Call this function to start and show a work indicator animated splash like window.
       *  @see start()  @see step()  @see end() */
      static HPleaseWaitWindow * start(void);
      /** Call this function to step the animation of the work indicator  window.
       *  If you don't call this function regulary the animation stops. Put this funtion to a work cycle to indicate the work.
       *  You don't need to care about the speed of animation, it handled automatically.
       *  @see start()  @see step()  @see end() */
      static void                step (void);
       /** Call this function to stop and close a work indicator animated splash line window.
       *  @see start()  @see step()  @see end() */
      static void                end  (void);
 };

/////////////////////////////////////////////////////////////////////////////////////////////

/** HDecorDialog is a decorated dialog box. Useful for special touch devides.
 *  This dialog is a descendat of the QDialog so you can use like that.
 *  -   Raster graphical (unique) look.
 *  -   Large border and header/title part. (For big fingers)
 *  -   You can set an automatic close time. For example if you don't move the mouse the dialog closes after 30 seconds.
 *
 *  You can see a HDecorDialog on the picture, with two HPressButton:
 *  \image html pictures_doc/hdecordialog.png
 *
 \code
    HDecorDialog *dd = new HDecorDialog(this,"DecorDialog",20);
    dd->frcolor = QColor(100,255,255);

    HPressButton *pb1=new HPressButton(dd,"First function","f1");
    HPressButton *pb2=new HPressButton(dd,"Second function","f2");
    pb1->setTextPointSize(15);
    pb2->setTextPointSize(15);
    QVBoxLayout *lay=new QVBoxLayout(dd);
    lay->setMargin(30);
    lay->setSpacing(10);

    lay->addSpacing(100);
    lay->addWidget(pb1);
    lay->addWidget(pb2);
    dd->exec();
 \endcode
 @see HPressButton  */
class HDecorDialog : public QDialog
{
    Q_OBJECT

    public:
        /** Creates a HDecorDialog
         *  @param parent the parent QWidget descendant
         *  @param title the title text of the dialog
         *  @param no_action_closetime If this parameter is greater then 0: The dialog is automatically closed after
         *      no_action_closetime second if no mouse moved or clicked.
         *      If the parameter is 0 the dialog doesn't closed automatically. */
        HDecorDialog(QWidget *parent,QString title,int no_action_closetime);

    public:
        /** Sets the font of the window title */
        void setTitleFont(QFont f);
        /** If this attribute is true the dialog is closed when the user click anywhere.
         *  Useful for information windows */
        bool anywhere_click_close;
        /** By default this attrubute is false. If this attribute is false the close timeout counter is resetted every time
         *  when the mouse moves. Otherwise the dialog closes after the coundown. */
        bool dont_reset_timer;

        /** Background color */
        QColor bgcolor;
        /** Width of the frame */
        int framewidth;
        /** Color of the dialog frame */
        QColor frcolor;
        /** Height of the window title */
        int titleheight;
        /** Size of the close X */
        int closerwidth;
        QColor titlecolor;
        QColor indcolor;
        /** Thick of the close X */
        int crossthick;
        int crossmargin;

    private slots:
        int incr_t(void);

    protected:
        void paintEvent(QPaintEvent *pe);

        void mousePressEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);
        void keyPressEvent(QKeyEvent *e);

    private:
        QFont  titlefont;
        QString title;
        int no_action_closetime;
        int end_t;
        int t_t;
        int t_time;
};


class HPressButtonSizeSyncronizer;
/** HPressButton is a large pusbutton optimised for touchscreens.
 *  \image html pictures_doc/hdecordialog.png
 *  You can define a code for every button, and when the button is pressed the clickedCode signal will pass the code.\n
 *  Optionally, you can define a size syncronizer object,
 *  which can control the size of a group of HPressButton-s and equal the sizes.     */
class HPressButton : public QFrame
{
    Q_OBJECT

    public:
        /** Creates a HPressButton
         *  @param parent the parent QWidget descendant
         *  @param text The text of the button.
         *  @param code The code of the button. Passed by clickedCode signal. */
        HPressButton(QWidget *parent,QString text,QString code="");
        /** Destructor */
        ~HPressButton(void);

        /** Sets the color of the button */
        void setColor(int red,int green,int blue,int mode = 0);
        /** Width of the pressing effect of the button */
        void setEffect(int effectwidth);
        /** Sets the margin of the button text */
        void setMargin(int margin);
        /** Sets the size of the button text */
        void setTextPointSize(int size);
        /** Sets the amount a smaller size when pushed down */
        void setDownSmaller(int sdm_pixel);
        /** Get the amount a smaller size when pushed down */
        int getDownSmaller(void);

        QColor downColor;
        QColor borderColor;

    private:
        bool enablestat;
        bool hidestat;
        int cmargin;
        int c3deffect;
        int downsmaller;

        QFont font,downFont;
        bool willmid;
        bool mid;
        bool down;
        QString textstr,idstr;

        int r,g,b;
        int rAlt,gAlt,bAlt;

        void calcSize();
        //static int mx,my;
        HPressButtonSizeSyncronizer* size_sync;
    public:

        /** Creates and returns a HPressButtonSizeSyncronizer object.
         *  Every HPressButton which have a same HPressButtonSizeSyncronizer will be equalzed size.
         *  You can set a HPressButtonSizeSyncronizer to a HPressButton with the setSizeSyncronizer() function
         *  @see setSizeSyncronizer()
         *  @see getSizeSyncronizer()   */
        static HPressButtonSizeSyncronizer* createSizeSyncronizer(void);

        /** Returns the currently set HPressButtonSizeSyncronizer object. Returns NULL if not set.
         *  @see createSizeSyncronizer()    @see setSizeSyncronizer()   */
        HPressButtonSizeSyncronizer* getSizeSyncronizer(void)    { return size_sync; }
        /** Sets the HPressButtonSizeSyncronizer object.
         *  @see createSizeSyncronizer()    @see getSizeSyncronizer()   */
        void setSizeSyncronizer(HPressButtonSizeSyncronizer* ss) { size_sync = ss;   }

    public slots:
        /** Recalculate the sizes of the HPressButtons which have same HPressButtonSizeSyncronizer object */
        int recalcSize();
        /** Hide the button */
        int setHide();
        /** Show/Unhide the button */
        int setShow();
        /** Set if the button is enabled or not.
         *  If the button is not enabled the text is red crossed and the button can't pressed.
         *  Note: If you like to hide the button use setHide()  */
        int setEnabled(bool enable);

    protected:
        void paintEvent(QPaintEvent *e);

        void keyPressEvent(QKeyEvent *e);
        void keyReleaseEvent(QKeyEvent *e);

        void mousePressEvent(QMouseEvent *e);
        void mouseReleaseEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);

    signals:
        /** This signal is emitted if the button is pressed */
        void clicked(void);
        /** This signal is emitted if the button is pressed, and the button code is passed too. */
        void clickedCode(QString code);
};

/** HPressButtonSizeSyncronizer is the helper class of HPressButton. Don't use it directly.
 *  @see HPressButton */
class HPressButtonSizeSyncronizer
{
    private:
        HPressButtonSizeSyncronizer()
        {
            mx = 0;
            my = 0;
        }

    private:
        int mx,my;
    friend class HPressButton;
};

/** This class can connect a HDynTable object to a QTableWidget object.
 *  It initializes the QTableWidget object the correct way (See HDynTable describe)
 *  It keeps the two object's connected and handles the data changes until it's deleted.
 *  See the using example in HDynTable describe.
 *  @see HDynTable */
class HDynTableDispConnector : public QObject
{
    Q_OBJECT

public:
    /** Initializes the connection between the HDynTable and the QTableWidget */
    HDynTableDispConnector(HDynTable *dynt,QTableWidget *qtw);
    ~HDynTableDispConnector(void);

private:
    typedef struct
    {
        QString name;
        int r,c;
        QTableWidgetItem *glink;
    } cellData;

    HDynTable *dynt;
    QList<cellData> cells;
    bool ccprogress;
    bool rfprogress;

public slots:
    /** It refresh the gui from the HDynTable */
    int refreshGui(void);
    /** The QTableWidget's cellChanged signal will be connected to this slot. You don't need to use. */
    int slotCellChanged(int row,int column);

signals:
    /** Emitted when the user changed the data in the gui.
     *  When thi signal is emitted the data was already changed in the HDynTable. */
    void dataChangedByGui(void);

};

/* @} */

#endif
