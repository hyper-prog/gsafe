/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2021 Peter Deak  (hyper80@gmail.com)

    License: LGPLv2.1

    dialib.cpp
*/

#include <QtCore>
#include <QtGui>

#include "dialib.h"
#include "dconsole.h"

HDialogData::HDialogData(void)
{
    table1 = NULL;
    table2 = NULL;
    list1  = NULL;
    list2  = NULL;
    dmatrix1 = NULL;
    dmatrix2 = NULL;
    toolbutton1 = NULL;
    toolbutton2 = NULL;
    topLabel = NULL;
    t1C    = false;
    t1AC   = false;
    t2C    = false;
    t2AC   = false;
    EscC   = false;
    inscroll = false;
    deletedata = false;
    declose = false;

    tb1clicked = false;
    tb2clicked = false;

}

int HDialogData::makeGui(QWidget *basew,QString caption,HBase *dobj1,HBase *dobj2,
                    QString func,QString ttext,QString tbutt1,QString tbutt2,QList<QPixmap *> *toolbuttons,int xsize,int ysize)
{
    sdebug("*** HDialog::makeGui ***");

    if(func.contains("EscC"))
        EscC = true;

    if(func.contains("TB1cl")) //undocumented option == TB1Close (compatibility with older programs)
        t1C = true;
    if(func.contains("TB1Close"))
        t1C = true;
    if(func.contains("TB2cl")) //undocumented option == TB2Close (compatibility with older programs)
        t2C = true;
    if(func.contains("TB2Close"))
        t2C = true;

    if(func.contains("TB1AcceptClose"))
        t1AC = true;
    if(func.contains("TB2AcceptClose"))
        t2AC = true;

    if(func.contains("InScroll"))
        inscroll = true;
    if(func.contains("DeleteData"))
        deletedata = true;
    if(func.contains("DestructiveClose"))
        declose = true;

    basew->setWindowTitle(caption.isEmpty() ? "Dialog..." : caption );

    //Creating widget elements
    if(dobj1 != NULL)
    {
        if(dobj1->getWhoami() == "HTable")
        {
            if(inscroll)
            {
                sv1 = new QScrollArea(basew);

                table1 = new HDispTable(sv1,(HTable *)dobj1,KEEP_ALL,deletedata?DELETE_DATA:DONT_DELETE_DATA);
                sv1->setWidget(table1);
                sv1->setWidgetResizable(true);
            }
            else
                table1 = new HDispTable(basew,(HTable *)dobj1,KEEP_ALL,deletedata?DELETE_DATA:DONT_DELETE_DATA);

            if(func.contains("StrToE"))
                table1->addStretchToEnd();
        }
        if(dobj1->getWhoami() == "HList")
        {
            list1  = new HDispList(basew,(HList *)dobj1,KEEP_ALL,deletedata ? DELETE_DATA:DONT_DELETE_DATA);
            basew->connect(list1,SIGNAL(listItemChanged()),basew,SLOT(lic()));
        }
        if(dobj1->getWhoami() == "HPlainDataMatrix")
        {
            dmatrix1  = new HDispPlainDataMatrix(basew,(HPlainDataMatrix *)dobj1,false,-1,deletedata ? DELETE_DATA:DONT_DELETE_DATA);
        }

    }
    if(dobj2 != NULL)
    {
        if(dobj2->getWhoami() == "HTable")
        {
            if(inscroll)
            {
                sv2 = new QScrollArea(basew);
                table2 = new HDispTable(sv2,(HTable *)dobj2,KEEP_ALL,deletedata?DELETE_DATA:DONT_DELETE_DATA);
                sv2->setWidget(table2);
                sv2->setWidgetResizable(true);
            }
            else
                table2 = new HDispTable(basew,(HTable *)dobj2,KEEP_ALL,deletedata?DELETE_DATA:DONT_DELETE_DATA);

            if(func.contains("StrToE"))
                table2->addStretchToEnd();
        }
        if(dobj2->getWhoami() == "HList")
        {
            list2  = new HDispList(basew,(HList *)dobj2,KEEP_ALL,deletedata?DELETE_DATA:DONT_DELETE_DATA);
            basew->connect(list2,SIGNAL(listItemChanged()),basew,SLOT(lic()));
        }
        if(dobj2->getWhoami() == "HPlainDataMatrix")
        {
            dmatrix2  = new HDispPlainDataMatrix(basew,(HPlainDataMatrix *)dobj2,false,-1,deletedata ? DELETE_DATA:DONT_DELETE_DATA);
        }

    }

    closebutton=NULL;

    if(!tbutt1.isEmpty())
        toolbutton1 = new QPushButton(tbutt1,basew);

    if(!tbutt2.isEmpty())
        toolbutton2 = new QPushButton(tbutt2,basew);

    //Layouts
    QVBoxLayout *mainLayout = new QVBoxLayout(basew);

    if(!ttext.isEmpty())
    {
        topLabel = new QLabel(ttext,basew);
        QBoxLayout *titleLayout = new QHBoxLayout(0);
            titleLayout->setMargin(5);
            titleLayout->setSpacing(5);
            titleLayout->addStretch();
            titleLayout->addWidget(topLabel);
            titleLayout->addStretch();
        mainLayout->addLayout(titleLayout);
    }

    if(toolbuttons)
    {
        int i;

        QHBoxLayout *pixtoollay = new QHBoxLayout(0);
            pixtoollay->setMargin(2);
            pixtoollay->setSpacing(2);
        for(i=0;i<8;i++)
            pixTool[i] = NULL;
        if(func.contains("ToolCenter"))
            pixtoollay->addStretch();
        for(i=0;i<(int)toolbuttons->count();i++)
        {
            sdebug(QString("Pixmap toolbutton %1 initializing...").arg(i));
            pixTool[i] = new QToolButton(basew);
            pixTool[i]->setIcon( *(toolbuttons->at(i)));
            pixTool[i]->resize(25,25);
            pixtoollay->addWidget(pixTool[i]);
        }

        pixtoollay->addStretch();
        mainLayout->addLayout(pixtoollay);
    }

    if(table1 != NULL || table2 != NULL || list1 != NULL || list2 != NULL || dmatrix1 != NULL || dmatrix2 != NULL)
    {
        QBoxLayout  *dviewLayout;
            if(func.contains("DHoriz"))
                dviewLayout = new QHBoxLayout(0);
            else
                dviewLayout = new QVBoxLayout(0);

            dviewLayout->setMargin(5);
            dviewLayout->setSpacing(5);
            if(table1 != NULL)
                dviewLayout->addWidget(inscroll ? (QWidget *)sv1 : (QWidget *)table1);
            if(list1  != NULL)
                dviewLayout->addWidget(list1 );
            if(dmatrix1 != NULL)
                dviewLayout->addWidget(dmatrix1);

            if(table2 != NULL)
                dviewLayout->addWidget(inscroll ? (QWidget *)sv2 : (QWidget *)table2);
            if(list2  != NULL)
                dviewLayout->addWidget(list2 );
            if(dmatrix2 != NULL)
                dviewLayout->addWidget(dmatrix2);

        mainLayout->addLayout(dviewLayout);
    }

    if(toolbutton1 != NULL || toolbutton2 != NULL)
    {
        QBoxLayout  *toolLayout;
            if(func.contains("THoriz"))
                toolLayout = new QHBoxLayout(0);
            else
                toolLayout = new QVBoxLayout(0);
            toolLayout->setMargin(5);
            toolLayout->setSpacing(5);
            if(func.contains("THoriz"))
                toolLayout->addStretch();
            if(toolbutton1 != NULL)
                toolLayout->addWidget(toolbutton1);
            if(toolbutton2 != NULL)
                toolLayout->addWidget(toolbutton2);
            if(func.contains("THoriz"))
                toolLayout->addStretch();
        mainLayout->addLayout(toolLayout);
    }

    if(func.contains("Ok"))
    {
            closebutton = new QPushButton("Ok",basew);

            QBoxLayout  *closeLayout = new QHBoxLayout(0);
                closeLayout->setMargin(5);
                closeLayout->setSpacing(5);
                closeLayout->addStretch();
                closeLayout->addWidget(closebutton);
                closeLayout->addStretch();
            mainLayout->addLayout(closeLayout);
    }

    //connects

    if(closebutton != NULL)
    {
        if(func.contains("OkIsAccept"))
        {
            if(!basew->connect(closebutton,SIGNAL(clicked()),basew,SLOT(accept())))
                error("In HDialogData::makeGui cannot connect: closebutton,SIGNAL(clicked()) to basew,SLOT(accept()) !");
        }
        else
        {
            if(!basew->connect(closebutton,SIGNAL(clicked()),basew,SLOT(close())))
                error("In HDialogData::makeGui cannot connect: closebutton,SIGNAL(clicked()) to basew,SLOT(close()) !");
        }
    }
    if(toolbutton1 != NULL)
        basew->connect(toolbutton1,SIGNAL(clicked()),basew,SLOT(tbutton1Cl()));
    if(toolbutton2 != NULL)
        basew->connect(toolbutton2,SIGNAL(clicked()),basew,SLOT(tbutton2Cl()));

    if(xsize != 0 || ysize != 0)
        basew->resize(xsize,ysize);

    sdebug("*** HDialog::makeGui *** END");

    return 0;
}

int HDialog::run(QWidget *parent,QString caption,HBase *dobj1,HBase *dobj2,QString func,
                  QString ttext,QString tbutt1,QString tbutt2,QList<QPixmap *> *toolbuttons,int xsize,int ysize)
{
    int result;
    HDialog *d;

    if(func.contains("AutoExec"))
    {
        error("Do not pass \"AutoExec\" option to HDialog::run !");
        return -1;
    }

    d = new HDialog(parent,caption,dobj1,dobj2,func,ttext,tbutt1,tbutt2,toolbuttons,xsize,ysize);
    result = d->exec();
    delete d;
    return result;
}

// ///////////////////////////////////////////////////////////////////////////////////////////////

HDialog::HDialog(QWidget *parent,QString caption,HBase *dobj1,HBase *dobj2,QString func,
                 QString ttext,QString tbutt1,QString tbutt2,QList<QPixmap *> *toolbuttons,int xsize,int ysize)
  : QDialog(parent,Qt::WindowMaximizeButtonHint|Qt::WindowCloseButtonHint),
    HDialogData()
{
    sdebug("*** HDialog::HDialog ***");

    if(!func.contains("NonModal"))
        setModal(true);

    parent_dialog = parent;

    makeGui(this,caption,dobj1,dobj2,func,ttext,tbutt1,tbutt2,toolbuttons,xsize,ysize);

    if(declose)
        setAttribute(Qt::WA_DeleteOnClose);

    setSizeGripEnabled(true);

    //Execute dialog if necessary
    if(func.contains("AutoExec"))
        exec();
    sdebug("*** HDialog::HDialog *** END");
}

HDialog::~HDialog(void)
{
    sdebug("*** HDialog::HDialog ***");
    sdebug("*** HDialog::HDialog *** END");
}

int HDialog::lic(void)
{
    emit listItemChanged();
    return 0;
}


int HDialog::tbutton1Cl(void)
{
    emit tbutton1Clicked();

    if(t1C)
    {
        tb1clicked = true;
        if(t1AC)
            accept();
        else
            close();
    }

    return 0;
}

int HDialog::tbutton2Cl(void)
{
    emit tbutton2Clicked();

    if(t2C)
    {
        tb2clicked = true;
        if(t2AC)
            accept();
        else
            close();
    }

    return 0;
}

void HDialog::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Escape)
    {
        if(EscC)
        {
            close();
            return;
        }
        else return;
    }

    QDialog::keyPressEvent(e);
}


// ///////////////////////////////////////////////////////////////////////////
// /class HGetTextBox                                                       //
// ///////////////////////////////////////////////////////////////////////////

HGetTextBox::HGetTextBox(QWidget *parent,QString cap,QString text,QList<QString> exptexts,bool largetexts)
: QDialog(parent)
{
    int i,size;

    large=largetexts;
    setWindowTitle(cap);
    QVBoxLayout *mainlay = new QVBoxLayout(this);
        mainlay->setMargin(5);
        mainlay->setSpacing(5);
     QHBoxLayout *toplay = new QHBoxLayout(0);
        toplay->setMargin(5);
        toplay->setSpacing(5);
     QLabel *label = new QLabel(text,this);
     toplay->addStretch();
     toplay->addWidget(label);
     toplay->addStretch();
    mainlay->addLayout(toplay);


    QVBoxLayout *leftlay = new QVBoxLayout(0);
        leftlay->setMargin(5);
        leftlay->setSpacing(5);

    QVBoxLayout *rightlay = new QVBoxLayout(0);
        rightlay->setMargin(5);
        rightlay->setSpacing(5);

    size=(int)exptexts.size();
    for(i=0;i<size;++i)
    {
        QHBoxLayout *midlay = new QHBoxLayout(0);
            midlay->setMargin(5);
            midlay->setSpacing(5);
        QLabel *sublabel = new QLabel(exptexts[i],this);

        if(largetexts)
        {
            QTextEdit *mline = new QTextEdit("",this);
            leftlay->addWidget(sublabel);
            rightlay->addWidget(mline);

            mledits.push_back(mline);
        }
        else
        {
            QLineEdit *line = new QLineEdit("",this);
            leftlay->addWidget(sublabel);
            rightlay->addWidget(line);

            ledits.push_back(line);
        }

    }

     QHBoxLayout *midlay = new QHBoxLayout(0);
        midlay->setMargin(5);
        midlay->setSpacing(5);

     midlay->addLayout(leftlay);
     midlay->addStretch();
     midlay->addLayout(rightlay);
     mainlay->addLayout(midlay);

     QHBoxLayout *sublay = new QHBoxLayout(0);
        sublay->setMargin(5);
        sublay->setSpacing(5);

     QPushButton *button = new QPushButton("Ok",this);
     sublay->addStretch();
     sublay->addWidget(button);
     sublay->addStretch();
    mainlay->addLayout(sublay);

    connect(button,SIGNAL(clicked()),this,SLOT(clickOk()));
}

int HGetTextBox::clickOk(void)
{
    int i,size;

    if(large)
    {
        size=(int)mledits.size();
        results.clear();
        for(i=0;i<size;++i)
        {
            results.push_back(mledits[i]->toPlainText());
        }

    }
    else
    {
        size=(int)ledits.size();
        results.clear();
        for(i=0;i<size;++i)
        {
            results.push_back(ledits[i]->text());
        }
    }
    accept();
    return 0;
}

//end code.
