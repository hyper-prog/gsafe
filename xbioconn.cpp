/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

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
    fieldconn.clear();
}

QString HXBaseRecordSync::className()
{
    return "HXBaseRecordSync";
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

int HXBaseRecordSync::syncHRecordToXBase(HRecord *record,HXBaseFileWriter *dbf)
{
    QList<HFieldPairs>::iterator i;
    for(i=fieldconn.begin();i!=fieldconn.end();++i)
    {
        QString gsafetype;
        HField *d;
        d = record->fieldByName(i->gsafefieldname);
        if(d == NULL)
        {
            sdebug(QString("ERROR in HXBaseRecordSync::syncHTableToXBase :\n"
                           "Can't find field in HRecord with name \"%1\" (Passed with assign) SKIPPING!")
                   .arg(i->gsafefieldname));
            continue;
        }
        gsafetype = d->className();

        if(gsafetype == "HStaticField")
            continue;
        if(gsafetype == "HNKeyField") dbf->setFieldDecimal(i->xbasefieldname, d->strValue().toInt() );
        if(gsafetype == "HSKeyField") dbf->setFieldStr(i->xbasefieldname, d->strValue() );
        if(gsafetype == "HSmallTextField") dbf->setFieldStr(i->xbasefieldname, d->strValue() );
        if(gsafetype == "HLargeTextField") dbf->setFieldStr(i->xbasefieldname, d->strValue() );
        if(gsafetype == "HNumberField") dbf->setFieldDecimal(i->xbasefieldname, ((HNumberField *)d)->value() );
        if(gsafetype == "HFloatingField") dbf->setFieldFloat(i->xbasefieldname, ((HFloatingField *)d)->value() );
        if(gsafetype == "HDateField") dbf->setFieldDate(i->xbasefieldname, ((HDateField *)d)->value() );
        if(gsafetype == "HTimestampField") dbf->setFieldStr(i->xbasefieldname, d->strValue() );
        if(gsafetype == "HCheckField") dbf->setFieldBool(i->xbasefieldname, ((HCheckField *)d)->value() );
        if(gsafetype == "HTxtSelectField") dbf->setFieldStr(i->xbasefieldname, d->strValue() );
        if(gsafetype == "HNumSelectField") dbf->setFieldDecimal(i->xbasefieldname, d->strValue().toInt() );
        if(gsafetype == "HSqlNChooseField") dbf->setFieldDecimal(i->xbasefieldname, d->strValue().toInt() );
        if(gsafetype == "HSqlSChooseField") dbf->setFieldStr(i->xbasefieldname, d->strValue() );
    }
    return 0;
}

int HXBaseRecordSync::syncXBaseToHRecord(HXBaseFileReader *dbf, HRecord *record)
{
    QList<HFieldPairs>::iterator i;
    for(i=fieldconn.begin();i!=fieldconn.end();++i)
    {
        QString gsafetype;
        HField *d;
        d = record->fieldByName(i->gsafefieldname);

        if(d == NULL)
        {
            sdebug(QString("ERROR in HXBaseRecordSync::syncXBaseToHTable :\n"
                           "Can't find field in HRecord with name \"%1\" (Passed with assign) SKIPPING!")
                   .arg(i->gsafefieldname));
            continue;
        }

        gsafetype = d->className();

        if(gsafetype == "HStaticField")
            continue;
        if(gsafetype == "HNKeyField") d->setStrValue( dbf->getFieldStr(i->xbasefieldname) );
        if(gsafetype == "HSKeyField") d->setStrValue( dbf->getFieldStr(i->xbasefieldname) );
        if(gsafetype == "HSmallTextField") d->setStrValue( dbf->getFieldStr(i->xbasefieldname) );
        if(gsafetype == "HLargeTextField") d->setStrValue( dbf->getFieldStr(i->xbasefieldname) );
        if(gsafetype == "HNumberField") ((HNumberField *)d)->setValue( (int) dbf->getFieldDecimal(i->xbasefieldname) );
        if(gsafetype == "HFloatingField") ((HFloatingField *)d)->setValue( dbf->getFieldFloat(i->xbasefieldname) );
        if(gsafetype == "HDateField") ((HDateField *)d)->setValue( dbf->getFieldDate(i->xbasefieldname) );
        if(gsafetype == "HTimestampField") d->setStrValue( dbf->getFieldStr(i->xbasefieldname) );
        if(gsafetype == "HCheckField") ((HCheckField *)d)->setValue( dbf->getFieldBool(i->xbasefieldname) );
        if(gsafetype == "HTxtSelectField") d->setStrValue( dbf->getFieldStr(i->xbasefieldname) );
        if(gsafetype == "HNumSelectField") d->setStrValue( QString("%1").arg(dbf->getFieldDecimal(i->xbasefieldname)) );
        if(gsafetype == "HSqlNChooseField") d->setStrValue( QString("%1").arg(dbf->getFieldDecimal(i->xbasefieldname)) );
        if(gsafetype == "HSqlSChooseField") d->setStrValue( dbf->getFieldStr(i->xbasefieldname) );
    }
    return 0;
}

#endif // ENABLE_XBASE_SUPPORT

//end code.
