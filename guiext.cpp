/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    guiext.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include "data.h"
#include "dm.h"
#include "ra.h"
#include "dmext.h"
#include "ftypes.h"
#include "gui.h"
#include "dconsole.h"
#include "builder.h"
#include "guiext.h"
#include <quicktable.h>

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

QColor html6HexColor(QString hc)
{
    bool ok;
    int r,g,b;
    r = hc.mid(0,2).toInt(&ok,16);
    if(!ok)
        r = 0;
    g = hc.mid(2,2).toInt(&ok,16);
    if(!ok)
        g = 0;
    b = hc.mid(4,2).toInt(&ok,16);
    if(!ok)
        b = 0;
    return QColor(r,g,b);
}
// /////////////////////////////////////////////////////////////////////////////////////
// HSqlXChoose related gui calss
// /////////////////////////////////////////////////////////////////////////////////////

HSqlXChooseDisplayPopup::HSqlXChooseDisplayPopup( HSqlXChooseDisplay* p)
: QDialog( p , Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint)
{
    int i;
    setModal(true);
    dparent = p;
    SqlChooseDialogLayout = new QVBoxLayout(this);
        SqlChooseDialogLayout->setContentsMargins(11,11,11,11);
        SqlChooseDialogLayout->setSpacing(6);

    disabled_recvkey = true;

    table = new HQuickTable( this  );
    SqlChooseDialogLayout->addWidget( table );

    layout1 = new QHBoxLayout(0);
        layout1->setContentsMargins(0,0,0,0);
        layout1->setSpacing(6);

    QSpacerItem* spacer = new QSpacerItem( 50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer );

    selectButton = new QPushButton(this);
    selectButton->setText(QObject::tr("Select","HSqlXChooseDisplayPopup"));


    QPushButton *nullButton = NULL;
    if(dparent->uniField_unknownAllowed())
    {
        nullButton = new QPushButton(this);
        nullButton->setMinimumHeight(25);
        nullButton->setMinimumWidth (25);
        nullButton->setMaximumHeight(25);
        nullButton->setMaximumWidth (25);
        nullButton->setText("N");

        layout1->addWidget( nullButton );
    }

    layout1->addWidget( selectButton );

    //plus toolbar button
    if(!dparent->myHSqlXChooseField()->attribute("popuplist_plusbutton").isEmpty())
    {
        QPushButton *plusbutton = new QPushButton(this);
        plusbutton->setText(dparent->myHSqlXChooseField()->attribute("popuplist_plusbutton"));
        connect(plusbutton,SIGNAL(clicked()),this,SLOT(toolbarButtonClicked()));
        layout1->addWidget(plusbutton);
    }

    //right click function on items of list
    bool have_extra_func = false;
    for(i=0;i<5;++i)
    {
        QString n = QString("popuplist_func_on_item_%1").arg(i+1);

        QString text_on = dparent->myHSqlXChooseField()->attribute(n);
        if(!text_on.isEmpty())
        {
            have_extra_func = true;
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


    setWindowTitle(dparent->myHSqlXChooseField()->attribute("popuplist_title").isEmpty()
                    ? QObject::tr("Select value","HSqlXChooseDisplayPopup")
                    : dparent->myHSqlXChooseField()->attribute("popuplist_title")
                  );

    updateList();

    resize( QSize(300, 300).expandedTo(minimumSizeHint()) );

    //event handling of own gui elements
    if(nullButton != NULL)
        connect(nullButton,SIGNAL(clicked()),this,SLOT(nullButtonPressed()));

    connect(selectButton,SIGNAL(clicked()),this,SLOT(selButtonPressed()));
    connect(table,SIGNAL(activateItem(const QString& )),
                this,SLOT(listviewitemSelected(const QString& )));
    //connecting, and data passig to parents (I connect the parent to my signal)
    connect(this,SIGNAL(itemSelected(const QString&))
                ,dparent,SLOT(valueUpdatedOnGuiStr(const QString&)));

    if(HRefreshAgent::getNotifyAgent() != NULL)
        connect(HRefreshAgent::getNotifyAgent(),SIGNAL(getnotify(QString)),
                this,SLOT(captureNotify(QString)));

    setSizeGripEnabled(true);
    setWindowState(Qt::WindowActive);
}

int HSqlXChooseDisplayPopup::guiElementsNeedUpdateSlot()
{
    updateList();
    return 0;
}

int HSqlXChooseDisplayPopup::captureNotify(QString tblname)
{
    sdebug("*** HSqlXChooseDisplayPopup::captureNotify ***");
    sdebug(QString(" %1 == %2").arg(tblname).arg(dparent->myHSqlXChooseField()->connectedTableName()));
    if(dparent->myHSqlXChooseField()->connectedTableName() == tblname)
    {
        sdebug("Call updateList()");
        updateList();
    }
    return 0;
}

int HSqlXChooseDisplayPopup::updateList(void)
{
    sdebug("*** SqlXChooseDisplayPopup::updateList ***");
    table->clear();

    QString headertext="";

    headertext = dparent->myHSqlXChooseField()->attribute("popuplist_heads");
    QStringList v = dparent->uniField_selectableValues();
    QStringList k = dparent->uniField_selectableKeysAsStr();

    table->setHeadTexts(headertext.split(";",Qt::KeepEmptyParts));

    QStringList::Iterator itv = v.begin();
    QStringList::Iterator itk = k.begin();
    while ( itv != v.end() && itk != k.end() )
    {
        QStringList vals = itv->split(";",Qt::KeepEmptyParts);
        table->addElement( new HQuickTableElement(*itk,&vals) );
        ++itv;
        ++itk;
    }

    if(!dparent->myHSqlXChooseField()->attribute("popuplist_sortbyidx").isEmpty())
    {
        int s_idx = dparent->myHSqlXChooseField()->attribute("popuplist_sortbyidx").toInt();
        if(s_idx > 0)
            table->sort(s_idx);
    }

    table->setCurrentElement(dparent->myHSqlXChooseField()->strValue());
    table->update();
    return 0;
}

int HSqlXChooseDisplayPopup::valueUpdatedInMemory()
{
    table->setCurrentElement(dparent->myHSqlXChooseField()->strValue());
    return 0;
}

int HSqlXChooseDisplayPopup::popUpMenuActivated(const QString& key)
{
    sdebug("*** HSqlXChooseDisplayPopup::popUpMenuActivated ***");
    int i;
    QMenu *popup = new QMenu(this);

    for(i=0;i<5;++i)
    {
        QString n = QString("popuplist_func_on_item_%1").arg(i+1);
        QString text_on = dparent->myHSqlXChooseField()->attribute(n);
        if(!text_on.isEmpty())
        {
            switch(i)
            {
                case 0: popup->addAction(text_on,this,SLOT(popup_catch_0())); break;
                case 1: popup->addAction(text_on,this,SLOT(popup_catch_1())); break;
                case 2: popup->addAction(text_on,this,SLOT(popup_catch_2())); break;
                case 3: popup->addAction(text_on,this,SLOT(popup_catch_3())); break;
                case 4: popup->addAction(text_on,this,SLOT(popup_catch_4())); break;
            }
        }
    }

    popupped_key = key;
    popup->exec(table->mapToGlobal(QPoint(table->mouseRightClickPosX,table->mouseRightClickPosY)));
    delete popup;
    return 0;
}

int HSqlXChooseDisplayPopup::popup_catch_0(void) { emit customUserEvent("popuponlist_1",popupped_key); return 0; }
int HSqlXChooseDisplayPopup::popup_catch_1(void) { emit customUserEvent("popuponlist_2",popupped_key); return 0; }
int HSqlXChooseDisplayPopup::popup_catch_2(void) { emit customUserEvent("popuponlist_3",popupped_key); return 0; }
int HSqlXChooseDisplayPopup::popup_catch_3(void) { emit customUserEvent("popuponlist_4",popupped_key); return 0; }
int HSqlXChooseDisplayPopup::popup_catch_4(void) { emit customUserEvent("popuponlist_5",popupped_key); return 0; }

int HSqlXChooseDisplayPopup::toolbarButtonClicked(void)
{
    emit customUserEvent("plusbutton",table->currentKey());
    return 0;
}

int HSqlXChooseDisplayPopup::nullButtonPressed(void)
{
    QString k;
    k = "__NULL__";
    emit itemSelected(k);
    return 0;
}

int HSqlXChooseDisplayPopup::selButtonPressed(void)
{
    QString k;
    k = table->currentKey();
    emit itemSelected(k);
    return 0;
}

int HSqlXChooseDisplayPopup::listviewitemSelected(const QString& s)
{
    sdebug(" *** HSqlXChooseDisplayPopup::listviewitemSelected *** ");
    emit itemSelected(s);
    sdebug(" *** HSqlXChooseDisplayPopup::listviewitemSelected *** END");
    return 0;
}

HSqlXChooseDisplayPopup::~HSqlXChooseDisplayPopup()
{
    delete table;
}

// ///////////////////////////////////////////////////////////////////////////////
// / DataMatrix class //////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDataMatrixDisplay::HDataMatrixDisplay(QWidget *parent,HDataMatrix *d,HDispObjectFlags flags)
: QFrame(parent)
{
    data = d;
    startflags = flags;
    dataobject_deleted = false;

    if(flagOn(startflags,HDispFlag_CleanDataOnStart) && data != NULL)
        data->clear();

    genhtml_parameters = "html center";
    QVBoxLayout  *layout= new QVBoxLayout(this);
                  layout->setContentsMargins(2,2,2,2);
                  layout->setSpacing(2);

    qtable = new HQuickTable(this);

    QHBoxLayout *toplay = new QHBoxLayout(0);

    toplay->addStretch();

    layout->addLayout(toplay);

    layout->addWidget(qtable);

    updateFromDataMatrixDataCells();

    connect(qtable,SIGNAL(activateItem(const QString&)),this,SLOT(slotActivateItem(const QString&)));
    connect(qtable,SIGNAL(alternateActivateItem(const QString&)),this,SLOT(itemAlternateActivated(const QString&)));
    connect(qtable,SIGNAL(itemChanged(const QString&)),this,SLOT(listItemChanged(const QString&)));
    connect(data,SIGNAL(dataChanged()),this,SLOT(updateFromDataMatrix()));
}

int HDataMatrixDisplay::listItemChanged(const QString key)
{
    data->soft_current_key = key;
    return 0;
}

int HDataMatrixDisplay::updateFromDataMatrix(void)
{
    updateFromDataMatrix_Common();
    return 0;
}

void HDataMatrixDisplay::updateFromDataMatrix_Common(void)
{
    updateFromDataMatrixDataCells();
}

void HDataMatrixDisplay::updateFromDataMatrixDataCells(void)
{
    if(data == NULL)
        return;

    int i = 0;
    QStringList *sl = new QStringList();
    QString k;
    qtable->clear();
    if(data->rowCount() > 0)
    {
        data->firstRow();
        do
        {
            ++i;
            *sl = data->currentRowStr();
            if(data->keyfield < 0)
                k = QString("%1").arg(i);
            else
                k = (*sl)[data->keyfield];
            qtable->addElement(new HQuickTableElement(k,sl));
        }
        while(data->nextRow());
    }
    qtable->setHeadTexts(data->getHeader());
    delete sl;
    qtable->update();
}

int HDataMatrixDisplay::slotActivateItem(const QString k)
{
    QString key;

    key = k;
    emit activateItem(key);
    data->actLine(k);
    return 0;
}

int HDataMatrixDisplay::itemAlternateActivated(const QString& s)
{
    data->alternateActLine(s);
    return 0;
}

void HDataMatrixDisplay::setColumnColor(int col,QColor color)
{
    qtable->setColumnColor(col,color);
}

HDataMatrixDisplay::~HDataMatrixDisplay(void)
{
    if(dataobject_deleted)
        return;

    if(flagOn(startflags,HDispFlag_CleanDataOnDestroy))
        data->clear();

    if(flagOn(startflags,HDispFlag_DeleteDataObjectOnDestroy))
    {
        dataobject_deleted = true;
        delete data;
    }
}
// ///////////////////////////////////////////////////////////////////////////////
// HRecordLinesDisplay
// ///////////////////////////////////////////////////////////////////////////////

HRecordLinesDisplay::HRecordLinesDisplay(QWidget *parent,HRecordLines *rl,HDispObjectFlags flags)
    : HDataMatrixDisplay(parent,rl->dataMatrix(),flags)
{
    rldata = rl;

    updateQTableFeaturesFromRecordLines();

    connect(qtable,SIGNAL(activateItem(const QString&)),rldata,SLOT(actionOnRecordSlot(const QString&)));
    connect(qtable,SIGNAL(itemChanged(const QString&)),this,SLOT(listItemChanged(const QString&)));
    if(rldata->allOnItemActions().count() > 0)
        connect(qtable,SIGNAL(alternateActivateItem(const QString&)),this,SLOT(popUpMenuActivated(const QString&)));
}

int HRecordLinesDisplay::popUpMenuActivated(const QString& key)
{
    sdebug("*** HRecordLinesDisplay::popUpMenuActivated ***");

    QMenu *popup = new QMenu(this);

    alternate_action_event_keys = rldata->allOnItemActions().keys();
    alternate_action_event_keys.sort();
    int i,c = alternate_action_event_keys.count();
    for(i = 0 ; i < c && i < 5 ; ++i)
    {
        if(rldata->allOnItemActions().contains(alternate_action_event_keys[i]))
        {
            QString text_on = rldata->allOnItemActions().value(alternate_action_event_keys[i]);
            if(!text_on.isEmpty())
            {
                if(alternate_action_event_keys[i].endsWith("_sepbefore"))
                    popup->addSeparator();
                switch(i)
                {
                    case 0: popup->addAction(text_on,this,SLOT(altact_catch_0())); break;
                    case 1: popup->addAction(text_on,this,SLOT(altact_catch_1())); break;
                    case 2: popup->addAction(text_on,this,SLOT(altact_catch_2())); break;
                    case 3: popup->addAction(text_on,this,SLOT(altact_catch_3())); break;
                    case 4: popup->addAction(text_on,this,SLOT(altact_catch_4())); break;
                }
                if(alternate_action_event_keys[i].endsWith("_sepafter"))
                    popup->addSeparator();
            }
        }
    }

    alternate_action_key = key;
    popup->exec(qtable->mapToGlobal(QPoint(qtable->mouseRightClickPosX,qtable->mouseRightClickPosY)));
    delete popup;
    return 0;
}

int HRecordLinesDisplay::altact_catch_0(void) { rldata->alternateActionOnRecordSlot(alternate_action_key,alternate_action_event_keys[0]); return 0; }
int HRecordLinesDisplay::altact_catch_1(void) { rldata->alternateActionOnRecordSlot(alternate_action_key,alternate_action_event_keys[1]); return 0; }
int HRecordLinesDisplay::altact_catch_2(void) { rldata->alternateActionOnRecordSlot(alternate_action_key,alternate_action_event_keys[2]); return 0; }
int HRecordLinesDisplay::altact_catch_3(void) { rldata->alternateActionOnRecordSlot(alternate_action_key,alternate_action_event_keys[3]); return 0; }
int HRecordLinesDisplay::altact_catch_4(void) { rldata->alternateActionOnRecordSlot(alternate_action_key,alternate_action_event_keys[4]); return 0; }

int HRecordLinesDisplay::listItemChanged(const QString key)
{
    rldata->soft_current_key = key;
    return 0;
}

void HRecordLinesDisplay::updateFromDataMatrix_Common(void)
{
    updateFromDataMatrixDataCells();
    updateQTableFeaturesFromRecordLines();
}

void HRecordLinesDisplay::updateQTableFeaturesFromRecordLines(void)
{
    int c,fi,fc = rldata->readedFields().count();
    for(fi = 0,c = 0 ; fi < fc ; ++fi,++c )
    {
        HField *f = rldata->fieldByName(rldata->readedFields().at(fi));
        if(!f->attribute("color").isEmpty())
        {
            QString chex = f->attribute("color");
            bool ok;
            int r,g,b;

            r = chex.mid(0,2).toInt(&ok,16);
            if(!ok)
                r = 0;
            g = chex.mid(2,2).toInt(&ok,16);
            if(!ok)
                g = 0;
            b = chex.mid(4,2).toInt(&ok,16);
            if(!ok)
                b = 0;
            qtable->setColumnColor(c,QColor(r,g,b));
        }
        if(!f->attribute("sortmode").isEmpty())
        {
            qtable->setSpecSort(c,f->attribute("sortmode"));
        }
        else
        {
            if(f->className() == "HNKeyField")
                qtable->setSpecSort(c,"number");
            if(f->className() == "HSKeyField")
                qtable->setSpecSort(c,"dropchar");
            if(f->className() == "HNumberField")
                qtable->setSpecSort(c,"number");
            if(f->className() == "HFloatingField")
                qtable->setSpecSort(c,"float");
        }
    }
}

HRecordLinesDisplay::~HRecordLinesDisplay()
{
    if(dataobject_deleted)
        return;

    if(flagOn(startflags,HDispFlag_CleanDataOnDestroy))
        rldata->dataMatrix()->clear();

    if(flagOn(startflags,HDispFlag_DeleteDataObjectOnDestroy))
    {
        dataobject_deleted = true;
        delete rldata;
    }
}

// ///////////////////////////////////////////////////////////////////////////////
// HDynTableDisplayConnector
// ///////////////////////////////////////////////////////////////////////////////

HDynTableDisplayConnector::HDynTableDisplayConnector(HDynTable *dynt,QTableWidget *qtw)
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

HDynTableDisplayConnector::~HDynTableDisplayConnector(void)
{
    dynt = NULL;
    cells.clear();
}

int HDynTableDisplayConnector::slotCellChanged(int row,int column)
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

int HDynTableDisplayConnector::refreshGui(void)
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

//End of gSAFE guiext.cpp
