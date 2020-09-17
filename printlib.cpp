/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2018 Peter Deak  (hyper80@gmail.com)

    License: GPLv2  http://www.gnu.org/licenses/gpl-2.0.html

    printlib.cpp
*/

#include <QtCore>
#include <QtGui>

#define LMARGIN 30
#define RMARGIN 30
#define TMARGIN 40
#define BMARGIN 30

#define CELLHMARGIN 5
#define CELLVMARGIN 3

#define BREAKPOLICY Qt::TextWrapAnywhere

#define A4_X    750
#define A4_Y    1080

#define PRBGCOLOR QColor(80,80,80)

#define SHADOWPATTERN Qt::Dense2Pattern

#include "datalib.h"
#include "printlib.h"
#include "dconsole.h"


////////////////////////////////////////////////////////////////////////////////
/// PreviewTable ///////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

HPreviewFrame::HPreviewFrame(HPrintTable *p,QWidget *parent)
:QFrame(parent)
{
    pt = p;
    scale=0.50;

    gr = new QPixmap();
    QPalette palette;
    palette.setColor(QPalette::Window,PRBGCOLOR);
    palette.setColor(QPalette::WindowText,QColor(0,0,0));
    setPalette(palette);
    setAutoFillBackground(true);


    setFocusPolicy(Qt::StrongFocus);
    setFocus();
    setCursor(Qt::PointingHandCursor);
    resize(A4_X+100,A4_Y+100);
    
    repaint();
}

HPreviewFrame::~HPreviewFrame(void)
{
    delete gr;
}

void HPreviewFrame::reset(void)
{
    scale=0.5;
    panx = 0;
    pany = 0;
}

void HPreviewFrame::mousePressEvent(QMouseEvent *e)
{
    dx = e->x();
    dy = e->y();
}

void HPreviewFrame::mouseMoveEvent(QMouseEvent *e)
{
    panx -= dx - e->x();
    pany -= dy - e->y();
    dx = e->x();
    dy = e->y();
    repaint();
}

void HPreviewFrame::mouseReleaseEvent(QMouseEvent *e)
{
    panx -= dx - e->x();
    pany -= dy - e->y();
    repaint();
}

void HPreviewFrame::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}

void HPreviewFrame::wheelEvent(QWheelEvent *e)
{
    int delta;
#ifdef COMPILED_WITH_QT4X
    delta = e->delta();
#else
    delta = e->angleDelta().y();
#endif

    if(e->modifiers() != Qt::NoModifier)
    {
        if(delta > 0)
        {
                pany += 50; 
        }
        else
        {
                pany -= 50; 
        }
    }
    else
    {
        if(delta > 0)
        {
            //Max scale
            if(scale >= 3) return;
            scale += 0.1;

        }
        else
        {
            //Min scale
            if(scale <= 0.2) return;
            scale -= 0.1;
        }
    }
    repaint();
}

void HPreviewFrame::keyPressEvent(QKeyEvent *e)
{
    sdebug("*** HPreviewFrame::keyPressEvent ***");
        if(e->key() == Qt::Key_PageUp)
        {
            pany += A4_Y;   
            repaint();
            return;
        }

        if(e->key() == Qt::Key_PageDown)
        {
            pany -= A4_Y;   
            repaint();
            return;
        }

        if(e->key() == Qt::Key_S)
        {
            pany += 10; 
            repaint();
            return;
        }

        if(e->key() == Qt::Key_W)
        {
            pany -= 10; 
            repaint();
            return;
        }

        if(e->key() == Qt::Key_A)
        {
            panx -= 10; 
            repaint();
            return;
        }

        if(e->key() == Qt::Key_D)
        {
            panx += 10; 
            repaint();
            return;
        }
}   

void HPreviewFrame::paintEvent(QPaintEvent *e)
{
    
    QFrame::paintEvent(e);
    
    QPainter *p;
    p = new QPainter();
    p->begin(this);
    pt->drawIt(p,scale);
    p->end();
    delete p;
}

////////////////////////////////////////////////////////////////////////////////
///  HPtintTable  //////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
HPrintTable::HPrintTable(QWidget *parent,HBase *d,QFont *pf)
:QDialog(parent)
{
  setWindowTitle("Print preview...");
  page = 1;
  data = d;
  printer = NULL;
  cellh = 20;
  cellw.clear();
  cellrh.clear();
  rownum = 0;
  column = 0;
  hide = false;

  QString tTitle;
  if(pf==NULL)
      printfont = QApplication::font();
  else
      printfont = *pf;

  QVBoxLayout *vl;
  QHBoxLayout *hl;

  QPushButton *printButton,*refreshButton,*closeButton;

  vl = new QVBoxLayout( this );
    vl->setMargin(2);
    vl->setSpacing(2);
  hl = new QHBoxLayout( 0 );
    hl->setMargin(2);
    hl->setSpacing(2);
  
  printButton    = new QPushButton(this);  
    printButton->setText("Print");
  refreshButton  = new QPushButton(this);  
    refreshButton->setText("Refresh");
  closeButton    = new QPushButton(this);  
    closeButton->setText("Close");
  pn             = new QLabel(this); 

  if(data->getWhoami() == "HTable" || data->getWhoami() == "HList")
    tTitle = ((HTableBase *)data)->tableTitle();
  else if(data->getWhoami() == "HPlainDataMatrix")
      tTitle = ((HPlainDataMatrix *)data)->getTitle();
  pn->setText(" Print preview: "+tTitle);

  
  preview = new HPreviewFrame(this,this);
  preview->reset();
  
  
  QPalette palette;
  palette.setColor(QPalette::Window,PRBGCOLOR);
  preview->setPalette(palette);
  preview->setAutoFillBackground(true);
  preview->setFrameShape ( QFrame::StyledPanel );
  preview->setFrameShadow( QFrame::Raised );
  preview->setLineWidth( 4 ); 

  hl->addWidget(printButton);
  hl->addWidget(refreshButton);
  hl->addWidget(pn);
  hl->addStretch();
  hl->addWidget(closeButton);

  vl->addLayout(hl);
  vl->addWidget(preview);

  resize( QSize(525, 639).expandedTo(minimumSizeHint()) ); 

  connect(closeButton  ,SIGNAL(clicked()),this,SLOT(close()));
  connect(refreshButton,SIGNAL(clicked()),this,SLOT(updateDisplay()));
  connect(printButton  ,SIGNAL(clicked()),this,SLOT(printIt()));

  scanIt(); // Have to call before the drawing

  show();
  updateDisplay();
}

void HPrintTable::keyPressEvent(QKeyEvent *e)
{
        
        QWidget::keyPressEvent(e);
}   

void HPrintTable::resizeEvent(QResizeEvent *e)
{
    Q_UNUSED(e);
}

// Determine the necessary data to drawing
void HPrintTable::scanIt(void)
{
    if(data->getWhoami() == "HPlainDataMatrix")
        scanPlainDataMatrix();
    else if(data->getWhoami() == "HTable")
        scanTable();
    else if(data->getWhoami() == "HList")
        scanList();
    else
        error("HPrintTable::scanIt - Cannot draw item: Not supported type!");
}

void HPrintTable::scanPlainDataMatrix(void)
{
    sdebug("*** HPrintTable::scanPlainDataMatrix ***");
    /* 
      Calculates the following data
        cellh  - cellheight
        cellw  - cellwidth
        rownum - number of rows
        colnum - number of columns
        page   - page number
    */

    int pageY;
    int t;
    int i,j;
    int curr_cellh=0;
    QStringList *slist;
    slist = new QStringList();
    
    HPlainDataMatrix *data = (HPlainDataMatrix *) this->data; //hide the class variable with a local
    QPainter *p = new QPainter();

    
    cellh = 20;
    cellw.clear();
    cellrh.clear();
    rownum=0;
    column = -1;

    //CELLHEIGHT
    
    //cellh = ( QApplication::fontMetrics() ).height();
    cellh =  QFontMetrics(printfont).height();

    cellh += CELLVMARGIN*2;

    //pagenum=1
    page = 1;
    //futoY = upper header
    pageY = 50+cellh;

    //BODY
    data->firstRow();
    do
    {
        ++rownum;
        *slist = data->currentRowStr();
        if(column == -1) //first row
        {
            column = (int)slist->size();
            //Initializing a column number int list for the columnwidth
            for(i=0;i<column;i++)
                cellw.push_back(0);
        }
        else //other rows
        {
            if(column != (int)slist->size())
            {
                error("*** HPrintTable::scanList *** - found different column number in lines!");
                delete p;
                close();    
            }
        }
        
        //working on columns
        curr_cellh = cellh;
        for(i=0;i < column;++i)
        {
            //CELLA : slist[i]
            j =  QFontMetrics(printfont).QFONTMETRICS_STRING_HORIZONTAL_WIDTH( (*slist)[i] );

            //We can wrap
            if(data->getColumnPrintWrap(i)) 
            {
                int maxcellw = 
                    data->getColumnPrintMaxWidth(i);

                if(maxcellw == 0) 
                    maxcellw = 100; //TODO !!!! sould be an average !!! 

                //need line wrap
                if(j > maxcellw)
                {
                    t = QFontMetrics(printfont)
                                        .boundingRect(0,0,maxcellw
                                              ,A4_Y,Qt::AlignLeft | BREAKPOLICY,(*slist)[i]).height();
                
                    t+=2*CELLVMARGIN;
                    if(curr_cellh < t)
                        curr_cellh = t;
                    cellw[i] = maxcellw;

//sdebug(QString("LINE WRAP! Examined cellheight: %1 (+ CELLVMARGIN*2)  /%2/").arg( t ).arg(cellrh.size()));
                }
                else //the line wrap is possible but don't need
                {
                    if(j > cellw[i])
                        cellw[i] = j;
//sdebug(QString("DON'T NEED LINE WERAP! (Fall to default: %1) /%2/").arg(cellh).arg(cellrh.size()));
                }
            }
            else //There is no line wrap possibility
            {
                //Saving the calculated cell width
                if(j > cellw[i])
                    cellw[i] = j;
                //Saving the normal cell height
        
            }
        }//end examinig of actual columns (for cycle)

        cellrh.push_back(curr_cellh);
        pageY+=curr_cellh;

        //Need more page or not?
        if(pageY > A4_Y - cellh)
            {
                ++page;
                //futoY = upper header
                pageY = 50+cellh;
            }

        
    }
    while(data->nextRow());

    //HEADLINE
    for(i=0;i < column;++i)
    {
        j = QFontMetrics(printfont).QFONTMETRICS_STRING_HORIZONTAL_WIDTH( data->getHeaderItem(i) );
            if(j > cellw[i])
                cellw[i] = j;
    }

    //Grows the cellwidth with CELLHMARGIN 
    for(i=0;i < column;++i)
    {
        cellw[i] += CELLHMARGIN*2;
    }

sdebug(QString(">>>>Calculated pagenum: %1").arg(page));

    delete p;
    delete slist;
    sdebug("*** HPrintTable::scanPlainDataMatrix *** END");

}

void HPrintTable::scanList(void)
{
    sdebug("*** HPrintTable::scanList ***");
    /* 
      Calculates the following data
        cellh  - cellheight
        cellw  - cellwidth
        rownum - number of rows
        colnum - number of columns
        page   - page number
    */

    int pageY;
    int t;
    int i,j;
    int curr_cellh=0;
    QStringList *slist;
    
    HList *data = (HList *) this->data; //hide the class variable with a local
    QPainter *p = new QPainter();

    
    cellh = 20;
    cellw.clear();
    cellrh.clear();
    rownum=0;
    column = -1;

    //CELLHEIGHT
    
    //cellh = ( QApplication::fontMetrics() ).height();
    cellh =  QFontMetrics(printfont).height();

    cellh += CELLVMARGIN*2;

    //pagenum=1
    page = 1;
    //futoY = upper header
    pageY = 50+cellh;

    //BODY
    QList<QStringList *>::Iterator iv = data->getValues()->begin();
    while(iv != data->getValues()->end())
    {
        ++rownum;
        slist = *iv;
        if(column == -1) //first line
        {
            column = (int)slist->size();
            //Initializing a column number int list for the columnwidth
            for(i=0;i<column;i++)
                cellw.push_back(0);
        }
        else //other lines
        {
            if(column != (int)slist->size())
            {
                error("*** HPrintTable::scanList *** - found different column number in lines!");
                delete p;
                close();    
            }
        }
        
        //working on columns
        curr_cellh = cellh;
        for(i=0;i < column;++i)
        {
            //CELLA : slist[i]
            j =  QFontMetrics(printfont).QFONTMETRICS_STRING_HORIZONTAL_WIDTH( (*slist)[i] );

            //Line wrapping is enabled
            if(data->fieldByIndex(i)->isPrintCellWrap()) 
            {
                int maxcellw = 
                    data->fieldByIndex(i)->getPrintCellWidth();

                if(maxcellw == 0) 
                    maxcellw = 100; /*TODO !!!! should be an average !!! */

                //need a line wrap
                if(j > maxcellw)
                {
                    t = QFontMetrics(printfont)
                                        .boundingRect(0,0,maxcellw
                                              ,A4_Y,Qt::AlignLeft | BREAKPOLICY,(*slist)[i]).height();
                
                    t+=2*CELLVMARGIN;
                    if(curr_cellh < t)
                        curr_cellh = t;
                    cellw[i] = maxcellw;

//sdebug(QString("LINE WRAP! Examined cellheight: %1 (+ CELLVMARGIN*2)  /%2/").arg( t ).arg(cellrh.size()));
                }
                else //Line wrap is possible, but don't need
                {
                    if(j > cellw[i])
                        cellw[i] = j;
//sdebug(QString("DON'T NEED LINE WERAP! (Fall to default: %1) /%2/").arg(cellh).arg(cellrh.size()));
                }
            }
            else //There is no line wrap possibility
            {
                //Save the calculated cellwidth
                if(j > cellw[i])
                    cellw[i] = j;
                //Save the calculated cellheight
        
            }
        }//end examinig of actual columns (for cycle)

        cellrh.push_back(curr_cellh);
        pageY+=curr_cellh;

        //Next page or not?
        if(pageY > A4_Y - cellh)
            {
                ++page;
                //futoY = upper header
                pageY = 50+cellh;
            }

        ++iv;
    }

    //HEADLINE
    for(i=0;i < column;++i)
    {
            j = QFontMetrics(printfont).QFONTMETRICS_STRING_HORIZONTAL_WIDTH( (*data)[i] );
            if(j > cellw[i])
                cellw[i] = j;
    }

    //grows the cellwidth with CELLHMARGIN
    for(i=0;i < column;++i)
    {
        cellw[i] += CELLHMARGIN*2;
    }

sdebug(QString(">>>>Calculated pagenum: %1").arg(page));

    delete p;
    sdebug("*** HPrintTable::scanList *** END");
}

void HPrintTable::scanTable(void)
{
    sdebug("*** HPrintTable::scanTable ***");
    int i;

    cellw.clear();
    cellh = QFontMetrics(printfont).height()*2+CELLVMARGIN*3;
    i = (A4_X-(LMARGIN+RMARGIN+4*CELLHMARGIN))/2;
    cellw.push_back(i);
    
    //Don't know... This will fail on big table. Fixme.
    page = 1;
    sdebug("*** HPrintTable::scanTable *** END");
}

int HPrintTable::drawIt(QPainter *p,double scale,bool print)
{
    //Scale, Pan
    
    //preview->panx = preview->pany = 0;
    if(!print)
    {
        p->setClipRect(4,4,preview->width()-8,preview->height()-8);
        p->translate(preview->panx,preview->pany); //scroll

        p->translate(60,10); //border
        p->scale(scale,scale);
    
        //Draw an empty page with border and shadow ;-)
        p->fillRect(20,20,A4_X,A4_Y,QBrush(QColor(50,50,50),SHADOWPATTERN));
        p->fillRect(0,0,A4_X,A4_Y,QBrush(QColor(255,255,255),Qt::SolidPattern));
    }
    p->setPen(QColor(0,0,0));
    //Page border
    p->drawRect(LMARGIN,TMARGIN,A4_X-(RMARGIN+LMARGIN),A4_Y-(BMARGIN+TMARGIN));

    if(data->getWhoami() == "HPlainDataMatrix")
        return drawPlainDataMatrix(p,print);
    else if(data->getWhoami() == "HTable")
        return drawTable(p,print);
    else if(data->getWhoami() == "HList")
        return drawList(p,print);
    else
        error("HPrintTable::drawIt - Cannot draw item: Not supported type!");


    return 0;
}

int HPrintTable::newPage(QPainter *p,bool print)
{
    //++page;
    if(!print) //Drawing to display
    {
        p->translate(0,(A4_Y+100));

        //Draw an empty page with borders
        p->fillRect(20,20,A4_X,A4_Y,QBrush(QColor(50,50,50),SHADOWPATTERN));
        p->fillRect(0,0,A4_X,A4_Y,QBrush(QColor(255,255,255),Qt::SolidPattern));

     //OFF   if(        p->xForm(QPoint(0,-100    )).y()  > preview->height()  )
     //OFF       hide = true;
     //OFF   else if(   p->xForm(QPoint(0,A4_Y+200)).y()  < 0  )
     //OFF       hide = true;
     //OFF   else

            hide = false;

    }
    else //We are printing now!
    {
        printer->newPage();
    }

    p->setPen(QColor(0,0,0));
    p->drawRect(LMARGIN,TMARGIN,A4_X-(RMARGIN+LMARGIN),A4_Y-(BMARGIN+TMARGIN));
    return 0;
}

#define HLINE \
            DRAWLINE(LMARGIN,downY,A4_X-RMARGIN,downY);

#define DRAWTEXT(a,b,c)     \
            p->drawText(a,b,c);

#define DRAWTEXTBOX(a,b,c,d,s)      \
            p->drawText(a,b,c,d,Qt::AlignLeft | BREAKPOLICY,s);

#define DRAWLINE(a,b,c,d)   \
            p->drawLine(a,b,c,d);

#define DRAWRECT(a,b,c,d)   \
            p->drawRect(a,b,c,d);

int HPrintTable::drawPlainDataMatrix(QPainter *p,bool print)
{

    QStringList *slist;
    int rowrun;
    int downX=LMARGIN,downY=TMARGIN;
    bool head=false;
    int i;
    pagerun = 1;
    HPlainDataMatrix *data = (HPlainDataMatrix *) this->data; //hide the class variable with a local
    slist = new QStringList();

    hide = false;

    rowrun = 0;
    data->firstRow();
    do
    {
        if(print || (!print && !hide )) //not printable
        {
            *slist = data->currentRowStr();
            if(!head) //Draw the header if haven't drawn yet
            {
                downY += 35;
                DRAWTEXT(LMARGIN + 20,TMARGIN+20,(
                    data->getTitle().isEmpty() ? "Printed page" : data->getTitle()) + QString("(%1)").arg(pagerun));

                downX = LMARGIN;
                for(i=0;i < column;++i)
                {
                    DRAWTEXTBOX(downX+CELLHMARGIN,downY+CELLVMARGIN,cellw[i],cellh,data->getHeaderItem(i));
                    downX += cellw[i]+2;
                }
                downY += 1;
                HLINE;
                downY += cellh;
                HLINE;
                downY += 1;
                downX =  LMARGIN;
                head = true;
            }
            //draw the cells
            for(i=0;i<column;++i)
            {
                DRAWTEXTBOX(downX+CELLHMARGIN,downY+CELLVMARGIN,cellw[i],cellrh[rowrun],(*slist)[i]);
                downX += cellw[i]+2;
            }
        }
        else //empty running
        {
           if(!head) 
            {
                head = true;
                downY += 35 + 2 + cellh;
            }

        }

        downY += cellrh[rowrun];
        downX =  LMARGIN;

        if(downY >= A4_Y - (TMARGIN+BMARGIN)) //we reach the bottom of the page
        {
            pagerun++;
            newPage(p,print);
            downX=LMARGIN;
            downY=TMARGIN;
            //Page Headline
            head=false;
        }
        ++rowrun;
    }
    while(data->nextRow());
    delete slist;
    return 0;
}


int HPrintTable::drawList(QPainter *p,bool print)
{
        QStringList *slist;
        int rowrun;
        int downX=LMARGIN,downY=TMARGIN;
        bool head=false;
        int i;
        pagerun = 1;
        HList *data = (HList *) this->data; //hide the class variable with a local

        hide = false;

    rowrun = 0;
    QList<QStringList *>::Iterator iv = data->getValues()->begin();
    while(iv != data->getValues()->end())
    {
        if(print || (!print && !hide )) //not printable
        {
            slist=*iv;
            if(!head) //Draw the header if haven't drawn yet
            {
                downY += 35;
                DRAWTEXT(LMARGIN + 20,TMARGIN+20,(
                    data->tableTitle ().isEmpty() ? "Printed page" : data->tableTitle ())+QString("(%1)").arg(pagerun));

                downX = LMARGIN;
                for(i=0;i < column;++i)
                {
                    DRAWTEXTBOX(downX+CELLHMARGIN,downY+CELLVMARGIN,cellw[i],cellh,(*data)[i]);
                    downX += cellw[i]+2;
                }
                downY += 1;
                HLINE;
                downY += cellh;
                HLINE;
                downY += 1;
                downX =  LMARGIN;
                head = true;
            }
            //draw the cells
            for(i=0;i<column;++i)
            {
                DRAWTEXTBOX(downX+CELLHMARGIN,downY+CELLVMARGIN,cellw[i],cellrh[rowrun],(*slist)[i]);
                downX += cellw[i]+2;
            }
        }
        else //empty running
        {
           if(!head) 
            {
                head = true;
                downY += 35 + 2 + cellh;
            }

        }

        downY += cellrh[rowrun];
        downX =  LMARGIN;

        if(downY >= A4_Y - (TMARGIN+BMARGIN)) //we reach the bottom of the page
        {
            pagerun++;
            newPage(p,print);
            downX=LMARGIN;
            downY=TMARGIN;
            //Page Headline
            head=false;
        }
        ++iv;
        ++rowrun;
    }
    return 0;
}

int HPrintTable::drawTable(QPainter *p,bool print)
{
    Q_UNUSED(print);
        int downX=LMARGIN,downY=TMARGIN;
        pagerun = 1;
        HTable *data = (HTable *) this->data; //hide the class variable with a local

    HDataField *df;
    
    downY += 35;
    DRAWTEXT(LMARGIN + 20,TMARGIN+20,(
        data->tableTitle ().isEmpty() ? "Printed page" : data->tableTitle ())+QString("(%1)").arg(pagerun));

    downX = LMARGIN;
    HLINE;
    downY += 3;
    HLINE;
    downY += 3;
    data->firstField();
    
    while((df = data->nextFieldAll()))
    {
        if(df->isShow())
        {
            DRAWTEXTBOX(downX+CELLHMARGIN,downY+CELLVMARGIN,
                cellw[0],cellh,df->getExplainText());
        
                
            if(df->getWhoami() != "HStatic" || !df->getValue().toString().isEmpty())        
            {
            downX += cellw[0];      

            DRAWLINE(downX,downY-2,downX,downY+cellh);
            DRAWTEXTBOX(downX+CELLHMARGIN,downY+CELLVMARGIN,
                    cellw[0],cellh,df->dbValueToDispValue(df->getValue()).toString());
            }
                    
            downX = LMARGIN;        
            downY += cellh;
            HLINE;
            downY += 2;
        }
    }
    return 0;
}

int HPrintTable::printIt(void)
{
    QPainter *p;
    p=new QPainter();
    sdebug("*** HPrintTable::printIt ***");
    printer = new QPrinter();
    printer->setPageSize(QPrinter::A4);
    QPrintDialog psdialog(printer,this);
    if(psdialog.exec())
    {
        p->begin(printer);
        drawIt(p,1.0,true);
        p->end();
    }
    delete p;
    delete printer;
    printer = NULL;
    return 0;
}

int HPrintTable::updateDisplay(void)
{
    scanIt();
    preview->reset();
    preview->repaint();
    return 0;
}
//end code
