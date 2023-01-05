/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    printlib.h
*/

#ifndef GSAFE__PRINTOUT_LIB_HEADER__
#define GSAFE__PRINTOUT_LIB_HEADER__

#include <QtCore>
#include <QtGui>

#ifndef COMPILED_WITH_QT4X
#include <QtWidgets>
#include <QPrinter>
#include <QPrintDialog>
#endif

/** \defgroup printlib printlib */
/*  @{  */

class HBase;
class HTableBase;
class HPrintTable;

/** HPreviewFrame is a print preview QFrame descendant which shows a HList or a HTable data
 *  before printing. You don't need to use this class directly. Recommend to use HPrintTable instead.
 *  @see HPrintTable    */
class HPreviewFrame : public QFrame
{
    Q_OBJECT
    
    private:
        int dx,dy;
        double scale;
        HPrintTable *pt;
        QPixmap *gr;

    public:
        int panx,pany;

    public:
        /** Creates a HPreviewFrame 
         *  @param p The related HPrintTable
         *  @param parent the QWidget descendant parent  */
        HPreviewFrame(HPrintTable *p,QWidget *parent);
        /** Destructor */
        ~HPreviewFrame(void);
        void reset(void);

    protected:
        void paintEvent(QPaintEvent *e);
        void wheelEvent(QWheelEvent *e);
        void keyPressEvent(QKeyEvent *e);

        void mousePressEvent(QMouseEvent *e);
        void mouseReleaseEvent(QMouseEvent *e);
        void mouseMoveEvent(QMouseEvent *e);
        void resizeEvent(QResizeEvent *e);
    
    signals:
        void scrollSignal(int x,int y);

};

/** HPrintTable is a print preview and printer dialog, which can print a HTable or HList or HPlainDataMatrix objects.
 *  When you create this object, a popup dialog will appear with a print preview.
 *  \image html pictures_doc/hprinttable.png
 *  This class will generate the printed document according the meta data class (HTable/HList/HPlainDataMatrix)
 *  and the data will came from these class too. 
 \code
    HTable *mytable=...
    ...
    HPrintTable *d = new HPrintTable(this,mytable);
    d->exec();
    delete d;
    ...
 \endcode
 * or you can print a result of a query
 \code
    HSqlHandler h;
    HPlainDataMatrix *pdm = h.submitNResultQuery(3,"SELECT name,age,address FROM peoples;","Error occured");
    HPrintTable *d = new HPrintTable(this,pdm);
    d->exec();
    delete d;
    delete pdm;
 \endcode

 @see HTable @see HList @see HPlainDataMatrix */
class HPrintTable : public QDialog
{
    Q_OBJECT

    protected:
        int page;      //page number (calculated)
        int pagerun;   //page number, during the drawing
        int cellh;     //cellheight
        QList<int> cellw; //cellwidth by column
        QList<int> cellrh; //cellheight by row
        int rownum,column; //row/column number

        bool hide;

        HBase *data;
        QVBoxLayout *layout;

        QLabel *pn;
        HPreviewFrame *preview;
        QScrollArea  *scrollp;
        QPrinter     *printer;

        QFont printfont;

    public:
        /** Creates a HPrintTable object
         *  @param parent the QWidget descendant parent
         *  @param d the meta-data class
         *  @param pf the front to print */
        HPrintTable(QWidget *parent,HBase *d,QFont *pf = NULL); 

    public slots:
        void scanIt  (void);
        int  drawIt  (QPainter *p,double scale=1.0,bool print=false);
        int  updateDisplay(void);
        int  printIt (void);

    protected:
        void scanList (void);
        void scanTable(void);
        void scanPlainDataMatrix(void);

        int drawList(QPainter *p,bool print=false);
        int drawTable(QPainter *p,bool print=false);
        int drawPlainDataMatrix(QPainter *p,bool print=false);
        int newPage  (QPainter *p,bool print=false);

    protected:
        void resizeEvent(QResizeEvent *e);
        void keyPressEvent(QKeyEvent *e);

    signals:
        /** The gui uses this signal to pass the refresh request to the HDataField descendants.
         *  This printing class won't use this signal, but I left this signal here avoid the connect error warnings. */
        void updateDisplaySignal(void);
};

/* @} */

#endif
