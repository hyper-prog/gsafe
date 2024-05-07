/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2024 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

        XBase Connection to gDAFE datelib (based on XBase Import/Export lib)
   xbioconn.h
*/

/*
 * WARNING: This file's is depends on xbio.h/xbio.cpp which requires XBase library
 *   (http://linux.techass.com/projects/xdb/)
 * If you don't want to use the XBase support just remove
 * the xbio* files from your project, or undefine (remove) the ENABLE_XBASE_SUPPORT macro!
 */

#ifdef ENABLE_XBASE_SUPPORT

#ifndef GSAFE_XBASE_IOCONN_H
#define GSAFE_XBASE_IOCONN_H

#include <QtCore>

#include "data.h"
#include "dm.h"
#include "dmext.h"
#include "ftypes.h"
#include "xbio.h"

/** \defgroup xbaseio xbaseio */
/* @{ */


class HFieldPairs
{
public:
    HFieldPairs(QString s,QString x)
    {
        gsafefieldname = s;
        xbasefieldname = x;
    }
    QString gsafefieldname;
    QString xbasefieldname;
};

/** The HXBaseRecordSync enables you to easely syncronise a HTable record to Xbase and reverse.
    After you assign the gSAFE fields (HDataField) to XBase fields you can copy the whole record in one step.
    Look this exaples:\n\n

    Import an XBase (.dbf) table to SQL table with gsafe by record to record.
    (Note: This is not a directy copy, both tables can contains more field's than copyed)
    \code
        int r;
        HXBaseRecordSync sync; //Make a syncronizer object
        HTable *t = myHFactory->genHTable("Dbf1Table"); //Generate/define a gSAFE record (HTable)
        HXBaseFileReader *dbf = new HXBaseFileReader(new HCodepage852(),true); //Make a dbf reader object

        sync.assign("name","NAME_CHAR");
        sync.assign("age","AGE_NUM");

        if(dbf->open("FILE.DBF"))
        {
            //error
        }
        r = dbf->firstRecord();
        while(r == 0)
        {
            sync.syncXBaseToHTable(dbf,t); //sync the current xbase record to the HTable
            t->insertRecord(); //Insert the HTable record to the sql table
            r = dbf->nextRecord();
        }
        dbf->close();
        delete dbf;
        delete t;
    \endcode

    Export a complete sql table by record to record to XBase (.dbf) file.
    (Note: This is not a directy copy, both tables can contains more field's than copyed)
    \code
        HXBaseRecordSync sync; //Make a syncronizer object
        HTable *t = myfactory->genHTable("partner"); //Generate/define a gSAFE record (HTable)

        HXBaseFileWriter *dbf = new HXBaseFileWriter(new HCodepage852(),true); //Make a dbf writer object

        dbf->defineField("NAME",Char,40); //Define the fields of dbf file
        dbf->defineField("ADDRESS",Char,40);

        sync.assign("name","NAME"); //Assign the XBase field to Sql field
        sync.assign("address","ADDRESS");

        if(dbf->create("EXPORT.DBF",3)) //Create the XBase file
        {
            //error
        }

        HPlainDataMatrix *m = t->submitNResultQuery(1,"SELECT key FROM partner ORDER BY name;","Error");
        m->firstRow();
        do
        {
            t->updateWithKey(m->currentRowStr()[0]);
            sync.syncHTableToXBase(t,dbf);
            dbf->appendRecord();
        }
        while(m->nextRow());
        dbf->close();
        delete dbf;
        delete t;
    \endcode
*/
class HXBaseRecordSync : public QObject, public HNamed
{
    Q_OBJECT

public:

    /** Creates a HXBaseRecordSync object */
    HXBaseRecordSync();
    ~HXBaseRecordSync();

    virtual QString className();

    /** Add a new gSAFE - XBase field assign */
    void assign(QString htablefieldsqlname,QString xbasefieldname);
    /** Clears all gSAFE - XBase field assign */
    void clearAssigns(void);

    /** Sync the assugned data from gSAFE HRecord to XBase */
    int syncHRecordToXBase(HRecord *record,HXBaseFileWriter *dbf);
    /** Sync the assugned data from XBase to gSAFE HRecord */
    int syncXBaseToHRecord(HXBaseFileReader *dbf,HRecord *record);

private:
    QList<HFieldPairs> fieldconn;

};

/* @} */

#endif // GSAFE_XBASE_IOCONN_H

#endif // ENABLE_XBASE_SUPPORT
