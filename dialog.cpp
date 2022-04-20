/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    dialog.cpp
*/

#include <QtCore>
#include <QtGui>

#include "dialog.h"
#include "dconsole.h"
#include "data.h"
#include "dmext.h"

QMap<QString,QString> HDialogData::configs = QMap<QString,QString>();

HDialogData::HDialogData()
{
    dispFlags = HDispFlag_Default;
    objs.clear();
    attributeMap.clear();

    toolButtonReceiverMap.clear();
    toolButtonMethodMap.clear();
}

HDialogData::HDialogData(QString configName)
{
    dispFlags = HDispFlag_Default;
    objs.clear();
    attributeMap.clear();

    toolButtonReceiverMap.clear();
    toolButtonMethodMap.clear();

    setConfig(configName);
}

QString HDialogData::className()
{
    return "HDialogData";
}

HDialogData::~HDialogData()
{

}

HDialogData* HDialogData::setConfig(QString configName)
{
    if(configs.contains(configName))
    {
        QList<QString> atts = configs[configName].split(",");
        int i,c = atts.count();
        for(i = 0 ; i < c ; ++i)
        {
            QList<QString> attparts = atts[i].split("=");
            if(attparts.count() == 2)
            {
                if(attparts[0] == "displayflags")
                {
                    QStringList dflagstr = attparts[1].split("|");
                    HDispObjectFlags dflag = stringListToDisplayFlags(dflagstr);
                    setDisplayFlags(dflag);
                }
                else
                {
                    setAttribute(attparts[0],attparts[1]);
                }
            }
        }
    }
    return this;
}

HDialogData* HDialogData::add(HNamed *obj)
{
    objs.push_back(obj);
    return this;
}

QString HDialogData::attribute(QString name)
{
    if(attributeMap.contains(name))
        return attributeMap[name];
    return "";
}

QStringList HDialogData::allDefinedAttributes()
{
    return attributeMap.keys();
}

HDialogData* HDialogData::setAttribute(QString name,QString value)
{
    attributeMap[name] = value;
    return this;
}

HDialogData* HDialogData::setAttribute(QString name,int ivalue)
{
    attributeMap[name] = QString::asprintf("%d",ivalue);
    return this;
}

HDialogData* HDialogData::setAttribute(QString name,double dvalue)
{
    attributeMap[name] = QString::asprintf("%f",dvalue);
    return this;
}

HDialogData* HDialogData::setDisplayFlags(HDispObjectFlags flags)
{
    dispFlags = flags;
    return this;
}

HDispObjectFlags HDialogData::displayFlags()
{
    return dispFlags;
}

void HDialogData::addConfig(QString name,QString attributes)
{
    configs[name] = attributes;
}

HDialog* HDialogData::toHDialog()
{
    if(className() == "HDialog")
        return (HDialog *)this;
    return NULL;
}

HDialogData* HDialogData::addToolButtonHandler(QString code,QObject *receiver,const char *method)
{
    toolButtonReceiverMap[code] = receiver;
    toolButtonMethodMap[code] = method;
    return this;
}

void HDialogData::makeGui(QWidget *base)
{
    base->setWindowTitle(!attribute("window_title").isEmpty() ? attribute("window_title") : "Dialog...");
    QVBoxLayout *mlay = new QVBoxLayout(base);
    if(!attribute("title").isEmpty())
    {
        QLabel *titleLabel = new QLabel(attribute("title"),base);
        QHBoxLayout *titleLayout = new QHBoxLayout(0);
        titleLayout->setContentsMargins(5,5,5,5);
        titleLayout->setSpacing(5);
        titleLayout->addStretch();
        titleLayout->addWidget(titleLabel);
        titleLayout->addStretch();
        mlay->addLayout(titleLayout);
    }

    if(!attribute("toolbutton_count").isEmpty() && attribute("toolbutton_count").toInt() > 0)
    {
        int ti,tc = attribute("toolbutton_count").toInt();
        QHBoxLayout *toolLayout = new QHBoxLayout(0);
        if(!attribute("toolbar_stretch_before").isEmpty() && attribute("toolbar_stretch_before") == "yes")
            toolLayout->addStretch();
        for(ti = 0 ; ti < tc ; ++ti)
        {
            QPushButton *tb = new QPushButton(attribute(QString("toolbutton_%1_text").arg(ti+1)),base);
            QString code = attribute(QString("toolbutton_%1_code").arg(ti+1));
            if(toolButtonReceiverMap.contains(code) && toolButtonMethodMap.contains(code))
                base->connect(tb,SIGNAL(clicked()),toolButtonReceiverMap[code],toolButtonMethodMap[code]);
            toolLayout->addWidget(tb);
        }
        if(!attribute("toolbar_stretch_after").isEmpty() && attribute("toolbar_stretch_after") == "yes")
            toolLayout->addStretch();
        mlay->addLayout(toolLayout);
    }

    QBoxLayout *coreLay;
    if(attribute("orientation") == "horizontal")
        coreLay = new QHBoxLayout(0);
    else
        coreLay = new QVBoxLayout(0);

    int i,c = objs.count();
    for(i = 0 ; i < c ; ++i)
    {
        HNamed *obj = objs.at(i);

        if(obj->className() == "HRecord")
        {
            HRecordDisplay *hrd;
            if(attribute(QString("scrollable_%1").arg(i+1)) == "yes")
            {
                QScrollArea *scr = new QScrollArea(base);
                hrd = new HRecordDisplay(scr,(HRecord *)obj,dispFlags);
                scr->setWidget(hrd);
                scr->setWidgetResizable(true);
                coreLay->addWidget(scr);
            }
            else
            {
                hrd = new HRecordDisplay(base,(HRecord *)obj,dispFlags);
                coreLay->addWidget(hrd);
            }
            if(attribute("start") == "read" && !attribute("key").isEmpty())
            {
                ((HRecord *)obj)->readWithKey(attribute("key"));
            }
        }
        if(obj->className() == "HRecordLines")
        {
            HRecordLinesDisplay *hrld;
            if(attribute(QString("scrollable_%1").arg(i+1)) == "yes")
            {
                QScrollArea *scr = new QScrollArea(base);
                hrld = new HRecordLinesDisplay(scr,(HRecordLines *)obj,dispFlags);
                scr->setWidget(hrld);
                scr->setWidgetResizable(true);
                coreLay->addWidget(scr);
            }
            else
            {
                hrld = new HRecordLinesDisplay(base,(HRecordLines *)obj,dispFlags);
                coreLay->addWidget(hrld);
            }
            if(attribute("start") == "read")
            {
                ((HRecordLines *)obj)->readLines();
            }
        }
        if(obj->className() == "HDataMatrix")
        {
            HDataMatrixDisplay *hdmd;
            if(attribute(QString("scrollable_%1").arg(i+1)) == "yes")
            {
                QScrollArea *scr = new QScrollArea(base);
                hdmd = new HDataMatrixDisplay(scr,(HDataMatrix *)obj,dispFlags);
                scr->setWidget(hdmd);
                scr->setWidgetResizable(true);
                coreLay->addWidget(scr);
            }
            else
            {
                hdmd = new HDataMatrixDisplay(base,(HDataMatrix *)obj,dispFlags);
                coreLay->addWidget(hdmd);
            }
        }
    }

    mlay->addLayout(coreLay);

    bool need_sep_space = false;
    QHBoxLayout *bottomLay = new QHBoxLayout(0);
    bottomLay->addStretch();
    for(i = 0 ; i < 5 ; ++i)
    {
        if(!attribute(QString("button_%1_text").arg(i+1)).isEmpty())
        {
            QPushButton *btn = new QPushButton(attribute(QString("button_%1_text").arg(i+1)),base);
            if(need_sep_space)
                bottomLay->addSpacing(!attribute("bottom_sep_space").isEmpty() ? attribute("bottom_sep_space").toInt() : 10);

            if(attribute(QString("button_%1_action").arg(i+1)) == "accept")
                QObject::connect(btn,SIGNAL(clicked()),base,SLOT(accept()));
            if(attribute(QString("button_%1_action").arg(i+1)) == "reject")
                QObject::connect(btn,SIGNAL(clicked()),base,SLOT(reject()));
            bottomLay->addWidget(btn);
            need_sep_space = true;
        }
    }

    bottomLay->addStretch();

    if(!attribute("stretch_before_bottom_buttons").isEmpty() && attribute("stretch_before_bottom_buttons") == "yes")
    {
        mlay->addStretch();
    }

    mlay->addLayout(bottomLay);

    if(!attribute("width").isEmpty() && !attribute("height").isEmpty())
        base->resize(attribute("width").toInt(),attribute("height").toInt());
}

HRecord* HDialogData::getHRecord(int objIndex)
{
    if(objs[objIndex]->className() == "HRecord")
        return (HRecord *)objs[objIndex];
    return NULL;
}

HRecordLines* HDialogData::getHRecordLines(int objIndex)
{
    if(objs[objIndex]->className() == "HRecordLines")
        return (HRecordLines *)objs[objIndex];
    return NULL;
}

HDataMatrix* HDialogData::getHDataMatrix(int objIndex)
{
    if(objs[objIndex]->className() == "HDataMatrix")
        return (HDataMatrix *)objs[objIndex];
    return NULL;
}

// ////////////////////////////////////////////////////////////////////////// //

HDialog::HDialog(QWidget *parent)
 : QDialog(parent)
{

}

HDialog::HDialog(QString configName,QWidget *parent)
 : QDialog(parent)
{
    setConfig(configName);
}

QString HDialog::className()
{
    return "HDialog";
}

int HDialog::exec()
{
    makeGui(this);
    return QDialog::exec();
}

HDialog::~HDialog()
{

}
//end code.
