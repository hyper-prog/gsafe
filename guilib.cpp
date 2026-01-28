/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    guilib.cpp
*/

#include "guilib.h"
#include "dconsole.h"
#include "printlib.h"
#include "xmlolib.h"

int HArrayButton::defaultSizeX = 0;
int HArrayButton::defaultSizeY = 0;

HArrayButton::HArrayButton(QString text,QWidget *parent)
:QPushButton(text,parent)
{
    connect(this,SIGNAL(clicked()),this,SLOT(clickHandler()));
    if(defaultSizeX != 0 && defaultSizeY != 0)
    {
        setMaximumSize(defaultSizeX,defaultSizeY);
        setMinimumSize(defaultSizeX,defaultSizeY);
    }
}

int HArrayButton::clickHandler(void)
{
    emit clickedValue(text());
    return 0;
}

QLayout * HArrayButton::abc_filt(QWidget *parent,const QObject *receiver,const char *member)
{
#define MAKE_A_BUTTON(a,b) {\
            tmp = new HArrayButton(b,parent);           \
             connect(tmp,SIGNAL(clickedValue(QString)),receiver,member); \
             (a)->addWidget(tmp); (a)->addSpacing(2);                     \
           }

    HArrayButton *tmp;

    QVBoxLayout *ml = new QVBoxLayout(0);

        QHBoxLayout *ll2 = new QHBoxLayout(0);
            ll2->setMargin(0);
            ll2->setSpacing(0);
        QHBoxLayout *ll3 = new QHBoxLayout(0);
            ll3->setMargin(0);
            ll3->setSpacing(0);

            ll2->addStretch();
            ll3->addStretch();
            MAKE_A_BUTTON(ll2,".");

#ifdef GSAFETEXT_LANG_HU
            MAKE_A_BUTTON(ll2,"A"); MAKE_A_BUTTON(ll2,"Á");  MAKE_A_BUTTON(ll2,"B");
            MAKE_A_BUTTON(ll2,"C"); MAKE_A_BUTTON(ll2,"D");  MAKE_A_BUTTON(ll2,"E");
            MAKE_A_BUTTON(ll2,"É"); MAKE_A_BUTTON(ll2,"F");  MAKE_A_BUTTON(ll2,"G");
            MAKE_A_BUTTON(ll2,"H"); MAKE_A_BUTTON(ll2,"I");  MAKE_A_BUTTON(ll2,"Í");
            MAKE_A_BUTTON(ll2,"J"); MAKE_A_BUTTON(ll2,"K");  MAKE_A_BUTTON(ll2,"L");
            MAKE_A_BUTTON(ll2,"M");

            MAKE_A_BUTTON(ll2,"N"); MAKE_A_BUTTON(ll3,"O"); MAKE_A_BUTTON(ll3,"Ó");
            MAKE_A_BUTTON(ll3,"Ö"); MAKE_A_BUTTON(ll3,"P");
            MAKE_A_BUTTON(ll3,"Q"); MAKE_A_BUTTON(ll3,"R"); MAKE_A_BUTTON(ll3,"S");
            MAKE_A_BUTTON(ll3,"T"); MAKE_A_BUTTON(ll3,"U"); MAKE_A_BUTTON(ll3,"Ú");
            MAKE_A_BUTTON(ll3,"Ü"); MAKE_A_BUTTON(ll3,"X");
            MAKE_A_BUTTON(ll3,"Y"); MAKE_A_BUTTON(ll3,"V"); MAKE_A_BUTTON(ll3,"W");
            MAKE_A_BUTTON(ll3,"Z");
#else
            MAKE_A_BUTTON(ll2,"A"); MAKE_A_BUTTON(ll2,"B");  MAKE_A_BUTTON(ll2,"C");
            MAKE_A_BUTTON(ll2,"D"); MAKE_A_BUTTON(ll2,"E");  MAKE_A_BUTTON(ll2,"F");
            MAKE_A_BUTTON(ll2,"G"); MAKE_A_BUTTON(ll2,"H");  MAKE_A_BUTTON(ll2,"I");
            MAKE_A_BUTTON(ll2,"J"); MAKE_A_BUTTON(ll2,"K");  MAKE_A_BUTTON(ll2,"L");
            MAKE_A_BUTTON(ll2,"M");

            MAKE_A_BUTTON(ll3,"N"); MAKE_A_BUTTON(ll3,"O"); MAKE_A_BUTTON(ll3,"P");
            MAKE_A_BUTTON(ll3,"Q"); MAKE_A_BUTTON(ll3,"R"); MAKE_A_BUTTON(ll3,"S");
            MAKE_A_BUTTON(ll3,"T"); MAKE_A_BUTTON(ll3,"U"); MAKE_A_BUTTON(ll3,"X");
            MAKE_A_BUTTON(ll3,"Y"); MAKE_A_BUTTON(ll3,"V"); MAKE_A_BUTTON(ll3,"W");
            MAKE_A_BUTTON(ll3,"Z");
#endif

            ll2->addStretch();
            ll3->addStretch();

    ml->addLayout(ll2);
    ml->addSpacing(2);
    ml->addLayout(ll3);

#undef MAKE_A_BUTTON
    return ml;
}

HArrayButton::~HArrayButton(void)
{

}

/////////////////////////////////////////////////////////////////////////////////
/// HSpinBox ////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
HSpinBox::HSpinBox(QWidget *parent)
:QFrame(parent)
{
    int h;

    min = 0;
    max = 1000000;
    setFocusPolicy(Qt::StrongFocus);

    setMaximumWidth(100);

    /*
    QFontMetrics *fm =
        new QFontMetrics( QApplication::font() );
    */

    QHBoxLayout *lay = new QHBoxLayout(this);
        lay->setMargin(2);
        lay->setSpacing(1);
    QVBoxLayout *swl = new QVBoxLayout(0);
        swl->setSpacing(0);
        swl->setSpacing(0);

    le = new QLineEdit(this);

    h = height()/2-4;

    QToolButton *bu = new QToolButton(this);
    bu->setIcon(QPixmap(":/GSAFEPIXMAPS/image_hsup.png"));
    bu->setMaximumHeight(h);
    bu->setMinimumHeight(h);
    bu->setMinimumWidth(14);
    bu->setMaximumWidth(14);

    QToolButton *bd = new QToolButton(this);
    bd->setIcon(QPixmap(":/GSAFEPIXMAPS/image_hsdown.png"));
    bd->setMaximumHeight(h);
    bu->setMinimumHeight(h);
    bd->setMinimumWidth(14);
    bd->setMaximumWidth(14);

    swl->addWidget(bu);
    swl->addWidget(bd);

    lay->addLayout(swl);
    lay->addWidget(le);
    le->setAlignment(Qt::AlignRight);

    val = 0;

    connect(le,SIGNAL(textChanged(const QString&)),this,SLOT(leChanged(const QString&)));
    connect(bu,SIGNAL(clicked()),this,SLOT(bUp()));
    connect(bd,SIGNAL(clicked()),this,SLOT(bDown()));
    oldtext = "";
}

int HSpinBox::bUp(void)
{
    val++;
    check();
    emit valueChanged(val);
    return 0;
}

int HSpinBox::bDown(void)
{
    val--;
    check();
    emit valueChanged(val);
    return 0;
}

int HSpinBox::leChanged(const QString& t)
{
    int v;
    bool ok;

    if(t.isEmpty())
    {
        val = 0;
        oldtext = t;
        emit valueChanged(val);
        return 0;
    }

    v = t.toInt(&ok);
    if(ok)
    {
        val = v;
        oldtext = t;
        emit valueChanged(val);
    }
    else
    {
        le->setText(oldtext);
    }
    return 0;
}

int HSpinBox::setValue(int v)
{
    if(val == v)
        return v;

    val = v;
    check();
    emit valueChanged(val);
    return v;
}

void HSpinBox::check(void)
{
    if(val > max)
        val=max;
    if(val < min)
        val=min;
    le->setText((oldtext = QString("%1").arg(val)));
}

void HSpinBox::wheelEvent(QWheelEvent *e)
{
    int d;

#ifdef COMPILED_WITH_QT4X
    d = e->delta();
#else
    d = e->angleDelta().y();
#endif

    if(d > 0)
        val++;
    else
        val--;
    check();
    emit valueChanged(val);
}

HSpinBox::~HSpinBox(void)
{

}

/////////////////////////////////////////////////////////////////////////////////
/// HTableBrowser  //////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#define CELLHMARGIN 3
#define CELLVMARGIN 3

#define TMARGIN 10
#define LMARGIN 10

#define BARW 12
#define BUTTH 18

#define DEFAULT_HEADCOLOR           QColor(244,180,200)
#define DEFAULT_HEADCOLOR_CURSOR    QColor(250,100,120)
#define DEFAULT_CURSORCOLOR         QColor(140,140,230)
#define DEFAULT_BGCOLOR             QColor(240,240,240)

bool     HTableBrowser::staticinit  = false;
QPixmap *HTableBrowser::pix_up      = NULL;
QPixmap *HTableBrowser::pix_down    = NULL;
QPixmap *HTableBrowser::pix_asc     = NULL;
QPixmap *HTableBrowser::pix_desc    = NULL;
QPixmap *HTableBrowser::pix_asc2    = NULL;
QPixmap *HTableBrowser::pix_desc2   = NULL;

HTableBrowser::HTableBrowser(QWidget *parent)
:QFrame(parent)
{
    start = NULL;
    end   = NULL;
    top   = NULL;
    cur   = NULL;

    curc  = 0;
    asc   = true;
    query_cursor = NULL;
    col_scroll = 0;
    lastkey="";
    inscroll = false;
    oldw = 0;
    oldh = 0;
    old_col_scroll = 0;

    precolor.clear();

    keycolumn  = -1;
    keycolumn2 = -1;

    cachedElements.clear();
    cacheLimit = 20; //An initial size. It increased automatically if necessary

    if(!staticinit)
    {
        staticinit = true;
        pix_up   = new QPixmap(":/GSAFEPIXMAPS/image_up.png");
        pix_down = new QPixmap(":/GSAFEPIXMAPS/image_down.png");
        pix_asc  = new QPixmap(":/GSAFEPIXMAPS/image_ascend.png");
        pix_desc = new QPixmap(":/GSAFEPIXMAPS/image_descend.png");
        pix_asc2 = new QPixmap(":/GSAFEPIXMAPS/image_ascend2.png");
        pix_desc2= new QPixmap(":/GSAFEPIXMAPS/image_descend2.png");
    }

    setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));
    setMidLineWidth(100);
    setMinimumHeight(100);
    setFocusPolicy(Qt::StrongFocus);
    setFocus();

    color_cursor        = DEFAULT_CURSORCOLOR;
    color_header        = DEFAULT_HEADCOLOR;
    color_header_cursor = DEFAULT_HEADCOLOR_CURSOR;
    color_background    = DEFAULT_BGCOLOR;

    QPalette palette;
    palette.setColor(QPalette::Window,color_background);
    setPalette(palette);
    setAutoFillBackground(true);

    fm = new QFontMetrics( QApplication::font() );
    clear();
}

 void HTableBrowser::setBackgroundColor(QColor c)
 {
     color_background = c;
     QPalette palette;
     palette.setColor(QPalette::Window,color_background);
     setPalette(palette);
     setAutoFillBackground(true);
 }

int HTableBrowser::setHeadTexts(QStringList h)
{
    head = h;
    return 0;
};

HTableBrowser::~HTableBrowser(void)
{
    sdebug("*** HTableBrowser::~HTableBrowser ***\nDestory HTableBrowser, delete items...");
    clear();
    delete fm;
    sdebug("*** HTableBrowser::~HTableBrowser ***END");
}

QString HTableBrowser::currentKey(void)
{
    if(cur == NULL)
        return "";
    return cur->key;
}

QString HTableBrowser::topKey(void)
{
    if(top == NULL)
        return "";
    return top->key;
}

int HTableBrowser::clear()
{
    HTableBrowserElement *tmp;

    fulshDrawingCache();

    number = 0;
    seek = "";
    head.clear();
    colc.clear();
    colw.clear();
    spec_sort.clear();
    asc = false;
    rowh = fm->height();

    //Freeing list
    cacheMutex.lock();
    tmp = end;
    while(tmp != NULL)
    {
        tmp = tmp->prev;
        delete end;
        end = tmp;
    }
    cacheMutex.unlock();

    start = NULL;
    top = NULL;
    end = NULL;
    cur = NULL;
    return 0;
}

void HTableBrowser::setColumnColor(int col,QColor c)
{
    if(col >= (int)colc.size())
    {
        precolor[col] = c;
        return;
    }
    precolor[col] = c;
    colc[col] = c;
}

int HTableBrowser::addElement(HTableBrowserElement *e)
{
    int i,l;

    e->index = number;
    //add to list, graphical analysis...
    if(start == NULL) //there is no element yet
    {
        if(head.size() > e->size())
        {
            QString errortxt="You try to add smaller record than previous. (Contains less cell. Why? Forget separate signals? \"|\"?) ";
            sdebug(errortxt);
            error(errortxt);
            return 0;
        }

        cur = top = start = end = e;

        for(i=0;i<e->size();++i)
        {
            colw.push_back( fm->QFONTMETRICS_STRING_HORIZONTAL_WIDTH((*e)[i]) );
            colc.push_back( precolor.contains(i) ? precolor[i] : QColor( (100+i*25)>255 ? 255 : (100+i*25) ,200-i*5,180 ));
            spec_sort.push_back("");
        }
        rowh = fm->height();

        while(head.size() < colw.size())
            head.push_back("");

        for(i=0;i<(int)head.size();++i)
        {
            l = fm->QFONTMETRICS_STRING_HORIZONTAL_WIDTH(head[i]) + BARW; //because the sort indicator
            if(l > colw[i])
                colw[i] = l;
        }

    }
    else //insert to the end
    {
        end->next = e;
        e->prev = end;
        end = e;

        for(i=0;i<e->size();++i)
        {
            l = fm->QFONTMETRICS_STRING_HORIZONTAL_WIDTH((*e)[i]) + BARW; //because the sort indicator
            if(l > colw[i])
                colw[i] = l;
        }
    }
    ++number;
    return 0;
}

int HTableBrowser::removeAllElementByKey(QString key)
{
    HTableBrowserElement *tmp = NULL;
    int deleteditems;

    if(start == NULL)
        return 0; // no element

    fulshDrawingCache();

    deleteditems = 0;
    //      HTableBrowserElement *start,*end,*top,*cur,*query_cursor;
    HTableBrowserElement *seek = start;
    while(seek != NULL)
    {
        if(seek->key == key)
        {   //Found->erase
            //================================//
            if(seek->prev != NULL)
                seek->prev->next = seek->next;
            if(seek->next != NULL)
                seek->next->prev = seek->prev;
            if(start == seek)
                start = seek->next;
            if(end == seek)
                end = seek->prev;
            if(top == seek)
            {
                if     (seek->prev != NULL) top = seek->prev;
                else if(seek->next != NULL) top = seek->next;
                else                        top = start;
            }
            if(cur == seek)
            {
                if     (seek->prev != NULL) cur = seek->prev;
                else if(seek->next != NULL) cur = seek->next;
                else                        cur = start;
            }
            query_cursor = start;

            tmp = seek->next;
            cacheMutex.lock();
            delete seek;
            cacheMutex.unlock();
            seek = tmp;
            number--;
            deleteditems++;
            //================================//
            continue;
        }
        seek = seek->next;
    }
    update();
    return deleteditems;
}

void HTableBrowser::setSpecSort(int col,QString specsortname)
{
    if(col >= (int)spec_sort.size())
    {
        sdebug("Warning: Less column exists than column index received in \"setSpecSort\"!!\n (Hint:Insert an element before you set the column colors!)");
        return;
    }

    spec_sort[col] = specsortname;
}

int HTableBrowser::setCurrentElement(QString c,QString t)
{
    HTableBrowserElement *topm=start,*step=start;
    while(step != NULL)
    {
        if(step->key == t)
            topm = step;
        if(step->key == c)
        {
            cur = step;
            if( height()-(rowh) > (cur->index - topm->index)*(rowh+2*CELLVMARGIN)+2*rowh) //this will be the top
               top = topm;
            else
               top = cur;

            update();
            return 1;
        }
        step = step->next;
    }
    return 0;
}

int HTableBrowser::setCurrentElement(int index)
{
    HTableBrowserElement *step=start;
    while(step != NULL)
    {
        if(step->index == index)
        {
            top = cur = step;
            update();
            return 1;
        }
        step = step->next;
    }
    return 0;
}

int HTableBrowser::setTopElement(int index)
{
    HTableBrowserElement *step=start;
    while(step != NULL)
    {
        if(step->index == index)
        {
            top = step;
            update();
            return 1;
        }
        step = step->next;
    }
    return 0;
}

HTableBrowserElement * HTableBrowser::topElement(void)
{
    if(top == NULL)
        return NULL;
    return top;
}

HTableBrowserElement * HTableBrowser::currentElement(void)
{
    if(cur == NULL)
        return NULL;
    return cur;
}

bool HTableBrowser::stepUp(void)
{
        bool fresh;

        if(cur == NULL)
            return false;

        fresh = false;
        if(cur->prev != NULL)
        {
            cur = cur->prev;
            fresh = true;
        }
        //if we need scroll...
        if(top->prev == cur)
        {
            top = cur;
            fresh = true;
        }
        else if(abs(top->index - cur->index) > (showrow+2))
        {
            top = cur;
            fresh = true;
        }
        return fresh;
}

bool HTableBrowser::stepDown(void)
{
        bool fresh;

        if(cur == NULL)
            return false;

        fresh = false;
        if(cur->next != NULL)
        {
            cur = cur->next;
            fresh = true;
        }

        //if we need scroll...
        if(cur->index < top->index) //out on top
        {
                top = cur;
                fresh = true;
        }
        while(height()-(rowh+4*CELLVMARGIN+12) < (cur->index - top->index)*(rowh+2*CELLVMARGIN)+2*rowh)
        {
            if(top->next != NULL)
            {
                top = top->next;
                fresh = true;
            }
            else
                break;
        }
        return fresh;
}

void HTableBrowser::wheelEvent(QWheelEvent *e)
{
    int d;

#ifdef COMPILED_WITH_QT4X
    d = e->delta();
#else
    d = e->angleDelta().y();
#endif

    if(d < 0)
    {
        if(stepDown())
            update();
        return;
    }
    else
    {
        if(stepUp())
            update();
        return;
    }
}

int HTableBrowser::cursorToBegin(void)
{
    seek = "";
    top = cur = start;
    update();
    return 0;
}

int HTableBrowser::cursorToEnd(void)
{
    seek = "";
    top = cur = end;
    update();
    return 0;
}

void HTableBrowser::keyPressEvent(QKeyEvent *e)
{
    int i;
    bool fresh;

    fresh = false;

    if(e->modifiers() == Qt::ControlModifier)
    {
        QFrame::keyPressEvent(e);
        return;
    }

    if(e->key() == Qt::Key_Down)
    {
        seek = "";
        if(stepDown())
            update();
        return;
    }

    if(e->key() == Qt::Key_Up)
    {
        seek = "";
        if(stepUp())
            update();
        return;
    }

    if(e->key() == Qt::Key_PageUp)
    {
        seek = "";
        for(i=0;i < showrow-2;++i)
            if(stepUp())
                fresh = true;

        if(fresh)
            update();
        return;
    }

    if(e->key() == Qt::Key_PageDown)
    {
        seek = "";
        for(i=0;i < showrow-2;++i)
            if(stepDown())
                fresh = true;
        if(fresh)
            update();
        return;
    }

    if(e->key() == Qt::Key_Home)
    {
        seek = "";
        top = cur = start;
        update();
        return;
    }

    if(e->key() == Qt::Key_End)
    {
        seek = "";
        top = cur = end;
        update();
        return;
    }

    if(e->key() == Qt::Key_Left)
    {
        seek = "";
        if(curc > 0)
        {
            --curc;
            //do horizontal scroll if necessary
            if(col_scroll > curc)
                col_scroll = curc;
            update();
        }
        return;
    }

    if(e->key() == Qt::Key_Right)
    {
        seek = "";
        if(curc+1 < (int)colw.size())
        {
            ++curc;

            //do horizontal scroll if necessary
            bool run;
            int  sum;
            do
            {
                sum = 0;
                run = false;
                for(i=col_scroll;i<=curc;++i)
                    sum += colw[i]+2*CELLHMARGIN;
                if(sum > (width()-(LMARGIN*2)))
                {
                    ++col_scroll;
                    run = true;
                }
            } while(run);

            update();
        }
        return;
    }

    if(e->key() == Qt::Key_Return || (e->key() == Qt::Key_Space && seek.isEmpty()))
    {
        seek = "";
        emit activateItem(cur->key);
        return;
    }

    //If the seek is empty I will pass the Esc. Probably it will be an exit.
    if(seek.isEmpty() && e->key() == Qt::Key_Escape)
    {
       QFrame::keyPressEvent(e);
       return;
    }

    //If there is some seek text, I'll delete it when esc pressed. Every time delete the seek text if delete pressed.
    if((!seek.isEmpty() && e->key() == Qt::Key_Escape) || (e->key() == Qt::Key_Delete))
    {
        seek = "";
        update();
        return;
    }

    if(e->key() == Qt::Key_Backspace)
    {
        seek.remove(QRegExp(".$"));
        sdebug("seek: "+seek);
        return;
    }

    if(e->key() == Qt::Key_F3)
    {
        HTableBrowserElement *t;
        for(t = cur ; t != NULL ; t=t->next)
        {
            if(((*t->rows)[curc]).startsWith(seek))
            {
                top = cur = t;
            }
        }
        update();
        return;
    }

    if(e->key() == Qt::Key_F5)
    {
        fulshDrawingCache();
        update();
        return;
    }

    //Click a mouse button
    HTableBrowserElement *t;

    seek.append(e->text());
    sdebug("seek: "+seek);
    for(t = start ; t != NULL ; t=t->next)
    {
        if(((*t->rows)[curc]).startsWith(seek))
        {
            //top = cur = t;
            cur = t;

            //Check the visibility of the current element, if needs I will recalc the top element

            if( cur->index < top->index ||
                height()-(rowh+4*CELLVMARGIN+12) < (cur->index - top->index)*(rowh+2*CELLVMARGIN)+2*rowh) //ok lesz a megadott top
               top = cur;

            update();
            return;
        }
    }
}

// Parameters: Lower limit, upper limit, value
int border(int a,int b,int c)
{
    if(a>c)
        return a;
    if(b<c)
        return b;
    return c;
}

#define inX(a,b) ( ((a) < e->x() && (b) > e->x()) || ((b) < e->x() && (a) > e->x()))
#define inY(a,b) ( ((a) < e->y() && (b) > e->y()) || ((b) < e->y() && (a) > e->y()))
bool HTableBrowser::mouseNetClick(QMouseEvent *e,int rmode)
{
    int x,y;
    int stepx;
    HTableBrowserElement *stepy;

    if(cur == NULL)
       return false;

    //out from everything
    if(!inX(0,width()) || !inY(0,height()))
    {
        sdebug("==CLICK=> Outer click (do nothing)");
        return false;
    }

    //left click from the first cell, is scroll the column to back
    if(inX(0,LMARGIN+CELLHMARGIN))
    {
        sdebug("==CLICK=> Left from the first cell (backscroll the columns)");
        if(col_scroll == 0)
            return false;
        curc = --col_scroll;
        return true;
    }

    for(stepx=col_scroll,x=LMARGIN;
            (x<(width()-(LMARGIN+BARW)));
            x+=(colw[stepx]+2*CELLHMARGIN+plusw),stepx++)
    {
        if(stepx >= colw.size())
            break;
        if(inX(x+CELLHMARGIN,x+colw[stepx]+plusw+CELLHMARGIN*2) && inY(TMARGIN+CELLVMARGIN,TMARGIN+rowh+CELLHMARGIN))
        {
            sdebug("==CLICK=> Column header (sorting)");

            sdebug(QString("Click header: column %1").arg(stepx));
            sort(stepx);
            return rmode == NEED_UPDATE ? true : false;
        }
    }

    for(stepy=top,y=TMARGIN+rowh+4*CELLVMARGIN;
        (y < height()-TMARGIN) && (stepy != NULL);
        y+=(rowh+2*CELLVMARGIN),stepy=stepy->next)

        if(inY(y,y+rowh+2*CELLHMARGIN))
        {
            for(stepx=col_scroll,x=LMARGIN;
                (x<(width()-(LMARGIN+BARW)));
                x+=(colw[stepx]+2*CELLHMARGIN+plusw),stepx++)
            {
                if(stepx >= colw.size())
                    break;
                if(inX(x,x+colw[stepx]+plusw+CELLHMARGIN*2))
                {
                    sdebug("==CLICK=> To Cell (Marking)");

                    if(cur != stepy || curc != stepx)
                    {
                        cur  = stepy;
                        curc = stepx;

                        return true;
                    }
                    return rmode == NEED_UPDATE ? false : true;
                }
            }
        }
    sdebug("==Click=> Nothing targetted, don't hit anything (exiting)");
    return false;
}

void HTableBrowser::mouseMoveEvent(QMouseEvent *e)
{
    int nTH;
    seek = "";
        //scrolling
    if( inscroll || inX(width()-BARW-LMARGIN+2,width()))
    {
        //sdebug("==MOVE/PRESS=> In the button area...");

        if( !inscroll && inY(8,8+BUTTH))
        {
            sdebug("==MOVE/PRESS=> Up button");
            if(stepUp())
                update();
            return;
        }
        if( !inscroll && inY(height()-(BUTTH+8),height()-8))
        {
            sdebug("==MOVE/PRESS=> Down button");
            if(stepDown())
                update();
            return;
        }

        nTH =  ( (number-showrow+2) * (border(0,height()-(8+BUTTH),e->y()-(8+BUTTH))) )  /  (height()-8*2-2*BUTTH);
        setTopElement(nTH);
        inscroll = true;
        return;
    }

    sdebug("==MOVE/PRESS=> Nothing hit --> Call mouseNetClick()");

    if(!inscroll && mouseNetClick(e))
        update();
}

void HTableBrowser::mouseReleaseEvent(QMouseEvent *e)
{
    inscroll = false;

    if(e->button() == Qt::RightButton)
    {
        sdebug("HTableBrowser::mouseReleaseEvent RIGHT button!");
         if(mouseNetClick(e,CLICK_ON_CELL))
             if(last_right_clicked == cur->key)
             {
                 sdebug("Emitting signal!");
                 mouseRightClickPosX = e->x();
                 mouseRightClickPosY = e->y();
                 emit alternateActivateItem(cur->key);
             }
        return;
    }
}

void HTableBrowser::mousePressEvent(QMouseEvent *e)
{
    seek = "";
    mouseMoveEvent(e);
    if(e->button() == Qt::RightButton)
        if(mouseNetClick(e,CLICK_ON_CELL))
            last_right_clicked = cur->key;
}

void HTableBrowser::mouseDoubleClickEvent(QMouseEvent *e)
{
    seek = "";

    if(inX(width()-BARW-LMARGIN+2,width()))
    {
        mouseMoveEvent(e);
        return;
    }

    if(mouseNetClick(e,CLICK_ON_CELL))
        emit activateItem(cur->key);
}

#undef inX
#undef inY


bool HTableBrowser::ch(bool m,HTableBrowserElement *a,HTableBrowserElement *b)
{
    bool asc  = this->asc;
    bool asc2 = this->asc2;

    if(m)
    {
        asc = !asc;
        asc2 = !asc2;
    }

   if(sortmode == 1) //Dropchar sort mode
   {
        if(dropchar(a->getCVal(keycolumn).toLocal8Bit()) > dropchar(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? true  : false);
        if(dropchar(a->getCVal(keycolumn).toLocal8Bit()) < dropchar(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? false : true );

        if(keycolumn2 == -1)
            return true;

        if(dropchar(a->getCVal(keycolumn2).toLocal8Bit()) > dropchar(b->getCVal(keycolumn2).toLocal8Bit()))
            return (asc2 ? true  : false);
        if(dropchar(a->getCVal(keycolumn2).toLocal8Bit()) < dropchar(b->getCVal(keycolumn2).toLocal8Bit()))
            return (asc2 ? false : true );
        return true;
   }

   if(QString::localeAwareCompare(a->getCVal(keycolumn),b->getCVal(keycolumn)) > 0 )
       return (asc  ? true  : false);
   if(QString::localeAwareCompare(a->getCVal(keycolumn),b->getCVal(keycolumn)) < 0 )
       return (asc  ? false : true );

   if(keycolumn2 == -1)
       return true;

   if(QString::localeAwareCompare(a->getCVal(keycolumn2),b->getCVal(keycolumn2)) > 0)
       return (asc2 ? true  : false);
   if(QString::localeAwareCompare(a->getCVal(keycolumn2),b->getCVal(keycolumn2)) < 0)
       return (asc2 ? false : true );
   return true;

}

void HTableBrowser::q_sort(void)
{
    HTableBrowserElementPair  *t;

    if(start == NULL)
        return;
    q_sort_stack = new QStack<HTableBrowserElementPair *> ();
    q_sort_stack->clear();
    q_sort_stack->push(new HTableBrowserElementPair(start,end)); //Verem inicializalas a starthoz

    while( ! q_sort_stack->isEmpty() )
    {
        q_sort_run( (t=q_sort_stack->pop()) );
        delete t;
    }
    fulshDrawingCache();
}

//Quick sort algoritmus
void HTableBrowser::q_sort_run(HTableBrowserElementPair *p)
    {
        int i;
        HTableBrowserElement *m;

        HTableBrowserElement *left=p->left;
        HTableBrowserElement *right=p->right;

        //it will be the middle element
        i = (int) ((p->left->index+p->right->index) / 2);
        for(m=p->left;m->index != i && m != p->right ;m=m->next) ;

        while(left->index < right->index)
        {
            while(ch(true,left,m) && (left->index < m->index))
               left = left->next;

            while(ch(false,right,m) && (right->index  > m->index))
               right = right->prev;

            if(left !=  right)
            {
                   if(m == left)
                       m= right;
                   else if(m == right)
                       m = left;

                   right->swapWith(left);
            }
        }

        if(p->left->index  < m->index)
                q_sort_stack->push(new HTableBrowserElementPair(p->left,m->prev));
        if(p->right->index > m->index)
                q_sort_stack->push(new HTableBrowserElementPair(m->next,p->right));
    }

void HTableBrowser::sort(int keycol)
{
  if(colw.size() == 0 )
      return;

  if(keycolumn == keycol)
  {
      if(asc) asc=false;
      else    asc=true;
  }
  else
  {
      keycolumn2 = keycolumn;
      asc2       = asc;

      asc        = true;
      keycolumn  = keycol;
  }

  sortmode = 0;
  if(spec_sort[keycol] == "dropchar")
  {
      sdebug("Special sort mask with->DROPCHAR");
      sortmode = 1;
  }

  sdebug(QString("start Qsort.. %1 %2").arg(keycolumn).arg(asc));

  q_sort();

  sdebug("..end");
}

int HTableBrowser::fulshDrawingCache(void)
{
    //sdebug("Flush drawing cache...");
    cacheMutex.lock();
    std::list<HTableBrowserElement *>::iterator i = cachedElements.begin();
    while(i != cachedElements.end())
    {
        if((*i)->cachePixmap != NULL)
            delete (*i)->cachePixmap;
        (*i)->cachePixmap = NULL;

        ++i;
    }
    cachedElements.clear();
    cacheMutex.unlock();
    return 0;
}

int HTableBrowser::limitCache(void)
{
    HTableBrowserElement *toremove=NULL;
    cacheMutex.lock();
    while(cacheLimit <= (int)cachedElements.size())
    {
        toremove = cachedElements.back();
        cachedElements.pop_back();

        if(toremove->cachePixmap != NULL)
            delete toremove->cachePixmap;
        toremove->cachePixmap = NULL;
    }
    cacheMutex.unlock();
    return 0;
}

void HTableBrowser::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    int c;
    int allcw=0;
    int dcY=0;

    QPixmap *pix=NULL;
    QColor color,cellcolor;
    HTableBrowserElement *run=top;
    HTableBrowserElement *dcI=NULL;

    //gr->fill(QColor(240,240,240));

    QPainter *p = new QPainter();

    color = color_cursor;
    p->begin(this);
    //p->setRenderHint(QPainter::Antialiasing);

    //Frame of the browser
    if(hasFocus())
    {
        p->setPen(QColor(190,190,190));
        p->drawRect(0,0,width()-1,height()-1);
        p->drawRect(1,1,width()-3,height()-3);
        p->setPen(QColor(130,130,130));
        p->drawRect(2,2,width()-5,height()-5);
        p->drawRect(3,3,width()-7,height()-7);
        p->setPen(QColor(90,90,90));
        p->drawRect(4,4,width()-9,height()-9);
        p->drawRect(5,5,width()-11,height()-11);
        p->setPen(QColor(0,0,0));
        p->drawRect(6,6,width()-13,height()-13);
        p->setClipRect(7,7,width()-15,height()-15);
    }
    else
    {
        p->setPen(QColor(220,220,220));
        p->drawRect(0,0,width()-1,height()-1);
        p->drawRect(1,1,width()-3,height()-3);
        p->setPen(QColor(190,190,190));
        p->drawRect(2,2,width()-5,height()-5);
        p->drawRect(3,3,width()-7,height()-7);
        p->setPen(QColor(130,130,130));
        p->drawRect(4,4,width()-9,height()-9);
        p->drawRect(5,5,width()-11,height()-11);
        p->setPen(QColor(50,50,50));
        p->drawRect(6,6,width()-13,height()-13);
        p->setClipRect(7,7,width()-15,height()-15);
    }

    int posX=LMARGIN,posY=TMARGIN;

    p->setPen(QColor(0,0,0));

    if(top == NULL || cur == NULL || start == NULL)
    {
        p->end();
        delete p;
        return;
    }

    //Calculating the width of columns
    double dv;
    for(c=0;c<(int)colw.size();++c)
        allcw+=colw[c]+CELLHMARGIN*2;
    dv = (double)(width()-(allcw+2*LMARGIN+BARW)) / (double)colw.size();
    if(dv < 1)
        plusw = 0;
    else
        plusw = (int)dv;

    while(head.size() < colw.size())
        head.push_back("");

    //Flush the ring cache if something marginal thing is changed
    if(oldw != width() || oldh != height() || old_col_scroll != col_scroll)
        fulshDrawingCache();

    old_col_scroll = col_scroll;
    oldw = width();
    oldh = height();

    //Allow bigger cache if necessary
    if( (int)((height()+50)/(rowh + 2*CELLVMARGIN)) > cacheLimit)
         cacheLimit = ((int)((height()+50)/(rowh + 2*CELLVMARGIN)) + 5);

    //Draw the headers
    //...Little back arrow, if we scroll horizontally
    if(col_scroll != 0)
    {
        QPen p1=p->pen(),
             p2=p->pen();

        p2.setWidth(3);
        p2.setColor(color_header);
        p->setPen(p2);
        p->drawLine(LMARGIN+CELLHMARGIN,TMARGIN+CELLVMARGIN,LMARGIN+CELLHMARGIN-5,TMARGIN+CELLVMARGIN+(rowh+CELLVMARGIN)/2);
        p->drawLine(LMARGIN+CELLHMARGIN,TMARGIN+CELLVMARGIN+rowh+CELLVMARGIN,LMARGIN+CELLHMARGIN-5,TMARGIN+CELLVMARGIN+(rowh+CELLVMARGIN)/2);
        p->setPen(p1);
    }
    //...Cells of header cells
    for(c=col_scroll;c<(int)colw.size();++c)
    {
        p->fillRect(posX+CELLHMARGIN,posY+CELLVMARGIN,colw[c]+CELLHMARGIN+plusw-1,rowh+CELLVMARGIN,
                        QBrush( c == curc ? color_header_cursor : color_header));

        p->drawText(posX+CELLHMARGIN+1,posY+CELLVMARGIN+1,colw[c]+CELLHMARGIN+plusw,rowh+CELLVMARGIN,QTextOption::NoWrap, head[c] );

        //Draws the little sorting arrows
        if(keycolumn == c && asc) //0 - NO sort  1 - ascend  2 - descend
          p->drawPixmap(posX+CELLHMARGIN+colw[c]+CELLHMARGIN+plusw-BARW,posY+CELLVMARGIN,*pix_asc);
        if(keycolumn == c && !asc) //0 - NO sort  1 - ascend  2 - descend
          p->drawPixmap(posX+CELLHMARGIN+colw[c]+CELLHMARGIN+plusw-BARW,posY+CELLVMARGIN,*pix_desc);

        if(keycolumn2 == c && asc2) //0 - NO sort  1 - ascend  2 - descend
          p->drawPixmap(posX+CELLHMARGIN+colw[c]+CELLHMARGIN+plusw-BARW,posY+CELLVMARGIN,*pix_asc2);
        if(keycolumn2 == c && !asc2) //0 - NO sort  1 - ascend  2 - descend
          p->drawPixmap(posX+CELLHMARGIN+colw[c]+CELLHMARGIN+plusw-BARW,posY+CELLVMARGIN,*pix_desc2);

        posX += colw[c] + 2*CELLHMARGIN + plusw;
    }

    //put a little more place between the header and the first cell...
    posY += rowh + 4*CELLVMARGIN;

    dcI=NULL;
    showrow=0;
    //Drawing data rows
    for(;(posY  < height()+rowh) && run != NULL;run = run->next)
    {
        posX=LMARGIN;

        if(cur != run) //If we are't drawing the cursor's row
        {
            cacheMutex.lock();
            if(run->cachePixmap == NULL) //if the cache is empty
            {
                run->cachePixmap = pix = new QPixmap(width()-5,rowh + 2*CELLVMARGIN);
                pix->fill(Qt::white);
                QPainter *cP = new QPainter(pix);

                //drawing cache pixmap..........................................
                posX=0;
                for(c=col_scroll;c<(int)colw.size();++c)
                {
                    if(run->owncolor)
                        cellcolor = QColor(run->ocr,run->ocg,run->ocb);
                    else
                        cellcolor = colc[c];

                    //Color of the body of the cells.
                    cP->fillRect(posX+CELLHMARGIN,CELLVMARGIN,colw[c]+CELLHMARGIN+plusw-1,rowh+CELLVMARGIN-1,
                                QBrush(cellcolor));

                    cP->drawText(posX+CELLHMARGIN+1,CELLVMARGIN+1,colw[c]+CELLHMARGIN+plusw,rowh+CELLVMARGIN,
                                QTextOption::NoWrap,(*run)[c] );
                    posX += colw[c] + 2*CELLHMARGIN + plusw;

                }
                //end drawing cache pixmap.......................................
                cP->end();
                delete cP;

                cachedElements.push_front(run);
            }

            p->drawPixmap(LMARGIN,posY,*(run->cachePixmap));
            cacheMutex.unlock();

            posY += rowh + 2*CELLVMARGIN;
            ++showrow;
            continue;
        }

        dcY = posY;
        dcI = run;

        posY += rowh + 2*CELLVMARGIN;
        ++showrow;
    }

    //Draw the cursor line if visible
    if(dcI != NULL)
    {
        posX=LMARGIN;
        for(c=col_scroll;c<(int)colw.size();++c)
        {
            cellcolor =  color;
            //...Little back arrow,if we scroll horizontally
            if(c==col_scroll && col_scroll != 0)
            {
                QPen p1=p->pen(),
                     p2=p->pen();

                p2.setWidth(3);
                p2.setColor(cellcolor);
                p->setPen(p2);
                p->drawLine(LMARGIN+CELLHMARGIN,dcY+CELLVMARGIN,LMARGIN+CELLHMARGIN-5,dcY+CELLVMARGIN+(rowh+CELLVMARGIN)/2);
                p->drawLine(LMARGIN+CELLHMARGIN,dcY+CELLVMARGIN+rowh+CELLVMARGIN,LMARGIN+CELLHMARGIN-5,dcY+CELLVMARGIN+(rowh+CELLVMARGIN)/2);
                p->setPen(p1);
            }

            //Color of the body of the cells.
            p->fillRect(posX+CELLHMARGIN,dcY+CELLVMARGIN,colw[c]+CELLHMARGIN+plusw-1,rowh+CELLVMARGIN-1,
                        QBrush(cellcolor));
            if(c == curc)
            {
                //cursor inner
                p->setPen(QColor(60,60,60));
                p->drawRect(posX+CELLHMARGIN-2          ,dcY+CELLVMARGIN-2,
                            colw[c]+CELLHMARGIN+3+plusw ,rowh+CELLVMARGIN+3); //Qt4-Qt3 4->3

                //...lines
                p->setPen(QColor(220,220,220));

                p->drawLine(posX+CELLHMARGIN-2+20                             ,dcY+CELLVMARGIN-2,
                            posX+CELLHMARGIN-2+colw[c]+CELLHMARGIN+4+plusw-20 ,dcY+CELLVMARGIN-2);

                p->drawLine(posX+CELLHMARGIN-2+20                             ,dcY+CELLVMARGIN-2+rowh+CELLVMARGIN+3,
                            posX+CELLHMARGIN-2+colw[c]+CELLHMARGIN+4+plusw-20 ,dcY+CELLVMARGIN-2+rowh+CELLVMARGIN+3);

                // ... cursor outer
                p->setPen(QColor(0,0,0));
                p->drawRect(posX+CELLHMARGIN-1,dcY+CELLVMARGIN-1,
                            colw[c]+CELLHMARGIN+1+plusw,rowh+CELLVMARGIN+1); //Qt4-Qt3 2->1

                if(!seek.isEmpty()) //Seeking is active
                {
                    if( (*dcI)[c].startsWith(seek) )
                    {
                        p->fillRect(posX+CELLHMARGIN,dcY+CELLVMARGIN,
                                    QApplication::fontMetrics().QFONTMETRICS_STRING_HORIZONTAL_WIDTH(seek),rowh+CELLVMARGIN,
                                    QColor(254,230,100));
                    }
                }
                //Inner white cursor (frame)
                p->setPen(QColor(254,254,254));
                p->drawRect(posX+CELLHMARGIN,dcY+CELLVMARGIN,colw[c]+CELLHMARGIN+plusw-1,rowh+CELLVMARGIN-1); //Qt3-Qt4 w,h = -1
                p->setPen(QColor(0,0,0));
            }
            p->drawText(posX+CELLHMARGIN+1,dcY+CELLVMARGIN+1,colw[c]+CELLHMARGIN+plusw,rowh+CELLVMARGIN,
                        QTextOption::NoWrap,(*dcI)[c] );
            posX += colw[c] + 2*CELLHMARGIN + plusw;
        }
    }

    //side BAR
    int kx,ky,sx,sy;

    p->fillRect(width()-BARW-LMARGIN,6,BARW+LMARGIN,height()-12,color);
    p->drawRect(width()-BARW-LMARGIN,6,BARW+LMARGIN,height()-12);

    kx = width()-BARW-LMARGIN+4-1;
    sx = BARW-3;

    if(number == 0 || number <= showrow)
    {
        sy = height()-(2*TMARGIN+BUTTH);
        ky = TMARGIN+BUTTH;
    }
    else
    {
        sy = border(15,height()-(2*TMARGIN+BUTTH),
                    (int)(showrow * (height()-(2*(BUTTH+8))) / number) );

        ky = border(TMARGIN+BUTTH,height()-(2*TMARGIN+BUTTH),
                    (int)TMARGIN+BUTTH+(top->index * (height()-2*(BUTTH+TMARGIN)-sy) / (number-showrow))  );
    }

    p->fillRect(kx,ky,sx,sy,QColor(230,230,250));
    p->drawLine(kx+2,ky+2,kx+2,ky+sy-2);
    p->drawLine(kx+6,ky+2,kx+6,ky+sy-2);
    p->drawLine(kx+2,ky+sy/2,kx+6,ky+sy/2);

    //BAR-BUTTONS
    p->drawPixmap(width()-BARW-LMARGIN+2 ,8,*pix_up);
    p->drawPixmap(width()-BARW-LMARGIN+2 ,height()-(BUTTH+8),*pix_down);

    p->end();
    delete p;

    //freeing some cache data if necessary
    limitCache();

    if(lastkey != cur->key)
    {
        lastkey = cur->key;
        emit itemChanged(lastkey);
    }
}

void HTableBrowser::resizeEvent(QResizeEvent *e)
{
    //fulshDrawingCache();
    QFrame::resizeEvent(e);
}

int HTableBrowser::numberOfElements(void)
{
    return number;
}

HTableBrowserElement * HTableBrowser::elementByIndex(int n)
{
    HTableBrowserElement *c;

    for(c = start; c != NULL ;c=c->next)
        if(c->index == n)
            return c;
    return c;
}

QStringList *HTableBrowser::valuesByIndex(int n)
{
    HTableBrowserElement *c;

    c = elementByIndex(n);

    if(c == NULL)
        return NULL;

    return new QStringList(*c->rows);
}

QString *HTableBrowser::keyByIndex(int n)
{
    HTableBrowserElement *c;

    c = elementByIndex(n);

    if(c == NULL)
        return NULL;

    return new QString(c->key);
}

void HTableBrowser::queryToStart(void)
{   query_cursor = start;   }

void HTableBrowser::queryToTop(void)
{   query_cursor = top;     }

void HTableBrowser::queryToCurrent(void)
{   query_cursor = cur;     }

HTableBrowserElement *HTableBrowser::queryNextElement(void)
{
    HTableBrowserElement *r;

    r = query_cursor;
    if(query_cursor != NULL)
        query_cursor = query_cursor->next;
    return r;
}

QStringList *HTableBrowser::queryNextValues(void)
{
    HTableBrowserElement *r;

    r = query_cursor;

    if(query_cursor != NULL)
    {
        query_cursor = query_cursor->next;
        return new QStringList(*r->rows);
    }
    return NULL;
}

QString *HTableBrowser::queryNextKey(void)
{
    HTableBrowserElement *r;

    r = query_cursor;
    if(query_cursor != NULL)
    {
        query_cursor = query_cursor->next;
        return new QString(r->key);
    }
    return NULL;
}

bool HTableBrowser::hasKey(QString key)
{
    QString *strp;

    bool found = false;
    queryToTop();

    do
    {
        strp = queryNextKey();
        if(strp == NULL)
            continue;
        if(*strp == key)
            found = true;
        delete strp;
    }
    while(strp != NULL && !found);
    return found;
}
// /////////////////////////////////////////////////////////////////////////////////////////

HTableBrowserElement::HTableBrowserElement(QString k,QStringList *c)
{
    key = k;
    rows = new QStringList(*c);
    owncolor = false;
    ocr=0; ocg=0; ocb=0;

    next = prev = NULL;
    cachePixmap = NULL;
}

HTableBrowserElement::HTableBrowserElement(QString k,QString v1,QString v2,QString v3,QString v4,QString v5,
                                           QString v6,QString v7,QString v8,QString v9)
{
    key = k;
    rows = new QStringList();
    rows->push_back(v1);
    owncolor = false;
    ocr=0; ocg=0; ocb=0;

    if(v2.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v2);           }

    if(v3.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v3);           }

    if(v4.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v4);           }

    if(v5.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v5);           }

    if(v6.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v6);           }

    if(v7.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v7);           }

    if(v8.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v8);           }

    if(v9.isEmpty())   { next = prev = NULL; cachePixmap = NULL; return;   }
    else               { rows->push_back(v9);           }

    next = prev = NULL;
    cachePixmap = NULL;
}

HTableBrowserElement::~HTableBrowserElement(void)
{
    if(cachePixmap != NULL)
        delete cachePixmap;
    cachePixmap = NULL;

    delete rows;
}

HTableBrowserElement * HTableBrowserElement::setColor(int r,int g,int b)
{
    owncolor = true;
    ocr=r;
    ocg=g;
    ocb=b;
    return this;
}

HTableBrowserElement * HTableBrowserElement::clearColor(void)
{
    owncolor = false;
    ocr=0;
    ocg=0;
    ocb=0;
    return this;
}


QString HTableBrowserElement::operator[](int i)
{
    if(rows == NULL)
        return "";
    if(rows->size() <= i)
        return "";
    return (*rows)[i];
}

int HTableBrowserElement::size(void)
{
    if(rows == NULL)
        return 0;
    return rows->size();
}

HTableBrowserElement& HTableBrowserElement::operator=(HTableBrowserElement& e)
{
    rows->clear();
    *rows = *(e.rows);
    key  = e.key;

    owncolor = e.owncolor;
    ocr = e.ocr; ocg = e.ocg; ocb = e.ocb;

    if(cachePixmap != NULL)
        delete cachePixmap;
    cachePixmap = NULL;

    return *this;
}

HTableBrowserElement& HTableBrowserElement::operator=(HTableBrowserElement* e)
{
    rows->clear();
    *rows = *(e->rows);
    key  = e->key;

    owncolor = e->owncolor;
    ocr = e->ocr; ocg = e->ocg; ocb = e->ocb;

    if(cachePixmap != NULL)
        delete cachePixmap;
    cachePixmap = NULL;

    return *this;
}

void HTableBrowserElement::swapWith(HTableBrowserElement *x)
{
    QStringList *t1;
    QString      t2;
    bool towncolor;
    int  tocr,tocg,tocb;

    if(cachePixmap != NULL)
        delete cachePixmap;
    cachePixmap = NULL;

    if(x->cachePixmap != NULL)
        delete x->cachePixmap;
    x->cachePixmap = NULL;

    t1      = rows;
    t2      = key;
    towncolor = owncolor;
    tocr = ocr; tocg = ocg; tocb = ocb;

    rows    = x->rows;
    key     = x->key;
    owncolor = x->owncolor;
    ocr = x->ocr; ocg = x->ocg; ocb = x->ocb;

    x->rows = t1;
    x->key  = t2;
    x->owncolor = towncolor;
    x->ocr = tocr; x->ocg = tocg; x->ocb = tocb;
}

// ///////////////////////////////////////////////////////////////////////////////
// / Plain matrix class //////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDispPlainDataMatrix::HDispPlainDataMatrix(QWidget *parent,HBase *d,bool htmlmode,int keyfield,int ddata)
: QFrame(parent)
{
    int i;
    data = (HPlainDataMatrix *)d;
    deletedata = ddata;

    genhtml_parameters = "html center";
    QVBoxLayout  *layout= new QVBoxLayout(this);
                  layout->setMargin(2);
                  layout->setSpacing(2);

    list = new HTableBrowser(this);

    QToolButton *tp,*tx,*th;
    QHBoxLayout *toplay = new QHBoxLayout(0);
    tx = new QToolButton(this);
    tp = new QToolButton(this);
    tx->setIcon(QPixmap(":/GSAFEPIXMAPS/image_x.png"));
    tp->setIcon(QPixmap(":/GSAFEPIXMAPS/image_p.png"));

    toplay->addSpacing(5);
    toplay->addWidget(tp);
    toplay->addSpacing(3);
    toplay->addWidget(tx);
    if(htmlmode)
    {
        th = new QToolButton(this);
        th->setIcon(QPixmap(":/GSAFEPIXMAPS/image_f.png"));
        toplay->addSpacing(3);
        toplay->addWidget(th);

        connect(th,SIGNAL(clicked()),this,SLOT(dialogHtmlShow()));
    }
    toplay->addStretch();

    layout->addLayout(toplay);
    connect(tx,SIGNAL(clicked()),this,SLOT(dialogXml()));
    connect(tp,SIGNAL(clicked()),this,SLOT(dialogPrint()));

    layout->addWidget(list);

    if(data->keyfield != -2)
        keyfield = data->keyfield;

    i = 0;
    QStringList *sl = new QStringList();
    QString k;
    if(data->rowCount() > 0)
    {
        data->firstRow();
        do
        {
            ++i;
            *sl = data->currentRowStr();
            if(keyfield == -1)
                k = QString("%1").arg(i);
            else
                k = (*sl)[keyfield];
            list->addElement(new HTableBrowserElement(k,sl));
        }
        while(data->nextRow());
    }
    list->setHeadTexts(data->getHeader());
    delete sl;

    connect(list,SIGNAL(activateItem(const QString&)),this,SLOT(slotActivateItem(const QString&)));
    connect(list,SIGNAL(alternateActivateItem(const QString&)),this,SLOT(itemAlternateActivated(const QString&)));
}

int HDispPlainDataMatrix::slotActivateItem(const QString k)
{
    QString key;

    key = k;
    emit activateItem(key);
    data->actLine(k);
    return 0;
}

int HDispPlainDataMatrix::itemAlternateActivated(const QString& s)
{
    data->alternateActLine(s);
    return 0;
}

int HDispPlainDataMatrix::dialogPrint(void)
{
    HPrintTable *d = new HPrintTable(this,data);
    d->exec();
    delete d;
    return 0;
}

int HDispPlainDataMatrix::dialogXml(void)
{
    HXmloTable *d = new HXmloTable(this,data);
    d->exec();
    delete d;
    return 0;
}

int HDispPlainDataMatrix::dialogHtmlShow(void)
{
    HShowPrintHtml *d = new HShowPrintHtml(this);
    d->setContent(data->getContentAsHtml(genhtml_parameters));
    d->setWinTitle(data->getTitle());
    d->exec();
    delete d;
    return 0;
}

HDispPlainDataMatrix::~HDispPlainDataMatrix(void)
{
    if(deletedata == 1)
    {
        delete data;
    }
}

/////////////////////////////////////////////////////////////////////////////////

HPlainDMD::HPlainDMD(QWidget *parent,HBase *d,bool htmlmode)
: QDialog(parent)
{
    QVBoxLayout  *layout= new QVBoxLayout(this);
    QHBoxLayout  *bottom= new QHBoxLayout(0);
    dpdm = new HDispPlainDataMatrix(this,d,htmlmode);

    QPushButton *okb = new QPushButton("Ok",this);
    bottom->addStretch();
    bottom->addWidget(okb);
    bottom->addStretch();


    layout->addWidget(dpdm);
    layout->addLayout(bottom);
    setWindowTitle(((HPlainDataMatrix *)d)->getTitle());
    setSizeGripEnabled(true);

    connect(okb,SIGNAL(clicked()),this,SLOT(accept()));

    resize(400,200);
}

HPlainDMD::~HPlainDMD(void)
{

}

// ///////////////////////////////////////////////////////////////////////////////
// / HDispList class /////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDispList::HDispList(QWidget *parent,HList *d,int datamodep,int ddata)
:QFrame(parent)
{
    sdebug("*** HDispList::HDispList ***");
    bool ok;
    list = NULL;
    data = d;

    deletedata = ddata;
    datamode = datamodep;

    if(datamode == FULL_CLEAN || datamode == START_CLEAN)
        data->clearList();

    QVBoxLayout  *layout= new QVBoxLayout(this);
                  layout->setMargin(2);
                  layout->setSpacing(2);
    list = new HTableBrowser(this);

    if(data->extrafeatures)
    {
        QToolButton *tp,*tx;

        QHBoxLayout *toplay = new QHBoxLayout(0);
        tx = new QToolButton(this);
        tp = new QToolButton(this);
        tx->setIcon(QPixmap(":/GSAFEPIXMAPS/image_x.png"));
        tp->setIcon(QPixmap(":/GSAFEPIXMAPS/image_p.png"));
        toplay->addSpacing(5);
        toplay->addWidget(tp);
        toplay->addSpacing(3);
        toplay->addWidget(tx);
        toplay->addStretch();

        layout->addLayout(toplay);
        connect(tx,SIGNAL(clicked()),this,SLOT(dialogXml()));
        connect(tp,SIGNAL(clicked()),this,SLOT(dialogPrint()));
    }

    if(data->gui_showtabletitle && !data->tableTitle().isEmpty())
    {
        QLabel *titlelabel = new QLabel(this);
        titlelabel->setText(QString("<strong>%1</strong>").arg(data->tableTitle()));
        QHBoxLayout *titlelayout = new QHBoxLayout(0);
        titlelayout->addStretch();
        titlelayout->addWidget(titlelabel);
        titlelayout->addStretch();
        layout->addLayout(titlelayout);
    }

    layout->addWidget(list);

    ok = connect(data,SIGNAL(dataUpdatedSignal()),this,SLOT(updateDisplay()));
    sdebug(QString("connect0: ") + (ok ? "ok" : "not-ok"));

    updateDisplay();
    connect(list,SIGNAL(activateItem(const QString&)),this,SLOT(itemActivated(const QString&)));
    connect(list,SIGNAL(alternateActivateItem(const QString&)),this,SLOT(itemAlternateActivated(const QString&)));
    connect(list,SIGNAL(itemChanged(const QString&)),this,SLOT(litemChanged(const QString&)));
    sdebug("*** HDispList::HDispList *** END");
}

int HDispList::litemChanged(const QString& s)
{
    data->soft_current_key = s;
    emit listItemChanged();
    return 0;
}

int HDispList::sortByColumn(int col)
{
    list->sort(col);
    list->update();
    return 0;
}

int HDispList::dialogPrint(void)
{
    HPrintTable *d = new HPrintTable(this,data);
    d->exec();
    delete d;
    return 0;
}

int HDispList::dialogXml(void)
{
    HXmloTable *d = new HXmloTable(this,data);
    d->exec();
    delete d;
    return 0;
}

HDispList::~HDispList(void)
{
    sdebug("*** HDispList::~HDispList ***");
    if(datamode == FULL_CLEAN || datamode == END_CLEAN)
        data->clearList();
    if(deletedata == 1)
    {
        data->clearAndFreeConnections();
        delete data;
    }
    sdebug("*** HDispList::~HDispList *** END");
}

int HDispList::updateDisplay(void)
{
    QString oldtop;
    sdebug("*** HDispList::updateDisplay *** #"+ data->sqlTableName());
    int i;

    oldtop = list->topKey();
    list->clear();
    list->setHeadTexts(data->getShortTitleHeads());
    QStringList::Iterator ik               = data->getKeys()->begin();
    QList<QStringList *>::Iterator iv = data->getValues()->begin();
    while(iv != data->getValues()->end() && ik != data->getKeys()->end())
    {
        list->addElement(new HTableBrowserElement(*ik,*iv));
        ++iv;
        ++ik;
    }
    i=0;
    data->firstField();
    HDataField *df;
    while((df = data->nextFieldAll()))
    {
        if(df->isShow() && df->isSQLField())
        {
            list->setColumnColor(i,QColor(df->getRColor(),df->getGColor(),df->getBColor()));
            if(!df->getSpecDisplaySortMode().isEmpty())
                list->setSpecSort(i,df->getSpecDisplaySortMode());
            ++i;
        }
    }

    list->setCurrentElement(data->activeKey(),oldtop);
    list->repaint();
    repaint();

    sdebug("*** HDispList::updateDisplay *** END");
    return 0;
}
void HDispList::keyPressEvent(QKeyEvent *e)
{
    if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_R)
    {
        data->reReadList();
    }
    QFrame::keyPressEvent(e);
}

int HDispList::itemActivated(const QString& s)
{
    sdebug("*** HDispList::itemActivated ***");
    data->setActiveKey(s);
    data->actLine();
    sdebug("*** HDispList::itemActivated *** END");
    return 0;
}

int HDispList::itemAlternateActivated(const QString& s)
{
    sdebug("*** HDispList::itemAlternateActivated ***");
    data->setActiveKey(s);
    data->alternateActLine();
    sdebug("*** HDispList::itemAlternateActivated *** END");
    return 0;
}

// ///////////////////////////////////////////////////////////////////////////////
// / HDispTable class ////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDispTable::HDispTable(QWidget *parent,HTable *d,int datamodep,int ddata)
:QFrame(parent)
{
    sdebug("*** HDispTable::HDispTable ***");

    bool ok;
    HDataField *df=NULL;
    HDispDataField *ddf=NULL;

    deletedata = ddata;
    datamode = datamodep;
    layout= new QVBoxLayout(this);
    layout->setMargin(2);
    layout->setSpacing(2);

    data = d;

    ro_mask = data->readonlyMask();

    if(datamode == FULL_CLEAN || datamode == START_CLEAN)
        data->returnToDefault();

    if(data->extrafeatures)
    {
        QToolButton *tp,*tx;

        QHBoxLayout *toplay = new QHBoxLayout(0);
        tx = new QToolButton(this);
        tp = new QToolButton(this);
        tx->setIcon(QPixmap(":/GSAFEPIXMAPS/image_x.png"));
        tp->setIcon(QPixmap(":/GSAFEPIXMAPS/image_p.png"));
        toplay->addSpacing(5);
        toplay->addWidget(tp);
        toplay->addSpacing(3);
        toplay->addWidget(tx);
        toplay->addStretch();

        layout->addLayout(toplay);
        connect(tx,SIGNAL(clicked()),this,SLOT(dialogXml()));
        connect(tp,SIGNAL(clicked()),this,SLOT(dialogPrint()));
    }

    if(data->gui_showtabletitle && !data->tableTitle().isEmpty())
    {
        QLabel *titlelabel = new QLabel(this);
        titlelabel->setText(QString("<strong>%1</strong>").arg(data->tableTitle()));
        QHBoxLayout *titlelayout = new QHBoxLayout(0);
        titlelayout->addStretch();
        titlelayout->addWidget(titlelabel);
        titlelayout->addStretch();
        layout->addLayout(titlelayout);
    }

    data->firstField();
    while((df=data->nextFieldAll()) != NULL)
        {
            QHBoxLayout* static_sublayout_before = HDispDataField::static_sublayout;
            if(!df->isShow()) continue;

            if(df->getWhoami() == "HKey")           { ddf=new HDispKey(this,df,ro_mask);        }
            if(df->getWhoami() == "HSmallText")     { ddf=new HDispSmallText(this,df,ro_mask);  }
            if(df->getWhoami() == "HLargeText")     { ddf=new HDispLargeText(this,df,ro_mask);  }
            if(df->getWhoami() == "HCharHash")      { ddf=new HDispCharHash(this,df,ro_mask);   }
            if(df->getWhoami() == "HNumHash")       { ddf=new HDispNumHash(this,df,ro_mask);    }
            if(df->getWhoami() == "HNumber")        { ddf=new HDispNumber(this,df,ro_mask);     }
            if(df->getWhoami() == "HStatic")        { ddf=new HDispStatic(this,df,ro_mask);     }
            if(df->getWhoami() == "HFloating")      { ddf=new HDispFloating(this,df,ro_mask);   }
            if(df->getWhoami() == "HDate")          { ddf=new HDispDate(this,df,ro_mask);       }
            if(df->getWhoami() == "HCheck")         { ddf=new HDispCheck(this,df,ro_mask);      }
            if(df->getWhoami() == "HTimestamp")     { ddf=new HDispTimestamp(this,df,ro_mask);  }
            if(df->getWhoami() == "HSqlChoose")     { ddf=new HDispSqlChoose(this,df,ro_mask);  }

            ok = connect(this,SIGNAL(updateDisplaySignal()),ddf,SLOT(updateDisplaySlot()));
            if(!ok) error("libGUI system HDispTable constructor:\n Cannot connect to subfield slot! (1)");

            ok = connect(df,SIGNAL(disableControl()),ddf,SLOT(disableControlSlot()));
            if(!ok) error("libGUI system HDispTable constructor:\n Cannot connect to subfield slot! (2)");
            ok = connect(df,SIGNAL( enableControl()),ddf,SLOT( enableControlSlot()));
            if(!ok) error("libGUI system HDispTable constructor:\n Cannot connect to subfield slot! (3)");

            ok = connect(df,SIGNAL(updateStaticDisplayElementsSignal()),ddf,SLOT(updateStaticGuiElementsSlot()));
            if(!ok) error("libGUI system HDispTable constructor:\n Cannot connect to subfield slot! (4)");


            if(df->isGuienabled())
                ddf->enableControlSlot();
            else
                ddf->disableControlSlot();

            if(static_sublayout_before == NULL)
                layout->addWidget(ddf);
            else
                static_sublayout_before->addWidget(ddf);

        }
    connect(data,SIGNAL(dataUpdatedSignal()),this,SLOT(updateDisplay()));
    sdebug("*** HDispTable::HDispTable *** END");
}

int HDispTable::addStretchToEnd(void)
{
    //layout->addStretch();
    QFrame *fr = new QFrame(this);

    QPalette palette;
    palette.setColor(QPalette::Window,QColor(180,180,180));
    fr->setPalette(palette);
    fr->setAutoFillBackground(true);

    QVBoxLayout *lay = new QVBoxLayout(fr);
    lay->addStretch();

    layout->addWidget(fr);

    return 0;
}

int HDispTable::dialogPrint(void)
{
    HPrintTable *d = new HPrintTable(this,data);
    d->exec();
    delete d;
    return 0;
}

int HDispTable::dialogXml(void)
{
    HXmloTable *d = new HXmloTable(this,data);
    d->exec();
    delete d;
    return 0;
}

HDispTable::~HDispTable(void)
{
    sdebug("*** HDispTable::~HDispTable ***");
    if(datamode == FULL_CLEAN || datamode == END_CLEAN)
        data->returnToDefault();
    if(deletedata == 1)
    {
        data->clearAndFreeConnections();
        delete data;
    }
    sdebug("*** HDispTable::~HDispTable *** END");
}

int HDispTable::updateDisplay(void)
{
    sdebug("*** HDispTable::updateDisplay ***");
    /*
     I emits a signal which is connected the the slots of the child widgets.
     That's way I notify them to refresh the display from the memory.
    */
    emit updateDisplaySignal();
    sdebug("*** HDispTable::updateDisplay *** END");
    return 0;
}

// ///////////////////////////////////////////////////////////////////////////////
// / Standard data fields ////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////
QHBoxLayout* HDispDataField::static_sublayout = NULL;

HDispDataField::HDispDataField(QWidget *parent,HDataField *d,bool _ro_mask)
:QFrame(parent)
{
    data = d;
    ro_mask = _ro_mask;

    blink = 0;

    if(data->getRColor() != 0 &&
       data->getGColor() != 0 &&
       data->getBColor() != 0   )
    {
            QPalette palette;
            palette.setColor(QPalette::Window,QColor(data->getRColor(),
                                                     data->getGColor(),
                                                     data->getBColor() ));

            setPalette(palette);
            setAutoFillBackground(true);
    }

    sublayout = new QHBoxLayout(this);
    sublayout->setMargin(2);
    sublayout->setSpacing(2);

    if(d->isMeldedWithNext())
        static_sublayout = sublayout;
    else
        static_sublayout = NULL;

    tbool = false;
}

void HDispDataField::enableControl(void)
{
}

void HDispDataField::disableControl(void)
{
}


int HDispDataField::updateDisplaySlot(void)
{
    /* Empty. Don't metter here. It's reimplemented in descendants */
    return 0;
}

int HDispDataField::timedUpdateSlot(void)
{
    sdebug("*** HDispDataField::timedUpdateSlot ***");

    if(!data->getLastValidateStatus() )
    {
        blink = 0;
        update();
    }

    if(blink > 0)
    {
        blink++;
            if(blink >= 3) blink = 1;

        update();
        QTimer::singleShot(500,this,SLOT(timedUpdateSlot()));
    }
    return 0;
}

void HDispDataField::paintEvent(QPaintEvent *e)
{

    if((blink == 0 && data->getLastValidateStatus()) ) //Field contains invalid data or already run blinking
    {
        blink = 1;
        timedUpdateSlot();
    }

    if(blink > 0)
    {
            QPainter p(this);
            p.setPen( blink%2 == 0 ? Qt::red : Qt::black);
            p.drawRect(0,0,width()-1,height()-1);
    }
    QFrame::paintEvent(e);
}

HDispDataField::~HDispDataField(void)
{

}

// ///////////////////////////////////////////////////////////////////////////////
// / Special data fields /////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDispKey::HDispKey(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    explain   = new QLabel(this);
    key       = new QLabel(this);

    explain->setText(data->getExplainText());
    key->setText(data->getValue().toString());

    sublayout->addWidget(explain);
    sublayout->addWidget(key);
}

void HDispKey::enableControl(void)
{
}

void HDispKey::disableControl(void)
{
}

int HDispKey::updateDisplaySlot(void)
{
    //sdebug("*** HDispKey::updateDisplaySlot ***");
    key->setText(data->getValue().toString());
    //sdebug("*** HDispKey::updateDisplaySlot *** END ");
    return 0;
}

int HDispKey::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    return 0;
}

HDispKey::~HDispKey(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////

HDispSmallText::HDispSmallText(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edtext = NULL;
    nedtext = NULL;
    tailtext = NULL;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {
            edtext    = new QLineEdit(this);
            edtext->setText(data->getValue().toString());
            if(d->editBoxMaxWidth() != 0)
                edtext->setMaximumWidth(d->editBoxMaxWidth());

            sublayout->addWidget(edtext);
            if(!connect(edtext,SIGNAL(textChanged(const QString&)),this,SLOT(textUpdate(const QString&))))
                error("Error, connect(edtext,SIGNAL(textChanged(const QString&)),this,SLOT(textUpdate(const QString&)))");
            if(((HSmallText *)data)->isInitialSelected())
            {
                edtext->selectAll();
                sdebug(" HDispSmallText: Initial selection is YES -- call selectAll()");
            }
    }
    else
    {
            nedtext   = new QLabel(this);
            nedtext->setText(data->getValue().toString());
            sublayout->addWidget(nedtext);
    }

    if(!data->getTailText().isEmpty())
    {
        tailtext  = new QLabel(this);
        tailtext->setText(data->getTailText());
        sublayout->addWidget(tailtext);
    }
}

void HDispSmallText::enableControl(void)
{
    if(edtext != NULL)
        edtext->setEnabled(true);
}

void HDispSmallText::disableControl(void)
{
    if(edtext != NULL)
        edtext->setEnabled(false);
}

int HDispSmallText::textUpdate(const QString& n) //SLOT!
{
    sdebug("*** HDispSmallText::textUpdate ***");
    data->setValue(QVariant(n));
    sdebug("*** HDispSmallText::textUpdate *** END");
    return 0;
}

int HDispSmallText::updateDisplaySlot(void)
{
    sdebug("*** HDispSmallText::updateDisplaySlot ***");
    if(edtext != NULL)
    {
        if(edtext->text() != data->getValue().toString())
        {
            edtext->setText ( data->getValue().toString() );
            if(((HSmallText *)data)->isInitialSelected())
            {
                edtext->selectAll();
                sdebug(" HDispSmallText: Initial selection is YES -- call selectAll()");
            }
        }
    }
    if(nedtext != NULL) nedtext->setText( data->getValue().toString() );
    sdebug("*** HDispSmallText::updateDisplaySlot *** END");
    return 0;
}

int HDispSmallText::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    if(tailtext != NULL)
        tailtext->setText(data->getTailText());
    return 0;
}

HDispSmallText::~HDispSmallText(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////

HDispLargeText::HDispLargeText(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edtext = NULL;
    nedtext = NULL;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {
            edtext    = new QTextEdit(this);
            edtext->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            edtext->setText(data->getValue().toString());
            edtext->setTabChangesFocus(true);
            edtext->setMinimumSize(100,50);
            sublayout->addWidget(edtext);
            if(!connect(edtext,SIGNAL(textChanged()),this,SLOT(textUpdate())))
                error("Error, connect(edtext,SIGNAL(textChanged()),this,SLOT(textUpdate()))");
    }
    else
    {
            nedtext   = new QTextEdit(this);
            nedtext->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            nedtext->setText(data->getValue().toString());
            nedtext->setReadOnly(true);
            nedtext->setMinimumSize(100,50);
            QPalette palette;
            palette.setColor(QPalette::Base,QColor(data->getRColor(),
                                                         data->getGColor(),
                                                         data->getBColor()));
            nedtext->setPalette(palette);
            sublayout->addWidget(nedtext);
    }
}

void HDispLargeText::enableControl(void)
{
    if(edtext != NULL)
        edtext->setEnabled(true);
}

void HDispLargeText::disableControl(void)
{
    if(edtext != NULL)
        edtext->setEnabled(false);
}

int HDispLargeText::textUpdate(void) //SLOT!
{
    data->setValue(QVariant(edtext->toPlainText()));
    return 0;
}

int HDispLargeText::updateDisplaySlot(void)
{
    if(edtext != NULL)
    {
        if(data->getValue().toString() != edtext->toPlainText())
        {
            edtext->setText ( data->getValue().toString() );
        }
    }
    if(nedtext != NULL)
    {
        nedtext->setText( data->getValue().toString() );
    }
    return 0;
}

int HDispLargeText::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    return 0;
}


HDispLargeText::~HDispLargeText(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////
HDispCharHash::HDispCharHash(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edval = NULL;
    nedtext = NULL;
    tailtext = NULL;
    int curr;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {
            edval     = new QComboBox(this);
            edval->clear();
            edval->addItems( ((HCharHash *)data)->values );

            curr = getIndexOf( ((HCharHash *)data)->keys , data->getValue().toString());
            edval->setCurrentIndex(curr);
            itemSelected(curr);

            sublayout->addWidget(edval);

            if(!connect(edval,SIGNAL(activated(int)),this,SLOT(itemSelected(int))))
                error("Error, connect(edval,SIGNAL(activated(int)),this,SLOT(itemSelected(int)))");
    }
    else
    {
            nedtext   = new QLabel(this);
            curr = getIndexOf( ((HCharHash *)data)->keys , data->getValue().toString());
            nedtext->setText( ((HCharHash *)data)->values[curr] );

            sublayout->addWidget(nedtext);
    }

    if(!data->getTailText().isEmpty())
    {
        tailtext  = new QLabel(this);
        tailtext->setText(data->getTailText());
        sublayout->addWidget(tailtext);
    }
}


void HDispCharHash::enableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(true);
}

void HDispCharHash::disableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(false);
}

int HDispCharHash::itemSelected(int index) //SLOT
{
    sdebug("*** HDispCharHash::itemSelected ***");
    data->setValue(QVariant( ((HCharHash *)data)->keys[index] ));
    sdebug("*** HDispCharHash::itemSelected *** END");
    return 0;
}

int HDispCharHash::updateDisplaySlot(void)
{
    int curr;
    curr = getIndexOf( ((HCharHash *)data)->keys , data->getValue().toString());
    if(edval != NULL)  edval->setCurrentIndex(curr);
    if(nedtext != NULL) nedtext->setText( ((HCharHash *)data)->values[curr] );
    return 0;
}

int HDispCharHash::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    if(tailtext != NULL)
        tailtext->setText(data->getTailText());
    return 0;
}

HDispCharHash::~HDispCharHash(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////

HDispNumHash::HDispNumHash(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edval = NULL;
    nedtext = NULL;
    tailtext = NULL;
    int curr;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {
            edval     = new QComboBox(this);
            edval->clear();
            edval->addItems( ((HNumHash *)data)->values );

            curr = getIndexOf( ((HNumHash *)data)->keys , data->getValue().toString());
            edval->setCurrentIndex(curr);
            itemSelected(curr);

            sublayout->addWidget(edval);

            if(!connect(edval,SIGNAL(activated(int)),this,SLOT(itemSelected(int))))
                error("Error, connect(edval,SIGNAL(activated(int)),this,SLOT(itemSelected(int)))");
    }
    else
    {
            nedtext   = new QLabel(this);
            curr = getIndexOf( ((HNumHash *)data)->keys , data->getValue().toString());
            nedtext->setText( ((HNumHash *)data)->values[curr] );

            sublayout->addWidget(nedtext);
    }

    if(!data->getTailText().isEmpty())
    {
        tailtext  = new QLabel(this);
        tailtext->setText(data->getTailText());
        sublayout->addWidget(tailtext);
    }
}

void HDispNumHash::enableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(true);
}

void HDispNumHash::disableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(false);
}

int HDispNumHash::itemSelected(int index) //SLOT
{
    sdebug("HDispNumHash::itemSelected");
    data->setValue(QVariant( ((HNumHash *)data)->keys[index] ));
    return 0;
}

int HDispNumHash::updateDisplaySlot(void)
{
    int curr;
    curr = getIndexOf( ((HNumHash *)data)->keys , data->getValue().toString());
    if(edval != NULL)  edval->setCurrentIndex(curr);
    if(nedtext != NULL) nedtext->setText( ((HNumHash *)data)->values[curr] );
    return 0;
}

int HDispNumHash::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    if(tailtext != NULL)
        tailtext->setText(data->getTailText());
    return 0;
}


HDispNumHash::~HDispNumHash(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////

HDispNumber::HDispNumber(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edval = NULL;
    nedtext = NULL;
    tailtext = NULL;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {
            edval     = new HSpinBox(this);
            edval->setMinValue( ((HNumber *)data)->getMinimum() );
            edval->setMaxValue( ((HNumber *)data)->getMaximum() );
            edval->setValue( data->getValue().toInt() );
            sublayout->addWidget(edval);

            valueUpdate(edval->value());
            if(!connect(edval,SIGNAL(valueChanged(int)),this,SLOT(valueUpdate(int))))
                error("Error, connect(edval,SIGNAL(valueChanged(int)),this,SLOT(valueUpdate(int)))");
    }
    else
    {
            nedtext   = new QLabel(this);
            nedtext->setText( QString("%1").arg(data->getValue().toInt()) );
            sublayout->addWidget(nedtext);
    }

    if(!data->getTailText().isEmpty())
    {
        tailtext  = new QLabel(this);
        tailtext->setText(data->getTailText());
        sublayout->addWidget(tailtext);
    }
}

void HDispNumber::enableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(true);
}

void HDispNumber::disableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(false);
}

int HDispNumber::valueUpdate(int value) //SLOT
{
    data->setValue(QVariant(value));
    updateDisplaySlot();
    return 0;
}

int HDispNumber::updateDisplaySlot(void)
{
    if(edval != NULL) edval->setValue( data->getValue().toInt() );
    if(nedtext != NULL) nedtext->setText( QString("%1").arg(data->getValue().toInt()) );
    return 0;
}

int HDispNumber::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    if(tailtext != NULL)
        tailtext->setText(data->getTailText());
    return 0;
}

HDispNumber::~HDispNumber()
{
}

// ///////////////////////////////////////////////////////////////////////////////

HDispStatic::HDispStatic(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    nedtext = NULL;

    if(data->getValue().toString().isEmpty())
    {
        sublayout->addStretch(99);
    }

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());

    sublayout->addWidget(explain);

    sublayout->addStretch(99);

    if(!data->getValue().toString().isEmpty())
    {
        nedtext   = new QLabel(this);
        nedtext->setText( data->getValue().toString() );
        sublayout->addWidget(nedtext);

    }
}

void HDispStatic::enableControl(void)
{
}

void HDispStatic::disableControl(void)
{
}

int HDispStatic::updateDisplaySlot(void)
{
    //sdebug("*** HDispStatic::updateDisplaySlot ***");
    if(!data->getValue().toString().isEmpty())
    {
        if(nedtext != NULL)
            nedtext->setText( data->getValue().toString() );
    }
    //sdebug("*** HDispStatic::updateDisplaySlot *** END");
    return 0;
}

int HDispStatic::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    return 0;
}

HDispStatic::~HDispStatic(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////
HDispCheck::HDispCheck(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edval = NULL;
    nedtext = NULL;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {
            edval     = new QCheckBox(this);
            edval->setChecked( data->getValue().toBool() );
            edval->setText( data->getTailText() );
            sublayout->addWidget(edval);
            valueUpdate();
            if(!connect(edval,SIGNAL(clicked()),this,SLOT(valueUpdate())))
                error("Error, connect(edval,SIGNAL(clicked()),this,SLOT(valueUpdate()))");
    }
    else
    {
            nedtext   = new QLabel(this);
            nedtext->setText( QString("%1")
                       .arg(data->getValue().toBool() ?
                                             ((HCheck *)data)->trues : ((HCheck *)data)->falses ));

            sublayout->addWidget(nedtext);
    }
}

void HDispCheck::enableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(true);
}

void HDispCheck::disableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(false);
}

int HDispCheck::valueUpdate(void) //SLOT
{
    data->setValue(QVariant(edval->isChecked()));
    return 0;
}

int HDispCheck::updateDisplaySlot(void) //SLOT
{
    if(edval != NULL)
    {
        edval->setChecked( data->getValue().toBool() );
        edval->setText( data->getTailText() );
    }
    if(nedtext != NULL)
    {
        nedtext->setText( QString("%1")
               .arg(data->getValue().toBool() ?
                       ((HCheck *)data)->trues : ((HCheck *)data)->falses  ));
    }
    return 0;
}

int HDispCheck::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    return 0;
}

HDispCheck::~HDispCheck(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////
HDispFloating::HDispFloating(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edtext = NULL;
    nedtext = NULL;
    tailtext = NULL;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);


    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {
            edtext    = new QLineEdit(this);
            if(d->editBoxMaxWidth() != 0)
                edtext->setMaximumWidth(d->editBoxMaxWidth());
            edtext->setAlignment(Qt::AlignRight);
            edtext->setText(data->getValue().toString());
            //edtext->setText( ((HFloating *)data)->getValueAsString() );

            sublayout->addWidget(edtext);
            if(!connect(edtext,SIGNAL(textChanged(const QString&)),this,SLOT(textUpdate(const QString&))))
                error("Error, connect(edtext,SIGNAL(textChanged(const QString&)),this,SLOT(textUpdate(const QString&)))");
    }
    else
    {
            nedtext   = new QLabel(this);
            nedtext->setText(data->dbValueToDispValue(data->getValue()).toString());
            sublayout->addWidget(nedtext);
    }

    if(!data->getTailText().isEmpty() && nedtext==NULL)
        //If nedtext==NULL the field is readonly so the dbValueToDispValue function appends the tailtext to the value
        //so needless to set this label.
    {
        tailtext  = new QLabel(this);
        tailtext->setText(data->getTailText());
        sublayout->addWidget(tailtext);
    }
}

void HDispFloating::enableControl(void)
{
    if(edtext != NULL)
        edtext->setEnabled(true);
}

void HDispFloating::disableControl(void)
{
    if(edtext != NULL)
        edtext->setEnabled(false);
}

int HDispFloating::textUpdate(const QString& n) //SLOT!
{
    QString number = n;

    sdebug("*** HDispFloating::textUpdate ***");
    bool ok;
    double d;

    number.replace(" ","",Qt::CaseInsensitive);

    if(number.isEmpty() || number == "-" || number==".") //deleted to empty (or a "-" sign), it means 0 for me.
    {
        data->setValue(QVariant(0));
        return 0;
    }

    d = number.toDouble(&ok);
    if(!ok) //can't convert
    {
      error(QString("Cannot convert the string vale to floating!\nColumn \"%1\" (%2) \nPlease corrent the value!")
                                    .arg(data->getExplainText())
                                    .arg(data->sqlSelectHead()));
      updateDisplaySlot(); //set the value last time when we could convert it
      return 0;
    }

    data->setValue(QVariant(d));
    return 0;
}

int HDispFloating::updateDisplaySlot(void) //SLOT
{
    sdebug("*** HDispFloating::updateDisplaySlot ***");
    if(edtext != NULL)  edtext->setText (data->getValue().toString() );
    //if(edtext != NULL)  edtext->setText( ((HFloating *)data)->getValueAsString() );

    if(nedtext != NULL) nedtext->setText(data->dbValueToDispValue(data->getValue()).toString());
    return 0;
}

int HDispFloating::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    if(tailtext != NULL)
        tailtext->setText(data->getTailText());
    return 0;
}

HDispFloating::~HDispFloating(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////
HDispDate::HDispDate(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edval = NULL;
    nedtext = NULL;
    unk = NULL;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    if(!ro_mask && data->isEditable())
    {

           edval      = new QDateEdit(this);
           edval->setCalendarPopup(true);
           edval->setDate(((HDate *)data)->getKnownValue().toDate());

           if(((HDate *)data)->is_unknown_alive())
            {
                unk = new QCheckBox(this);
                unk->setText( GSAFETEXT_UNKNOWN " ");
                if(((HDate *)data)->is_value_unknown())
                {
                    unk->setChecked(true);
                    edval->setEnabled(false);
                }
                else
                {
                    unk->setChecked(false);
                    edval->setEnabled(true);
                }

                sublayout->addWidget(unk);
                if(!connect(unk,SIGNAL(clicked()),this,SLOT(valueUnkUpdate())))
                    error("Error, connect(unk,SIGNAL(clicked()),this,SLOT(valueUnkUpdate()))");

            }
            sublayout->addWidget(edval);
            if(!connect(edval,SIGNAL(dateChanged(const QDate&)),this,SLOT(valueUpdate(const QDate&))))
                error("Error, connect(edval,SIGNAL(dateChanged(const QDate&)),this,SLOT(valueUpdate(const QDate&)))!");
    }
    else
    {
            nedtext   = new QLabel(this);
            if( ((HDate *)data)->is_unknown_alive() && ((HDate *)data)->is_value_unknown() )
                nedtext->setText(GSAFETEXT_TEXT_UNKNOWN);
            else
                nedtext->setText(data->getValue().toString());
            sublayout->addWidget(nedtext);
    }
}

void HDispDate::enableControl(void)
{
    if(edval != NULL)
    {
        if(unk != NULL)
            unk->setEnabled(true);

        if(((HDate *)data)->is_value_unknown())
        {
            edval->setEnabled(false);
        }
        else
        {
            edval->setEnabled(true);
        }
    }
}

void HDispDate::disableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(false);
    if(unk != NULL)
        unk->setEnabled(false);
}

int HDispDate::valueUnkUpdate(void) //SLOT
{
    if(unk->isChecked())
    {
        ((HDate *)data)->value_is_unknown();
        edval->setEnabled( false );
        return 0;
    }

    ((HDate *)data)->value_is_known();
    edval->setEnabled( true );
    return 0;
}

int HDispDate::valueUpdate(const QDate &d) //SLOT
{
    data->setValue(QVariant(d));
    return 0;
}

int HDispDate::updateDisplaySlot(void) //SLOT
{
    if(edval != NULL)
    {
           edval->setDate(data->getValue().toDate());
           if(unk != NULL)
            {
                unk->setChecked( ((HDate *)data)->is_value_unknown() );
                edval->setEnabled( !((HDate *)data)->is_value_unknown() );
            }
           else
                edval->setEnabled(true);
    }

    if(nedtext != NULL)
    {
            if( ((HDate *)data)->is_unknown_alive() && ((HDate *)data)->is_value_unknown() )
                nedtext->setText(GSAFETEXT_TEXT_UNKNOWN);
            else
                nedtext->setText(data->getValue().toString());
    }
    return 0;
}

int HDispDate::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    return 0;
}

HDispDate::~HDispDate(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////

HDispTimestamp::HDispTimestamp(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    nedtext = NULL;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();

    nedtext   = new QLabel(this);
    nedtext->setText(data->dbValueToDispValue(data->getValue()).toString());
    sublayout->addWidget(nedtext);

}

void HDispTimestamp::enableControl(void)
{
}

void HDispTimestamp::disableControl(void)
{
}

int HDispTimestamp::updateDisplaySlot(void)
{
    nedtext->setText( data->getValue().toString() );
    return 0;
}

int HDispTimestamp::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    return 0;
}

HDispTimestamp::~HDispTimestamp(void)
{
}

// ///////////////////////////////////////////////////////////////////////////////

HDispSqlChoose::HDispSqlChoose(QWidget *parent,HDataField *d,bool _ro_mask)
: HDispDataField(parent,d,_ro_mask)
{
    edval = NULL;
    nedtext = NULL;
    rfbutt = NULL;
    pbutt = NULL;
    int curr;

    explain   = new QLabel(this);
    explain->setText(data->getExplainText());
    sublayout->addWidget(explain);

    if(!d->isNoMiddleStretch())
        sublayout->addStretch();
    if(!ro_mask && data->isEditable())
    {
        if(((HSqlChoose *)data)->getEasyForm())
        {
            edval     = new QComboBox(this);
            edval->clear();

            if(((HSqlChoose *)data)->getKeys().count() == 0)
            {
                QString mess;
                mess=QString("Warning: You selected an sqlchoose with easyform,\n"
                     "but you don't have any element in the targetted table!\n"
                     "You can't use initial null value in easyform sqlchoose!\n"
                     "(%1)")
                        .arg(data->sqlInsertHead());

                sdebug(mess);
                error(mess);
            }
            else
            {
                edval->addItems( ((HSqlChoose *)data)->getValues() );
                curr = getIndexOf( ((HSqlChoose *)data)->getKeys() , data->getValue().toString());
                //I'll set it. (The result is same, unless is bullshit the value)
                data->setValue(QVariant((((HSqlChoose *)data)->getKeys())[curr] ));
                itemSelected(curr);
                edval->setCurrentIndex(curr);
            }

            rfbutt = new QToolButton(this);
            rfbutt->setIcon(QPixmap(":/GSAFEPIXMAPS/refreshbutton.png"));
            sublayout->addWidget(edval);
            sublayout->addWidget(rfbutt);
            if(!connect(edval,SIGNAL(activated(int)),this,SLOT(itemSelected(int))))
                error("Error, connect(edval,SIGNAL(activated(int)),this,SLOT(itemSelected(int)))");
            if(!connect(rfbutt,SIGNAL(clicked()),this,SLOT(refresh())))
                error("Error, connect(rfbutt,SIGNAL(clicked()),this,SLOT(refresh()))");
        }
        else
        {
            pbutt = new QPushButton(this);

            if(data->getValue().toString() == "NULL" || ((HSqlChoose *)data)->getKeys().count() == 0)
            {
                curr = -1;
                pbutt->setText(" - ");
            }
            else
            {
                curr = getIndexOf( ((HSqlChoose *)data)->getKeys() , data->getValue().toString());
                pbutt->setText((((HSqlChoose *)data)->getValues())[curr]);
                //I'll set it. (The result is same, unless is bullshit the value)
                data->setValue(QVariant( (((HSqlChoose *)data)->getKeys())[curr] ));
            }

            sublayout->addWidget(pbutt);
            if(!connect(pbutt,SIGNAL(clicked()),this,SLOT(popupSelector())))
                error("Error, connect(pbutt,SIGNAL(clicked()),this,SLOT(popupSelector()))");

        }
    }
    else
    {
            nedtext   = new QLabel(this);
            if(data->getValue().toString() == "NULL")
            {
                curr = -1;
                nedtext->setText(" - ");
            }
            else
            {
                curr = getIndexOf( ((HSqlChoose *)data)->getKeys() , data->getValue().toString());
                nedtext->setText( (((HSqlChoose *)data)->getValues())[curr] );
            }
            sublayout->addWidget(nedtext);
    }
}

void HDispSqlChoose::enableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(true);
    if(pbutt != NULL)
        pbutt->setEnabled(true);
}

void HDispSqlChoose::disableControl(void)
{
    if(edval != NULL)
        edval->setEnabled(false);
    if(pbutt != NULL)
        pbutt->setEnabled(false);
}

int HDispSqlChoose::popupSelector(void)
{
    pupup_dialog=new SqlChooseDialog(this);
    /*The dialog box is connected to slot of this class(HDispSqlChoose)
     This slot is set the selected data, and call the close of the dialog.
     We don't do anything here, just cleans the data */
    pupup_dialog->exec();
    delete pupup_dialog;
    return 0;
}

int HDispSqlChoose::refresh(void) //SLOT
{
    ((HSqlChoose *)data)->refreshSqlValues();
    updateDisplaySlot();
    return 0;
}

int HDispSqlChoose::itemSelected(int index) //SLOT
{
    data->setValue(QVariant( (((HSqlChoose *)data)->getKeys())[index] ));
    return 0;
}

int HDispSqlChoose::updateDisplaySlot(void)
{
    sdebug("*** HDispSqlChoose::updateDisplaySlot ***");
    int curr;

    if(data->getValue().toString() == "NULL" || ((HSqlChoose *)data)->getKeys().count() == 0)
        curr = -1;
    else
        curr = getIndexOf( (((HSqlChoose *)data)->getKeys()) , data->getValue().toString());

    if(edval != NULL)
    {
        if(curr == -1)
            curr = 0;
        edval->clear();
        edval->addItems( ((HSqlChoose *)data)->getValues() );
        edval->setCurrentIndex(curr);
    }

    if(pbutt != NULL)
    {
        if(curr == -1)
            pbutt->setText(" - ");
        else
            pbutt->setText((((HSqlChoose *)data)->getValues())[curr]);
    }

    if(nedtext != NULL)
    {
        if(curr == -1)
            nedtext->setText(" - ");
        else
            nedtext->setText( (((HSqlChoose *)data)->getValues())[curr]);
    }
    sdebug("*** HDispSqlChoose::updateDisplaySlot ***END");
    return 0;
}

int HDispSqlChoose::updateStaticGuiElementsSlot(void)
{
    if(explain != NULL)
        explain->setText(data->getExplainText());
    return 0;
}

void HDispSqlChoose::setToolbarbuttonSignalOn(QString text)
{
    ((HSqlChoose *)data)->toolbarbutton_down_text = text;
}

void HDispSqlChoose::setToolbarbuttonSignalOff()
{
    ((HSqlChoose *)data)->toolbarbutton_down_text = "";
}

int HDispSqlChoose::itemSelected(const QString& s)
{
    if(s.isEmpty())
        return 0;
    data->setValue(QVariant( s ));
    pupup_dialog->close();
    updateDisplaySlot();
    return 0;
}

HDispSqlChoose::~HDispSqlChoose(void)
{
}

SqlChooseDialog::SqlChooseDialog( HDispSqlChoose* p )
: QDialog( p , Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint)
{
    int i;
    setModal(true);
    parent = p;
    SqlChooseDialogLayout = new QVBoxLayout(this);
        SqlChooseDialogLayout->setMargin(11);
        SqlChooseDialogLayout->setSpacing(6);

    disabled_recvkey = true;

    table = new HTableBrowser( this  );
    SqlChooseDialogLayout->addWidget( table );

    layout1 = new QHBoxLayout(0);
        layout1->setMargin(0);
        layout1->setSpacing(6);

    QSpacerItem* spacer = new QSpacerItem( 50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer );

    button = new QPushButton(this);
    button->setText( GSAFETEXT_DISPSQLCH_SELECTBUTTON );

    QPushButton *xbutton = new QPushButton(this);

    xbutton->setMinimumHeight(25);
    xbutton->setMinimumWidth (25);
    xbutton->setMaximumHeight(25);
    xbutton->setMaximumWidth (25);
    xbutton->setText("N");

    layout1->addWidget( xbutton );
    layout1->addWidget( button );

    if( parent->getHSqlChoose()->toolbrec != NULL  &&  !parent->getHSqlChoose()->btext.isEmpty() )
    {
        QPushButton *plusbutton = new QPushButton(this);
        plusbutton->setText(parent->getHSqlChoose()->btext);
        //own chain:
        if(!connect(plusbutton,SIGNAL(clicked()),this,SLOT(toolbarButtonClicked())))
            error("Cannot connect to the plus_toolbar button!");
        //chaining out...
        if(!connect(this,SIGNAL(callToolButtonHandler()),parent->getHSqlChoose()->toolbrec,SLOT(toolbuttonclicked()) ))
            error("Cannot connect to the plus_toolbar button!");

        if(!connect(parent->getHSqlChoose()->toolbrec,SIGNAL(setKeyTo(QString)),this,SLOT(receivedASelectedKey(QString))))
        {
            ; //do nothing, it's normal
        }

        layout1->addWidget( plusbutton );
    }

    //Extra functionality
    have_extra_func = false;
    for(i=0;i<5;++i)
        if(!parent->getHSqlChoose()->extrafunc_text[i].isEmpty())
        {
            have_extra_func = true;
            switch(i)
            {
                case 0:
                    connect(this,SIGNAL(sigToExtraFunc_0(QString)),
                        parent->getHSqlChoose()->extrafunc_receiver[i],SLOT(extrafunc_0(QString))); break;
                case 1:
                    connect(this,SIGNAL(sigToExtraFunc_1(QString)),
                        parent->getHSqlChoose()->extrafunc_receiver[i],SLOT(extrafunc_1(QString))); break;
                case 2:
                    connect(this,SIGNAL(sigToExtraFunc_2(QString)),
                        parent->getHSqlChoose()->extrafunc_receiver[i],SLOT(extrafunc_2(QString))); break;
                case 3:
                    connect(this,SIGNAL(sigToExtraFunc_3(QString)),
                        parent->getHSqlChoose()->extrafunc_receiver[i],SLOT(extrafunc_3(QString))); break;
                case 4:
                    connect(this,SIGNAL(sigToExtraFunc_4(QString)),
                        parent->getHSqlChoose()->extrafunc_receiver[i],SLOT(extrafunc_4(QString))); break;
            }
        }

    if(have_extra_func)
    {
        connect(table,SIGNAL(alternateActivateItem(const QString& )),
                    this,SLOT(popUpMenuActivated(const QString&)));
    }

    QSpacerItem* spacer_2 = new QSpacerItem( 50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer_2 );
    SqlChooseDialogLayout->addLayout( layout1 );

    setWindowTitle(parent->getHSqlChoose()->getPopupDialogTitle().isEmpty()
                    ? GSAFETEXT_DISPSQLCH_TITLE
                    : parent->getHSqlChoose()->getPopupDialogTitle()
                  );

    updateList();

    resize( QSize(300, 300).expandedTo(minimumSizeHint()) );

    //event handling of own gui elements
    connect(xbutton,SIGNAL(clicked()),this,SLOT(nullButtonPressed()));
    connect(button,SIGNAL(clicked()),this,SLOT(buttonPressed()));
    connect(table,SIGNAL(activateItem(const QString& )),
                this,SLOT(listviewitemSelected(const QString& )));
    //connecting, and data passig to parents (I connect the parent to my signal)
    connect(this,SIGNAL(itemSelected(const QString&))
                ,parent,SLOT(itemSelected(const QString&)));

    if(HRefreshAgent::getNotifyAgent() != NULL)
        connect(HRefreshAgent::getNotifyAgent(),SIGNAL(getnotify(QString)),
                this,SLOT(captureNotify(QString)));

    setSizeGripEnabled(true);
    setWindowState(Qt::WindowActive);
}

int SqlChooseDialog::captureNotify(QString tblname)
{
    sdebug("*** SqlChooseDialog::captureNotify ***");
    QStringList::iterator i;

    sdebug(QString(" %1 == %2").arg(tblname).arg(parent->getHSqlChoose()->getConnectedTableName()));
    if(parent->getHSqlChoose()->getConnectedTableName() == tblname)
    {
        sdebug("Call updateList()");
        updateList();
    }
    return 0;
}

int SqlChooseDialog::updateList(void)
{
    int i;
    HPleaseWaitWindow *pww = NULL;

    sdebug("*** SqlChooseDialog::updateList ***");
    table->clear();

    QString headertext="";
    HTableBrowserElement *record;

    #ifdef PROGRESS_POSSIBILITY
    if(parent->getHSqlChoose()->progress_on_read)
    {
        pww = HPleaseWaitWindow::start();
        if(!connect(parent->getHSqlChoose(),SIGNAL(doWorking()),pww,SLOT(progress())))
            sdebug("Cannot connect the SQLChoose-progress-signal to progress-dialog-slot!");
    }
    #endif

    parent->getHSqlChoose()->refreshSqlValues(false);
    headertext=parent->getHSqlChoose()->getFieldHeads();
    QStringList v = parent->getHSqlChoose()->getSeparatedValues();
    QStringList k = parent->getHSqlChoose()->getKeys();
    QStringList h;

    if(headertext.isEmpty())
    {   h = QString("A|B|C|D|E|F|G|H|I|J|K|L|M|N|O|P|R|S|T").split("|",QT_SKIP_EMPTY_PARTS);    }
    else
    {   h = headertext.split("|",QT_SKIP_EMPTY_PARTS);                                          }
    table->setHeadTexts(h);
    QStringList::Iterator itv = v.begin();
    QStringList::Iterator itk = k.begin();
    while ( itv != v.end() && itk != k.end() )
    {
        QStringList vals = itv->split("|",QT_KEEP_EMPTY_PARTS);
        table->addElement( record = new HTableBrowserElement(*itk,&vals) );
        ++itv;
        ++itk;
    }

    if((i = parent->getHSqlChoose()->getGuiSortIndicator()) != -1)
        table->sort(i);

    #ifdef PROGRESS_POSSIBILITY
    if(pww != NULL)
        HPleaseWaitWindow::end();
    #endif

    table->setCurrentElement(parent->getHSqlChoose()->getValue().toString());
    return 0;
}

int SqlChooseDialog::popUpMenuActivated(const QString& key)
{
    sdebug("*** SqlChooseDialog::popUpMenuActivated ***");
    int i;
    QMenu *popup = new QMenu(this);
    popup->setTitle(QString("Function on \"%1\"").arg(key));
    popup->addAction(" ");
    popup->addSeparator();

    for(i=0;i<5;++i)
        if(!parent->getHSqlChoose()->extrafunc_text[i].isEmpty())
        {
            switch(i)
            {
                case 0:
                    popup->addAction(parent->getHSqlChoose()->extrafunc_text[i],this,SLOT(popup_catch_0()));
                    break;
                case 1:
                    popup->addAction(parent->getHSqlChoose()->extrafunc_text[i],this,SLOT(popup_catch_1()));
                    break;
                case 2:
                    popup->addAction(parent->getHSqlChoose()->extrafunc_text[i],this,SLOT(popup_catch_2()));
                    break;
                case 3:
                    popup->addAction(parent->getHSqlChoose()->extrafunc_text[i],this,SLOT(popup_catch_3()));
                    break;
                case 4:
                    popup->addAction(parent->getHSqlChoose()->extrafunc_text[i],this,SLOT(popup_catch_4()));
                    break;
            }
        }
    popupped_key = key;
    popup->exec(table->mapToGlobal(QPoint(table->mouseRightClickPosX,table->mouseRightClickPosY)));
    delete popup;
    return 0;
}

int SqlChooseDialog::popup_catch_0(void) { emit sigToExtraFunc_0(popupped_key); return 0; }
int SqlChooseDialog::popup_catch_1(void) { emit sigToExtraFunc_1(popupped_key); return 0; }
int SqlChooseDialog::popup_catch_2(void) { emit sigToExtraFunc_2(popupped_key); return 0; }
int SqlChooseDialog::popup_catch_3(void) { emit sigToExtraFunc_3(popupped_key); return 0; }
int SqlChooseDialog::popup_catch_4(void) { emit sigToExtraFunc_4(popupped_key); return 0; }

int SqlChooseDialog::toolbarButtonClicked(void)
{
    QString ckey;

    parent->setToolbarbuttonSignalOn( parent->getHSqlChoose()->btext );
    key_received = false;
    ckey = table->currentKey();
    disabled_recvkey = false;
    emit callToolButtonHandler();

    disabled_recvkey = true;

    updateList();

    if(key_received)
        ckey = received_key;

    table->setCurrentElement(ckey);
    key_received = false;
    parent->setToolbarbuttonSignalOff();
    return 0;
}

int SqlChooseDialog::nullButtonPressed(void)
{
    QString k;
    k = "NULL";
    emit itemSelected(k);
    return 0;
}

int SqlChooseDialog::buttonPressed(void)
{
    QString k;
    k = table->currentKey();
    emit itemSelected(k);
    return 0;
}

int SqlChooseDialog::listviewitemSelected(const QString& s)
{
    sdebug(" *** SqlChooseDialog::listviewitemSelected *** ");
    emit itemSelected(s);
    sdebug(" *** SqlChooseDialog::listviewitemSelected *** END");
    return 0;
}

SqlChooseDialog::~SqlChooseDialog()
{
    delete table;
}

int SqlChooseDialog::receivedASelectedKey(QString key)
{
    if(disabled_recvkey)
        return 0;
    if(key.isEmpty())
        return 0;

    received_key = key;
    key_received = true;
    return 0;
}

// /////////////////////////////////////////////////////////////////////////////////////

HShowPrintHtml::HShowPrintHtml(QWidget *parent)
: QDialog(parent)
{
    QVBoxLayout *vl = new QVBoxLayout(this);
    QHBoxLayout *toolbuttons = new QHBoxLayout(0);
    QHBoxLayout *bottom = new QHBoxLayout(0);

    QToolButton *tp = new QToolButton(this);
    QToolButton *tst = new QToolButton(this);
    QToolButton *ts = new QToolButton(this);

    tp->setIcon(QPixmap(":/GSAFEPIXMAPS/image_p.png"));
    ts->setIcon(QPixmap(":/GSAFEPIXMAPS/image_y.png"));
    tst->setIcon(QPixmap(":/GSAFEPIXMAPS/image_e.png"));
    toolbuttons->addWidget(tp);
    toolbuttons->addSpacing(3);
    toolbuttons->addWidget(tst);
    toolbuttons->addSpacing(3);
    toolbuttons->addWidget(ts);
    toolbuttons->addStretch();

    QPushButton *okbutt = new QPushButton("Ok",this);
    bottom->addStretch();
    bottom->addWidget(okbutt);
    bottom->addStretch();

#ifdef WEBKITBASED_HTMLSHOW
    te = new QWebView(this);
#else
    te = new QTextEdit(this);
    te->setReadOnly(true);
#endif

    vl->addLayout(toolbuttons);
    vl->addWidget(te);
    vl->addLayout(bottom);

    connect(tp,SIGNAL(clicked()),this,SLOT(slotPrint()));
    connect(ts,SIGNAL(clicked()),this,SLOT(slotSave()));
    connect(tst,SIGNAL(clicked()),this,SLOT(slotStart()));

    connect(okbutt,SIGNAL(clicked()),this,SLOT(accept()));
    setSizeGripEnabled(true);
    html="";

    resize(640,480);
}

void HShowPrintHtml::setWinTitle(QString t)
{
    setWindowTitle(t);
}

void HShowPrintHtml::setContent(QString str)
{
    te->setHtml(html=str);
}

void HShowPrintHtml::slotSave(void)
{
    QString o;
    o = QFileDialog::getSaveFileName(this,"The output .html file","","*.html");

    if(o.isEmpty())
        return;
    QFile f(o);
    if (f.open(QIODevice::WriteOnly))
     {
      QTextStream *s = new QTextStream(&f);    // we will serialize the data into file f
      //s->setCodec(QTextCodec::codecForName("UTF-8"));
      *s << html;
      f.close();
      delete s;
     }
    else
    {
        error("Cannot create file!");
        return;
    }

}

void HShowPrintHtml::slotStart(void)
{
    QString o="_temptoext_edit.html";
    QFile f(o);
    if (f.open(QIODevice::WriteOnly))
     {
      QTextStream *s = new QTextStream(&f);
      *s << convNationalToHtmlCodes(html);
      f.close();
      delete s;
     }
    else
    {
        error("Cannot create temporally file!");
        return;
    }

    QString program = EXT_EDIT_PROCESS;
    QStringList arguments;
    arguments << o ;

    QProcess *myProcess = new QProcess(this);
    myProcess->start(program, arguments);

    QDialog *d = new QDialog(this);
    d->setModal(true);
    d->setWindowTitle(GSAFETEXT_CALLEXTERNAL);
    QLabel *l=new QLabel(d);
    l->setText(GSAFETEXT_STARTDIATEXT1 EXT_EDIT_PROCESS GSAFETEXT_STARTDIATEXT2);

    QHBoxLayout *lay = new QHBoxLayout(d);
    lay->addWidget(l);
    d->resize(200,100);
    connect(myProcess,SIGNAL(finished(int,QProcess::ExitStatus)),d,SLOT(close()));

    d->exec();

    delete d;
    delete myProcess;
 }

void HShowPrintHtml::slotPrint(void)
{
    QPrinter *printer = new QPrinter();
    printer->setPageSize(QPrinter::A4);
    QPrintDialog printDialog(printer, this);

    if (printDialog.exec() == QDialog::Accepted)
    {

#ifdef WEBKITBASED_HTMLSHOW
            te->print(printer);
#else
        int dpi = printer->logicalDpiY();
        QTextDocument *textdoc = te->document();

        QRectF pageRect(printer->pageRect());
        QRectF body = QRectF(0,0, pageRect.width(),pageRect.height());

        textdoc->setPageSize(body.size());

        QTextFrame *rootframe = textdoc->rootFrame();
        QTextFrameFormat tff = rootframe->frameFormat();

        //setting up 2 cm margin
        tff.setMargin((2/2.54)*dpi);
        tff.setTopMargin((2/2.54)*dpi);
        tff.setLeftMargin((2/2.54)*dpi);
        tff.setRightMargin((2/2.54)*dpi);
        tff.setBottomMargin((2/2.54)*dpi);

        rootframe->setFrameFormat(tff);

        textdoc->print(printer);
#endif
    }
}


HShowPrintHtml::~HShowPrintHtml(void)
{

}

///////////////////////////////////////////////////////////////////////////////////////

HPleaseWaitWindow *HPleaseWaitWindow::pww = NULL;

HPleaseWaitWindow::HPleaseWaitWindow(int sizex_,int sizey_,int refreshTime_) :
QWidget(0,Qt::SplashScreen | Qt::CustomizeWindowHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::BypassGraphicsProxyWidget)
{
    sizex = sizex_;
    sizey = sizey_;

    refreshTime = refreshTime_;
    setMinimumSize(sizex,sizey);
    setMaximumSize(sizex,sizey);


    //determine the window position
    if(QApplication::activeWindow() == NULL)
    {
        QRect r;
#ifdef COMPILED_WITH_QT4X
        QDesktopWidget *d = QApplication::desktop();
        r = d->screenGeometry();
#else
        r = QGuiApplication::primaryScreen()->availableGeometry();
#endif
        setGeometry((r.width()-sizex)/2,(r.height()-sizey)/2,sizex,sizey);
    }
    else
    {
        QWidget *w;

        w = QApplication::activeWindow();
        QPoint gpos=w->mapToGlobal(QPoint(0,0));
        setGeometry(gpos.x()+((w->width()-sizex)/2),gpos.y()+((w->height()-sizey)/2),sizex,sizey);
    }

    /*Get the firt value of the timer*/
    t=QTime::currentTime();
    seq=0;
    show();
}

HPleaseWaitWindow::~HPleaseWaitWindow(void)
{

}

void HPleaseWaitWindow::progress(void)
    {
     QTime tt;
     tt=QTime::currentTime();
     if(t.msecsTo(tt) < refreshTime) return;
     t = tt;

     if(!isActiveWindow())
       show();

     update();
     QApplication::processEvents();
    }

void HPleaseWaitWindow::mousePressEvent(QMouseEvent *me)
    {
     lastx = me->x();
     lasty = me->y();
    }

void HPleaseWaitWindow::mouseMoveEvent(QMouseEvent *me)
    {
     move(me->globalX()-lastx,me->globalY()-lasty);
    }

void HPleaseWaitWindow::paintEvent(QPaintEvent *e)
    {
     Q_UNUSED(e);

     int i;
     QPainter p(this);

     p.setRenderHint(QPainter::Antialiasing);
     p.setClipping(false);
     p.drawRect(1,1,78,78);

     p.setBrush(Qt::SolidPattern);

     for(i=0;i<=7;i++)
     {
        p.setBrush(QColor(7*30-i*30,7*30-i*30,7*30-i*30));
        p.setPen(QColor(7*30-i*30,7*30-i*30,7*30-i*30));
        p.drawPie(15,15,50,50,360*16-(seq*640+i*10*16),10*16);
     }

     if(++seq==9) seq=0;
    }

HPleaseWaitWindow * HPleaseWaitWindow::start(void)
    {
        if(pww == NULL)
            {
                pww = new HPleaseWaitWindow();
                pww->progress();
            }
        return pww;
    }

void HPleaseWaitWindow::step(void)
    {
         if(pww != NULL)
            pww->progress();
         return;
    }

void HPleaseWaitWindow::end(void)
    {
          if(pww != NULL)
            {
                pww->close();
                delete pww;
                pww=NULL;
            }
          return;
    }

////////////////////////////////////////////////////////////////////////////////////

HDecorDialog::HDecorDialog(QWidget *parent,QString title,int no_action_closetime)
: QDialog(parent,Qt::FramelessWindowHint)
{
    setModal(true);
    setMouseTracking(true);

    setAttribute(Qt::WA_TranslucentBackground);

    this->no_action_closetime = no_action_closetime;
    this->title = title;

    anywhere_click_close = false;
    dont_reset_timer = false;

    titlefont = QFont("times", 24);
    bgcolor = QColor(80,80,80); //QColor(100,100,255);
    framewidth = 10;
    frcolor = QColor(255,0,0);
    titleheight = 80;
    closerwidth = 80;
    crossthick  = 10;
    crossmargin = 10;
    titlecolor = QColor(0,0,0);
    indcolor = QColor(160,100,100);

    t_time = 100;
    if(no_action_closetime != 0)
    {
        end_t = ((int)(1000 / t_time)) * no_action_closetime;
        t_t = 0;
        QTimer::singleShot(t_time,this,SLOT(incr_t()));
    }
    setTitleFont(titlefont);
}

void HDecorDialog::setTitleFont(QFont f)
{
    int mw;

    titlefont = f;
    QFontMetrics fm(titlefont);
    mw = fm.QFONTMETRICS_STRING_HORIZONTAL_WIDTH(title);
    mw += 4*framewidth+closerwidth+20;
    setMinimumWidth(mw);
    update();
}

int HDecorDialog::incr_t(void)
{
    ++t_t;
    if(t_t >= end_t)
    {
        close();
    }
    else
    {
        update();
        QTimer::singleShot(t_time,this,SLOT(incr_t()));
    }
    return 0;
}

void HDecorDialog::paintEvent(QPaintEvent *pe)
{

    QPainter p(this);

    p.setRenderHint(QPainter::Antialiasing);
    /*
    p.fillRect(0,0,width(),height(),QBrush(bgcolor,Qt::SolidPattern));
    p.setPen(frcolor);
    p.drawRect(framewidth,framewidth,width()-2*framewidth-1,height()-2*framewidth-1);
    p.fillRect(framewidth,framewidth,width()-2*framewidth-1,titleheight,QBrush(frcolor,Qt::SolidPattern));
    p.setPen(titlecolor);
    p.drawText(framewidth,framewidth,width()-2*framewidth-closerwidth-1,titleheight,Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine ,title);
    p.fillRect(width()-framewidth-closerwidth,framewidth+2,closerwidth-2,titleheight-4,QBrush(titlecolor,Qt::SolidPattern));
    */

    /*
    QPainterPath path;
    QPen pen(Qt::black, 1);
    p.setPen(pen);
    path.addRoundedRect(QRectF(0,0,width(),height()), 20, 20);
    p.fillPath(path,QColor(r,g,b));
    p.drawPath(path);*/

    QPainterPath outer;
    QPen pen1(Qt::gray,1);
    outer.addRoundedRect(0,0,width(),height(),15,15);
    p.setPen(pen1);
    p.fillPath(outer,bgcolor);
    //p.fillRect(0,0,width(),height(),QBrush(bgcolor,Qt::SolidPattern));
    //p.drawPath(outer);



    //p.drawRect(framewidth,framewidth,width()-2*framewidth-1,height()-2*framewidth-1);
    //p.fillRect(framewidth,framewidth,width()-2*framewidth-1,titleheight,QBrush(frcolor,Qt::SolidPattern));

    QPainterPath decorFrame;
    QPen pen2(frcolor,3);
    p.setPen(pen2);
    decorFrame.addRoundedRect(framewidth,framewidth,width()-2*framewidth-1,height()-2*framewidth-1,15,15);
    //p.fillPath(decor,frcolor);
    p.drawPath(decorFrame);

    QPainterPath decorTitle;
    decorTitle.setFillRule(Qt::WindingFill);
    decorTitle.addRoundedRect(framewidth,framewidth,width()-2*framewidth-1,titleheight,15,15);
    decorTitle.addRect(framewidth,framewidth+15,width()-2*framewidth-1,titleheight-15);
    p.fillPath(decorTitle.simplified(),frcolor);

    p.setPen(titlecolor);
    p.drawText(framewidth,framewidth,width()-2*framewidth-closerwidth-1,titleheight,Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine ,title);


    QPainterPath closerBox;
    closerBox.setFillRule(Qt::WindingFill);
    closerBox.addRoundedRect(width()-framewidth-closerwidth,framewidth+2,closerwidth-2,titleheight-4,15,15);
    closerBox.addRect(width()-framewidth-closerwidth+15,framewidth+2+15,closerwidth-2-15,titleheight-4-15);
    p.fillPath(closerBox.simplified(),QColor(20,20,20));

    //p.fillRect(width()-framewidth-closerwidth,framewidth+2,closerwidth-2,titleheight-4,QBrush(titlecolor,Qt::SolidPattern));


    if(t_t >= (int)((1000 / t_time)))
    {
        float max,curr;
        max  = end_t-(1000 / t_time);
        curr = t_t  -(1000 / t_time);
        p.setPen(indcolor);
        p.setBrush(QBrush(indcolor,Qt::SolidPattern));                                                       //5760 => 2*3.1416f
        p.drawPie(width()-framewidth-closerwidth+(3),framewidth+2+(3),closerwidth-2-(6),titleheight-4-(6),0,5760*(curr/max));
    }

    p.setPen(QPen(QBrush(frcolor,Qt::SolidPattern),crossthick,Qt::SolidLine,Qt::RoundCap));
    p.drawLine(width()-framewidth-closerwidth+crossmargin,
               framewidth+2+crossmargin,
               width()-framewidth-closerwidth+closerwidth-2-crossmargin,
               framewidth+2+titleheight-4-crossmargin);
    p.drawLine(width()-framewidth-closerwidth+crossmargin,
               framewidth+2+titleheight-4-crossmargin ,
               width()-framewidth-closerwidth+closerwidth-2-crossmargin,
               framewidth+2+crossmargin);

    QDialog::paintEvent(pe);
}

void HDecorDialog::mousePressEvent(QMouseEvent *e)
{
    if(anywhere_click_close)
    {
        close();
        return;
    }

    if(e->x() > width()-framewidth-closerwidth               &&
       e->x() < width()-framewidth-closerwidth+closerwidth-2 &&
       e->y() > framewidth+2 &&
       e->y() < framewidth+2+titleheight-4 )
    {
        close();
    }
    if(!dont_reset_timer)
        t_t = 0;
}

void HDecorDialog::mouseMoveEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
    if(!dont_reset_timer)
        t_t = 0;
}

void HDecorDialog::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e);
    if(!dont_reset_timer)
        t_t = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

HPressButton::HPressButton(QWidget *parent,QString text,QString code)
: QFrame(parent)
{
    size_sync = NULL;
    textstr = text;
    idstr = code;
    font = QApplication::font();
    font.setBold(true);
    font.setPointSize(8);
    downFont = font;
    downFont.setPointSize(7);
    cmargin = 5;
    c3deffect = 12;
    downsmaller = 5;

    r=200; rAlt=220;
    g=200; gAlt=220;
    b=200; bAlt=254;

    downColor = QColor(220,220,254);
    borderColor = QColor(40,40,40);
    setFocusPolicy(Qt::StrongFocus);

    mid = false;
    down = false;

    hidestat = false;
    enablestat = true;
    calcSize();
}

HPressButton::~HPressButton(void)
{

}

int HPressButton::setHide()
{
    hidestat = true;
    update();
    return 0;
}
int HPressButton::setShow()
{
    hidestat = false;
    update();
    return 0;
}

int HPressButton::setEnabled(bool enable)
{
    enablestat = enable;
    update();
    return 0;

}

void HPressButton::setEffect(int effectwidth)
{
    c3deffect = effectwidth;
    calcSize();
}

void HPressButton::setDownSmaller(int sdm_pixel)
{
    downsmaller = sdm_pixel;
}

int HPressButton::getDownSmaller(void)
{
    return downsmaller;
}

void HPressButton::setMargin(int margin)
{
    cmargin = margin;
    calcSize();
}

void HPressButton::setTextPointSize(int size)
{
    font.setPointSize(size);
    downFont.setPointSize(size-1);
    calcSize();
}

int HPressButton::recalcSize()
{
    calcSize();
    return 0;
}

void HPressButton::calcSize()
{
    int x,y;
    QFontMetrics fm(font);
    QRect boundingr;

    boundingr = fm.boundingRect(textstr);

    x = boundingr.width()  + 2*(c3deffect + cmargin);
    y = boundingr.height() + 2*(c3deffect + cmargin);
    if(size_sync != NULL)
    {
        if(size_sync->mx < x ) size_sync->mx = x;
        if(size_sync->my < y ) size_sync->my = y;
        x = size_sync->mx;
        y = size_sync->my;
    }
    setMinimumSize(x,y);
    resize(x,y);
}

void HPressButton::setColor(int red,int green,int blue,int mode)
{
    if(mode == 1)
    {
        rAlt = red;
        gAlt = green;
        bAlt = blue;
    }
    else
    {
        r = red;
        g = green;
        b = blue;
    }
}

void HPressButton::paintEvent(QPaintEvent *e)
{
    int sx,sy,ex,ey;
    QPainter p(this);

    if(hidestat)
    {
        QWidget::paintEvent(e);
        return;
    }

    sx = c3deffect;
    sy = c3deffect;
    ex = width()-c3deffect;
    ey = height()-c3deffect;

    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    QPen pen(borderColor, 1);
    p.setPen(pen);

    if(down)
    {
        path.addRoundedRect(QRectF(downsmaller,downsmaller,
                                   width()-(downsmaller*2),height()-(downsmaller*2)), 15, 15);

        p.fillPath(path,downColor);
        p.setFont(downFont);
    }
    else
    {
        path.addRoundedRect(QRectF(0,0,width(),height()), 15, 15);
        p.fillPath(path,QColor(r,g,b));
        p.setFont(font);
    }

    p.drawPath(path);

/*  old style (now unused)
    if(!down)
        for(i=0;i<c3deffect;++i)
        {
            p.setPen(QColor(r+rd*i,g+gd*i,b+bd*i)); p.drawLine(sx-i,sy-i,ex+i,sy-i);
            p.setPen(QColor(r-rd*i,g-gd*i,b-bd*i)); p.drawLine(ex+i,sy-i,ex+i,ey+i);
            p.setPen(QColor(r-rd*i,g-gd*i,b-bd*i)); p.drawLine(ex+i,ey+i,sx-i,ey+i);
            p.setPen(QColor(r+rd*i,g+gd*i,b+bd*i)); p.drawLine(sx-i,sy-i,sx-i,ey+i);
        }
    else
        for(i=0;i<c3deffect;++i)
        {
            p.setPen(QColor(r-rd*i,g-gd*i,b-bd*i)); p.drawLine(sx-i,sy-i,ex+i,sy-i);
            p.setPen(QColor(r+rd*i,g+gd*i,b+bd*i)); p.drawLine(ex+i,sy-i,ex+i,ey+i);
            p.setPen(QColor(r+rd*i,g+gd*i,b+bd*i)); p.drawLine(ex+i,ey+i,sx-i,ey+i);
            p.setPen(QColor(r-rd*i,g-gd*i,b-bd*i)); p.drawLine(sx-i,sy-i,sx-i,ey+i);
        }
*/

    if(enablestat)
        p.setPen(QColor(0,0,0));
    else
        p.setPen(QColor(100,100,100));

    p.drawText(sx,sy,ex-sx,ey-sy,
                    Qt::AlignHCenter | Qt::AlignVCenter , textstr);

    if(!enablestat)
    {
        p.setPen(QPen(QBrush(QColor(255,0,0),Qt::SolidPattern),6,Qt::SolidLine,Qt::RoundCap));
        p.drawLine(c3deffect+cmargin+15,
                   c3deffect/2+cmargin,
                   width()-(c3deffect+cmargin+15),
                   height()-(c3deffect/2+cmargin));
        p.drawLine(c3deffect+cmargin+15,
                   height()-(c3deffect/2+cmargin),
                   width()-(c3deffect+cmargin+15),
                   c3deffect/2+cmargin);
    }

    QWidget::paintEvent(e);
}

void HPressButton::keyPressEvent(QKeyEvent *e)
{
    if(hidestat || !enablestat)
    {
        QFrame::keyPressEvent(e);
        return;
    }

    down = true;
    update();
    QFrame::keyPressEvent(e);
}

void HPressButton::keyReleaseEvent(QKeyEvent *e)
{
    if(hidestat || !enablestat)
    {
        QFrame::keyReleaseEvent(e);
        return;
    }

    if(down)
    {
        emit clicked();
        emit clickedCode(idstr);
    }
    down = false;
    update();
    QFrame::keyReleaseEvent(e);
}

void HPressButton::mousePressEvent(QMouseEvent *e)
{
    if(hidestat || !enablestat)
    {
        QFrame::mousePressEvent(e);
        return;
    }

    down = true;
    update();
    QFrame::mousePressEvent(e);
}

void HPressButton::mouseReleaseEvent(QMouseEvent *e)
{
    if(hidestat || !enablestat)
    {
        QFrame::mouseReleaseEvent(e);
        return;
    }

    bool willemit=false;

    if(down)
        willemit = true;
    down = false;
    update();
    QFrame::mouseReleaseEvent(e);

    if(willemit)
    {
        emit clicked();
        emit clickedCode(idstr);
    }
}

void HPressButton::mouseMoveEvent(QMouseEvent *e)
{
    if(hidestat || !enablestat)
    {
        QFrame::mouseMoveEvent(e);
        return;
    }

    if(down && ( e->x() < 0 || e->x() > width()))
    {
        down = false;
        update();
        return;
    }

    if(down && ( e->y() < 0 || e->y() > height()))
    {
        down = false;
        update();
        return;
    }
    QFrame::mouseMoveEvent(e);
}

HPressButtonSizeSyncronizer* HPressButton::createSizeSyncronizer(void)
{
    return new HPressButtonSizeSyncronizer();
}

// ///////////////////////////////////////////////////////////////////////////////
// / Classes of HDynTable (and related) //////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDynTableDispConnector::HDynTableDispConnector(HDynTable *dynt,QTableWidget *qtw)
{
    ccprogress = false;
    rfprogress = false;
    int c,r,cc,rr;
    QList<QString> hhead;
    QList<QString> vhead;

    this->dynt = dynt;

    dynt->firstElement();
    while(!dynt->isEnded())
    {
        if(!hhead.contains( dynt->currentElementColumnName() ))
            hhead.push_back( dynt->currentElementColumnName() );
        if(!vhead.contains( dynt->currentElementRowName() ))
            vhead.push_back( dynt->currentElementRowName() );

        dynt->nextElement();
    }

    c = hhead.count();
    r = vhead.count();
    qtw->setColumnCount(c);
    qtw->setRowCount(r);

    qtw->setHorizontalHeaderLabels(hhead);
    qtw->setVerticalHeaderLabels(vhead);

    QTableWidgetItem *item;
    bool f;
    QString val;
    for(cc = 0 ; cc < c ; ++cc)
        for(rr = 0; rr < r ; ++rr)
        {
            item = new QTableWidgetItem();

            f = false;
            dynt->firstElement();
            while(!dynt->isEnded())
            {
                if(dynt->currentElementRowName() == vhead[rr]    &&
                   dynt->currentElementColumnName() == hhead[cc]    )
                {
                    cellData cd;

                    cd.name = dynt->currentElementName();
                    cd.c = cc;
                    cd.r = rr;
                    cd.glink = item;
                    cells.push_back(cd);

                    f = true;
                    val = dynt->currentElementValueString();
                }
                dynt->nextElement();
            }

            if(f)
            {
                item->setText(val);
                item->setBackground(Qt::white);
            }
            else
            {
                //item->setFlags(!Qt::ItemIsEnabled);
                item->setFlags(item->flags() & ~(Qt::ItemIsEditable));
                item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

                item->setBackground(Qt::darkGray);
            }
            qtw->setItem(rr,cc,item);
        }
    //change  gui->data
    connect(qtw,SIGNAL(cellChanged(int,int)),this,SLOT(slotCellChanged(int,int)));
    //change data->gui
    connect(dynt,SIGNAL(dataChangedNongui()),this,SLOT(refreshGui()));
}

HDynTableDispConnector::~HDynTableDispConnector(void)
{
    dynt = NULL;
    cells.clear();
}

int HDynTableDispConnector::slotCellChanged(int row,int column)
{
    if(dynt == NULL)
        return 0;
    if(rfprogress)
        return 0;
    bool realchange;
    realchange = false;
    ccprogress = true;
    QTableWidgetItem *item = NULL;
    QList<cellData>::iterator i = cells.begin();
    while(i != cells.end())
    {
        if(i->r == row && i->c == column)
        {
            QColor c = Qt::white;

            item = i->glink;
            realchange = true;
            dynt->setElementValue(i->name,item->text());
            if(item->text() != dynt->getElementValueString(i->name))
            {
                realchange = false;
                c.setRgb(255,100,100);
            }
            item->setBackground(QBrush(c));
        }
        ++i;
    }
    ccprogress = false;
    if(realchange)
        emit dataChangedByGui();
    return 0;
}

int HDynTableDispConnector::refreshGui(void)
{
    if(ccprogress)
        return 0;
    rfprogress = true;
    QTableWidgetItem *item = NULL;
    QList<cellData>::iterator i = cells.begin();
    while(i != cells.end())
    {
        item = i->glink;
        if(item != NULL)
        {
            item->setText( dynt->getElementValueString(i->name) );
            item->setBackground(QBrush(Qt::white));
        }

        ++i;
    }
    rfprogress = false;
    return 0;
}

//end code
