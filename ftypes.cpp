/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   dm.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore>

#include "data.h"
#include "dm.h"
#include "dconsole.h"
#include "builder.h"
#include "ftypes.h"

HStaticField::HStaticField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    fNoSql = true;
}

HStaticField::~HStaticField(void)
{
}

QString HStaticField::className(void)
{
    return "HStaticField";
}

HSqlBuilder_FieldValueType HStaticField::fvType(void)
{
    return BQInvalid;
}

bool HStaticField::isUpdateRequired(bool forRealUpdate)
{
    Q_UNUSED(forRealUpdate)
    return false;
}

void HStaticField::putsOnSetter(HSqlBuilder *b,QString tableName)
{
    Q_UNUSED(b)
    Q_UNUSED(tableName)
}

void HStaticField::putsOnGetter(HSqlBuilder *b,QString tableName)
{
    Q_UNUSED(b)
    Q_UNUSED(tableName)
}

HSKeyField::HSKeyField(QString sqlname,QString description,QString title)
: HField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "varchar(16) primary key autoincrement";
    defaultCreateTypes["pgsql"]  = "varchar(16) primary key default nextval('__SEQUENCENAME__')";
    defaultCreateTypes["mysql"]  = "varchar(16) primary key NOT-IMPLEMENTED";
}

HSKeyField::~HSKeyField(void)
{
}

QString HSKeyField::className(void)
{
    return "HSKeyField";
}

bool HSKeyField::isKey(void)
{
    return true;
}

HSqlBuilder_FieldValueType HSKeyField::fvType(void)
{
    return Quoted;
}

bool HSKeyField::isUpdateRequired(bool forRealUpdate)
{
    Q_UNUSED(forRealUpdate)
    return false;
}

QString HSKeyField::convertToDisplay(QString fv)
{
    if(fv.isEmpty())
        return QString("- %1 -").arg(QObject::tr("None","HKeyField"));
    return fv;
}

HNKeyField::HNKeyField(QString sqlname,QString description,QString title)
: HField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "integer primary key autoincrement";
    defaultCreateTypes["pgsql"]  = "serial primary key";
    defaultCreateTypes["mysql"]  = "serial";
}

HNKeyField::~HNKeyField(void)
{

}

QString HNKeyField::className(void)
{
    return "HNKeyField";
}

bool HNKeyField::isKey(void)
{
    return true;
}

HSqlBuilder_FieldValueType HNKeyField::fvType(void)
{
    return Unquoted;
}

bool HNKeyField::isUpdateRequired(bool forRealUpdate)
{
    Q_UNUSED(forRealUpdate)
    return false;
}

QString HNKeyField::convertToDisplay(QString fv)
{
    if(fv.isEmpty())
        return QString("- %1 -").arg(QObject::tr("None","HKeyField"));
    return fv;
}

HSmallTextField::HSmallTextField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "varchar";
    defaultCreateTypes["pgsql"]  = "varchar";
    defaultCreateTypes["mysql"]  = "varchar(128)";
}

HSmallTextField::~HSmallTextField(void)
{
}

QString HSmallTextField::className(void)
{
    return "HSmallTextField";
}

HSqlBuilder_FieldValueType HSmallTextField::fvType(void)
{
    return Quoted;
}

HLargeTextField::HLargeTextField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = HSqlBuilder::translateDialect("<<<longtext_type>>>","sqlite");
    defaultCreateTypes["pgsql"]  = HSqlBuilder::translateDialect("<<<longtext_type>>>","pgsql");
    defaultCreateTypes["mysql"]  = HSqlBuilder::translateDialect("<<<longtext_type>>>","mysql");
}

HLargeTextField::~HLargeTextField(void)
{
}

QString HLargeTextField::className(void)
{
    return "HLargeTextField";
}

HSqlBuilder_FieldValueType HLargeTextField::fvType(void)
{
    return Quoted;
}

HTxtSelectField::HTxtSelectField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "varchar(16)";
    defaultCreateTypes["pgsql"]  = "varchar(16)";
    defaultCreateTypes["mysql"]  = "varchar(16)";
}

HTxtSelectField::~HTxtSelectField(void)
{
}

void HTxtSelectField::initialize(void)
{
    if(!attribute("autofill_selectables_start").isEmpty() && !attribute("autofill_selectables_end").isEmpty())
    {
        int i;
        for(i = attribute("autofill_selectables_start").toInt() ; i <= attribute("autofill_selectables_end").toInt() ; ++i)
            addSelectableItem(QString("%1").arg(i),QString("%1").arg(i));
    }
    HField::initialize();
}

QString HTxtSelectField::className(void)
{
    return "HTxtSelectField";
}

HSqlBuilder_FieldValueType HTxtSelectField::fvType(void)
{
    return Quoted;
}

HTxtSelectField* HTxtSelectField::clearSelectables()
{
    selectables.clear();
    emit featureChanged();
    return this;
}

HTxtSelectField* HTxtSelectField::addSelectableItem(QString key,QString val)
{
    selectables.addItem(key,val);
    emit featureChanged();
    return this;
}

HTxtSelectField* HTxtSelectField::setSelectableItems(QList<QString>& keys,QList<QString>& values)
{
    selectables.setItems(keys,values);
    emit featureChanged();
    return this;
}

HTxtSelectField* HTxtSelectField::addUnSelectableItem(QString key,QString val)
{
    selectables.addUnlistedItem(key,val);
    return this;
}

HTxtSelectField* HTxtSelectField::setUnSelectableItems(QList<QString>& keys,QList<QString>& values)
{
    selectables.setUnlistedItems(keys,values);
    return this;
}

void HTxtSelectField::setStrValue_NoStatusChange(QString value)
{
    v = str2str(value,selectables.keys(),selectables.unlistedKeys(),def);
}

QString HTxtSelectField::str2str(QString s,QList<QString> selectableKeys,QList<QString> unSelectableKeys,QString def)
{
    if(selectableKeys.contains(s))
        return s;
    if(unSelectableKeys.contains(s))
        return s;
    if(selectableKeys.contains(def))
        return def;
    if(unSelectableKeys.contains(def))
        return def;
    if(selectableKeys.size() > 0)
        return selectableKeys.at(0);
    return "";
}

QString HTxtSelectField::convertToDisplay(QString fv)
{
    if(selectables.hasKey(fv,true))
        return selectables.value(fv,true);
    return QObject::tr("Error - Unknown","HTxtSelectField");
}

void HTxtSelectField::toJson_inWork_spec(QJsonObject& fo)
{
    jsonGenerateHelper(fo);
}

void HTxtSelectField::applyJson_inWork_spec(QJsonObject& fo)
{
    jsonApplyHelper(fo);
}

HNumSelectField::HNumSelectField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "integer";
    defaultCreateTypes["pgsql"]  = "numeric(8)";
    defaultCreateTypes["mysql"]  = "numeric(8)";
}

HNumSelectField::~HNumSelectField(void)
{
}

void HNumSelectField::initialize(void)
{
    if(!attribute("autofill_selectables_start").isEmpty() && !attribute("autofill_selectables_end").isEmpty())
    {
        int i;
        for(i = attribute("autofill_selectables_start").toInt() ; i <= attribute("autofill_selectables_end").toInt() ; ++i)
            addSelectableItem(i,QString("%1").arg(i));
    }
    HField::initialize();
}

QString HNumSelectField::className(void)
{
    return "HNumSelectField";
}

HSqlBuilder_FieldValueType HNumSelectField::fvType(void)
{
    return Unquoted;
}

HNumSelectField* HNumSelectField::clearSelectables()
{
    selectables.clear();
    emit featureChanged();
    return this;
}

HNumSelectField* HNumSelectField::addSelectableItem(int key,QString val)
{
    selectables.addItem(key,val);
    emit featureChanged();
    return this;
}

HNumSelectField* HNumSelectField::setSelectableItems(QList<int>& keys,QList<QString>& values)
{
    selectables.setItems(keys,values);
    emit featureChanged();
    return this;
}

HNumSelectField* HNumSelectField::addUnSelectableItem(int key,QString val)
{
    selectables.addUnlistedItem(key,val);
    return this;
}

HNumSelectField* HNumSelectField::setUnSelectableItems(QList<int>& keys,QList<QString>& values)
{
    selectables.setUnlistedItems(keys,values);
    return this;
}

void HNumSelectField::setStrValue_NoStatusChange(QString value)
{
    v = str2str(value,selectables.keys(),selectables.unlistedKeys(),def);
}

QString HNumSelectField::str2str(QString s,QList<int> selectableKeys,QList<int> unSelectableKeys,QString def)
{
    bool ok;
    int iS = s.toInt(&ok);
    if(ok)
    {
        if(selectableKeys.contains(iS))
            return s;
        if(unSelectableKeys.contains(iS))
            return s;
    }
    int iD = def.toInt(&ok);
    if(ok)
    {
        if(selectableKeys.contains(iD))
            return def;
        if(unSelectableKeys.contains(iD))
            return def;
    }
    if(selectableKeys.size() > 0)
        return QString("%1").arg(selectableKeys.at(0));
    return "";
}

QString HNumSelectField::convertToDisplay(QString fv)
{
    bool ok;
    int iFv = fv.toInt(&ok);
    if(ok)
    {
        if(selectables.hasKey(iFv,true))
            return selectables.value(iFv,true);
    }
    return QObject::tr("Error - Unknown","HNumSelectField");
}

void HNumSelectField::toJson_inWork_spec(QJsonObject& fo)
{
    jsonGenerateHelper(fo);
}

void HNumSelectField::applyJson_inWork_spec(QJsonObject& fo)
{
    jsonApplyHelper(fo);
}

HCheckField::HCheckField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    fDatabaseAndMemoryRepDiffers = true;
    v = str2str(def);
    defaultCreateTypes["sqlite"] = "boolean";
    defaultCreateTypes["pgsql"]  = "boolean";
    defaultCreateTypes["mysql"]  = "boolean";
}

HCheckField::~HCheckField(void)
{
}

QString HCheckField::className(void)
{
    return "HCheckField";
}

HSqlBuilder_FieldValueType HCheckField::fvType(void)
{
    return Unquoted;
}

void HCheckField::setStrValue_NoStatusChange(QString value)
{
    v = str2str(value);
}

QString HCheckField::str2str(QString s)
{
    if(s.toLower() == "true" ||
       s.toLower() == "t"    ||
       s.toLower() == "1"    ||
       s.toLower() == "y"    ||
       s.toLower() == "yes" )
        return "true";
    return "false";
}

bool HCheckField::str2val(QString s)
{
    if(s == "true")
        return true;
    return false;
}

QString HCheckField::val2str(bool b)
{
    if(b)
        return "true";
    return "false";
}

bool HCheckField::value()
{
    return str2val(strValue());
}

HCheckField* HCheckField::setValue(bool bv)
{
    setStrValue(val2str(bv));
    return this;
}

void HCheckField::setValue_Gui(bool bv)
{
    setStrValue_Gui(val2str(bv));
}

QString HCheckField::convertToDisplay(QString fv)
{
    QString conv = str2str(fv);
    if(conv == "true")
        return QObject::tr("yes","HCheckField");
    if(conv == "false")
        return QObject::tr("no","HCheckField");
    return "Error";
}

void HCheckField::putsOnSetter(HSqlBuilder *b,QString tableName)
{
    Q_UNUSED(tableName)
    if(fNoSql)
        return;
    b->set_fe(derivedSqlFieldName(),strValue(),genBuilderOptions(SqlBuilderGet));
}

HNumberField::HNumberField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    def = "0";
    defaultCreateTypes["sqlite"] = "integer";
    defaultCreateTypes["pgsql"]  = "numeric(16)";
    defaultCreateTypes["mysql"]  = "numeric(16)";
}

HNumberField::~HNumberField(void)
{
}

QString HNumberField::className(void)
{
    return "HNumberField";
}

HSqlBuilder_FieldValueType HNumberField::fvType(void)
{
    return Unquoted;
}

void HNumberField::setStrValue_NoStatusChange(QString value)
{
    QString v_set = str2str(value,attribute("minimum"),attribute("maximum"));
    if(!v_set.isEmpty())
        v = v_set;
}

QString HNumberField::str2str(QString s,QString mins,QString maxs)
{
    bool ok;
    int i,m;
    i = s.toInt(&ok);
    if(!ok)
        return "";

    if(!mins.isEmpty())
    {
        m = mins.toInt(&ok);
        if(ok && m > i)
            return "";
    }
    if(!maxs.isEmpty())
    {
        m = maxs.toInt(&ok);
        if(ok && m < i)
            return "";
    }
    return s;
}

int HNumberField::str2val(QString s)
{
    bool ok;
    int i;
    i = s.toInt(&ok);
    if(!ok)
        return 0;
    return i;
}

QString HNumberField::val2str(int i,QString mins,QString maxs)
{
    bool ok;
    int m;
    if(!mins.isEmpty())
    {
        m = mins.toInt(&ok);
        if(ok && m > i)
            i = m;
    }
    if(!maxs.isEmpty())
    {
        m = maxs.toInt(&ok);
        if(ok && m < i)
            i = m;
    }
    return QString::asprintf("%d",i);
}

int HNumberField::value()
{
    return str2val(strValue());
}

HNumberField* HNumberField::setValue(int iv)
{
    setStrValue(val2str(iv,attribute("minimum"),attribute("maximum")));
    return this;
}

void HNumberField::setValue_Gui(int iv)
{
    setStrValue_Gui(val2str(iv,attribute("minimum"),attribute("maximum")));
}

QString HNumberField::convertToDisplay(QString fv)
{
    return fv;
}

HFloatingField::HFloatingField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    def = "0";
    defaultCreateTypes["sqlite"] = "real";
    defaultCreateTypes["pgsql"]  = "numeric(22,6)";
    defaultCreateTypes["mysql"]  = "numeric(22,6)";
}

HFloatingField::~HFloatingField(void)
{
}

QString HFloatingField::className(void)
{
    return "HFloatingField";
}

HSqlBuilder_FieldValueType HFloatingField::fvType(void)
{
    return Unquoted;
}

void HFloatingField::setStrValue_NoStatusChange(QString value)
{
    QString v_set = str2str(value,attribute("minimum"),attribute("maximum"));
    if(!v_set.isEmpty())
        v = v_set;
}

QString HFloatingField::str2str(QString s,QString mins,QString maxs)
{
    double d,m;
    bool ok;
    d = s.toDouble(&ok);
    if(!ok)
        return "";

    if(!mins.isEmpty())
    {
        m = mins.toDouble(&ok);
        if(ok && m > d)
            return "";
    }
    if(!maxs.isEmpty())
    {
        m = maxs.toDouble(&ok);
        if(ok && m < d)
            return "";
    }
    return s;
}

double HFloatingField::str2val(QString s)
{
    bool ok;
    double d;
    d = s.toDouble(&ok);
    if(!ok)
        return 0;
    return d;
}

QString HFloatingField::val2str(double d,QString mins,QString maxs,QString prec)
{
    bool ok;
    double m;
    if(!mins.isEmpty())
    {
        m = mins.toDouble(&ok);
        if(ok && m > d)
            d = m;
    }
    if(!maxs.isEmpty())
    {
        m = maxs.toDouble(&ok);
        if(ok && m < d)
            d = m;
    }

    QString format = "%f";
    if(!prec.isEmpty())
    {
        bool ok = false;
        int precision = prec.toInt(&ok);
        if (ok && precision >= 0)
            format = QString("%.%1f").arg(precision);
    }

    return QString::asprintf(format.toUtf8().constData(), d);
}

double HFloatingField::value()
{
    return str2val(strValue());
}

HFloatingField* HFloatingField::setValue(double dv)
{
    setStrValue(val2str(dv,attribute("minimum"),attribute("maximum"),attribute("strconv_precision")));
    return this;
}

void HFloatingField::setValue_Gui(double dv)
{
    setStrValue_Gui(val2str(dv,attribute("minimum"),attribute("maximum"),attribute("strconv_precision")));
}

QString HFloatingField::convertToDisplay(QString fv)
{
    int dMminDf = 0,dMmaxDf = 4,dMgroup = 0;
    if(!attribute("display_min_decimals").isEmpty())
        dMminDf = attribute("display_min_decimals").toInt();
    if(!attribute("display_max_decimals").isEmpty())
        dMmaxDf = attribute("display_max_decimals").toInt();
    if(!attribute("display_group_thousands").isEmpty())
        dMgroup = attribute("display_group_thousands").toInt();
    return doubleToQString(str2val(fv),dMminDf,dMmaxDf,dMgroup);
}

// /////////////////////////////////////////////////////////////////////////// //

QString HDateField::fallbackDateValue = "1900-01-01";

HDateField::HDateField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    unkAllowed = false;
    lastValidValue = v;
    defaultCreateTypes["sqlite"] = "varchar";
    defaultCreateTypes["pgsql"]  = "date";
    defaultCreateTypes["mysql"]  = "date";
}

HDateField::~HDateField(void)
{
}

QString HDateField::className(void)
{
    return "HDateField";
}

HSqlBuilder_FieldValueType HDateField::fvType(void)
{
    return Quoted;
}

QString HDateField::convertToDisplay(QString fv)
{
    if(fv.isEmpty())
    {
        if(unknownAllowed())
            QObject::tr("Unknown","HDateField_convertToDisplay");
        else
            return fallbackDateValue;
    }
    return fv;
}

void HDateField::setStrValue_NoStatusChange(QString value)
{
    v = str2str(value,unknownAllowed());
    if(!v.isEmpty() && v != fallbackDateValue)
        lastValidValue = v;
    if(v == fallbackDateValue)
        v = str2str(def,unknownAllowed());
}

QDate HDateField::value()
{
    return str2val(strValue(),unknownAllowed());
}

HDateField* HDateField::setValue(QDate dv)
{
    QString sv = val2str(dv,unknownAllowed());
    if(!sv.isEmpty())
        lastValidValue = sv;
    setStrValue(sv);
    return this;
}

void HDateField::setValue_Gui(QDate dv)
{
    QString sv = val2str(dv,unknownAllowed());
    if(!sv.isEmpty())
        lastValidValue = sv;
    setStrValue_Gui(sv);
}

HDateField* HDateField::setUnknownAllowed(bool ua,QString setDefaultStrValue)
{
    unkAllowed = ua;
    if(!setDefaultStrValue.isEmpty())
        setStrDefault(setDefaultStrValue);

    if(!unkAllowed && strValue().isEmpty())
    {
        if(!lastValidValue.isEmpty())
            setStrValue(lastValidValue);
        else
        {
            setStrValue(def);
            sts = HFieldStatus_Init;
        }
    }
    return this;
}

bool HDateField::unknownAllowed()
{
    return unkAllowed;
}

QString HDateField::str2str(QString s,bool unknownAllowed)
{
    if(s.isEmpty())
    {
        if(unknownAllowed)
            return s;
        else
            return fallbackDateValue;
    }
    QRegularExpression r("^\\d\\d\\d\\d-\\d\\d-\\d\\d$");
    QRegularExpressionMatch m = r.match(s);
    if(m.hasMatch())
        return s;

    if(unknownAllowed)
        return QString("");

    return fallbackDateValue;
}

QDate HDateField::str2val(QString s,bool unknownAllowed)
{
    if(s.isEmpty())
    {
        if(unknownAllowed)
            return QDate();
        else
            return QDate::fromString(fallbackDateValue,Qt::ISODate);
    }

    return QDate::fromString(str2str(s,unknownAllowed),Qt::ISODate);
}

QString HDateField::val2str(QDate d,bool unknownAllowed)
{
    if(d.isNull())
    {
        if(unknownAllowed)
            return QString("");
        else
            return fallbackDateValue;
    }
    return d.toString(Qt::ISODate);
}

QString HDateField::defaultValueCalc(QString def)
{
    if(def.toLower() == "now")
        return QDate::currentDate().toString(Qt::ISODate);
    return def;
}

void HDateField::putsOnSetter(HSqlBuilder *b,QString tableName)
{
    Q_UNUSED(tableName)
    if(fNoSql)
        return;
    if(unknownAllowed() && strValue().isEmpty())
        b->set_fe(derivedSqlFieldName(),"<<<datetype_null>>>","dialected_element=yes");
    else
        b->set_fv(derivedSqlFieldName(),fvType(),QVariant(strValue()),genBuilderOptions(SqlBuilderGet));
}

HTimestampField::HTimestampField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "varchar";
    defaultCreateTypes["pgsql"]  = "timestamp";
    defaultCreateTypes["mysql"]  = "datetime";
    setAttribute("autoupdate","none");
}

HTimestampField::~HTimestampField(void)
{
}

QString HTimestampField::className(void)
{
    return "HTimestampField";
}

HSqlBuilder_FieldValueType HTimestampField::fvType(void)
{
    return Quoted;
}

bool HTimestampField::isUpdateRequired(bool forRealUpdate)
{
    if(attribute("autoupdate") == "insert" && status() == HFieldStatus_Database)
        return false;

    if(!forRealUpdate)
        return HField::isUpdateRequired();

    if(attribute("autoupdate") == "update" || attribute("autoupdate") == "always")
        return true;

    return HField::isUpdateRequired();
}

void HTimestampField::putsOnSetter(HSqlBuilder *b,QString tableName)
{
    Q_UNUSED(tableName)
    if(fNoSql)
        return;
    if(attribute("autoupdate") == "none")
    {
        if(strValue().isEmpty())
            b->set_fe(derivedSqlFieldName(),"<<<datetype_null>>>","dialected_element=yes");
        else
            b->set_fv(derivedSqlFieldName(),fvType(),QVariant(strValue()),genBuilderOptions(SqlBuilderSet));
    }
    else
        b->set_fe(derivedSqlFieldName(),"<<<current_timestamp>>>","dialected_element=yes");
}

void HTimestampField::setStrValue_NoStatusChange(QString value)
{
    QString v_set = str2str(value);
    if(!v_set.isEmpty())
        v = v_set;
}

QString HTimestampField::str2str(QString s)
{
    QRegularExpression r("^\\d\\d\\d\\d-\\d\\d-\\d\\d[\\sT]\\d\\d:\\d\\d:\\d\\d(\\.\\d+)?$");
    QRegularExpressionMatch m = r.match(s);
    if(m.hasMatch())
        return s;
    return "";
}

QDateTime HTimestampField::str2val(QString s)
{
    return QDateTime::fromString(s,Qt::ISODateWithMs);
}

QString HTimestampField::val2str(QDateTime dt)
{
    return dt.toString(Qt::ISODate);
}

QDateTime HTimestampField::value()
{
    return str2val(strValue());
}

QString HTimestampField::convertToDisplay(QString fv)
{
    return fv.replace("T"," ").mid(0,19);
}

HSqlXChooseField::HSqlXChooseField(QString sqlname,QString description,QString title)
 : HField(sqlname,description,title)
{
    unkAllowed = false;
    read = false;
    setAttribute("popuplist_heads","");
}

HSqlXChooseField::~HSqlXChooseField(void)
{
}

void HSqlXChooseField::setUnknownAllowed_X(bool ua,QString setDefaultStrValue)
{
    unkAllowed = ua;
    if(!setDefaultStrValue.isEmpty())
        setStrDefault(setDefaultStrValue);

    if(!unkAllowed && strValue().isEmpty())
        setStrValue(def);
}

bool HSqlXChooseField::unknownAllowed_X()
{
    return unkAllowed;
}

void HSqlXChooseField::setKVSource_X(QString tablename,QString keyfield,QString showfields,QString filter,QString sort)
{
    jTableName = tablename;
    jKeyFieldName = keyfield;
    jShowFields = showfields;
    jFilter = filter;
    jSort = sort;
    read = false;
}

void HSqlXChooseField::refreshKVs_X()
{
    if(inConfigure)
        return;

    if(jKeyFieldName.isEmpty() || jShowFields.isEmpty())
        return;

    HSql sql = getSql();

    internal_clearKVs();

    HSqlBuilder b = db_query(jTableName);
    b.get(jKeyFieldName,"KEY_OF_SQLCHOOSE");
    QStringList fds = jShowFields.split(";");
    int fi,fc = fds.count();
    for(fi = 0 ; fi < fc ; fi++)
    {
        if(!fds.at(fi).startsWith("$"))
            b.get(fds.at(fi),QString("SHOW_%1").arg(fi + 1,4,10,QChar('0')));
    }

    if(!jFilter.isEmpty())
    {
        QStringList condparts = jFilter.split("#");
        int ci,cc = condparts.count();
        HSqlBuilderCondition topc(And);
        QList<HSqlBuilderCondition> subconds;
        HSqlBuilderCondition *c = &topc;
        for(ci = 0 ; ci < cc ; ci++)
        {
            QString p = condparts.at(ci).trimmed();
            if(p.toLower() == "(and")
            {
                subconds.push_back(HSqlBuilderCondition(And));
                c = &(subconds.last());
                continue;
            }
            if(p.toLower() == "(or")
            {
                subconds.push_back(HSqlBuilderCondition(Or));
                c = &(subconds.last());
                continue;
            }
            if(p.toLower() == ")")
            {
                HSqlBuilderCondition toDel = subconds.last();
                subconds.removeLast();
                if(subconds.isEmpty())
                {
                    topc.add(toDel);
                    c = &topc;
                }
                else
                {
                    c = &(subconds.last());
                    c->add(toDel);
                }
                continue;
            }

            QStringList subparts = p.split(",");
            QString p1 = "",p2 = "",p3 = "",p4 = "";

            if(subparts.count() > 1) p1 = subparts[1];
            if(subparts.count() > 2) p2 = subparts[2];
            if(subparts.count() > 3) p3 = subparts[3];
            if(subparts.count() > 4) p4 = subparts[4];

            if(subparts[0].toLower() == "ff") c->ff(p1,p2,p3,p4);
            if(subparts[0].toLower() == "fq") c->fv(p1,Quoted,p2,p3,p4);
            if(subparts[0].toLower() == "fu") c->fv(p1,Unquoted,p2,p3,p4);
            if(subparts[0].toLower() == "fe") c->fe(p1,p2,p3,p4);
            if(subparts[0].toLower() == "fb") c->fb(p1,p2);
        }
        b.cond(topc);
    }

    QStringList sds = jSort.split(";");
    int si,sc = sds.count();
    for(si = 0 ; si < sc ; si++)
    {
        if(!sds.at(si).isEmpty())
        {
            QStringList sortparts = sds.at(si).split(",");
            if(sortparts.count() == 1)
                b.sort(sortparts[0]);
            if(sortparts.count() == 2)
                b.sort(sortparts[0],sortparts[1]);
        }
    }

    QStringList lstHeads;

    auto res = sql.execMulti(b,"Error getting key-value pairs for HSqlSChooseField");
    if(!res->errorStatus())
    {
        int idx = 0;
        while(res->nextRecord())
        {
            bool was_not_hc = true;
            QString key,val;
            key = res->value("KEY_OF_SQLCHOOSE").toString();
            val = "";
            for(fi = 0 ; fi < fc ; fi++)
            {
                if(fds.at(fi).startsWith("$"))
                {
                    if(fds.at(fi).startsWith("$TXT:"))
                        val.append(fds.at(fi).mid(5));
                    if(fds.at(fi).startsWith("$H:") || fds.at(fi).startsWith("$C:"))
                    {
                        if(idx == 0)
                            lstHeads.push_back(fds.at(fi).mid(3));

                        if(was_not_hc)
                            was_not_hc = false;
                        else
                            val.append(";");
                    }
                    if(fds.at(fi) == "$IDX")
                        val.append(QString("%1").arg(idx + 1));
                    if(fds.at(fi) == "$S")
                        val.append(" ");
                    if(fds.at(fi) == "$T")
                        val.append("\t");
                }
                else
                {
                    val.append(res->value(QString("SHOW_%1").arg(fi + 1,4,10,QChar('0'))).toString());
                }
            }

            internal_addKV(key,val);
            idx++;
        }
        setAttribute("popuplist_heads",lstHeads.join(";"));
        read = true;
    }
}

void HSqlXChooseField::internal_clearKVs()
{

}

void HSqlXChooseField::internal_addKV(QString k,QString v)
{
    Q_UNUSED(k)
    Q_UNUSED(v)
}

QString HSqlXChooseField::connectedTableName()
{
    return jTableName;
}


int HSqlXChooseField::customUserEventDispatcher(QString what,QString ckey)
{
    emit customUserEvent(what,ckey);
    return 0;
}

HSqlSChooseField::HSqlSChooseField(QString sqlname,QString description,QString title)
 : HSqlXChooseField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "varchar(16)";
    defaultCreateTypes["pgsql"]  = "varchar(16)";
    defaultCreateTypes["mysql"]  = "varchar(16)";
}

HSqlSChooseField::~HSqlSChooseField(void)
{
}

QString HSqlSChooseField::className(void)
{
    return "HSqlSChooseField";
}

HSqlBuilder_FieldValueType HSqlSChooseField::fvType(void)
{
    return Quoted;
}

HSqlSChooseField* HSqlSChooseField::setUnknownAllowed(bool ua,QString setDefaultStrValue)
{
    setUnknownAllowed_X(ua,setDefaultStrValue);
    return this;
}

bool HSqlSChooseField::unknownAllowed()
{
    return unknownAllowed_X();
}

HSqlSChooseField* HSqlSChooseField::setKVSource(QString tablename,QString keyfield,QString showfields,QString filter,QString sort)
{
    setKVSource_X(tablename,keyfield,showfields,filter,sort);
    return this;
}

HSqlSChooseField* HSqlSChooseField::refreshKVs()
{
    refreshKVs_X();
    QString newVal = str2str(v,unknownAllowed(),selectableKeys(),def);
    emit featureChanged();
    if(newVal != v)
        setStrValue(newVal);
    return this;
}

void HSqlSChooseField::initialize(void)
{
    HField::initialize();
    refreshKVs();
}

void HSqlSChooseField::internal_clearKVs()
{
    selectables.clear();
}

void HSqlSChooseField::internal_addKV(QString k,QString v)
{
    selectables.addItem(k,v);
}

void HSqlSChooseField::setStrValue_NoStatusChange(QString value)
{
    v = str2str(value,unknownAllowed(),selectableKeys(),def);
}

QString HSqlSChooseField::str2str(QString s,bool unknownAllowed,QList<QString> selectableKeys,QString def)
{
    if(selectableKeys.contains(s))
        return s;
    if(s.isEmpty() && unknownAllowed)
        return s;
    if(selectableKeys.contains(def))
        return def;
    if(selectableKeys.size() > 0)
        return selectableKeys.at(0);
    return "";
}

QString HSqlSChooseField::convertToDisplay(QString fv)
{
    if(unknownAllowed() && fv.isEmpty())
        return " - ";
    return selectables.value(fv).replace(";"," ");
}

HSqlSChooseField* HSqlSChooseField::setAttributeRc(QString name,QString value)
{
    setAttribute(name,value);
    return this;
}

void HSqlSChooseField::refreshRelatedDatabaseData()
{
    refreshKVs();
}

HSqlNChooseField::HSqlNChooseField(QString sqlname,QString description,QString title)
 : HSqlXChooseField(sqlname,description,title)
{
    defaultCreateTypes["sqlite"] = "integer";
    defaultCreateTypes["pgsql"]  = "numeric(8)";
    defaultCreateTypes["mysql"]  = "numeric(8)";
}

HSqlNChooseField::~HSqlNChooseField(void)
{
}

QString HSqlNChooseField::className(void)
{
    return "HSqlNChooseField";
}

HSqlBuilder_FieldValueType HSqlNChooseField::fvType(void)
{
    return Unquoted;
}

HSqlNChooseField* HSqlNChooseField::setUnknownAllowed(bool ua,QString setDefaultStrValue)
{
    setUnknownAllowed_X(ua,setDefaultStrValue);
    return this;
}

bool HSqlNChooseField::unknownAllowed()
{
    return unknownAllowed_X();
}

void HSqlNChooseField::putsOnSetter(HSqlBuilder *b,QString tableName)
{
    Q_UNUSED(tableName)
    if(fNoSql)
        return;
    if(unknownAllowed() && strValue().isEmpty())
        b->set_fe(derivedSqlFieldName(),"<<<numeric_null>>>","dialected_element=yes");
    else
        b->set_fv(derivedSqlFieldName(),fvType(),QVariant(strValue()),genBuilderOptions(SqlBuilderGet));
}

HSqlNChooseField* HSqlNChooseField::setKVSource(QString tablename,QString keyfield,QString showfields,QString filter,QString sort)
{
    setKVSource_X(tablename,keyfield,showfields,filter,sort);
    return this;
}

HSqlNChooseField* HSqlNChooseField::refreshKVs()
{
    refreshKVs_X();
    QString newVal = str2str(v,unknownAllowed(),selectableKeys(),def);
    emit featureChanged();
    if(newVal != v)
        setStrValue(newVal);
    return this;
}

void HSqlNChooseField::initialize(void)
{
    HField::initialize();
    refreshKVs();
}

void HSqlNChooseField::internal_clearKVs()
{
    selectables.clear();
}

void HSqlNChooseField::internal_addKV(QString k,QString v)
{
    selectables.addItem(k.toInt(),v);
}

void HSqlNChooseField::setStrValue_NoStatusChange(QString value)
{
    v = str2str(value,unknownAllowed(),selectableKeys(),def);
}

QString HSqlNChooseField::str2str(QString s,bool unknownAllowed,QList<int> selectableKeys,QString def)
{
    int siv = s.toInt();

    if(siv >= 0 && selectableKeys.contains(siv))
        return s;
    if(s.isEmpty() && unknownAllowed)
        return s;

    int div = def.toInt();
    if(div >= 0 && selectableKeys.contains(div))
        return def;

    if(selectableKeys.size() > 0)
        return QString("%1").arg(selectableKeys.at(0));
    return "0";
}

QString HSqlNChooseField::convertToDisplay(QString fv)
{
    if(unknownAllowed() && fv.isEmpty())
        return " - ";
    return selectables.value((int)(fv.toInt())).replace(";"," ");
}

HSqlNChooseField* HSqlNChooseField::setAttributeRc(QString name,QString value)
{
    setAttribute(name,value);
    return this;
}

void HSqlNChooseField::refreshRelatedDatabaseData()
{
    refreshKVs();
}

//End of gSAFE ftypes.cpp
