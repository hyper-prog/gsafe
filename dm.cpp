/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2024 Péter Deák (hyper80@gmail.com)

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
#include "ra.h"
#include "dmext.h"
#include "dconsole.h"
#include "builder.h"

QString statusToString(HField_Status s)
{
    switch(s)
    {
        case HFieldStatus_Init:         return "Init";
        case HFieldStatus_Default:      return "Default";
        case HFieldStatus_Database:     return "Database";
        case HFieldStatus_GuiChanged:   return "GuiChanged";
        case HFieldStatus_Changed:      return "Changed";
    }
    return "Unknown";
}

QString getOptionValue(QString options,QString name,QString defval)
{
    int oi,oc;
    QList<QString> opts = options.split(";");
    oc = opts.count();
    QString searchfor = QString("%1=").arg(name);
    for(oi = 0 ; oi < oc ; ++oi)
        if(opts.at(oi).startsWith(searchfor))
            return opts.at(oi).mid(searchfor.length());
    return defval;
}

// //////////////////////////////////////////////////////////////////////////////////////// //
// HTableSkel
// //////////////////////////////////////////////////////////////////////////////////////// //

HTableSkel::HTableSkel(QString tableName,QString title)
{
    tblName = tableName;
    tblTitle = title;
}

HTableSkel::~HTableSkel()
{
}

QString HTableSkel::className()
{
    return "HTableSkel";
}

QString HTableSkel::tableName()
{
    return tblName;
}

HTableSkel* HTableSkel::setTableName(QString name)
{
    tblName = name;
    return this;
}

QString HTableSkel::tableTitle()
{
    return tblTitle;
}

HTableSkel* HTableSkel::setTableTitle(QString title)
{
    tblTitle = title;
    return this;
}

// //////////////////////////////////////////////////////////////////////////////////////// //
// HRecordSkel
// //////////////////////////////////////////////////////////////////////////////////////// //

HRecordSkel::HRecordSkel(QString tableName,QString title)
 : HTableSkel(tableName,title)
{
}

HRecordSkel::~HRecordSkel()
{
    clearFields();
}

QString HRecordSkel::className()
{
    return "HRecordSkel";
}

HRecordSkel* HRecordSkel::addField(HField *nf)
{
    fields.push_back(nf);
    nf->db(currentDb());
    connect(this,SIGNAL(databaseChanged(QString)),nf,SLOT(changeDatabaseTo(QString)));
    connect(nf,SIGNAL(dataChanged()),SIGNAL(dataChanged()));
    nf->initialize();
    return this;
}

void HRecordSkel::clearFields()
{
    QList<HField *>::iterator fi;
    fi = fields.begin();
    while(fi != fields.end())
    {
        delete (*fi);
        fi++;
    }
}

int HRecordSkel::fieldCount()
{
    return fields.count();
}

HField* HRecordSkel::fieldByIndex(int index)
{
    return fields[index];
}

HField* HRecordSkel::fieldByName(QString sqlname)
{
    int idx = fieldIndexByName(sqlname);
    if(idx < 0)
        return NULL;
    return fields[idx];
}

int HRecordSkel::fieldIndexByName(QString sqlname)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->sqlName() == sqlname)
            return i;
    return -1;
}

int HRecordSkel::keyIndex()
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->isKey())
            return i;
    return -1;
}

QString HRecordSkel::keySqlName()
{
    int i = keyIndex();
    if(i >= 0)
        return fields[i]->sqlName();
    return QString();
}

HSqlBuilder_FieldValueType HRecordSkel::keyFvType()
{
    int i = keyIndex();
    if(i >= 0)
        return fields[i]->fvType();
    return BQInvalid;
}

HRecordSkel& HRecordSkel::db(QString databaseName)
{
    HSqlRelated::db(databaseName);
    emit databaseChanged(databaseName);
    return *this;
}

void HRecordSkel::putsOnGetter(HSqlBuilder *b)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        fields[i]->putsOnGetter(b,tblName);
}

QString HRecordSkel::generateString(int verbose)
{
    QString o = "";
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
    {
        o.append(QString("%1 ").arg(i));

        if(verbose > 1)
            o.append(QString("%1:")
                     .arg(fields[i]->className()));

        o.append(QString("%1")
                     .arg(fields[i]->sqlName()));
        o.append("\n");
    }
    return o;
}

HRecord * HRecordSkel::toHRecord()
{
    if(className() == "HRecord")
        return (HRecord *)this;
    return NULL;
}

HRecordLines * HRecordSkel::toHRecordLines()
{
    if(className() == "HRecordLines")
        return (HRecordLines *)this;
    return NULL;
}

void HRecordSkel::tagSetFieldEditType(QString t,HFieldEditType feType)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setFieldEditType(feType);
}

void HRecordSkel::tagSetNoSql(QString t,bool ns)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setNoSql(ns);
}

void HRecordSkel::tagSetDisplayFlags(QString t,HDispObjectFlags flags)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setDisplayFlags(flags);
}

void HRecordSkel::tagSetAttribute(QString t,QString name,QString value)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setAttribute(name,value);
}

void HRecordSkel::tagSetAttribute(QString t,QString name,int ivalue)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setAttribute(name,ivalue);
}

void HRecordSkel::tagSetAttribute(QString t,QString name,double dvalue)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setAttribute(name,dvalue);
}

void HRecordSkel::tagSetColor(QString t,int r,int g,int b)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setColor(r,g,b);
}

// //////////////////////////////////////////////////////////////////////////////////////// //
// HField
// //////////////////////////////////////////////////////////////////////////////////////// //

HField::HField(QString sqlname,QString description,QString title)
{
    inConfigure = true;
    fDatabaseAndMemoryRepDiffers = false;
    fSqlName = sqlname;
    fDescription = description;
    fTitle = title;
    fNoSql = false;
    v = "";
    attributeMap.clear();
    sts = HFieldStatus_Init;
    fet = HFieldEdit_DefaultEditable;
    dispFlags = HDispFlag_Default;
    defaultCreateTypes.clear();
    vValidators.clear();
    lastValidatorCheckFailed = false;
}

HField::~HField()
{
    clearValidators();
}

QString HField::className()
{
    return "HField";
}

QString HField::sqlName()
{
    return fSqlName;
}

HField_Status HField::status()
{
    return sts;
}

void HField::statusToDatabaseAfterStore()
{
    sts = HFieldStatus_Database;
}

void HField::clearValidators()
{
    QList<HBaseValidator *>::iterator vi;
    vi = vValidators.begin();
    while(vi != vValidators.end())
    {
        delete (*vi);
        vi++;
    }
}

QString HField::alias()
{
    return attribute("sql_alias");
}

QString HField::derivedSqlFieldName()
{
    if(attribute("override_sql_name").isEmpty())
        return fSqlName;
    return attribute("override_sql_name");
}

QString HField::derivedAlias()
{
    if(attribute("sql_alias").isEmpty())
        return fSqlName;
    return attribute("sql_alias");
}

HField* HField::setOverrideSqlName(QString oName)
{
    setAttribute("override_sql_name",oName);
    return this;
}

HField* HField::setAlias(QString alias)
{
    setAttribute("sql_alias",alias);
    return this;
}

HSqlBuilder_FieldValueType HField::fvType()
{
    return BQInvalid;
}

QString HField::title()
{
    return fTitle;
}

QString HField::description()
{
    return fDescription;
}

HField* HField::setTitle(QString title)
{
    fTitle = title;
    emit featureChanged();
    return this;
}

HField* HField::setDescription(QString descr)
{
    fDescription = descr;
    emit featureChanged();
    return this;
}


HField* HField::setFieldEditType(HFieldEditType feType)
{
    fet = feType;
    emit featureChanged();
    return this;
}

HFieldEditType HField::fieldEditType()
{
    return fet;
}

HField* HField::setHidden()
{
    fet = HFieldEdit_Invisible;
    return this;
}

HField* HField::setVisibleEditable()
{
    fet = HFieldEdit_DefaultEditable;
    emit featureChanged();
    return this;
}

bool HField::isVisible()
{
    return (fet != HFieldEdit_Invisible);
}

HField* HField::setVisibleReadonly()
{
    fet = HFieldEdit_Readonly;
    emit featureChanged();
    return this;
}

HField* HField::setShowOnlyReadonly()
{
    fet = HFieldEdit_ShowReadonly;
    return this;
}

bool HField::isEditable()
{
    return (fet == HFieldEdit_DefaultEditable);
}

HField* HField::setColor(int r,int g,int b)
{
    setAttribute("color",QString::asprintf("%2x%2x%2x",r,g,b));
    emit featureChanged();
    return this;
}

HField* HField::setNoSql(bool ns)
{
    fNoSql = ns;
    return this;
}

bool HField::noSql()
{
    return fNoSql;
}

HField* HField::setSqlCreateType(QString sct)
{
    setAttribute("override_sqlcreatetype",sct);
    return this;
}

QString HField::sqlCreateType(QString options)
{
    if(!attribute("override_sqlcreatetype").isEmpty())
        return attribute("override_sqlcreatetype");

    QString forsql = getOptionValue(options,"forsql","sqlite");
    if(defaultCreateTypes.contains(forsql))
        return defaultCreateTypes[forsql];
    return "";
}

QString HField::attribute(QString name)
{
    if(attributeMap.contains(name))
        return attributeMap[name];
    return "";
}

QStringList HField::allDefinedAttributes()
{
    return attributeMap.keys();
}

HField* HField::setAttribute(QString name,QString value)
{
    attributeMap[name] = value;
    emit featureChanged();
    return this;
}

HField* HField::setAttribute(QString name,int ivalue)
{
    attributeMap[name] = QString::asprintf("%d",ivalue);
    return this;
}

HField* HField::setAttribute(QString name,double dvalue)
{
    attributeMap[name] = QString::asprintf("%f",dvalue);
    return this;
}

HField* HField::setDisplayFlags(HDispObjectFlags flags)
{
    dispFlags = flags;
    return this;
}

HDispObjectFlags HField::displayFlags()
{
    return dispFlags;
}

bool HField::isKey()
{
    return false;
}

HField* HField::setBeforeText(QString bText)
{
    setAttribute("txt_before",bText);
    emit featureChanged();
    return this;
}

HField* HField::setAfterText(QString aText)
{
    setAttribute("txt_after",aText);
    emit featureChanged();
    return this;
}

QString HField::beforeText()
{
    return attribute("txt_before");
}

QString HField::afterText()
{
    return attribute("txt_after");
}

void HField::clearTags()
{
    mytags.clear();
}

HField* HField::addTag(QString t)
{
    mytags.push_back(t);
    return this;
}

HField* HField::addTags(QString tags,QString selectBy)
{
    QStringList tl = tags.split(selectBy);
    mytags.append(tl);
    return this;
}

bool HField::hasTag(QString t)
{
    if(t == "*")
        return true;
    if(className() == t)
        return true;
    if(mytags.contains(t))
        return true;
    return false;
}

const QList<QString> HField::tags()
{
    return mytags;
}

QString HField::convertToDisplay(QString fv)
{
    return fv;
}

QString HField::displayValue()
{
    return convertToDisplay(strValue());
}

HField *HField::addValidator(HBaseValidator* v)
{
    v->setConnectedHField(this);
    vValidators.push_back(v);
    return this;
}

const QList<HBaseValidator *> HField::validators()
{
    return vValidators;
}

QString HField::validate()
{
    int vNr = 1;
    QString vtxt = "";
    lastValidatorCheckFailed = false;
    QList<HBaseValidator *>::iterator vi;
    vi = vValidators.begin();
    while(vi != vValidators.end())
    {
        QString vres = (*vi)->validate(strValue());
        if(!vres.isEmpty())
        {
            lastValidatorCheckFailed = true;
            vtxt.append(QString("%1 - %2: %3\n").arg(title()).arg(vNr).arg(vres));
            vNr++;
        }
        vi++;
    }
    return vtxt;
}

void HField::putsOnSetter(HSqlBuilder *b,QString tableName)
{
    Q_UNUSED(tableName)
    if(fNoSql)
        return;
    b->set_fv(derivedSqlFieldName(),fvType(),QVariant(strValue()),genBuilderOptions(SqlBuilderSet));
}

void HField::putsOnGetter(HSqlBuilder *b,QString tableName)
{
    if(fNoSql)
        return;
    if(tableName.isEmpty())
        b->get(derivedSqlFieldName(),derivedAlias(),genBuilderOptions(SqlBuilderGet));
    else
        b->get_withtable(tableName,derivedSqlFieldName(),derivedAlias(),genBuilderOptions(SqlBuilderGet));
}

QString HField::genBuilderOptions(HField_GenOptsForWhat forWhat)
{
    QString opts = "";
    if(!attribute("get_sql_function").isEmpty() && forWhat == SqlBuilderGet)
    {
        if(!opts.isEmpty())
            opts.append(";");
        opts.append(QString("function=%1").arg(attribute("get_sql_function")));
    }
    if(!attribute("set_sql_function").isEmpty() && forWhat == SqlBuilderSet)
    {
        if(!opts.isEmpty())
            opts.append(";");
        opts.append(QString("function=%1").arg(attribute("set_sql_function")));
    }
    if(!attribute("sql_function").isEmpty())
    {
        if(!opts.isEmpty())
            opts.append(";");
        opts.append(QString("function=%1").arg(attribute("sql_function")));
    }
    return opts;
}

int HField::changeDatabaseTo(QString toName)
{
    db(toName);
    return 0;
}

HField::operator const QString()
{
    return strValue();
}

QString HField::strValue()
{
    return v;
}

void HField::setStrValue_NoStatusChange(QString value)
{
    v = value;
}

HField* HField::setStrValue(QString value)
{
    setStrValue_NoStatusChange(value);
    sts = HFieldStatus_Changed;
    emit dataChanged();
    return this;
}

void HField::setStrValue_Database(QString value)
{
    setStrValue_NoStatusChange(value);
    sts = HFieldStatus_Database;
    if(!fDatabaseAndMemoryRepDiffers && value != v)
        sts = HFieldStatus_Changed;
    emit dataChanged();
}

void HField::setStrValue_Gui(QString value)
{
    setStrValue_NoStatusChange(value);
    sts = HFieldStatus_GuiChanged;
    emit dataChanged();
}

HField& HField::operator=(QString val)
{
    setStrValue(val);
    return *this;
}

QString HField::strDefault()
{
    return def;
}

HField* HField::setStrDefault(QString value,bool setValue)
{
    def = value;
    if(setValue)
        restoreDefaultValue();
    return this;
}

void HField::restoreDefaultValue()
{
    setStrValue(defaultValueCalc(def));
    sts = HFieldStatus_Default;
}

bool HField::isUpdateRequired(bool forRealUpdate)
{
    Q_UNUSED(forRealUpdate)

    if(isKey())
        return false;
    if(sts != HFieldStatus_Database)
        return true;
    return false;
}

QString HField::defaultValueCalc(QString def)
{
    return def;
}

void HField::initialize(void)
{
    inConfigure = false;
    restoreDefaultValue();
}

void HField::refreshRelatedDatabaseData()
{

}

bool HField::getLastValidatorCheckStatus()
{
    return lastValidatorCheckFailed;
}

void HField::setLastValidatorCheckToFail()
{
    lastValidatorCheckFailed = true;
}

void HField::resetLastValidatorStatus()
{
    lastValidatorCheckFailed = false;
}

// //////////////////////////////////////////////////////////////////////////////////////// //
// HKeyValueStore
// //////////////////////////////////////////////////////////////////////////////////////// //

template class HKeyValueStore<QString>;
template class HKeyValueStore<int>;

template <typename T>
HKeyValueStore<T>::HKeyValueStore()
{
    clear();
}

template <typename T>
HKeyValueStore<T>::~HKeyValueStore()
{

}

template <typename T>
void HKeyValueStore<T>::clear()
{
    vKeys.clear();
    vValues.clear();
    uKeys.clear();
    uValues.clear();
}

template <typename T>
void HKeyValueStore<T>::addItem(T key,QString val)
{
    vKeys.push_back(key);
    vValues.push_back(val);
}

template <typename T>
void HKeyValueStore<T>::setItems(QList<T>& keys,QList<QString>& values)
{
    vKeys.clear();
    vValues.clear();
    if(keys.count() == values.count())
    {
        vKeys = keys;
        vValues = values;
    }
}

template <typename T>
void HKeyValueStore<T>::addUnlistedItem(T key,QString val)
{
    uKeys.push_back(key);
    uValues.push_back(val);
}

template <typename T>
void HKeyValueStore<T>::setUnlistedItems(QList<T>& keys,QList<QString>& values)
{
    uKeys.clear();
    uValues.clear();
    if(keys.count() == values.count())
    {
        uKeys = keys;
        uValues = values;
    }
}

template <typename T>
const QList<T>& HKeyValueStore<T>::keys()
{
    return vKeys;
}

template <typename T>
const QList<QString>& HKeyValueStore<T>::values()
{
    return vValues;
}

template <typename T>
const QList<T>& HKeyValueStore<T>::unlistedKeys()
{
    return uKeys;
}

template <typename T>
const QList<QString>& HKeyValueStore<T>::unlistedValues()
{
    return uValues;
}

template <typename T>
bool HKeyValueStore<T>::hasKey(T key,bool unlistedIncluded)
{
    if(vKeys.contains(key))
        return true;
    if(unlistedIncluded && uKeys.contains(key))
        return true;
    return false;
}

template <typename T>
QString HKeyValueStore<T>::value(T key,bool unlistedIncluded)
{
    int idx;
    idx = vKeys.indexOf(key);
    if(idx >= 0)
        return vValues[idx];
    if(!unlistedIncluded)
        return "";
    idx = uKeys.indexOf(key);
    if(idx >= 0)
        return uValues[idx];
    return "";
}

template <typename T>
T HKeyValueStore<T>::keyByIdx(int index)
{
    return vKeys[index];
}

template <typename T>
QString HKeyValueStore<T>::valueByIdx(int index)
{
    return vValues[index];
}

template <typename T>
int HKeyValueStore<T>::indexOfKey(T key)
{
    return vKeys.indexOf(key);
}

template <typename T>
int HKeyValueStore<T>::indexOfValue(QString val)
{
    return vValues.indexOf(val);
}

// //////////////////////////////////////////////////////////////////////////////////////// //
// HFieldWithValueStore
// //////////////////////////////////////////////////////////////////////////////////////// //

template class HFieldWithValueStore<QString>;
template class HFieldWithValueStore<int>;

template <typename T>
HFieldWithValueStore<T>::HFieldWithValueStore()
{

}

template <typename T>
HFieldWithValueStore<T>::~HFieldWithValueStore()
{

}

template <typename T>
const QList<T> HFieldWithValueStore<T>::selectableKeys()
{
    return selectables.keys();
}

template <typename T>
const QList<QString> HFieldWithValueStore<T>::selectableValues()
{
    return selectables.values();
}

template <typename T>
T HFieldWithValueStore<T>::selectableKey(int index)
{
    return selectables.keyByIdx(index);
}

template <typename T>
QString HFieldWithValueStore<T>::selactableValue(int index)
{
    return selectables.valueByIdx(index);
}

template <typename T>
int HFieldWithValueStore<T>::indexOfSelectableKey(T key)
{
    return selectables.indexOfKey(key);
}

template <typename T>
int HFieldWithValueStore<T>::indexOfSelectableValue(QString value)
{
    return selectables.indexOfValue(value);
}

// //////////////////////////////////////////////////////////////////////////////////////// //
// HRecord
// //////////////////////////////////////////////////////////////////////////////////////// //

HRecord::HRecord(QString tableName,QString title)
    : HRecordSkel(tableName,title)
{

}

HRecord::~HRecord()
{

}

QString HRecord::className()
{
    return "HRecord";
}

HField& HRecord::operator[](int index)
{
    return *(fields[index]);
}

HField& HRecord::operator[](QString sqlname)
{
    return *(fields[fieldIndexByName(sqlname)]);
}

QString HRecord::keyStrValue()
{
    int i = keyIndex();
    if(i >= 0)
        return fields[i]->strValue();
    return QString();
}

QString HRecord::strValue(QString sqlname)
{
    return fields[fieldIndexByName(sqlname)]->strValue();
}

QString HRecord::strValue(int index)
{
    return fields[index]->strValue();
}

HRecord* HRecord::setStrValue(QString sqlname,QString value)
{
    fields[fieldIndexByName(sqlname)]->setStrValue(value);
    return this;
}

HRecord* HRecord::setStrValue(int index,QString value)
{
    fields[index]->setStrValue(value);
    return this;
}

void HRecord::putsOnSetter(HSqlBuilder *b)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->isUpdateRequired(true))
            fields[i]->putsOnSetter(b,tblName);
}

QMap<QString,QString> HRecord::getValueMap(bool sqlonly)
{
    QMap<QString,QString> vm;
    vm.clear();
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(!fields[i]->noSql() || !sqlonly)
            vm[fields[i]->sqlName()] = fields[i]->strValue();
    return vm;
}

QMap<QString,QString> HRecord::getDisplayValueMap(bool sqlonly)
{
    QMap<QString,QString> vm;
    vm.clear();
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(!fields[i]->noSql() || !sqlonly)
            vm[fields[i]->sqlName()] = fields[i]->displayValue();
    return vm;
}

int HRecord::returnToDefault(SqlOperationFlags flags)
{
    Q_UNUSED(flags)

    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        fields[i]->restoreDefaultValue();
    emit dataChanged();
    return 0;
}

int HRecord::readWithKey(QString keyValue,SqlOperationFlags flags)
{
    bool tr_disabled = false;

    if(flagOn(flags,SqlOpFlag_TransactionDisabled))
        tr_disabled = true;

    HSql sql = getSql();
    HSqlBuilder q(Select,tblName);
    putsOnGetter(&q);
    q.cond_fv(keySqlName(),keyFvType(),keyValue,"=");

    auto result = sql.execMulti(q,"Error HRecord::readWithKey",tr_disabled);
    if(result->nextRecord())
    {
        int i,fc = fieldCount();
        for(i = 0 ; i < fc ; ++i )
            if(!fields[i]->noSql())
                fields[i]->setStrValue_Database( result->value(fields[i]->derivedAlias()).toString() );
    }
    emit dataChanged();
    return 0;
}

int HRecord::reloadTimestamps(bool tr_disabled)
{
    HSql sql = getSql();
    HSqlBuilder q(Select,tblName);
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->className() == "HTimestampField" && !fields[i]->noSql())
            fields[i]->putsOnGetter(&q);
    q.cond_fv(keySqlName(),keyFvType(),keyStrValue(),"=");

    auto result = sql.execMulti(q,"Error HRecord::reloadTimestamps",tr_disabled);
    if(result->nextRecord())
    {
        for(i = 0 ; i < fc ; ++i )
            if(fields[i]->className() == "HTimestampField" && !fields[i]->noSql())
                fields[i]->setStrValue_Database( result->value(fields[i]->derivedAlias()).toString() );
    }
    emit dataChanged();
    return 0;
}

bool HRecord::isUpdateRequired(bool forRealUpdate)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(!fields[i]->noSql() && fields[i]->isUpdateRequired(forRealUpdate))
            return true;
    return false;
}

int HRecord::update(SqlOperationFlags flags)
{
    bool tr_disabled = false;

    if(flagOn(flags,SqlOpFlag_TransactionDisabled))
        tr_disabled = true;
    if(keyStrValue().isEmpty())
    {
        sdebug("HRecord::update, Disabled update: Empty key!");
        return 1;
    }
    if(!isUpdateRequired(false))
        return 0;
    if(!flagOn(flags,SqlOpFlag_ValidationOmit))
    {
        QString vtxt = validate();
        if(!vtxt.isEmpty())
        {
            emit validationFailed(vtxt);
            sdebug(QString("HRecord::update, validation error:\n%1").arg(vtxt));
            return 1;
        }
    }
    if(updateGuardAlert())
    {
        sdebug("HRecord::update, Disabled update: Update guard found modified timestamp!");
        return 1;
    }

    HSql sql = getSql();
    HSqlBuilder q(Update,tblName);
    putsOnSetter(&q);
    q.cond_fv(keySqlName(),keyFvType(),keyStrValue(),"=");

    sql.exec(q,"Error HRecord::update",tr_disabled);
    if(!sql.errorStatus() && !flagOn(flags,SqlOpFlag_LeaveStatusesUntouched))
        resetFieldStsAfterDbStore();
    if(sql.errorStatus())
        return 1;

    reloadTimestamps(tr_disabled);
    if(sql.errorStatus())
        return 1;
    return 0;
}

int HRecord::insert(SqlOperationFlags flags)
{
    bool tr_disabled = false;

    if(flagOn(flags,SqlOpFlag_TransactionDisabled))
        tr_disabled = true;
    if(!keyStrValue().isEmpty())
    {
        sdebug("HRecord::insert, Disabled insert: Key value not empty!");
        return 1;
    }

    if(!flagOn(flags,SqlOpFlag_ValidationOmit))
    {
        QString vtxt = validate();
        if(!vtxt.isEmpty())
        {
            emit validationFailed(vtxt);
            sdebug(QString("HRecord::insert, validation error:\n%1").arg(vtxt));
            return 1;
        }
    }

    HSql sql = getSql();
    HSqlBuilder q(Insert,tblName);
    putsOnSetter(&q);

    sql.exec(q,"Error HRecord::insert",tr_disabled);
    if(!sql.errorStatus() && !flagOn(flags,SqlOpFlag_LeaveStatusesUntouched))
        resetFieldStsAfterDbStore();
    if(sql.errorStatus())
        return 1;
    return 0;
}

void HRecord::resetFieldStsAfterDbStore()
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->isUpdateRequired() && !fields[i]->noSql())
            fields[i]->statusToDatabaseAfterStore();
}

bool HRecord::updateGuardAlert()
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->className() == "HTimestampField" && !fields[i]->noSql() && fields[i]->attribute("guardupdate") == "yes")
        {
            HSql sql = getSql();
            HSqlBuilder q(Select,tblName);
            fields[i]->putsOnGetter(&q);
            q.cond_fv(keySqlName(),keyFvType(),keyStrValue(),"=");
            QString dbValue = sql.execSingle(q,"HRecord::updateGuardAlert").toString();
            sdebug(QString("Double write guard: %1 ? %2").arg(fields[i]->strValue()).arg(dbValue));
            if(fields[i]->strValue() != dbValue)
                return true;
            return false;
        }
    return false;
}

int HRecord::refreshRelatedDatabaseData()
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        fields[i]->refreshRelatedDatabaseData();
    return 0;
}

void HRecord::tagSetStrDefault(QString t,QString value,bool setValue)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setStrDefault(value,setValue);
}

void HRecord::tagRestoreDefaultValue(QString t)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->restoreDefaultValue();
}

void HRecord::tagSetStrValue(QString t,QString value)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
        if(fields[i]->hasTag(t))
            fields[i]->setStrValue(value);
}

QString HRecord::validate()
{
    QString vtxt = "";
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
    {
        QString vres;
        vres = fields[i]->validate();
        if(!vres.isEmpty())
            vtxt.append(vres);
    }
    return vtxt;
}

QString HRecord::generateString(int verbose)
{
    QString o = "";
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
    {
        o.append(QString("%1 ").arg(i));

        if(verbose > 1)
            o.append(QString("%1:")
                     .arg(fields[i]->className()));

        o.append(QString("%1 -> %2")
                     .arg(fields[i]->sqlName())
                     .arg(fields[i]->strValue()));

        if(verbose > 2)
            o.append(QString(" (%1)")
                     .arg(statusToString(fields[i]->status())));
        o.append("\n");
    }
    return o;
}

// //////////////////////////////////////////////////////////////////////////////////////// //
// HRecordLines
// //////////////////////////////////////////////////////////////////////////////////////// //

HRecordLines::HRecordLines(QString tableName,QString title)
 : HRecordSkel(tableName,title), sQuery(Select,tableName)
{
    matrix = new HDataMatrix();
    matrix->setEmbedded(className());
    soft_current_key = "";
    readed_fields.clear();
    keyValues.clear();

    if(HRefreshAgent::getNotifyAgent() != NULL)
        connect(HRefreshAgent::getNotifyAgent(),SIGNAL(getnotify(QString)),this,SLOT(captureNotify(QString)));
}

HRecordLines::~HRecordLines()
{
    if(matrix != NULL)
        delete matrix;
    matrix = NULL;
}

QString HRecordLines::className()
{
    return "HRecordLines";
}

int HRecordLines::captureNotify(QString tblname)
{
    if(tblname == tblName)
        readLines();
    return 0;
}

void HRecordLines::putsOnGetter(HSqlBuilder *b)
{
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
    {
        if(!flagOn(fields[i]->displayFlags(),HDispFlag_Invisible) || fields[i]->isKey())
            fields[i]->putsOnGetter(b,tblName);
    }
}

bool HRecordLines::readLines(bool tdisabled)
{
    HSql sql = getSql();
    HSqlBuilder b(Select,tblName);

    putsOnGetter(&b);
    b.join(bJoins);
    b.cond(bCond);
    b.sort(bSort);

    sQuery = b;
    matrix->clear();
    readed_fields.clear();
    keyValues.clear();

    QStringList visibleFieldNames;
    QStringList visibleFieldDerivedSqlNames;
    int i,fc = fieldCount();
    for(i = 0 ; i < fc ; ++i )
    {
        if(!flagOn(fields[i]->displayFlags(),HDispFlag_Invisible))
        {
            visibleFieldNames.push_back(fields[i]->sqlName());
            visibleFieldDerivedSqlNames.push_back(fields[i]->derivedAlias());
        }
    }

    QString keyDerivedAlias = fieldByName(keySqlName())->derivedAlias();
    int vfc = visibleFieldNames.count();

    HSqlConnector* result = sql.execMultiUnsafe(
                                sQuery,QString("Error in HRecordLines::readLines (%1/%2)")
                                            .arg(tblName)
                                            .arg(tblTitle),
                                        tdisabled);
    if(!sql.errorStatus())
    {
        QList<HValue> recordData;

        HField **visibleFieldPointers = new HField * [ vfc ];
        for(i = 0 ; i < visibleFieldNames.count() ; ++i)
            visibleFieldPointers[i] = fieldByName(visibleFieldNames[i]);

        while(result->nextRecord())
        {
            keyValues.push_back( result->value(keyDerivedAlias).toString() );
            recordData.clear();
            for(i = 0 ; i < vfc; ++i)
                recordData.push_back( visibleFieldPointers[i]->convertToDisplay( result->value(visibleFieldDerivedSqlNames[i]) ) );
            matrix->addRow(recordData);
        }

        readed_fields = visibleFieldNames;
        setMatrixHeaders(visibleFieldNames);
        matrix->sendDataChanged();
        delete[] visibleFieldPointers;
    }
    delete result;
    return sql.errorStatus();
}

const QStringList& HRecordLines::keyValueArray()
{
    return keyValues;
}

void HRecordLines::setMatrixHeaders(QStringList fields)
{
    matrix->setTitle(tableTitle());
    int fi,fc = fields.count();
    for(fi = 0 ; fi < fc ; ++fi )
    {
        HField *f = fieldByName(fields[fi]);
        matrix->setHeaderCell(fi,f->title());
    }
}

const QStringList& HRecordLines::readedFields()
{
    return readed_fields;
}

int HRecordLines::actionOnRecordSlot(QString key)
{
    emit actionOnRecord(key);
    return 0;
}

int HRecordLines::alternateActionOnRecordSlot(QString key,QString eventText)
{
    emit alternateAactionOnRecord(key,eventText);
    return 0;
}

HDataMatrix* HRecordLines::dataMatrix()
{
    return matrix;
}

void HRecordLines::setConditions(HSqlBuilderCondition c)
{
    bCond = c;
}

void HRecordLines::setJoins(HSqlBuilderJoin j)
{
    bJoins = j;
}

void HRecordLines::setSorts(HSqlBuilderSort s)
{
    bSort = s;
}

void HRecordLines::addOnItemAction(QString actionName,QString displayText)
{
    onItemActions[actionName] = displayText;
}

const QMap<QString,QString> HRecordLines::allOnItemActions()
{
    return onItemActions;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////// //

HBaseValidator::HBaseValidator(QString failMessage)
{
    vType = "";
    fMessage = failMessage;
    attributeMap.clear();
}

HBaseValidator::~HBaseValidator()
{
}

QString HBaseValidator::validate(QString strValue)
{
    Q_UNUSED(strValue);
    return "";
}

QString HBaseValidator::attribute(QString name)
{
    if(attributeMap.contains(name))
        return attributeMap[name];
    return "";
}

QStringList HBaseValidator::allDefinedAttributes()
{
    return attributeMap.keys();
}

HBaseValidator* HBaseValidator::setAttribute(QString name,QString value)
{
    attributeMap[name] = value;
    return this;
}

HBaseValidator* HBaseValidator::setAttribute(QString name,int ivalue)
{
    attributeMap[name] = QString::asprintf("%d",ivalue);
    return this;
}

HBaseValidator* HBaseValidator::setAttribute(QString name,double dvalue)
{
    attributeMap[name] = QString::asprintf("%f",dvalue);
    return this;
}

QString HBaseValidator::validatorType()
{
    return vType;
}

QString HBaseValidator::failMessage()
{
    return fMessage;
}

void HBaseValidator::setConnectedHField(HField *c)
{
    connected = c;
}

HNotEmptyValidator::HNotEmptyValidator(QString failMessage)
: HBaseValidator(failMessage)
{
    vType = "notempty";
}

HNotEmptyValidator::~HNotEmptyValidator()
{
}

QString HNotEmptyValidator::validate(QString strValue)
{
    if(strValue.isEmpty())
        return fMessage;
    return "";
};

HRegexValidator::HRegexValidator(QString failMessage)
: HBaseValidator(failMessage)
{
    vType = "regex";
}

HRegexValidator::~HRegexValidator()
{
}

QString HRegexValidator::validate(QString strValue)
{
    QRegularExpression::MatchType mt = QRegularExpression::NormalMatch;
    QRegularExpression::PatternOptions po = QRegularExpression::NoPatternOption;

    if(attribute("unicode") == "yes")
        po |= (QRegularExpression::UseUnicodePropertiesOption);
    if(attribute("case_insesitive") == "yes")
        po |= (QRegularExpression::CaseInsensitiveOption);

    if(!attribute("valid_regex").isEmpty())
    {
        QRegularExpression rx(attribute("valid_regex"),po);
        QRegularExpressionMatch m = rx.match(strValue,0,mt);
        if(m.hasMatch())
            return "";
        else
            return fMessage;

    }
    if(!attribute("notvalid_regex").isEmpty())
    {
        QRegularExpression rx(attribute("notvalid_regex"),po);
        QRegularExpressionMatch m = rx.match(strValue,0,mt);
        if(m.hasMatch())
            return fMessage;
        else
            return "";
    }
    return "";
}

HRangeValidator::HRangeValidator(QString failMessage)
: HBaseValidator(failMessage)
{
    vType = "range";
}

HRangeValidator::~HRangeValidator()
{
}

QString HRangeValidator::validate(QString strValue)
{
    bool ok;
    double v,m;
    v = strValue.toDouble(&ok);
    if(!ok)
        return "";

    if(!attribute("minimum").isEmpty())
    {
        m = attribute("minimum").toDouble(&ok);
        if(ok && m > v)
            return fMessage;
    }
    if(!attribute("maximum").isEmpty())
    {
        m = attribute("maximum").toDouble(&ok);
        if(ok && m < v)
            return fMessage;
    }
    return "";
}

HSetValidator::HSetValidator(QString failMessage)
: HBaseValidator(failMessage)
{
    vType = "set";
}

HSetValidator::~HSetValidator()
{
}

QString HSetValidator::validate(QString strValue)
{
    QString separator = ";";
    if(!attribute("separator").isEmpty())
        separator = attribute("separator");

    if(!attribute("valid_set").isEmpty())
    {
        QStringList sset = attribute("valid_set").split(separator);
        if(sset.contains(strValue))
            return "";
        else
            return fMessage;
    }
    if(!attribute("notvalid_set").isEmpty())
    {
        QStringList sset = attribute("notvalid_set").split(separator);
        if(sset.contains(strValue))
            return fMessage;
        else
            return "";
    }
    return "";
}

// ////////////////////////////////////////////////////////////////////////////////////////////////// //

QString genereteSqlCreate(HRecordSkel *sk,QString forDatabase)
{
    bool first = true;
    int fi,fc;
    QString out;
    HField *f;

    out = QString("create table %1 (\n").arg(sk->tableName());
    fc = sk->fieldCount();
    QString opts = QString("forsql=%1").arg(forDatabase);
    for(fi = 0 ; fi < fc ; ++fi)
    {
        f = sk->fieldByIndex(fi);
        if(!f->noSql())
        {
            if(!first)
                out.append(",\n");
            out.append(QString("  %1 %2").arg(f->derivedSqlFieldName()).arg(f->sqlCreateType(opts)));
            first = false;
        }
    }
    out.append("\n);");
    return out;
}

//End of gSAFE datalib.cpp
