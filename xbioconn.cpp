/*
    gSAFE - LIB
    general Sql dAtabase FrontEnd

   (C) 2011-2020 Peter Deak  (hyper80@gmail.com)

    License: LGPLv2.1

        XBase Connection to gDAFE datelib (based on XBase Import/Export lib)
    xbioconn.cpp
*/

/*
 * WARNING: This file's is depends on xbio.h/xbio.cpp which requires XBase library
 *   (http://linux.techass.com/projects/xdb/)
 * If you don't want to use the XBase support just remove
 * the xbio* files from your project, or undefine (remove) the ENABLE_XBASE_SUPPORT macro!
 */

#include "xbioconn.h"
#include "dconsole.h"

#ifdef ENABLE_XBASE_SUPPORT


HXBaseRecordSync::HXBaseRecordSync(void)
{
    whoami = "HXBaseRecordSync";
    fieldconn.clear();
}

HXBaseRecordSync::~HXBaseRecordSync(void)
{
    fieldconn.clear();
}

void HXBaseRecordSync::assign(QString htablefieldsqlname, QString xbasefieldname)
{
    fieldconn.push_back(HFieldPairs(htablefieldsqlname,xbasefieldname));
}

void HXBaseRecordSync::clearAssigns(void)
{
    fieldconn.clear();
}

int HXBaseRecordSync::syncHTableToXBase(HTable *record,HXBaseFileWriter *dbf)
{
    QList<HFieldPairs>::iterator i;
    for(i=fieldconn.begin();i!=fieldconn.end();++i)
    {
        QString gsafetype;
        HDataField *d;
        d = record->fieldBySqlName(i->gsafefieldname);
        if(d == NULL)
        {
            sdebug(QString("ERROR in HXBaseRecordSync::syncHTableToXBase :\n"
                           "Can't find datafield in HTable with name \"%1\" (Passed with assign) SKIPPING!")
                   .arg(i->gsafefieldname));
            continue;
        }
        gsafetype = d->getWhoami();

        if(gsafetype == "HKey"       ||
           gsafetype == "HSmallText" ||
           gsafetype == "HLargeText" ||
           gsafetype == "HCharHash"  ||
           gsafetype == "HSqlChoose" )
            dbf->setFieldStr(i->xbasefieldname,d->getValue().toString() );

        if(gsafetype == "HNumHash" ||
           gsafetype == "HNumber" )
            dbf->setFieldDecimal(i->xbasefieldname,d->getValue().toInt() );

        if(gsafetype == "HFloating")
            dbf->setFieldFloat(i->xbasefieldname,d->getValue().toDouble());

        if(gsafetype == "HDate")
            dbf->setFieldDate(i->xbasefieldname,d->getValue().toDate());

        if(gsafetype == "HCheck")
            dbf->setFieldBool(i->xbasefieldname,d->getValue().toBool());

        if(gsafetype == "HTimestamp")
            dbf->setFieldStr(i->xbasefieldname,d->getValue().toString());

        if(gsafetype == "HStatic")
            continue;
    }

    return 0;
}

int HXBaseRecordSync::syncXBaseToHTable(HXBaseFileReader *dbf, HTable *record)
{
    QList<HFieldPairs>::iterator i;
    for(i=fieldconn.begin();i!=fieldconn.end();++i)
    {
        QString gsafetype;
        HDataField *d;
        d = record->fieldBySqlName(i->gsafefieldname);

        if(d == NULL)
        {
            sdebug(QString("ERROR in HXBaseRecordSync::syncXBaseToHTable :\n"
                           "Can't find datafield in HTable with name \"%1\" (Passed with assign) SKIPPING!")
                   .arg(i->gsafefieldname));
            continue;
        }

        gsafetype = d->getWhoami();

        if(gsafetype == "HKey"       ||
           gsafetype == "HSmallText" ||
           gsafetype == "HLargeText" ||
           gsafetype == "HCharHash"  ||
           gsafetype == "HSqlChoose" )
            d->setValue(QVariant( dbf->getFieldStr(i->xbasefieldname) ));

        if(gsafetype == "HNumHash" ||
           gsafetype == "HNumber" )
            d->setValue(QVariant( (int) dbf->getFieldDecimal(i->xbasefieldname) ));

        if(gsafetype == "HFloating")
            d->setValue(QVariant( dbf->getFieldFloat(i->xbasefieldname) ));

        if(gsafetype == "HDate")
            d->setValue(QVariant( dbf->getFieldDate(i->xbasefieldname) ));

        if(gsafetype == "HCheck")
            d->setValue(QVariant( dbf->getFieldBool(i->xbasefieldname) ));

        if(gsafetype == "HTimestamp")
            d->setValue(QVariant( dbf->getFieldStr(i->xbasefieldname) ));

        if(gsafetype == "HStatic")
            continue;
    }

    return 0;

}



#endif // ENABLE_XBASE_SUPPORT

//end code.
