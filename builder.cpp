/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0
*/

#include <string.h>
#include "builder.h"

#include "dconsole.h"

#define FIELD_GET 1
#define FIELD_SET_V 2
#define FIELD_SET_E 3

#define COND_ZZ       0
#define COND_FF       1
#define COND_FV       2
#define COND_FE       3
#define COND_FB       4
#define COND_SPEC_F   5
#define COND_SPEC_V   6
#define COND_SPEC_VF  7
#define COND_GRP      8
#define COND_SORT     9

QMap<QString,QString> HSqlBuilder::spec_conds = QMap<QString,QString>();

HSqlBuilderCondition cond(HSqlBuilder_ConditionRelation r)
{
    return HSqlBuilderCondition(r);
}

HSqlBuilderCondition not_cond(HSqlBuilder_ConditionRelation r)
{
    return HSqlBuilderCondition(r,"opposite=true");
}

HSqlBuilder db_query(QString tablename,QString alias)
{
    return HSqlBuilder(Select,tablename,alias);
}

HSqlBuilder db_insert(QString tablename,QString alias)
{
    return HSqlBuilder(Insert,tablename,alias);
}

HSqlBuilder db_update(QString tablename,QString alias)
{
    return HSqlBuilder(Update,tablename,alias);
}

HSqlBuilder db_delete(QString tablename,QString alias)
{
    return HSqlBuilder(Delete,tablename,alias);
}
// Sql dialect table ////////////////////////////////////////////////////////////
const char *dialectTablePuzzles[]  = {//Empty string is the closer element !
        "<<<current_timestamp>>>",
        "<<<datetype_null>>>",
        "<<<numeric_null>>>",
        "<<<longtext_type>>>",
        "<<<regex>>>",
        "",
};

const char *dialectTableDialects[] = //Empty string is the closer element !
        {"nochange"               , "sqlite"           , "pgsql"    , "mysql"    , ""};

const char *dialectTableResolve[][4] = {
        {"<<<current_timestamp>>>", "CURRENT_TIMESTAMP", "now()"    , "CURRENT_TIMESTAMP" },
        {"<<<datetype_null>>>"    , "NULL"             , "NULL"     , "NULL" },
        {"<<<numeric_null>>>"     , "NULL"             , "NULL"     , "NULL" },
        {"<<<longtext_type>>>"    , "longtext"         , "text"     , "longtext" },
        {"<<<regex>>>"            , "regexp"           , "~"        , "regexp" },
};

// HSqlBuilderField ////////////////////////////////////////////////////////////

HSqlBuilderField::HSqlBuilderField()
{
    type = FIELD_GET;
    table = "";
    name = "";
    alias = "";
    vt = Quoted;
    value_expression = "";
    options = "";
}

HSqlBuilderField::~HSqlBuilderField()
{
}

bool HSqlBuilderField::isGetType(void)
{
    if(type == FIELD_GET)
        return true;
    return false;
}

bool HSqlBuilderField::isSetType(void)
{
    if(type == FIELD_SET_V || type == FIELD_SET_E)
        return true;
    return false;
}

QString HSqlBuilderField::getVisibleName(void)
{
    if(alias.isEmpty())
        return name;
    return alias;
}

HSqlBuilderField& HSqlBuilderField::get(const QString fieldName,const QString aliasName,const QString optionsString)
{
    get_withtable("",fieldName,aliasName,optionsString);
    return *this;
}

HSqlBuilderField& HSqlBuilderField::get(const QString fieldSpec[2],const QString aliasName,const QString optionsString)
{
    get_withtable(fieldSpec[0],fieldSpec[1],aliasName,optionsString);
    return *this;
}

HSqlBuilderField& HSqlBuilderField::get_withtable(const QString tableName,const QString fieldName,const QString aliasName,const QString optionsString)
{
    type = FIELD_GET;
    name  = fieldName;
    table = tableName;
    alias = aliasName;
    options = optionsString;
    return *this;
}

HSqlBuilderField& HSqlBuilderField::set_fv(const QString fieldName,HSqlBuilder_FieldValueType vType,QVariant value,const QString optionsString)
{
    type = FIELD_SET_V;
    table = "";
    name = fieldName;
    alias = "";
    vt = vType;
    value_expression = value;
    options = optionsString;
    return *this;
}

HSqlBuilderField& HSqlBuilderField::set_fe(const QString fieldName,const QString expression,const QString optionsString)
{
    type = FIELD_SET_E;
    table = "";
    name = fieldName;
    alias = "";
    vt = Unquoted;
    value_expression = expression;
    options = optionsString;
    return *this;
}

QString HSqlBuilderField::local_cmd_Get(void)
{
    if(type != FIELD_GET)
        return QString();

    QString str;
    if(table.isEmpty())
        str = name;
    else
        str = table + "." + name;

    QMap<QString,QString> opts = HSqlBuilder::genOptions(options);
    if(!opts.value("function","").isEmpty())
    {
        QString modstr;
        modstr = opts.value("function","") + QString("(");
        if(!opts.value("more_args_before","").isEmpty())
            modstr.append(opts.value("more_args_before","") + ",");
        modstr.append(str);
        if(!opts.value("more_args","").isEmpty())
            modstr.append(QString(",") + opts.value("more_args","") + ",");
        if(!opts.value("more_args_after","").isEmpty())
            modstr.append(QString(",") + opts.value("more_args_after","") + ",");
        modstr.append(")");
        str = modstr;
    }
    if(!alias.isEmpty())
        str += " AS " + alias;
    return str;
}

QString HSqlBuilderField::local_cmd_Key(void)
{
    if(type != FIELD_SET_V && type != FIELD_SET_E)
        return QString();
    return name;
}

QString HSqlBuilderField::local_cmd_Val(HSqlBuilder *builder,bool vmm,QString dialect)
{
    if(type != FIELD_SET_V && type != FIELD_SET_E)
        return QString();

    QString v = "";
    QMap<QString,QString> opts = HSqlBuilder::genOptions(options);
    if(type == FIELD_SET_V)
    {
        if(vmm)
        {
            v = builder->addValueForBind(value_expression);
        }
        else
        {
            v = value_expression.toString();
            if(vt == Quoted)
                v = QString("\'%1\'").arg(value_expression.toString());
        }
    }
    if(type == FIELD_SET_E)
    {
        v = value_expression.toString();

        if(opts.value("dialected_element","no") == "yes")
            v = HSqlBuilder::translateDialect(v,dialect);

        if(vt == Quoted)
            v = QString("\'%1\'").arg(value_expression.toString());
    }

    if(!opts.value("function","").isEmpty())
    {
        QString modfsql;

        modfsql = opts.value("function","") + QString("(");
        if(!opts.value("more_args_before","").isEmpty())
            modfsql.append(opts.value("more_args_before","") + ",");
        modfsql.append(v);
        if(!opts.value("more_args","").isEmpty())
            modfsql.append(QString(",") + opts.value("more_args","") + ",");
        if(!opts.value("more_args_after","").isEmpty())
            modfsql.append(QString(",") + opts.value("more_args_after","") + ",");
        modfsql.append(")");

        v = modfsql;
    }
    return v;
}

QString HSqlBuilderField::json_string(void)
{
    QString jstr = "";
    if(isGetType())
    {
        jstr += "{";
        jstr += QString("\"table\": \"%1\",").arg(table);
        jstr += QString("\"name\": \"%1\",").arg(name);
        jstr += QString("\"alias\": \"%1\",").arg(alias);
        jstr += QString("\"options\": \"%1\"").arg(options);
        jstr += "}";
    }
    if(isSetType())
    {
        jstr += "{";
        if(type == FIELD_SET_V)
            jstr += QString("\"type\": \"set_val\",");
        if(type == FIELD_SET_E)
            jstr += QString("\"type\": \"set_expr\",");

        if(vt == Quoted)
            jstr += QString("\"valtype\": \"quoted\",");
        if(vt == Unquoted)
            jstr += QString("\"valtype\": \"unquoted\",");

        jstr += QString("\"name\": \"%1\",").arg(name);
        jstr += QString("\"value\": \"%1\",").arg(strToJsonStr(value_expression.toString()));
        jstr += QString("\"options\": \"%1\"").arg(options);
        jstr += "}";
    }
    return jstr;
}

// HSqlBuilderCondition ////////////////////////////////////////////////////////////

HSqlBuilderCondition::HSqlBuilderCondition(HSqlBuilder_ConditionRelation r,QString opts)
{
    ct = COND_GRP;
    relation = r;
    vt = Unquoted;
    cname = "";
    value_expression = "";
    op = "";
    field1 = "";
    field2 = "";
    table1 = "";
    table2 = "";
    options = opts;
    sub_conds.clear();
}

HSqlBuilderCondition::~HSqlBuilderCondition()
{
}

HSqlBuilderCondition& HSqlBuilderCondition::ff(const QString fieldName1,const QString fieldName2,const QString operation,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FF;
    newcond.op = operation;
    newcond.field1 = fieldName1;
    newcond.table1 = "";
    newcond.field2 = fieldName2;
    newcond.table2 = "";
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::ff(const QString fieldSpec1[2],const QString fieldSpec2[2],const QString operation,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FF;
    newcond.op = operation;
    newcond.table1 = fieldSpec1[0];
    newcond.field1 = fieldSpec1[1];
    newcond.table2 = fieldSpec2[0];
    newcond.field2 = fieldSpec2[1];
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::ff(const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2,const QString operation,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FF;
    newcond.op = operation;
    newcond.table1 = tableName1;
    newcond.field1 = fieldName1;
    newcond.table2 = tableName2;
    newcond.field2 = fieldName2;
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::fv(const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FV;
    newcond.vt = vType;
    newcond.value_expression = value;
    newcond.op = operation;
    newcond.table1 = "";
    newcond.field1 = fieldName;
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::fv(const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FV;
    newcond.vt = vType;
    newcond.value_expression = value;
    newcond.op = operation;
    newcond.table1 = fieldSpec[0];
    newcond.field1 = fieldSpec[1];
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::fe(const QString fieldName,const QString expression,const QString operation,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FE;
    newcond.value_expression = expression;
    newcond.op = operation;
    newcond.table1 = "";
    newcond.field1 = fieldName;
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::fe(const QString fieldSpec[2],const QString expression,const QString operation,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FE;
    newcond.value_expression = expression;
    newcond.op = operation;
    newcond.table1 = fieldSpec[0];
    newcond.field1 = fieldSpec[1];
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::fb(const QString fieldName,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FB;
    newcond.table1 = "";
    newcond.field1 = fieldName;
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::fb(const QString fieldSpec[2],const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_FB;
    newcond.table1 = fieldSpec[0];
    newcond.field1 = fieldSpec[1];
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::spec_f(const QString condSpec,const QString fieldName,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_SPEC_F;
    newcond.cname = condSpec;
    newcond.table1 = "";
    newcond.field1 = fieldName;
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::spec_f(const QString condSpec,const QString fieldSpec[2],const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_SPEC_F;
    newcond.cname = condSpec;
    newcond.table1 = fieldSpec[0];
    newcond.field1 = fieldSpec[1];
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::spec_v(const QString condSpec,const HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_SPEC_V;
    newcond.cname = condSpec;
    newcond.vt = vType;
    newcond.value_expression = value;
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::spec_fv(const QString condSpec,const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_SPEC_VF;
    newcond.cname = condSpec;
    newcond.vt = vType;
    newcond.value_expression = value;
    newcond.table1 = "";
    newcond.field1 = fieldName;
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::spec_fv(const QString condSpec,const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString)
{
    HSqlBuilderCondition newcond;
    newcond.ct = COND_SPEC_VF;
    newcond.cname = condSpec;
    newcond.vt = vType;
    newcond.value_expression = value;
    newcond.table1 = fieldSpec[0];
    newcond.field1 = fieldSpec[1];
    newcond.options = optionsString;
    add(newcond);
    return *this;
}

HSqlBuilderCondition& HSqlBuilderCondition::add(HSqlBuilderCondition cond)
{
    ct = COND_GRP;
    sub_conds.push_back(cond);
    return *this;
}

bool HSqlBuilderCondition::isGrpCond(void)
{
    if(ct == COND_GRP)
        return true;
    return false;
}

bool HSqlBuilderCondition::isEmpty()
{
    if((ct == COND_GRP || ct == COND_ZZ) && sub_conds.count() == 0)
        return true;
    return false;
}

HSqlBuilder_ConditionRelation HSqlBuilderCondition::topRelation(void)
{
    return relation;
}

QString HSqlBuilderCondition::local_cmd(HSqlBuilder *builder,bool top,bool vmm,QString dialect)
{
    QString local_cmd = "";
    if(ct == COND_ZZ)
        return local_cmd;

    QMap<QString,QString> opts = HSqlBuilder::genOptions(options);
    if(opts.value("opposite","") == "true")
        local_cmd += "NOT ";

    if(ct == COND_GRP)
    {
        QString sub_cmd = "";

        QList<HSqlBuilderCondition>::iterator ci = sub_conds.begin();
        while(ci != sub_conds.end())
        {
            if(!sub_cmd.isEmpty())
                sub_cmd += relation == And ? " AND " : " OR ";
            sub_cmd += ci->local_cmd(builder,false,vmm,dialect);
            ++ci;
        }

        if(!top)
            local_cmd += "(" + sub_cmd + ")";
        else
            local_cmd += sub_cmd;
    }

    if(ct == COND_FF)
    {
        QString f1,f2;
        f1 = (table1.isEmpty() ? "" : table1 + ".") + field1;
        f2 = (table2.isEmpty() ? "" : table2 + ".") + field2;
        if(!opts.value("f1function","").isEmpty())
            f1 = opts.value("f1function") + "(" + f1 + ")";
        if(!opts.value("f2function","").isEmpty())
            f2 = opts.value("f2function") + "(" + f2 + ")";
        local_cmd += f1 + " " + op + " " + f2;
    }
    if(ct == COND_FV)
    {
        QString v,f;
        if(vmm)
        {
            v = builder->addValueForBind(value_expression);
        }
        else
        {
            if(vt == Quoted)
                v = QString("\'%1\'").arg(value_expression);
            if(vt == Unquoted)
                v = value_expression;
        }
        f = (table1.isEmpty() ? "" : table1 + ".") + field1;
        if(!opts.value("vfunction","").isEmpty())
            v = opts.value("vfunction") + "(" + v + ")";
        if(!opts.value("ffunction","").isEmpty())
            f = opts.value("ffunction") + "(" + f + ")";
        local_cmd += f + " " + op + " " + v;
    }
    if(ct == COND_FE)
    {
        QString e,f;
        e = value_expression;
        f = (table1.isEmpty() ? "" : table1 + ".") + field1;
        if(!opts.value("efunction","").isEmpty())
            e = opts.value("efunction") + "(" + e + ")";
        if(!opts.value("ffunction","").isEmpty())
            f = opts.value("ffunction") + "(" + f + ")";
        local_cmd += f + " " + op + " " + e;
    }
    if(ct == COND_FB)
    {
        QString f;
        f = (table1.isEmpty() ? "" : table1 + ".") + field1;
        if(!opts.value("ffunction","").isEmpty())
            f = opts.value("ffunction") + "(" + f + ")";
        local_cmd += f;
    }

    if(ct == COND_SPEC_VF || ct == COND_SPEC_F || ct == COND_SPEC_V)
    {
        QString v,f,t;
        t = HSqlBuilder::special_cond_template(cname);
        if(ct == COND_SPEC_VF || ct == COND_SPEC_V)
        {
            if(vmm)
            {
                v = builder->addValueForBind(value_expression);
            }
            else
            {
                if(vt == Quoted)
                    v = QString("\'%1\'").arg(value_expression);
                if(vt == Unquoted)
                    v = value_expression;
            }
            if(!opts.value("vfunction","").isEmpty())
                v = opts.value("vfunction") + "(" + v + ")";
            t.replace(QString("__VALUE__"),v);
        }
        if(ct == COND_SPEC_VF || ct == COND_SPEC_F)
        {
            f = (table1.isEmpty() ? "" : table1 + ".") + field1;
            if(!opts.value("ffunction","").isEmpty())
                f = opts.value("ffunction") + "(" + f + ")";
            t.replace(QString("__FIELD__"),f);
        }
        local_cmd += t;
    }
    return local_cmd;
}

QString HSqlBuilderCondition::json_string(void)
{
    QString cstr = "";
    if(ct == COND_ZZ)
        return cstr;

    if(ct == COND_GRP)
    {
        QMap<QString,QString> opts = HSqlBuilder::genOptions(options);

        cstr += "{";
        if(opts.value("opposite","") == "true")
        {
            if(relation == And) cstr += QString("\"relation\": \"nand\",");
            if(relation == Or)  cstr += QString("\"relation\": \"nor\",");
        }
        else
        {
            if(relation == And) cstr += QString("\"relation\": \"and\",");
            if(relation == Or)  cstr += QString("\"relation\": \"or\",");
        }
        cstr += QString("\"type\": \"sub\",");
        cstr += QString("\"subcond\": [");
        QList<HSqlBuilderCondition>::iterator ci = sub_conds.begin();
        QString cas = "";
        while(ci != sub_conds.end())
        {
            QString s = ci->json_string();
            if(!s.isEmpty())
            {
                if(!cas.isEmpty())
                    cas += ",";
                cas += s;
            }
            ++ci;
        }
        cstr += cas + "]}";
        return cstr;
    }

    cstr += "{";
    if(ct == COND_FF)  cstr += QString("\"type\": \"ff\",");
    if(ct == COND_FV)  cstr += QString("\"type\": \"fv\",");
    if(ct == COND_FE)  cstr += QString("\"type\": \"fe\",");
    if(ct == COND_FB)  cstr += QString("\"type\": \"fb\",");
    if(ct == COND_SPEC_F)  cstr += QString("\"type\": \"spec_f\",");
    if(ct == COND_SPEC_V)  cstr += QString("\"type\": \"spec_v\",");
    if(ct == COND_SPEC_VF) cstr += QString("\"type\": \"spec_vf\",");

    if(vt == Quoted)   cstr += QString("\"valtype\": \"quoted\",");
    if(vt == Unquoted) cstr += QString("\"valtype\": \"unquoted\",");

    if(ct == COND_SPEC_F || ct == COND_SPEC_V || ct == COND_SPEC_VF)
        cstr += QString("\"template\": \"%1\",").arg(cname);
    else
        cstr += QString("\"op\": \"%1\",").arg(op);

    cstr += QString("\"value\": \"%1\",").arg(strToJsonStr(value_expression));
    cstr += QString("\"field1\": \"%1\",").arg(field1);
    cstr += QString("\"table1\": \"%1\",").arg(table1);
    cstr += QString("\"field2\": \"%1\",").arg(field2);
    cstr += QString("\"table2\": \"%1\",").arg(table2);
    cstr += QString("\"options\": \"%1\"").arg(options);
    cstr += "}";
    return cstr;
}

QString HSqlBuilderCondition::json_string_top(void)
{
    QString cstr = "";
    if(ct == COND_ZZ)
        return cstr;

    if(ct == COND_GRP)
    {
        cstr += "[";
        QList<HSqlBuilderCondition>::iterator ci = sub_conds.begin();
        QString cas = "";
        while(ci != sub_conds.end())
        {
            QString s = ci->json_string();
            if(!s.isEmpty())
            {
                if(!cas.isEmpty())
                    cas += ",";
                cas += s;
            }
            ++ci;
        }
        cstr += cas + "]";
        return cstr;
    }
    return cstr;
}

// HSqlBuilderJoin ///////////////////////////////////////////////////////////////

HSqlBuilderJoin::HSqlBuilderJoin()
{
    empty = true;
    jtype = Inner;
    totable = "";
    toalias = "";
    jcond = HSqlBuilderCondition(And);
}

HSqlBuilderJoin::~HSqlBuilderJoin()
{
}

HSqlBuilderJoin& HSqlBuilderJoin::join(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond)
{
    empty = false;
    jtype = Inner;
    totable = toTable;
    toalias = toAlias;
    jcond = joinCond;
    return *this;
}

HSqlBuilderJoin& HSqlBuilderJoin::join_opt(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond)
{
    empty = false;
    jtype = LeftOuter;
    totable = toTable;
    toalias = toAlias;
    jcond = joinCond;
    return *this;
}

bool HSqlBuilderJoin::isEmpty()
{
    return empty;
}

QString HSqlBuilderJoin::local_cmd(HSqlBuilder *builder,bool vmm,QString dialect)
{
    if(empty)
        return "";

    QString jstr = "";
    QString jcondstr = "";
    if(jtype == Inner)
        jstr += QString("INNER JOIN ");
    if(jtype == LeftOuter)
        jstr += QString("LEFT OUTER JOIN ");
    jstr += totable;
    if(!toalias.isEmpty())
        jstr += QString(" AS %1").arg(toalias);
    jcondstr = jcond.local_cmd(builder,true,vmm,dialect);
    if(!jcondstr.isEmpty())
        jstr += QString(" ON %1").arg(jcondstr);
    return jstr;
}

QString HSqlBuilderJoin::json_string(void)
{
    QString joinstring = "";

    if(!jcond.isGrpCond() || empty)
        return joinstring;

    joinstring += "{";
    if(jtype == Inner)
        joinstring += QString("\"type\": \"inner\",");
    if(jtype == LeftOuter)
        joinstring += QString("\"type\": \"leftouter\",");

    joinstring += QString("\"jointable\": \"%1\",").arg(totable);
    joinstring += QString("\"jointablealias\": \"%1\",").arg(toalias);

    if(jcond.topRelation() == And)
        joinstring += QString("\"condtoprel\": \"and\",");
    if(jcond.topRelation() == Or)
        joinstring += QString("\"condtoprel\": \"or\",");

    joinstring += QString("\"cond\": %1").arg(jcond.json_string_top());
    joinstring += "}";
    return joinstring;
}

// HSqlBuilderSort ///////////////////////////////////////////////////////////////

HSqlBuilderSort::HSqlBuilderSort()
{
    empty = true;
    field = "";
    table = "";
    options = "";
}

HSqlBuilderSort::~HSqlBuilderSort()
{
}

void HSqlBuilderSort::set(const QString fieldName,const QString optionsString)
{
    empty = false;
    field = fieldName;
    options = optionsString;
}

void HSqlBuilderSort::set(const QString fieldSpec[2],const QString optionsString)
{
    empty = false;
    table = fieldSpec[0];
    field = fieldSpec[1];
    options = optionsString;
}

void HSqlBuilderSort::set(const QString tableName,const QString fieldName,const QString optionsString)
{
    empty = false;
    table = tableName;
    field = fieldName;
    options = optionsString;
}

QString HSqlBuilderSort::local_cmd(QString dialect)
{
    Q_UNUSED(dialect)

    if(empty)
        return "";

    QString str = "";
    QMap<QString,QString> opts = HSqlBuilder::genOptions(options);

    QString f = (table.isEmpty() ? "" : table + ".") + field;
    if(opts.value("function","").isEmpty())
        str = f;
    else
        str = QString("%1(%2%3%4%5%6%7%8)")
                .arg(opts.value("function"))
                .arg(opts.value("more_args_before",""))
                .arg(opts.value("more_args_before","").isEmpty() ? "":",")
                .arg(f)
                .arg(opts.value("more_args","").isEmpty() ? "":",")
                .arg(opts.value("more_args",""))
                .arg(opts.value("more_args_after","").isEmpty() ? "":",")
                .arg(opts.value("more_args_after",""));

    if(opts.value("direction","") == "REVERSE")
        str.append(" DESC");
    return str;
}

QString HSqlBuilderSort::json_string(void)
{
    QString sstr;

    if(empty)
        return "";

    sstr += "{";
    sstr += QString("\"field\": \"%1\",").arg(field);
    sstr += QString("\"table\": \"%1\",").arg(table);
    sstr += QString("\"options\": \"%1\"").arg(options);
    sstr += "}";
    return sstr;
}

bool HSqlBuilderSort::isEmpty()
{
    return empty;
}

// HSqlBuilder //////////////////////////////////////////////////////////////////

QString HSqlBuilder::translateDialect(QString subjectString,QString dialect)
{
    int d_idx = 0,p_idx = 0;
    while(true)
    {
        if(strlen(dialectTableDialects[d_idx]) == 0)
            return subjectString;
        if(dialect == dialectTableDialects[d_idx])
            break;
        ++d_idx;
    }

    while(true)
    {
        if(strlen(dialectTablePuzzles[p_idx]) == 0)
            return subjectString;
        if(subjectString == dialectTablePuzzles[p_idx])
            break;
        ++p_idx;
    }
    return dialectTableResolve[p_idx][d_idx];
}

HSqlBuilder::HSqlBuilder(HSqlBuilder_QueryType qtype,QString tablename,QString alias)
{
    type = qtype;
    base_table = tablename;
    base_alias = alias;
    count_alias = "";
    count_field = "";
    count_table = "";
    limitquery = 0;
    condition = HSqlBuilderCondition(And);
    forBind.clear();
    jsonExModeReq = UnspecifiedAuto;
}

HSqlBuilder::~HSqlBuilder()
{
}

void HSqlBuilder::add_special_cond_template(QString id,QString templ)
{
    spec_conds[id] = templ;
}

QString HSqlBuilder::special_cond_template(QString id)
{
    return spec_conds.value(id,"");
}


QMap<QString,QString> HSqlBuilder::genOptions(QString options)
{
    int i;
    QMap<QString,QString> m;
    QStringList opts = options.split(";",Qt::SkipEmptyParts);
    for(i = 0 ; i < opts.count() ; ++i)
    {
        QStringList o2 = opts.at(i).split("=");
        if(o2.count() == 2)
            m[o2[0]] = o2[1];
    }
    return m;
}

QString HSqlBuilder::addValueForBind(QVariant v)
{
    QString pholder;
    int index = forBind.count();
    ++index;
    pholder = QString(":ph_%1_pos").arg(index);
    forBind[pholder] = v;
    return pholder;
}

QMap<QString,QVariant> HSqlBuilder::bind_array(void)
{
    return forBind;
}

QStringList HSqlBuilder::query_field_list(void)
{
    QStringList lst;
    lst.clear();

    QList<HSqlBuilderField>::iterator fi = field_list.begin();
    while(fi != field_list.end())
    {
        if(!fi->isGetType())
        {
            ++fi;
            continue;
        }
        lst.push_back(fi->getVisibleName());
        ++fi;
    }
    return lst;
}

HSqlBuilder& HSqlBuilder::countingField(QString alias,QString field,QString table)
{
    count_alias = alias;
    count_field = field;
    count_table = table;
    return *this;
}

HSqlBuilder& HSqlBuilder::get(const QString fieldName,const QString aliasName,const QString optionsString)
{
    get_withtable("",fieldName,aliasName,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::get(const QString fieldSpec[2],const QString aliasName,const QString optionsString)
{
    get_withtable(fieldSpec[0],fieldSpec[1],aliasName,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::get_withtable(const QString tableName,const QString fieldName,const QString aliasName,const QString optionsString)
{
    HSqlBuilderField f;
    f.get_withtable(tableName,fieldName,aliasName,optionsString);
    field_list.push_back(f);
    return *this;
}

HSqlBuilder& HSqlBuilder::get_a(const QStringList fields,QString table)
{
    int i;
    for(i=0;i<fields.count();++i)
        get_withtable(table,fields.at(i));
    return *this;
}

HSqlBuilder& HSqlBuilder::get(HSqlBuilderField f)
{
    if(!f.isGetType())
        return *this;
    field_list.push_back(f);
    return *this;
}

HSqlBuilder& HSqlBuilder::set_fv(const QString fieldName,HSqlBuilder_FieldValueType vType,QVariant value,const QString optionsString)
{
    HSqlBuilderField f;
    f.set_fv(fieldName,vType,value,optionsString);
    field_list.push_back(f);
    return *this;
}

HSqlBuilder& HSqlBuilder::set_fe(const QString fieldName,const QString expression,const QString optionsString)
{
    HSqlBuilderField f;
    f.set_fe(fieldName,expression,optionsString);
    field_list.push_back(f);
    return *this;
}
HSqlBuilder& HSqlBuilder::set_fv_a(QMap<QString,QVariant> toSet,HSqlBuilder_FieldValueType vType)
{
    QMap<QString,QVariant>::iterator i;
    for(i = toSet.begin() ; i != toSet.end() ; ++i)
        set_fv(i.key(),vType,i.value());
    return *this;
}

HSqlBuilder& HSqlBuilder::set(HSqlBuilderField f)
{
    if(!f.isSetType())
        return *this;
    field_list.push_back(f);
    return *this;
}

HSqlBuilder& HSqlBuilder::join(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond)
{
    HSqlBuilderJoin j;
    j.join(toTable,toAlias,joinCond);
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join_opt(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond)
{
    HSqlBuilderJoin j;
    j.join_opt(toTable,toAlias,joinCond);
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join_ffe(const QString toTable,const QString toAlias,const QString fieldName1,const QString fieldName2)
{
    HSqlBuilderJoin j;
    j.join(toTable,toAlias,HSqlBuilderCondition().ff(fieldName1,fieldName2,"="));
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join_ffe(const QString toTable,const QString toAlias,const QString fieldSpec1[2],const QString fieldSpec2[2])
{
    HSqlBuilderJoin j;
    j.join(toTable,toAlias,HSqlBuilderCondition().ff(fieldSpec1,fieldSpec2,"="));
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join_ffe(const QString toTable,const QString toAlias,const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2)
{
    HSqlBuilderJoin j;
    j.join(toTable,toAlias,HSqlBuilderCondition().ff(tableName1,fieldName1,tableName2,fieldName2,"="));
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join_opt_ffe(const QString toTable,const QString toAlias,const QString fieldName1,const QString fieldName2)
{
    HSqlBuilderJoin j;
    j.join_opt(toTable,toAlias,HSqlBuilderCondition().ff(fieldName1,fieldName2,"="));
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join_opt_ffe(const QString toTable,const QString toAlias,const QString fieldSpec1[2],const QString fieldSpec2[2])
{
    HSqlBuilderJoin j;
    j.join_opt(toTable,toAlias,HSqlBuilderCondition().ff(fieldSpec1,fieldSpec2,"="));
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join_opt_ffe(const QString toTable,const QString toAlias,const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2)
{
    HSqlBuilderJoin j;
    j.join_opt(toTable,toAlias,HSqlBuilderCondition().ff(tableName1,fieldName1,tableName2,fieldName2,"="));
    join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::join(HSqlBuilderJoin j)
{
    if(!j.isEmpty())
        join_list.push_back(j);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_ff(const QString fieldName1,const QString fieldName2,const QString operation,const QString optionsString)
{
    condition.ff(fieldName1,fieldName2,operation,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_ff(const QString fieldSpec1[2],const QString fieldSpec2[2],const QString operation,const QString optionsString)
{
    condition.ff(fieldSpec1,fieldSpec2,operation,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_ff(const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2,const QString operation,const QString optionsString)
{
    condition.ff(tableName1,fieldName1,tableName2,fieldName2,operation,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_fv(const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString)
{
    condition.fv(fieldName,vType,value,operation,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_fv(const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString)
{
    condition.fv(fieldSpec,vType,value,operation,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_fe(const QString fieldName,const QString expression,const QString operation,const QString optionsString)
{
    condition.fe(fieldName,expression,operation,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_fe(const QString fieldSpec[2],const QString expression,const QString operation,const QString optionsString)
{
    condition.fe(fieldSpec,expression,operation,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_fb(const QString fieldName,const QString optionsString)
{
    condition.fb(fieldName,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_fb(const QString fieldSpec[2],const QString optionsString)
{
    condition.fb(fieldSpec,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_spec_f(const QString condSpec,const QString fieldName,const QString optionsString)
{
    condition.spec_f(condSpec,fieldName,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_spec_f(const QString condSpec,const QString fieldSpec[2],const QString optionsString)
{
    condition.spec_f(condSpec,fieldSpec,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_spec_v(const QString condSpec,const HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString)
{
    condition.spec_v(condSpec,vType,value,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_spec_fv(const QString condSpec,const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString)
{
    condition.spec_fv(condSpec,fieldName,vType,value,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond_spec_fv(const QString condSpec,const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString)
{
    condition.spec_fv(condSpec,fieldSpec,vType,value,optionsString);
    return *this;
}

HSqlBuilder& HSqlBuilder::cond(HSqlBuilderCondition c)
{
    if(!c.isEmpty())
        condition.add(c);
    return *this;
}

HSqlBuilder& HSqlBuilder::sort(const QString fieldName,const QString optionsString)
{
    HSqlBuilderSort s;
    s.set(fieldName,optionsString);
    sort_list.push_back(s);
    return *this;
}

HSqlBuilder& HSqlBuilder::sort(const QString fieldSpec[2],const QString optionsString)
{
    HSqlBuilderSort s;
    s.set(fieldSpec,optionsString);
    sort_list.push_back(s);
    return *this;
}

HSqlBuilder& HSqlBuilder::sort(const QString tableName,const QString fieldName,const QString optionsString)
{
    HSqlBuilderSort s;
    s.set(tableName,fieldName,optionsString);
    sort_list.push_back(s);
    return *this;
}

HSqlBuilder& HSqlBuilder::sort(HSqlBuilderSort s)
{
    if(!s.isEmpty())
        sort_list.push_back(s);
    return *this;
}

HSqlBuilder& HSqlBuilder::length(int limit)
{
    limitquery = limit;
    return *this;
}

HSqlBuilder& HSqlBuilder::setJsonExecutionMode(HSqlBuilder_JsonExecutionModeRequest exm)
{
    jsonExModeReq = exm;
    return *this;
}

QString HSqlBuilder::local_cmd(bool vmm,bool nice,QString dialect)
{
    if(type == Select)
        return local_cmd_Select(vmm,nice,dialect);
    if(type == Insert)
        return local_cmd_Insert(vmm,nice,dialect);
    if(type == Update)
        return local_cmd_Update(vmm,nice,dialect);
    if(type == Delete)
        return local_cmd_Delete(vmm,nice,dialect);
    return QString();
}

QString HSqlBuilder::local_cmd_Select(bool vmm,bool nice,QString dialect)
{
    QString sql = "";
    forBind.clear();
    if(type != Select)
        return "";

    sql = QString("SELECT ");
    if(nice)
        sql += "\n";

    if(!count_field.isEmpty())
    {
        sql += "COUNT(";
        sql += count_table.isEmpty() ? count_field : (count_table + "." + count_field);
        sql += ")";
        if(count_alias.isEmpty())
            count_alias = "count";
        sql += " AS " + count_alias;
    }
    else
    {
        int fc = 0;
        QList<HSqlBuilderField>::iterator fi = field_list.begin();
        while(fi != field_list.end())
        {
            sql += (fc == 0 ? "":",") + fi->local_cmd_Get();
            ++fc;
            ++fi;
        }
    }

    sql += " ";
    if(nice)
        sql += "\n";
    sql += "FROM " + base_table;
    if(!base_alias.isEmpty())
        sql += " AS " + base_alias;

    sql += local_cmd_JoinPart(vmm,nice,dialect);

    QString condPart = condition.local_cmd(this,true,vmm,dialect);
    if(!condPart.isEmpty())
    {
        sql += " ";
        if(nice)
            sql += "\n";
        sql += "WHERE ";
        sql += condPart;
    }
    sql += local_cmd_SortingPart(vmm,nice,dialect);

    if(limitquery > 0)
    {
        sql += " ";
        if(nice)
            sql += "\n";
        sql += QString("LIMIT %1").arg(limitquery);
    }
    return sql;
}

QString HSqlBuilder::local_cmd_JoinPart(bool vmm,bool nice,QString dialect)
{
    QString jstr = "";
    QList<HSqlBuilderJoin>::iterator ji = join_list.begin();
    while(ji != join_list.end())
    {
        QString toTableName;
        HSqlBuilderJoin j = *ji;
        jstr += " ";
        if(nice)
            jstr += "\n";
        jstr += ji->local_cmd(this,vmm,dialect);
        ++ji;
    }
    return jstr;
}

QString HSqlBuilder::local_cmd_SortingPart(bool vmm,bool nice,QString dialect)
{
    Q_UNUSED(vmm)
    QString sql = "";
    int sc = 0;
    QList<HSqlBuilderSort>::iterator si = sort_list.begin();
    while(si != sort_list.end())
    {
        if(sc != 0)
            sql += ",";
        else
        {
            sql += " ";
            if(nice)
               sql += "\n";
            sql += "ORDER BY ";
        }

        sql += si->local_cmd(dialect);

        ++sc;
        ++si;
    }
    return sql;
}

QString HSqlBuilder::local_cmd_Insert(bool vmm,bool nice,QString dialect)
{
    QString sql = "";
    forBind.clear();
    if(type != Insert)
        return "";

    int fc = 0;
    QString keypart = "",valpart = "";
    QList<HSqlBuilderField>::iterator fi = field_list.begin();
    while(fi != field_list.end())
    {
        keypart += (fc == 0 ? "":",") + fi->local_cmd_Key();
        valpart += (fc == 0 ? "":",") + fi->local_cmd_Val(this,vmm,dialect);
        ++fc;
        ++fi;
    }

    sql = QString("INSERT INTO ") + base_table + "(" + keypart + ") ";
    if(nice)
        sql += "\n";
    sql += "VALUES(" + valpart + ")";
    return sql;
}

QString HSqlBuilder::local_cmd_Update(bool vmm,bool nice,QString dialect)
{
    QString sql = "";
    forBind.clear();
    if(type != Update)
        return "";

    sql = QString("UPDATE ") + base_table + " SET ";
    if(nice)
        sql += "\n";

    int fc = 0;
    QList<HSqlBuilderField>::iterator fi = field_list.begin();
    while(fi != field_list.end())
    {
        sql += (fc == 0 ? "":",");
        sql += fi->local_cmd_Key() + "=" + fi->local_cmd_Val(this,vmm,dialect);
        ++fc;
        ++fi;
    }

    QString condPart = condition.local_cmd(this,true,vmm,dialect);
    if(!condPart.isEmpty())
    {
        sql += " ";
        if(nice)
            sql += "\n";
        sql += "WHERE ";
        sql += condPart;
    }
    return sql;
}

QString HSqlBuilder::local_cmd_Delete(bool vmm,bool nice,QString dialect)
{
    QString sql = "";
    forBind.clear();
    if(type != Delete)
        return "";

    sql = QString("DELETE FROM ") + base_table;
    if(nice)
        sql += "\n";

    QString condPart = condition.local_cmd(this,true,vmm,dialect);
    if(!condPart.isEmpty())
    {
        sql += " ";
        if(nice)
            sql += "\n";
        sql += "WHERE ";
        sql += condPart;
    }
    return sql;
}

QString HSqlBuilder::json_string(void)
{
    if(type == Select)
        return json_string_Select();
    if(type == Insert)
        return json_string_Insert();
    if(type == Update)
        return json_string_Update();
    if(type == Delete)
        return json_string_Delete();
    return "";
}

HSqlBuilder_JsonExecutionModeRequest HSqlBuilder::getJsonExecutionMode(void)
{
    return jsonExModeReq;
}

QString HSqlBuilder::json_string_ExecutionModeReqStr(void)
{
    QString m = "auto";
    if(jsonExModeReq == DryRun)
        m = "dryrun";
    if(jsonExModeReq == NoReturn)
        m = "noreturn";
    if(jsonExModeReq == SingleReturn)
        m = "single";
    if(jsonExModeReq == TableReturn)
        m = "table";
    if(jsonExModeReq == UnspecifiedAuto)
    {
        if(type == Insert || type == Update || type == Delete)
            m = "noreturn";
        if(type == Select)
        {
            if(query_field_list().count() > 1)
                m = "table";
            else
                m = "single";
        }
    }
    return QString("\"return\": \"%1\",").arg(m);
}

QString HSqlBuilder::json_string_Select(void)
{
    QString json = "";
    if(type != Select)
        return "";

    QString fieldstring = json_string_FieldPart();
    QString joinstring  = json_string_JoinPart();
    QString sortstring  = json_string_SortingPart();

    QString condstring  = condition.json_string_top();

    json = QString("{");
    json += QString("\"type\": \"select\",");
    json += json_string_ExecutionModeReqStr();
    json += QString("\"table_name\": \"%1\",").arg(base_table);
    json += QString("\"table_alias\": \"%1\",").arg(base_alias);
    json += QString("\"fields\": [%1],").arg(fieldstring);
    json += QString("\"joins\": [%1],").arg(joinstring);
    json += QString("\"conditions\": %1,").arg(condstring);
    json += QString("\"sort\": [%1],").arg(sortstring);
    json += QString("\"limit\": \"%1\",").arg(limitquery);
    json += QString("\"countfield\": \"%1\",").arg(count_field);
    json += QString("\"counttable\": \"%1\",").arg(count_table);
    json += QString("\"countalias\": \"%1\"").arg(count_alias);

    json += QString("}");
    return json;
}

QString HSqlBuilder::json_string_Insert(void)
{
    QString json = "";
    if(type != Insert)
        return "";

    QString fieldstring = json_string_FieldPart();

    json = QString("{");
    json += QString("\"type\": \"insert\",");
    json += json_string_ExecutionModeReqStr();
    json += QString("\"table_name\": \"%1\",").arg(base_table);
    json += QString("\"fields\": [%1]").arg(fieldstring);
    json += QString("}");
    return json;
}

QString HSqlBuilder::json_string_Update(void)
{
    QString json = "";
    if(type != Update)
        return "";

    QString fieldstring = json_string_FieldPart();
    QString condstring  = condition.json_string_top();

    json = QString("{");
    json += QString("\"type\": \"update\",");
    json += json_string_ExecutionModeReqStr();
    json += QString("\"table_name\": \"%1\",").arg(base_table);
    json += QString("\"fields\": [%1],").arg(fieldstring);
    json += QString("\"conditions\": %1").arg(condstring);
    json += QString("}");
    return json;
}

QString HSqlBuilder::json_string_Delete(void)
{
    QString json = "";
    if(type != Delete)
        return "";

    QString fieldstring = json_string_FieldPart();
    QString condstring  = condition.json_string_top();

    json = QString("{");
    json += QString("\"type\": \"delete\",");
    json += json_string_ExecutionModeReqStr();
    json += QString("\"table_name\": \"%1\",").arg(base_table);
    json += QString("\"conditions\": %1").arg(condstring);
    json += QString("}");
    return json;
}

QString HSqlBuilder::json_string_FieldPart(void)
{
    QString fieldstr = "";
    QList<HSqlBuilderField>::iterator fi = field_list.begin();
    while(fi != field_list.end())
    {
        if(!fieldstr.isEmpty())
            fieldstr += ",";
        fieldstr += fi->json_string();
        ++fi;
    }
    return fieldstr;
}

QString HSqlBuilder::json_string_JoinPart(void)
{
    QString joinstring = "";
    QList<HSqlBuilderJoin>::iterator ji = join_list.begin();
    while(ji != join_list.end())
    {
        if(!joinstring.isEmpty())
            joinstring += ",";
        joinstring += ji->json_string();
        ++ji;
    }
    return joinstring;
}

QString HSqlBuilder::json_string_SortingPart(void)
{
    int sc = 0;
    QString sstr = "";
    QList<HSqlBuilderSort>::iterator si = sort_list.begin();

    while(si != sort_list.end())
    {
        if(!sstr.isEmpty())
            sstr += ",";
        sstr += si->json_string();
        ++sc;
        ++si;
    }
    return sstr;
}

QString strToJsonStr(QString s)
{
    return s.replace("\\","\\\\")
            .replace("\n","\\n")
            .replace("\"","\\\"");
}

//end code.
