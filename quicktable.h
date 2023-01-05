/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   quicktable.h
*/

#ifndef GSAFE__QUICKTABLE_HEADER_FILE_X_
#define GSAFE__QUICKTABLE_HEADER_FILE_X_

#include <QtCore>
#include <QFrame>
#include <QDialog>

#include <data.h>
#include <dm.h>
#include <ftypes.h>


class HQuickTableElement;

/* NOT DOCUMENTED: Helper class of HQuickTable. Do not use directly.
 * The HQuickTable class uses this to qsort the elements. */
class HQuickTableElementPair
{
    public:
        HQuickTableElementPair(HQuickTableElement *lp,HQuickTableElement *rp)
            { left = lp; right = rp; }

        HQuickTableElement *left;
        HQuickTableElement *right;
};

/** HQuickTable is a table view class, which can show the list of data groups (they can be a records).\n
 *  This browser is designed to show/handle really big data tables without slow down.\n
 *  Shows N column and M rows.\n
 *  This viewer is a readonly viewer, you can't edit the content with it.
 *  Hovewer the list is readonly there is a cursor which marked an element to currently active.
 *  The HQuickTable holds the data in a double linked list, memorize the beginning and the end of the displayed data part,
 *  so the browsing will be same speed, when the browser hold a dozen elements or more thousends.
 *  The elements are HQuickTableElement objects. Every record have one key value, and N(column count) displayed value.
 *  -   The user can select an element. The event signal will pass the key value of this element.
 *  -   The user can search in the browser by typing letters. The matched letters are indicated until the search is finished.
 *      (In the example picture the user already pressed a "De" string)
 *      The search is finished if the user click to another cell, or moves the cursor to a different position.
 *  -   The user can sort the elements by clicking on the header. (Can sort by two column, first and secondary)
 *  -   You can set different colors for columns.
 *
 *  \image  html pictures_doc/htablebrowser.png
 *  \code
    HQuickTable *htb = new HQuickTable(parentwidget);
    htb->setHeadTexts(QString("Fruit;Color").split(";",QString::SkipEmptyParts));
    htb->addElement(new HQuickTableElement("001" , "Apple","Red"));
    htb->addElement(new HQuickTableElement("002" , "Pear","Green"));
    htb->addElement(new HQuickTableElement("003" , "Strawberry","Red"));
    \endcode
 *
 *  In the background this data browser uses bi-directional linked list to store the data, and handles
 *  the viewed area separatedly from the full list.
 *  There is not matter the size of the full list when drawing/scrolling or moving in the list
 *  but the displayed/drawed size only. This drawing is also fastened by pixmap caches, so a full screen scrollig is fast enough.
 *  The sorting is achived by quick sort algorithm to be the fastest as possible.
 *
 *  @see HQuickTableElement   */
class HQuickTable : public QFrame
{
    Q_OBJECT

    public:
        /** Creates a HQuickTable class
         *  @param parent the parent widget of the HQuickTable */
        HQuickTable(QWidget *parent);
        /** Destructor. Deletes the elements too. */
        ~HQuickTable(void);

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
            HQuickTable *htb = HQuickTable(parentwidget);
            HQuickTableElement *e = NULL;
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

        /** Returns the HQuickTableElement pointer in the position of the query cursor.
         *  After returning the pointer it steps to the next position.
         *  If we have already reached the end of the list NULL returned.\n
         *  You will get a direct pointer, don't delete it!
         *  @see queryToStart() @see queryToTop() @see queryToCurrent() */
        HQuickTableElement *queryNextElement(void);
        /** Returns the displayed values (in a QStringList pointer) of the HQuickTableElement
         *  in the position of the query cursor.
         *  After returning the values it steps to the next position.
         *  If we have already reached the end of the list NULL returned.\n
         *  You will get a new allocated object, you have to delete it!
         *  @see queryToStart() @see queryToTop() @see queryToCurrent() */
        QStringList          *queryNextValues(void);
        /** Returns the key (in a QString pointer) of the HQuickTableElement
         *  in the position of the query cursor.
         *  After returning the key value it steps to the next position.
         *  If we have already reached the end of the list NULL returned.\n
         *  You will get a new allocated object, you have to delete it!
         *  @see queryToStart() @see queryToTop() @see queryToCurrent() */
        QString              *queryNextKey(void);

        /** Returns the number of the elements in the table (Number of rows) */
        int numberOfElements(void);

        /** Returns the HQuickTableElement pointer in the position of the required index.
         *  If the element doesn't reachable NULL returned.\n
         *  You will get a direct pointer, don't delete it!
         *  @param n the required index. (The first element is 0)
         *  @see elementByIndex() @see valuesByIndex() @see keyByIndex() @see numberOfElements() */
        HQuickTableElement *elementByIndex(int n);
        /** Returns the displayed values (in a QStringList pointer) of HQuickTableElement
         *  in the position of the required index.
         *  If the element doesn't reachable NULL returned.\n
         *  You will get a new allocated object, you have to delete it!
         *  @param n the required index. (The first element is 0)
         *  @see elementByIndex() @see valuesByIndex() @see keyByIndex() @see numberOfElements()  */
        QStringList          *valuesByIndex(int n);
        /** Returns the key (in a QString pointer) of HQuickTableElement
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
         *  @see HQuickTableElement   */
        int addElement(HQuickTableElement *e);

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
        HQuickTableElement * currentElement(void);
        /** Returns the key value of the current element */
        QString                currentKey(void);
        /** Returns the first element on the display. (Top element)
         *  Don't delete the object! */
        HQuickTableElement * topElement(void);
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
        std::list<HQuickTableElement *> cachedElements;
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
        void q_sort_run(HQuickTableElementPair *p);

        bool ch(bool m,HQuickTableElement *a,HQuickTableElement *b);
        int  ch_dropchar(QString s);
        int  ch_number(QString s);
        double ch_float(QString s);

        QStack<HQuickTableElementPair *> *q_sort_stack;

        QString lastkey;

        int oldw,oldh,old_col_scroll;

        QMutex cacheMutex;

    protected:
        QString last_right_clicked;
        int number;
        int curc;
        HQuickTableElement *start,*end,*top,*cur,*query_cursor;
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
        QMap<int,QString>  spec_sort;
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

        struct PointLocatorResult
        {
            PointLocatorResult(bool need_update_val,bool click_on_cell_val)
            {
                need_update = need_update_val;
                click_on_cell = click_on_cell_val;
            }
            bool need_update,click_on_cell;
        };

        PointLocatorResult pointLocator(QMouseEvent *e);
        bool inX(int a,int b,QMouseEvent *e);
        bool inY(int a,int b,QMouseEvent *e);

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

/** One item (record) of the HQuickTable. Every row in HQuickTable correspond to a HQuickTableElement.
 *  When you add a new element to the browser you have to create this kind of object.
 *  @see HQuickTable() */
class HQuickTableElement
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
        HQuickTableElement(QString k,QStringList *c);

        /** Creates an element. You have to specify the key of the element and at least one visible value.
         *  You can specify the other visible value fields by adding more parameters.
         *  (Maximum 10 with this constructor. If you would like to more use the other constructor)
         *  @param k the key of the item */
        HQuickTableElement(QString k,QString v1,QString v2="",QString v3="",QString v4="",QString v5="",
                             QString v6="",QString v7="",QString v8="",QString v9="");
        /** Desctructor */
        ~HQuickTableElement(void);

        /** Returns the displayed value of the the i index column. */
        QString operator[](int i);
        /** Returns the displayed value of the the i index column. */
        QString getCVal(int i) { return (*rows)[i]; }

        /** Redefine the color of the element (Every cell in the row)
         *  If you don't call this function the cells are colored according to the column data.
         *  @return It returns the pointer of HQuickTableElement object (this)*/
        HQuickTableElement * setColor(int r,int g,int b);
        /** Clears the redefined color data
         *  @see setColor()
         *  @return It returns the pointer of HQuickTableElement object (this)*/
        HQuickTableElement * clearColor(void);

        /** Returns the number of columns. */
        int size(void);

        /** Swap this HQuickTableElement element with the parameter passed element.
         *  The sort function of the HQuickTable uses this function */
        void swapWith(HQuickTableElement *x);

        HQuickTableElement& operator=(HQuickTableElement& e);
        HQuickTableElement& operator=(HQuickTableElement* e);

    public:
        HQuickTableElement *next; ///< Pointer to the next element
        HQuickTableElement *prev; ///< Pointer to the previous element
};


#endif

//End of gSAFE quicktable.h
