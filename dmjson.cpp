/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    dmjson.cpp
*/

#include <QtCore>

#include "data.h"
#include "dm.h"
#include "dialog.h"
#include "ftypes.h"
#include "dconsole.h"

HField* newHFieldObjByName(QString type,QString sqlname,QString desc,QString title)
{
    if(type.toLower() == "static")     return new HStaticField(sqlname,desc,title);
    if(type.toLower() == "nkey")       return new HNKeyField(sqlname,desc,title);
    if(type.toLower() == "skey")       return new HSKeyField(sqlname,desc,title);
    if(type.toLower() == "smalltext")  return new HSmallTextField(sqlname,desc,title);
    if(type.toLower() == "largetext")  return new HLargeTextField(sqlname,desc,title);
    if(type.toLower() == "number")     return new HNumberField(sqlname,desc,title);
    if(type.toLower() == "floating")   return new HFloatingField(sqlname,desc,title);
    if(type.toLower() == "date")       return new HDateField(sqlname,desc,title);
    if(type.toLower() == "timestamp")  return new HTimestampField(sqlname,desc,title);
    if(type.toLower() == "check")      return new HCheckField(sqlname,desc,title);
    if(type.toLower() == "txtselect")  return new HTxtSelectField(sqlname,desc,title);
    if(type.toLower() == "numselect")  return new HNumSelectField(sqlname,desc,title);
    if(type.toLower() == "sqlnchoose") return new HSqlNChooseField(sqlname,desc,title);
    if(type.toLower() == "sqlschoose") return new HSqlSChooseField(sqlname,desc,title);
    return NULL;
}

HBaseValidator* newHValidatorObjByName(QString type,QString failMessage)
{
    if(type == "notempty")  return new HNotEmptyValidator(failMessage);
    if(type == "regex")     return new HRegexValidator(failMessage);
    if(type == "range")     return new HRangeValidator(failMessage);
    if(type == "set")       return new HSetValidator(failMessage);
    return NULL;
}
// //////// HRecordSkel ///////////////////////////////////////////////////////////////

QString HRecordSkel::toJson(HJsonFlag flags)
{
    QJsonDocument jd;
    jd.setObject(toJson_inWork(flags).toObject());
    return jd.toJson(flagOn(flags,HJsonFlag_Compacted) ? QJsonDocument::Compact : QJsonDocument::Indented );
}

bool HRecordSkel::applyJson(QString jsonData)
{
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg(jpe.errorString()));
        return true;
    }
    return applyJson_inWork(jdoc.object());
}

bool HRecordSkel::applyJsonFile(QString jsonFileName)
{
    applyJson(readJsonFile_inWork(jsonFileName));
    return false;
}

bool HRecordSkel::applyJson_inWork(QJsonObject jsonObject)
{
    if(jsonObject.contains("name"))
        setTableName(jsonObject.value("name").toString());

    if(jsonObject.contains("title"))
        setTableTitle(jsonObject.value("title").toString());

    if(jsonObject.contains("fields"))
    {
        QJsonArray fa = jsonObject.value("fields").toArray();
        int i,c = fa.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject fo = fa.at(i).toObject();
            if(!fo.contains("sqlname"))
                continue;
            HField *f = fieldByName(fo.value("sqlname").toString());
            if(f == NULL)
                continue;
            f->applyJson_inWork(fo);
        }
    }
    subspec_applyJson_inWork(&jsonObject);
    return false;
}

bool HRecordSkel::applyAddFieldsJson_inWork(QJsonObject jsonObject)
{
    QJsonArray fa = jsonObject.value("fields").toArray();
    int i,c = fa.count();
    for(i = 0 ; i < c ; ++i)
    {
        HField *f = HField::fromJson_inWork(fa.at(i));
        if(f == NULL)
            continue;
        addField(f);
        f->applyJson_inWork_valueOnly(fa.at(i).toObject());
    }
    return false;
}

QJsonValue HRecordSkel::toJson_inWork(HJsonFlag flags)
{
    int i,c = fieldCount();

    if(flagOn(flags,HJsonFlag_KeyValueStyle))
    {
        QJsonObject kvobject;
        for(i = 0 ; i < c ; ++i)
            if(!fieldByIndex(i)->noSql())
                kvobject.insert(fieldByIndex(i)->sqlName(),fieldByIndex(i)->strValue());
        return QJsonValue(kvobject);
    }

    QJsonObject toptable;
    QJsonArray fields;

    for(i = 0 ; i < c ; ++i)
        fields.push_back(fieldByIndex(i)->toJson_inWork(flags));

    toptable.insert("fields",fields);
    if(!flagOn(flags,HJsonFlag_NoStructure))
    {
        toptable.insert("name",tableName());
        toptable.insert("title",tableTitle());
    }
    subspec_toJson_inWork(&toptable);
    return QJsonValue(toptable);
}

QJsonValue HRecordSkel::toJson_embedded(HJsonFlag flags)
{
    return toJson_inWork(flags);
}

QString HRecordSkel::readJsonFile_inWork(QString jsonFileName)
{
    QFile f(jsonFileName);
    if(!f.exists())
    {
        sdebug("HRecordSkel::readJsonFile_inWork - File does not exists!");
        return "";
    }
    if(!f.open(QIODeviceBase::ReadOnly))
    {
        sdebug("HRecordSkel::readJsonFile_inWork - Cannot open file for read!");
        return "";
    }

    QString jsonData = QString::fromUtf8(f.readAll());
    f.close();
    return jsonData;
}

void HRecordSkel::subspec_toJson_inWork(QJsonObject *top)
{
    Q_UNUSED(top)
}

void HRecordSkel::subspec_applyJson_inWork(QJsonObject *top)
{
    Q_UNUSED(top);
}

// //////// HRecord ///////////////////////////////////////////////////////////////////

HRecord* HRecord::fromJson(QString jsonData,QString inDatabase)
{
    if(jsonData.trimmed().isEmpty())
        return NULL;
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        QString err = QString(QObject::tr("Error: Not valid JSON.\n Message: %1\n Offset: %2"))
                                .arg(jpe.errorString())
                                .arg(jpe.offset);
        sdebug(err);
        return NULL;
    }
    return fromJson_inWork(jdoc.object(),inDatabase);
}

HRecord* HRecord::fromJsonFile(QString jsonFileName,QString inDatabase)
{
    HRecord *r = HRecord::fromJson(readJsonFile_inWork(jsonFileName),inDatabase);
    return r;
}

HRecord* HRecord::fromJson_inWork(QJsonObject jsonObject,QString inDatabase)
{
    if(!jsonObject.contains("name") ||
       !jsonObject.contains("title") ||
       !jsonObject.contains("fields") ||
       !jsonObject.value("fields").isArray())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg("Missing fields"));
        return NULL;
    }
    HRecord *r = new HRecord(jsonObject.value("name").toString(),
                             jsonObject.value("title").toString());
    if(!inDatabase.isEmpty())
        r->db(inDatabase);
    r->applyAddFieldsJson_inWork(jsonObject);
    r->subspec_applyJson_inWork(&jsonObject);
    return r;
}

HRecord* HRecord::fromJson_embedded(QJsonObject jsonObject)
{
    return fromJson_inWork(jsonObject);
}

void HRecord::subspec_toJson_inWork(QJsonObject *top)
{
    Q_UNUSED(top)
}

void HRecord::subspec_applyJson_inWork(QJsonObject *top)
{
    Q_UNUSED(top);
}


// //////// HRecordLines //////////////////////////////////////////////////////////////

HRecordLines* HRecordLines::fromJson(QString jsonData)
{
    if(jsonData.trimmed().isEmpty())
        return NULL;
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg(jpe.errorString()));
        return NULL;
    }
    return fromJson_inWork(jdoc.object());
}

HRecordLines* HRecordLines::fromJsonFile(QString jsonFileName)
{
    HRecordLines *rl = HRecordLines::fromJson(readJsonFile_inWork(jsonFileName));
    return rl;
}

HRecordLines* HRecordLines::fromJson_inWork(QJsonObject jsonObject)
{
    if(!jsonObject.contains("name") ||
       !jsonObject.contains("title") ||
       !jsonObject.contains("fields") ||
       !jsonObject.value("fields").isArray())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg("Missing fields"));
        return NULL;
    }
    HRecordLines *rl = new HRecordLines(jsonObject.value("name").toString(),
                                        jsonObject.value("title").toString());
    rl->applyAddFieldsJson_inWork(jsonObject);
    rl->subspec_applyJson_inWork(&jsonObject);
    return rl;
}

HRecordLines* HRecordLines::fromJson_embedded(QJsonObject jsonObject)
{
    return fromJson_inWork(jsonObject);
}

void HRecordLines::subspec_toJson_inWork(QJsonObject *top)
{
    QStringList keys = onItemActions.keys();
    keys.sort();
    if(keys.count() > 0)
    {
        QJsonArray aa;
        int i,c = keys.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject ao;
            ao.insert(keys[i],onItemActions.value(keys[i]));
            aa.push_back(ao);
        }
        top->insert("alternate_actions",aa);
    }
}

void HRecordLines::subspec_applyJson_inWork(QJsonObject *top)
{
    if(top->contains("alternate_actions"))
    {
        QJsonArray aa = top->value("alternate_actions").toArray();
        int i,c = aa.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject ao = aa.at(i).toObject();
            QString key = ao.keys().at(0);
            if(key.isEmpty())
                continue;
            QString value = ao.value(key).toString();
            addOnItemAction(key,value);
        }
    }
}

// //////// HField //////////////////////////////////////////////////////////////

HField* HField::fromJson(QString jsonData)
{
    if(jsonData.trimmed().isEmpty())
        return NULL;
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg(jpe.errorString()));
        return NULL;
    }
    return fromJson_inWork(jdoc.object());
}

HField* HField::fromJsonFile(QString jsonFileName)
{
    QFile f(jsonFileName);
    if(!f.exists())
    {
        sdebug("HField::fromJsonFile - File does not exists!");
        return NULL;
    }
    if(!f.open(QIODeviceBase::ReadOnly))
    {
        sdebug("HField::fromJsonFile - Cannot open file for read!");
        return NULL;
    }

    HField *field = HField::fromJson(QString::fromUtf8(f.readAll()));
    f.close();
    return field;
}

bool HField::applyJson(QString jsonData)
{
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg(jpe.errorString()));
        return true;
    }
    return applyJson_inWork(jdoc.object());
}

bool HField::applyJsonFile(QString jsonFileName)
{
    QFile f(jsonFileName);
    if(!f.exists())
    {
        sdebug("HField::applyJsonFile - File does not exists!");
        return true;
    }
    if(!f.open(QIODeviceBase::ReadOnly))
    {
        sdebug("HField::applyJsonFile - Cannot open file for read!");
        return true;
    }

    applyJson(QString::fromUtf8(f.readAll()));
    f.close();
    return false;
}

QString HField::toJson(HJsonFlag flags)
{
    QJsonDocument jd;
    jd.setObject(toJson_inWork(flags).toObject());
    return jd.toJson(flagOn(flags,HJsonFlag_Compacted) ? QJsonDocument::Compact : QJsonDocument::Indented );
}

HField* HField::fromJson_inWork(QJsonValue jsonValue)
{
    if(!jsonValue.isObject())
        return NULL;
    QJsonObject jobj = jsonValue.toObject();
    if(!jobj.contains("type") ||
       !jobj.contains("sqlname") ||
       !jobj.contains("description"))
        return NULL;
    QString title = "";
    if(jobj.contains("title"))
        title = jobj.value("title").toString();
    HField *f = newHFieldObjByName(jobj.value("type").toString(),
                                   jobj.value("sqlname").toString(),
                                   jobj.value("description").toString(),
                                   title);
    f->applyJson_inWork(jobj);
    return f;
}

bool HField::applyJson_inWork_valueOnly(QJsonObject jsonObject)
{
    if(jsonObject.contains("value") && attribute("prevent_json_update_value") != "prevent")
        if(strValue() != jsonObject.value("value").toString())
            setStrValue(jsonObject.value("value").toString());
    return false;
}

bool HField::applyJson_inWork(QJsonObject jsonObject)
{
    if(jsonObject.contains("description"))
        setDescription(jsonObject.value("description").toString());

    if(jsonObject.contains("title"))
        setTitle(jsonObject.value("title").toString());

    if(jsonObject.contains("default"))
        setStrDefault(jsonObject.value("default").toString());

    if(jsonObject.contains("nosql") && jsonObject.value("nosql").toString() == "yes")
        setNoSql(true);

    if(jsonObject.contains("mode"))
    {
        QString m = jsonObject.value("mode").toString().toLower();
        if(m == "defaulteditable") setFieldEditType(HFieldEdit_DefaultEditable);
        if(m == "invisible"      ) setFieldEditType(HFieldEdit_Invisible      );
        if(m == "readonly"       ) setFieldEditType(HFieldEdit_Readonly       );
        if(m == "showreadonly"   ) setFieldEditType(HFieldEdit_ShowReadonly   );
    }

    if(jsonObject.contains("displayflags"))
    {
        QStringList dflagstr = jsonObject.value("displayflags").toString().split("|");
        HDispObjectFlags dflag = stringListToDisplayFlags(dflagstr);
        setDisplayFlags(dflag);
    }

    if(jsonObject.contains("attributes"))
    {
        QJsonArray aa = jsonObject.value("attributes").toArray();
        int i,c = aa.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject ao = aa.at(i).toObject();
            QString key = ao.keys().at(0);
            if(key.isEmpty())
                continue;
            QString value = ao.value(key).toString();
            setAttribute(key,value);
        }
    }

    if(jsonObject.contains("validators"))
    {
        QJsonArray va = jsonObject.value("validators").toArray();
        int vi,vc = va.count();
        for(vi = 0 ; vi < vc ; ++vi)
        {
            QJsonObject vao = va.at(vi).toObject();
            QString v_type = vao.value("type").toString();
            QString v_fmsg = vao.value("failmessage").toString();
            if(v_type.isEmpty() && v_fmsg.isEmpty())
                continue;

            HBaseValidator *n_validator = newHValidatorObjByName(v_type,v_fmsg);
            if(n_validator == NULL)
                continue;

            if(vao.contains("attributes"))
            {
                QJsonArray voaa = vao.value("attributes").toArray();
                int vai,vac = voaa.count();
                for(vai = 0 ; vai < vac ; ++vai)
                {
                    QJsonObject vaao = voaa.at(vai).toObject();
                    QString key = vaao.keys().at(0);
                    if(key.isEmpty())
                        continue;
                    QString value = vaao.value(key).toString();
                    n_validator->setAttribute(key,value);
                }
            }
            addValidator(n_validator);
        }
    }

    if(jsonObject.contains("tags"))
        addTags(jsonObject.value("tags").toString());

    applyJson_inWork_spec(jsonObject);

    if(jsonObject.contains("value") && attribute("prevent_json_update_value") != "prevent")
        if(strValue() != jsonObject.value("value").toString())
            setStrValue(jsonObject.value("value").toString());

    return false;
}

void HField::applyJson_inWork_spec(QJsonObject& fo)
{
    Q_UNUSED(fo)
}

QJsonValue HField::toJson_inWork(HJsonFlag flags)
{
    QJsonObject f;

    f.insert("sqlname",sqlName());

    if(!flagOn(flags,HJsonFlag_NoValues))
        f.insert("value",strValue());

    if(flagOn(flags,HJsonFlag_NoStructure))
        return QJsonValue(f);

    QStringList atts = allDefinedAttributes();
    if(atts.count() > 0)
    {
        QJsonArray aa;
        int ai,ac = atts.count();
        for(ai = 0 ; ai < ac ; ++ai)
        {
            QJsonObject ao;
            ao.insert(atts[ai],attribute(atts[ai]));
            aa.push_back(ao);
        }
        f.insert("attributes",aa);
    }

    if(validators().count() > 0)
    {
        QList<HBaseValidator *> vs = validators();
        QJsonArray va;
        int vi,vc = vs.count();
        for(vi = 0 ; vi < vc ; ++vi)
        {
            QJsonObject vo;
            vo.insert("type",vs[vi]->validatorType());
            vo.insert("failmessage",vs[vi]->failMessage());
            QStringList vatts = vs[vi]->allDefinedAttributes();
            if(vatts.count() > 0)
            {
                QJsonArray vaa;
                int vai,vac = vatts.count();
                for(vai = 0 ; vai < vac ; ++vai)
                {
                    QJsonObject vao;
                    vao.insert(vatts[vai],vs[vi]->attribute(vatts[vai]));
                    vaa.push_back(vao);
                }
                vo.insert("attributes",vaa);
            }

            va.push_back(vo);
        }
        f.insert("validators",va);
    }

    if(tags().count() > 0)
        f.insert("tags",tags().join(","));

    if(className() != "HStaticField" && noSql())
        f.insert("nosql","yes");

    if(fieldEditType() != HFieldEdit_DefaultEditable)
    {
        QString m = "DefaultEditable";
        if(fieldEditType() == HFieldEdit_Invisible   ) m = "Invisible";
        if(fieldEditType() == HFieldEdit_Readonly    ) m = "Readonly";
        if(fieldEditType() == HFieldEdit_ShowReadonly) m = "ShowReadonly";
        f.insert("mode",m);
    }

    if(displayFlags() != HDispFlag_Default)
        f.insert("displayflags",displayFlagsToStringList(displayFlags()).join("|"));

    f.insert("default",strDefault());
    f.insert("description",description());
    f.insert("title",title());
    f.insert("type",className().mid(1).replace(QRegularExpression("Field$"),""));
    toJson_inWork_spec(f);
    return QJsonValue(f);
}

void HField::toJson_inWork_spec(QJsonObject& fo)
{
    Q_UNUSED(fo)
}

template <>
void HFieldWithValueStore<int>::jsonApplyHelper(QJsonObject& job)
{
    if(job.contains("selectables") && job.value("selectables").isArray())
    {
        QJsonArray ja = job.value("selectables").toArray();
        int i,c = ja.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject o = ja.at(i).toObject();
            if(o.keys().count() == 1)
                selectables.addItem(o.keys().at(0).toInt(),o.value(o.keys().at(0)).toString());
        }
    }

    if(job.contains("unlistedSelectables") && job.value("unlistedSelectables").isArray())
    {
        QJsonArray ja = job.value("unlistedSelectables").toArray();
        int i,c = ja.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject o = ja.at(i).toObject();
            if(o.keys().count() == 1)
                selectables.addUnlistedItem(o.keys().at(0).toInt(),o.value(o.keys().at(0)).toString());
        }
    }
}

template <>
void HFieldWithValueStore<QString>::jsonApplyHelper(QJsonObject& job)
{
    if(job.contains("selectables") && job.value("selectables").isArray())
    {
        QJsonArray ja = job.value("selectables").toArray();
        int i,c = ja.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject o = ja.at(i).toObject();
            if(o.keys().count() == 1)
                selectables.addItem(o.keys().at(0),o.value(o.keys().at(0)).toString());
        }
    }

    if(job.contains("unlistedSelectables") && job.value("unlistedSelectables").isArray())
    {
        QJsonArray ja = job.value("unlistedSelectables").toArray();
        int i,c = ja.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject o = ja.at(i).toObject();
            if(o.keys().count() == 1)
                selectables.addUnlistedItem(o.keys().at(0),o.value(o.keys().at(0)).toString());
        }
    }
}

template <>
void HFieldWithValueStore<int>::jsonGenerateHelper(QJsonObject& job)
{
    QList<int> vkeys = selectables.keys();
    QList<int> ukeys = selectables.unlistedKeys();
    if(vkeys.count() + ukeys.count() <= 0)
        return;

    if(vkeys.count() > 0)
    {
        QJsonArray sa;
        int i,c = vkeys.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject so;
            so.insert(QString::asprintf("%d",vkeys[i]),selectables.value(vkeys[i]));
            sa.push_back(so);
        }
        job.insert("selectables",sa);
    }

    if(ukeys.count() > 0)
    {
        QJsonArray sa;
        int i,c = ukeys.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject so;
            so.insert(QString::asprintf("%d",ukeys[i]),selectables.value(ukeys[i],true));
            sa.push_back(so);
        }
        job.insert("unlistedSelectables",sa);
    }
}

template <>
void HFieldWithValueStore<QString>::jsonGenerateHelper(QJsonObject& job)
{
    QList<QString> vkeys = selectables.keys();
    QList<QString> ukeys = selectables.unlistedKeys();
    if(vkeys.count() + ukeys.count() <= 0)
        return;

    if(vkeys.count() > 0)
    {
        QJsonArray sa;
        int i,c = vkeys.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject so;
            so.insert(vkeys[i],selectables.value(vkeys[i]));
            sa.push_back(so);
        }
        job.insert("selectables",sa);
    }

    if(ukeys.count() > 0)
    {
        QJsonArray sa;
        int i,c = ukeys.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject so;
            so.insert(ukeys[i],selectables.value(ukeys[i],true));
            sa.push_back(so);
        }
        job.insert("unlistedSelectables",sa);
    }
}

void HDateField::toJson_inWork_spec(QJsonObject& fo)
{
    fo.insert("unknownallowed",unknownAllowed() ? "yes" : "no");
}

void HDateField::applyJson_inWork_spec(QJsonObject& fo)
{
    if(fo.contains("unknownallowed"))
    {
        if(fo.value("unknownallowed").toString() == "yes")
            setUnknownAllowed(true);
        if(fo.value("unknownallowed").toString() == "no")
            setUnknownAllowed(false);
    }
}

void HSqlXChooseField::toJson_inWork_spec(QJsonObject& fo)
{
    fo.insert("unknownallowed",unknownAllowed_X() ? "yes" : "no");

    fo.insert("conntable",jTableName);
    fo.insert("connkey",jKeyFieldName);
    fo.insert("connshow",jShowFields);
    fo.insert("connfilter",jFilter);
    fo.insert("connsort",jSort);
}

void HSqlXChooseField::applyJson_inWork_spec(QJsonObject& fo)
{
    if(fo.contains("unknownallowed"))
    {
        if(fo.value("unknownallowed").toString() == "yes")
            setUnknownAllowed_X(true);
        if(fo.value("unknownallowed").toString() == "no")
            setUnknownAllowed_X(false);
    }

    QString connTable  = "";
    QString connKey    = "";
    QString connShow   = "";
    QString connSort   = "";
    QString connFilter = "";

    if(fo.contains("conntable" )) connTable  = fo.value("conntable").toString();
    if(fo.contains("connkey"   )) connKey    = fo.value("connkey"  ).toString();
    if(fo.contains("connshow"  )) connShow   = fo.value("connshow" ).toString();
    if(fo.contains("connsort"  )) connSort   = fo.value("connsort" ).toString();
    if(fo.contains("connfilter")) connFilter = fo.value("connfilter").toString();

    setKVSource_X(connTable,connKey,connShow,connFilter,connSort);
}

// //////// HDialogData & HDialog /////////////////////////////////////////////////

HDialogData* HDialogData::fromJson(QString jsonData)
{
    if(jsonData.trimmed().isEmpty())
        return NULL;
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg(jpe.errorString()));
        return NULL;
    }
    return fromJson_inWork(jdoc.object());
}

HDialogData* HDialogData::fromJsonFile(QString jsonFileName)
{
    HDialogData *dd = HDialogData::fromJson(readJsonFile_inWork(jsonFileName));
    return dd;
}

HDialog* HDialog::fromJson(QWidget *parent,QString jsonData)
{
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg(jpe.errorString()));
        return NULL;
    }
    return fromJson_inWork(parent,jdoc.object());
}

HDialog* HDialog::fromJsonFile(QWidget *parent,QString jsonFileName)
{
    HDialog *d = HDialog::fromJson(parent,readJsonFile_inWork(jsonFileName));
    return d;
}

bool HDialogData::applyJson(QString jsonData)
{
    QJsonDocument jdoc;
    QJsonParseError jpe;
    jdoc = QJsonDocument::fromJson(jsonData.toUtf8(),&jpe);
    if(jdoc.isNull())
    {
        sdebug(QString(QObject::tr("Error: Not valid JSON.\n Message: %1"))
                        .arg(jpe.errorString()));
        return true;
    }
    return applyJson_inWork(jdoc.object());
}

bool HDialogData::applyJsonFile(QString jsonFileName)
{
    applyJson(readJsonFile_inWork(jsonFileName));
    return false;
}

QString HDialogData::toJson(HJsonFlag flags)
{
    QJsonDocument jd;
    jd.setObject(toJson_inWork(flags).toObject());
    return jd.toJson(flagOn(flags,HJsonFlag_Compacted) ? QJsonDocument::Compact : QJsonDocument::Indented );
}

QString HDialogData::readJsonFile_inWork(QString jsonFileName)
{
    QFile f(jsonFileName);
    if(!f.exists())
    {
        sdebug("HDialogData::readJsonFile_inWork - File does not exists!");
        return "";
    }
    if(!f.open(QIODeviceBase::ReadOnly))
    {
        sdebug("HDialogData::readJsonFile_inWork - Cannot open file for read!");
        return "";
    }

    QString jsonData = QString::fromUtf8(f.readAll());
    f.close();
    return jsonData;
}

HDialogData* HDialogData::fromJson_inWork(QJsonValue jsonValue)
{
    if(!jsonValue.isObject())
        return NULL;
    HDialogData *dd = new HDialogData();
    dd->applyJson_inWork(jsonValue.toObject());
    return dd;
}

HDialog* HDialog::fromJson_inWork(QWidget *parent,QJsonValue jsonValue)
{
    if(!jsonValue.isObject())
        return NULL;
    HDialog *d = new HDialog(parent);
    d->applyJson_inWork(jsonValue.toObject());
    return d;
}

bool HDialogData::applyJson_inWork(QJsonObject jsonObject)
{
    if(jsonObject.contains("displayflags"))
    {
        QStringList dflagstr = jsonObject.value("displayflags").toString().split("|");
        HDispObjectFlags dflag = stringListToDisplayFlags(dflagstr);
        setDisplayFlags(dflag);
    }

    if(jsonObject.contains("attributes"))
    {
        QJsonArray aa = jsonObject.value("attributes").toArray();
        int i,c = aa.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject ao = aa.at(i).toObject();
            QString key = ao.keys().at(0);
            if(key.isEmpty())
                continue;
            QString value = ao.value(key).toString();
            setAttribute(key,value);
        }
    }

    if(jsonObject.contains("objects"))
    {
        QJsonArray oa = jsonObject.value("objects").toArray();
        int i,c = oa.count();
        for(i = 0 ; i < c ; ++i)
        {
            QJsonObject oo = oa.at(i).toObject();
            QString key = oo.keys().at(0);
            if(key.isEmpty())
                continue;
            if(key == "hrecord")
            {
                add(HRecord::fromJson_embedded(oo.value(key).toObject()));
            }
            if(key == "hrecordlines")
            {
                add(HRecordLines::fromJson_embedded(oo.value(key).toObject()));
            }
        }
    }

    return false;
}

QJsonValue HDialogData::toJson_inWork(HJsonFlag flags)
{
    QJsonObject f;

    if(displayFlags() != HDispFlag_Default)
        f.insert("displayflags",displayFlagsToStringList(displayFlags()).join("|"));

    QStringList atts = allDefinedAttributes();
    if(atts.count() > 0)
    {
        QJsonArray aa;
        int ai,ac = atts.count();
        for(ai = 0 ; ai < ac ; ++ai)
        {
            QJsonObject ao;
            ao.insert(atts[ai],attribute(atts[ai]));
            aa.push_back(ao);
        }
        f.insert("attributes",aa);
    }

    if(objs.count() > 0 && !flagOn(flags,HJsonFlag_DontGenerateObject))
    {
        QJsonArray oa;
        int oi,oc = objs.count();
        for(oi = 0 ; oi < oc ; ++oi)
        {
            QJsonObject ao;
            QJsonValue jv;
            if(objs[oi]->className() == "HRecord")
            {
                jv = getHRecord(oi)->toJson_embedded(flags);
                ao.insert(objs[oi]->className().toLower(),jv);
                oa.push_back(ao);
            }
            if(objs[oi]->className() == "HRecordLines")
            {
                jv = getHRecordLines(oi)->toJson_embedded(flags);
                ao.insert(objs[oi]->className().toLower(),jv);
                oa.push_back(ao);
            }
        }
        f.insert("objects",oa);
    }
    return QJsonValue(f);
}

QStringList displayFlagsToStringList(HDispObjectFlags f)
{
    QStringList l;

    if(flagOn(f,HDispFlag_DeleteDataObjectOnDestroy))
        l.push_back("DeleteDataObjectOnDestroy");
    if(flagOn(f,HDispFlag_CleanDataOnStart))
        l.push_back("CleanDataOnStart");
    if(flagOn(f,HDispFlag_CleanDataOnDestroy))
        l.push_back("CleanDataOnDestroy");
    if(flagOn(f,HDispFlag_NoMergeFlagsWithHField))
        l.push_back("NoMergeFlagsWithHField");
    if(flagOn(f,HDispFlag_NoInheritFlagsFromHRecordDisplay))
        l.push_back("NoInheritFlagsFromHRecordDisplay");
    if(flagOn(f,HDispFlag_Readonly))
        l.push_back("Readonly");
    if(flagOn(f,HDispFlag_ShowOnly))
        l.push_back("ShowOnly");
    if(flagOn(f,HDispFlag_Invisible))
        l.push_back("Invisible");
    if(flagOn(f,HDispFlag_DisableStretchInMiddle))
        l.push_back("DisableStretchInMiddle");
    if(flagOn(f,HDispFlag_AddStretchToLineEnd))
        l.push_back("AddStretchToLineEnd");
    if(flagOn(f,HDispFlag_AddStretchAfterFields))
        l.push_back("AddStretchAfterFields");
    return l;
}

HDispObjectFlags stringListToDisplayFlags(QStringList l)
{
    HDispObjectFlags f = HDispFlag_Default;
    int i,c = l.count();
    for(i = 0 ; i < c ; ++i)
    {
        if(l[i] == "DeleteDataObjectOnDestroy")
            f = f | HDispFlag_DeleteDataObjectOnDestroy;
        if(l[i] == "CleanDataOnStart")
            f = f | HDispFlag_CleanDataOnStart;
        if(l[i] == "CleanDataOnDestroy")
            f = f | HDispFlag_CleanDataOnDestroy;
        if(l[i] == "NoMergeFlagsWithHField")
            f = f | HDispFlag_NoMergeFlagsWithHField;
        if(l[i] == "NoInheritFlagsFromHRecordDisplay")
            f = f | HDispFlag_NoInheritFlagsFromHRecordDisplay;
        if(l[i] == "Readonly")
            f = f | HDispFlag_Readonly;
        if(l[i] == "ShowOnly")
            f = f | HDispFlag_ShowOnly;
        if(l[i] == "Invisible")
            f = f | HDispFlag_Invisible;
        if(l[i] == "DisableStretchInMiddle")
            f = f | HDispFlag_DisableStretchInMiddle;
        if(l[i] == "AddStretchToLineEnd")
            f = f | HDispFlag_AddStretchToLineEnd;
        if(l[i] == "AddStretchAfterFields")
            f = f | HDispFlag_AddStretchAfterFields;
    }
    return f;
}

//end code
