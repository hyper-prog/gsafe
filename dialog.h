/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2021 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    dialog.h
*/

#ifndef GSAFE__DIALOG_HEADER_FILE_X_
#define GSAFE__DIALOG_HEADER_FILE_X_

/** \defgroup dialog dialog */
/*  @{  */

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "dm.h"
#include "gui.h"
#include "guiext.h"

class HDialog;

/** Base data class to rapid build dialogs for gSAFE structures. */
class HDialogData : public HNamed
{
public:
    HDialogData();
    HDialogData(QString configName);
    ~HDialogData();
    virtual QString className();

    void makeGui(QWidget *base);

    HDialogData* add(HNamed *obj);

    HDialogData* setConfig(QString configName);

    QString attribute(QString name);
    QStringList allDefinedAttributes();
    HDialogData* setAttribute(QString name,QString value);
    HDialogData* setAttribute(QString name,int ivalue);
    HDialogData* setAttribute(QString name,double dvalue);

    HDialogData* setDisplayFlags(HDispObjectFlags flags);
    HDispObjectFlags displayFlags();

    static void addConfig(QString name,QString attributes);

    static HDialogData* fromJson(QString jsonData);
    static HDialogData* fromJsonFile(QString jsonFileName);
    virtual bool applyJson(QString jsonData);
    virtual bool applyJsonFile(QString jsonFileName);
    virtual QString toJson(HJsonFlag flags = HJsonFlag_Default);

    static HDialogData* fromJson_inWork(QJsonValue jsonValue);
    virtual bool applyJson_inWork(QJsonObject jsonObject);
    virtual QJsonValue toJson_inWork(HJsonFlag flags);
    static QString readJsonFile_inWork(QString jsonFileName);

    HDialog* toHDialog();

    HRecord* getHRecord(int objIndex);
    HRecordLines* getHRecordLines(int objIndex);
    HDataMatrix* getHDataMatrix(int objIndex);

    HDialogData* addToolButtonHandler(QString code,QObject *receiver,const char *method);

protected:
    QList<HNamed *> objs;
    HDispObjectFlags dispFlags;
    QMap<QString,QString> attributeMap;

    QMap<QString,QObject *> toolButtonReceiverMap;
    QMap<QString,const char *> toolButtonMethodMap;

    static QMap<QString,QString> configs;

};

/** Rapid dialog for gSAFE structures, based on HDialogData. */
class HDialog : public QDialog , public HDialogData
{
public:
    HDialog(QWidget *parent);
    HDialog(QString configName,QWidget *parent);
    ~HDialog();
    virtual QString className();

    static HDialog* fromJson(QWidget *parent,QString jsonData);
    static HDialog* fromJsonFile(QWidget *parent,QString jsonFileName);

    static HDialog* fromJson_inWork(QWidget *parent,QJsonValue jsonValue);

    using HDialogData::setAttribute;

public slots:
    virtual int exec();

};

/* @} */

#endif
