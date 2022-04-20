/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    quicktable.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "data.h"
#include "dm.h"
#include "dmext.h"
#include "ftypes.h"
#include "gui.h"
#include "dconsole.h"
#include "builder.h"
#include "guiext.h"
#include "quicktable.h"

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

/////////////////////////////////////////////////////////////////////////////////
/// HQuickTable  //////////////////////////////////////////////////////////////
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

bool     HQuickTable::staticinit  = false;
QPixmap *HQuickTable::pix_up      = NULL;
QPixmap *HQuickTable::pix_down    = NULL;
QPixmap *HQuickTable::pix_asc     = NULL;
QPixmap *HQuickTable::pix_desc    = NULL;
QPixmap *HQuickTable::pix_asc2    = NULL;
QPixmap *HQuickTable::pix_desc2   = NULL;

HQuickTable::HQuickTable(QWidget *parent)
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

 void HQuickTable::setBackgroundColor(QColor c)
 {
     color_background = c;
     QPalette palette;
     palette.setColor(QPalette::Window,color_background);
     setPalette(palette);
     setAutoFillBackground(true);
 }

int HQuickTable::setHeadTexts(QStringList h)
{
    head = h;
    return 0;
};

HQuickTable::~HQuickTable(void)
{
    sdebug("*** HQuickTable::~HQuickTable ***\nDestroy HQuickTable, delete items...");
    clear();
    delete fm;
    sdebug("*** HQuickTable::~HQuickTable ***END");
}

QString HQuickTable::currentKey(void)
{
    if(cur == NULL)
        return "";
    return cur->key;
}

QString HQuickTable::topKey(void)
{
    if(top == NULL)
        return "";
    return top->key;
}

int HQuickTable::clear()
{
    HQuickTableElement *tmp;

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

void HQuickTable::setColumnColor(int col,QColor c)
{
    if(col >= (int)colc.size())
    {
        precolor[col] = c;
        return;
    }
    precolor[col] = c;
    colc[col] = c;
}

int HQuickTable::addElement(HQuickTableElement *e)
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
            return 0;
        }

        cur = top = start = end = e;

        for(i=0;i<e->size();++i)
        {
            colw.push_back( fm->horizontalAdvance((*e)[i]) );
            colc.push_back( precolor.contains(i) ? precolor[i] : QColor( (100+i*25)>255 ? 255 : (100+i*25) ,200-i*5,180 ));
        }
        rowh = fm->height();

        while(head.size() < colw.size())
            head.push_back("");

        for(i=0;i<(int)head.size();++i)
        {
            l = fm->horizontalAdvance(head[i]) + BARW; //because the sort indicator
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
            l = fm->horizontalAdvance((*e)[i]) + BARW; //because the sort indicator
            if(l > colw[i])
                colw[i] = l;
        }
    }
    ++number;
    return 0;
}

int HQuickTable::removeAllElementByKey(QString key)
{
    HQuickTableElement *tmp = NULL;
    int deleteditems;

    if(start == NULL)
        return 0; // no element

    fulshDrawingCache();

    deleteditems = 0;
    //      HQuickTableElement *start,*end,*top,*cur,*query_cursor;
    HQuickTableElement *seek = start;
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

void HQuickTable::setSpecSort(int col,QString specsortname)
{
    spec_sort[col] = specsortname;
}

int HQuickTable::setCurrentElement(QString c,QString t)
{
    HQuickTableElement *topm=start,*step=start;
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

int HQuickTable::setCurrentElement(int index)
{
    HQuickTableElement *step=start;
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

int HQuickTable::setTopElement(int index)
{
    HQuickTableElement *step=start;
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

HQuickTableElement * HQuickTable::topElement(void)
{
    if(top == NULL)
        return NULL;
    return top;
}

HQuickTableElement * HQuickTable::currentElement(void)
{
    if(cur == NULL)
        return NULL;
    return cur;
}

bool HQuickTable::stepUp(void)
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

bool HQuickTable::stepDown(void)
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

void HQuickTable::wheelEvent(QWheelEvent *e)
{
    int d;

    d = e->angleDelta().y();

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

int HQuickTable::cursorToBegin(void)
{
    seek = "";
    top = cur = start;
    update();
    return 0;
}

int HQuickTable::cursorToEnd(void)
{
    seek = "";
    top = cur = end;
    update();
    return 0;
}

void HQuickTable::keyPressEvent(QKeyEvent *e)
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
        seek.remove(QRegularExpression(".$"));
        sdebug("seek: "+seek);
        return;
    }

    if(e->key() == Qt::Key_F3)
    {
        HQuickTableElement *t;
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
    HQuickTableElement *t;

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
int withinBorders(int min,int max,int val)
{
    if(min > val)
        return min;
    if(max < val)
        return max;
    return val;
}

bool HQuickTable::inX(int a,int b,QMouseEvent *e)
{
    return  (a < e->position().x() && b > e->position().x()) ||
            (b < e->position().x() && a > e->position().x());
}
bool HQuickTable::inY(int a,int b,QMouseEvent *e)
{
    return  (a < e->position().y() && b > e->position().y()) ||
            (b < e->position().y() && a > e->position().y());
}

HQuickTable::PointLocatorResult HQuickTable::pointLocator(QMouseEvent *e)
{
    int x,y;
    int stepx;
    HQuickTableElement *stepy;

    if(cur == NULL)
       return PointLocatorResult(false,false);

    //out from everything
    if(!inX(0,width(),e) || !inY(0,height(),e))
    {
        sdebug("==CLICK=> Outer click (do nothing)");
        return PointLocatorResult(false,false);
    }

    //left click from the first cell, is scroll the column to back
    if(inX(0,LMARGIN+CELLHMARGIN,e))
    {
        sdebug("==CLICK=> Left from the first cell (backscroll the columns)");
        if(col_scroll == 0)
            return PointLocatorResult(false,false);
        curc = --col_scroll;
        return PointLocatorResult(true,false);
    }

    for(stepx=col_scroll,x=LMARGIN;
            (x<(width()-(LMARGIN+BARW)));
            x+=(colw[stepx]+2*CELLHMARGIN+plusw),stepx++)
    {
        if(stepx >= colw.size())
            break;
        if(inX(x+CELLHMARGIN,x+colw[stepx]+plusw+CELLHMARGIN*2,e) && inY(TMARGIN+CELLVMARGIN,TMARGIN+rowh+CELLHMARGIN,e))
        {
            sdebug("==CLICK=> Column header (sorting)");

            sdebug(QString("Click header: column %1").arg(stepx));
            sort(stepx);
            return PointLocatorResult(true,false);
        }
    }

    for(stepy=top,y=TMARGIN+rowh+4*CELLVMARGIN;
        (y < height()-TMARGIN) && (stepy != NULL);
        y+=(rowh+2*CELLVMARGIN),stepy=stepy->next)

        if(inY(y,y+rowh+2*CELLHMARGIN,e))
        {
            for(stepx=col_scroll,x=LMARGIN;
                (x<(width()-(LMARGIN+BARW)));
                x+=(colw[stepx]+2*CELLHMARGIN+plusw),stepx++)
            {
                if(stepx >= colw.size())
                    break;
                if(inX(x,x+colw[stepx]+plusw+CELLHMARGIN*2,e))
                {
                    sdebug("==CLICK=> To Cell (Marking)");

                    if(cur != stepy || curc != stepx)
                    {
                        cur  = stepy;
                        curc = stepx;

                        return PointLocatorResult(true,true);
                    }
                    return PointLocatorResult(false,true);
                }
            }
        }
    sdebug("==Click=> Nothing targetted, don't hit anything (exiting)");
    return PointLocatorResult(false,false);
}

void HQuickTable::mouseMoveEvent(QMouseEvent *e)
{
    int nTH;
    seek = "";

    //scrolling
    if( inscroll || inX(width()-BARW-LMARGIN+2,width(),e))
    {
        if( !inscroll && inY(8,8+BUTTH,e))
        {
            if(stepUp())
                update();
            return;
        }
        if( !inscroll && inY(height()-(BUTTH+8),height()-8,e))
        {
            if(stepDown())
                update();
            return;
        }

        nTH =  ( (number-showrow+2) * (withinBorders(0,height()-(8+BUTTH),e->position().y()-(8+BUTTH))) )  /  (height()-8*2-2*BUTTH);
        setTopElement(nTH);
        inscroll = true;
        return;
    }

    if(!inscroll && pointLocator(e).need_update)
        update();
}

void HQuickTable::mouseReleaseEvent(QMouseEvent *e)
{
    inscroll = false;

    if(e->button() == Qt::RightButton)
    {
        sdebug("HQuickTable::mouseReleaseEvent RIGHT button!");
         if(pointLocator(e).click_on_cell)
             if(last_right_clicked == cur->key)
             {
                 sdebug("Emitting signal!");
                 mouseRightClickPosX = e->position().x();
                 mouseRightClickPosY = e->position().y();
                 emit alternateActivateItem(cur->key);
             }
        return;
    }
}

void HQuickTable::mousePressEvent(QMouseEvent *e)
{
    seek = "";
    mouseMoveEvent(e);
    if(e->button() == Qt::RightButton)
        if(pointLocator(e).click_on_cell)
            last_right_clicked = cur->key;
}

void HQuickTable::mouseDoubleClickEvent(QMouseEvent *e)
{
    seek = "";

    if(inX(width()-BARW-LMARGIN+2,width(),e))
    {
        mouseMoveEvent(e);
        return;
    }

    if(pointLocator(e).click_on_cell)
        emit activateItem(cur->key);
}

int HQuickTable::ch_dropchar(QString s)
{
    while(!s.at(0).isDigit())
        s = s.mid(1);
    sdebug(s);
    return s.toInt();
}

int HQuickTable::ch_number(QString s)
{
    return s.toInt();
}

double HQuickTable::ch_float(QString s)
{
    return s.toDouble();
}

bool HQuickTable::ch(bool m,HQuickTableElement *a,HQuickTableElement *b)
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
        if(ch_dropchar(a->getCVal(keycolumn).toLocal8Bit()) > ch_dropchar(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? true  : false);
        if(ch_dropchar(a->getCVal(keycolumn).toLocal8Bit()) < ch_dropchar(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? false : true );

        if(keycolumn2 == -1)
            return true;

        if(ch_dropchar(a->getCVal(keycolumn2).toLocal8Bit()) > ch_dropchar(b->getCVal(keycolumn2).toLocal8Bit()))
            return (asc2 ? true  : false);
        if(ch_dropchar(a->getCVal(keycolumn2).toLocal8Bit()) < ch_dropchar(b->getCVal(keycolumn2).toLocal8Bit()))
            return (asc2 ? false : true );
        return true;
   }

   if(sortmode == 2) //Number sort mode
   {
        if(ch_number(a->getCVal(keycolumn).toLocal8Bit()) > ch_number(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? true  : false);
        if(ch_number(a->getCVal(keycolumn).toLocal8Bit()) < ch_number(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? false : true );

        if(keycolumn2 == -1)
            return true;

        if(ch_number(a->getCVal(keycolumn2).toLocal8Bit()) > ch_number(b->getCVal(keycolumn2).toLocal8Bit()))
            return (asc2 ? true  : false);
        if(ch_number(a->getCVal(keycolumn2).toLocal8Bit()) < ch_number(b->getCVal(keycolumn2).toLocal8Bit()))
            return (asc2 ? false : true );
        return true;
   }
   if(sortmode == 3) //Float sort mode
   {
        if(ch_float(a->getCVal(keycolumn).toLocal8Bit()) > ch_float(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? true  : false);
        if(ch_float(a->getCVal(keycolumn).toLocal8Bit()) < ch_float(b->getCVal(keycolumn).toLocal8Bit()))
            return (asc  ? false : true );

        if(keycolumn2 == -1)
            return true;

        if(ch_float(a->getCVal(keycolumn2).toLocal8Bit()) > ch_float(b->getCVal(keycolumn2).toLocal8Bit()))
            return (asc2 ? true  : false);
        if(ch_float(a->getCVal(keycolumn2).toLocal8Bit()) < ch_float(b->getCVal(keycolumn2).toLocal8Bit()))
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

void HQuickTable::q_sort(void)
{
    HQuickTableElementPair  *t;

    if(start == NULL)
        return;
    q_sort_stack = new QStack<HQuickTableElementPair *> ();
    q_sort_stack->clear();
    q_sort_stack->push(new HQuickTableElementPair(start,end)); //Verem inicializalas a starthoz

    while( ! q_sort_stack->isEmpty() )
    {
        q_sort_run( (t=q_sort_stack->pop()) );
        delete t;
    }
    fulshDrawingCache();
}

//Quick sort algoritmus
void HQuickTable::q_sort_run(HQuickTableElementPair *p)
{
    int i;
    HQuickTableElement *m;

    HQuickTableElement *left=p->left;
    HQuickTableElement *right=p->right;

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
            q_sort_stack->push(new HQuickTableElementPair(p->left,m->prev));
    if(p->right->index > m->index)
            q_sort_stack->push(new HQuickTableElementPair(m->next,p->right));
}

void HQuickTable::sort(int keycol)
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
    if(spec_sort.contains(keycol))
    {
        if(spec_sort[keycol] == "string")
            sortmode = 0;
        if(spec_sort[keycol] == "dropchar")
            sortmode = 1;
        if(spec_sort[keycol] == "number")
            sortmode = 2;
        if(spec_sort[keycol] == "float")
            sortmode = 3;
    }

    sdebug(QString("start QSort.. (key:%1 %2)").arg(keycolumn).arg(asc ? "asc" : "desc"));
    q_sort();
    sdebug("..end");
}

int HQuickTable::fulshDrawingCache(void)
{
    //sdebug("Flush drawing cache...");
    cacheMutex.lock();
    std::list<HQuickTableElement *>::iterator i = cachedElements.begin();
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

int HQuickTable::limitCache(void)
{
    HQuickTableElement *toremove=NULL;
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

void HQuickTable::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    int c;
    int allcw=0;
    int dcY=0;

    QPixmap *pix=NULL;
    QColor color,cellcolor;
    HQuickTableElement *run=top;
    HQuickTableElement *dcI=NULL;


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
                                    fontMetrics().horizontalAdvance(seek),rowh+CELLVMARGIN,
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
        sy = withinBorders(15,height()-(2*TMARGIN+BUTTH),
                    (int)(showrow * (height()-(2*(BUTTH+8))) / number) );

        ky = withinBorders(TMARGIN+BUTTH,height()-(2*TMARGIN+BUTTH),
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

void HQuickTable::resizeEvent(QResizeEvent *e)
{
    //fulshDrawingCache();
    QFrame::resizeEvent(e);
}

int HQuickTable::numberOfElements(void)
{
    return number;
}

HQuickTableElement * HQuickTable::elementByIndex(int n)
{
    HQuickTableElement *c;

    for(c = start; c != NULL ;c=c->next)
        if(c->index == n)
            return c;
    return c;
}

QStringList *HQuickTable::valuesByIndex(int n)
{
    HQuickTableElement *c;

    c = elementByIndex(n);

    if(c == NULL)
        return NULL;

    return new QStringList(*c->rows);
}

QString *HQuickTable::keyByIndex(int n)
{
    HQuickTableElement *c;

    c = elementByIndex(n);

    if(c == NULL)
        return NULL;

    return new QString(c->key);
}

void HQuickTable::queryToStart(void)
{   query_cursor = start;   }

void HQuickTable::queryToTop(void)
{   query_cursor = top;     }

void HQuickTable::queryToCurrent(void)
{   query_cursor = cur;     }

HQuickTableElement *HQuickTable::queryNextElement(void)
{
    HQuickTableElement *r;

    r = query_cursor;
    if(query_cursor != NULL)
        query_cursor = query_cursor->next;
    return r;
}

QStringList *HQuickTable::queryNextValues(void)
{
    HQuickTableElement *r;

    r = query_cursor;

    if(query_cursor != NULL)
    {
        query_cursor = query_cursor->next;
        return new QStringList(*r->rows);
    }
    return NULL;
}

QString *HQuickTable::queryNextKey(void)
{
    HQuickTableElement *r;

    r = query_cursor;
    if(query_cursor != NULL)
    {
        query_cursor = query_cursor->next;
        return new QString(r->key);
    }
    return NULL;
}

bool HQuickTable::hasKey(QString key)
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

HQuickTableElement::HQuickTableElement(QString k,QStringList *c)
{
    key = k;
    rows = new QStringList(*c);
    owncolor = false;
    ocr=0; ocg=0; ocb=0;

    next = prev = NULL;
    cachePixmap = NULL;
}

HQuickTableElement::HQuickTableElement(QString k,QString v1,QString v2,QString v3,QString v4,QString v5,
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

HQuickTableElement::~HQuickTableElement(void)
{
    if(cachePixmap != NULL)
        delete cachePixmap;
    cachePixmap = NULL;

    delete rows;
}

HQuickTableElement * HQuickTableElement::setColor(int r,int g,int b)
{
    owncolor = true;
    ocr=r;
    ocg=g;
    ocb=b;
    return this;
}

HQuickTableElement * HQuickTableElement::clearColor(void)
{
    owncolor = false;
    ocr=0;
    ocg=0;
    ocb=0;
    return this;
}


QString HQuickTableElement::operator[](int i)
{
    if(rows == NULL)
        return "";
    if(rows->size() <= i)
        return "";
    return (*rows)[i];
}

int HQuickTableElement::size(void)
{
    if(rows == NULL)
        return 0;
    return rows->size();
}

HQuickTableElement& HQuickTableElement::operator=(HQuickTableElement& e)
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

HQuickTableElement& HQuickTableElement::operator=(HQuickTableElement* e)
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

void HQuickTableElement::swapWith(HQuickTableElement *x)
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

//End of gSAFE quicktable.cpp
