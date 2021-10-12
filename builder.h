/*
   SAFE - LIB - C++ Sql Builder
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2021 Peter Deak (hyper80@gmail.com)
*/

#ifndef QUERY_BUILDER_H
#define QUERY_BUILDER_H

#include <QtCore>

enum HSqlBuilder_QueryType
{
    Select = 1,
    Insert = 2,
    Update = 3,
    Delete = 4
};

enum HSqlBuilder_FieldValueType {
    Unquoted = 1,
    Quoted   = 2
};

enum HSqlBuilder_JoinType {
    Inner     = 1,
    LeftOuter = 2
};

enum HSqlBuilder_ConditionRelation {
    And = 1,
    Or  = 2
};

enum HSqlBuilder_JsonExecutionModeRequest {
    UnspecifiedAuto  = 0,
    DryRun           = 1,
    NoReturn         = 2,
    SingleReturn     = 3,
    TableReturn      = 4
};

class HSqlBuilder;
class HSqlBuilderField;
class HSqlBuilderCondition;
class HSqlBuilderJoin;
class HSqlBuilderSort;

class HSqlBuilderField
{
public:
    HSqlBuilderField();
    ~HSqlBuilderField();

    HSqlBuilderField& get(const QString fieldName,const QString aliasName = "",const QString optionsString = "");
    HSqlBuilderField& get(const QString fieldSpec[2],const QString aliasName = "",const QString optionsString = "");
    HSqlBuilderField& get_withtable(const QString tableName,const QString fieldName,const QString aliasName = "",const QString optionsString = "");
    HSqlBuilderField& set_fv(const QString fieldName,HSqlBuilder_FieldValueType vType,QVariant value,const QString optionsString = "");
    HSqlBuilderField& set_fe(const QString fieldName,const QString expression,const QString optionsString = "");

    bool isGetType(void);
    bool isSetType(void);
    QString getVisibleName(void);

    QString local_cmd_Get(void);
    QString local_cmd_Key(void);
    QString local_cmd_Val(HSqlBuilder *builder,bool vmm);
    QString json_string(void);

private:
    int type;
    QString table;
    QString name;
    QString alias;
    HSqlBuilder_FieldValueType vt;
    QVariant value_expression;
    QString options;
};

class HSqlBuilderCondition
{
public:
    HSqlBuilderCondition(HSqlBuilder_ConditionRelation r = And);
    ~HSqlBuilderCondition();

    HSqlBuilderCondition& ff(const QString fieldName1,const QString fieldName2,const QString operation,const QString optionsString = "");
    HSqlBuilderCondition& ff(const QString fieldSpec1[2],const QString fieldSpec2[2],const QString operation,const QString optionsString = "");
    HSqlBuilderCondition& ff(const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2,const QString operation,const QString optionsString = "");
    HSqlBuilderCondition& fv(const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString = "");
    HSqlBuilderCondition& fv(const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString = "");
    HSqlBuilderCondition& fe(const QString fieldName,const QString expression,const QString operation,const QString optionsString = "");
    HSqlBuilderCondition& fe(const QString fieldSpec[2],const QString expression,const QString operation,const QString optionsString = "");
    HSqlBuilderCondition& fb(const QString fieldName,const QString optionsString = "");
    HSqlBuilderCondition& fb(const QString fieldSpec[2],const QString optionsString = "");
    HSqlBuilderCondition& spec_f(const QString condSpec,const QString fieldName,const QString optionsString = "");
    HSqlBuilderCondition& spec_f(const QString condSpec,const QString fieldSpec[2],const QString optionsString = "");
    HSqlBuilderCondition& spec_v(const QString condSpec,const HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString = "");
    HSqlBuilderCondition& spec_fv(const QString condSpec,const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString = "");
    HSqlBuilderCondition& spec_fv(const QString condSpec,const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString = "");
    HSqlBuilderCondition& add(HSqlBuilderCondition cond);

    bool isGrpCond(void);
    HSqlBuilder_ConditionRelation topRelation(void);
    QString local_cmd(HSqlBuilder *builder,bool top,bool vmm);
    QString json_string(void);
    QString json_string_top(void);

private:
    int ct;
    HSqlBuilder_ConditionRelation relation;
    HSqlBuilder_FieldValueType vt;
    QString cname;
    QString value_expression;
    QString op;
    QString field1,table1,field2,table2;
    QString options;
    QList<HSqlBuilderCondition> sub_conds;
};

class HSqlBuilderSort
{
public:
    HSqlBuilderSort();
    ~HSqlBuilderSort();

    void set(const QString fieldName,const QString optionsString = "");
    void set(const QString fieldSpec[2],const QString optionsString = "");
    void set(const QString tableName,const QString fieldName,const QString optionsString);

    QString local_cmd(void);
    QString json_string(void);

private:
    QString field;
    QString table;
    QString options;
};

class HSqlBuilderJoin
{
public:
    HSqlBuilderJoin();
    ~HSqlBuilderJoin();

    HSqlBuilderJoin& join(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond);
    HSqlBuilderJoin& join_opt(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond);

    QString local_cmd(HSqlBuilder *builder,bool vmm);
    QString json_string(void);

private:
    HSqlBuilder_JoinType jtype;
    QString totable,toalias;
    HSqlBuilderCondition jcond;
};

class HSqlBuilder
{

public:
    HSqlBuilder(HSqlBuilder_QueryType qtype,QString tablename,QString alias = "");
    ~HSqlBuilder();

    void countingField(QString alias = "count",QString field = "*",QString table="");

    HSqlBuilder& get(const QString fieldName,const QString aliasName = "",const QString optionsString = "");
    HSqlBuilder& get(const QString fieldSpec[2],const QString aliasName = "",const QString optionsString = "");
    HSqlBuilder& get_withtable(const QString tableName,const QString fieldName,const QString aliasName = "",const QString optionsString = "");
    HSqlBuilder& get_a(const QStringList fields,QString table = "");
    HSqlBuilder& get(HSqlBuilderField f);

    HSqlBuilder& set_fv(const QString fieldName,HSqlBuilder_FieldValueType vType,QVariant value,const QString optionsString = "");
    HSqlBuilder& set_fe(const QString fieldName,const QString expression,const QString optionsString = "");
    HSqlBuilder& set_fv_a(QMap<QString,QVariant> toSet,HSqlBuilder_FieldValueType vType = Quoted);
    HSqlBuilder& set(HSqlBuilderField f);

    HSqlBuilder& join(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond);
    HSqlBuilder& join_opt(const QString toTable,const QString toAlias,HSqlBuilderCondition joinCond);
    HSqlBuilder& join_ffe(const QString toTable,const QString toAlias,const QString fieldName1,const QString fieldName2);
    HSqlBuilder& join_ffe(const QString toTable,const QString toAlias,const QString fieldSpec1[2],const QString fieldSpec2[2]);
    HSqlBuilder& join_ffe(const QString toTable,const QString toAlias,const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2);
    HSqlBuilder& join_opt_ffe(const QString toTable,const QString toAlias,const QString fieldName1,const QString fieldName2);
    HSqlBuilder& join_opt_ffe(const QString toTable,const QString toAlias,const QString fieldSpec1[2],const QString fieldSpec2[2]);
    HSqlBuilder& join_opt_ffe(const QString toTable,const QString toAlias,const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2);

    HSqlBuilder& join(HSqlBuilderJoin j);

    HSqlBuilder& cond_ff(const QString fieldName1,const QString fieldName2,const QString operation,const QString optionsString = "");
    HSqlBuilder& cond_ff(const QString fieldSpec1[2],const QString fieldSpec2[2],const QString operation,const QString optionsString = "");
    HSqlBuilder& cond_ff(const QString tableName1,const QString fieldName1,const QString tableName2,const QString fieldName2,const QString operation,const QString optionsString = "");
    HSqlBuilder& cond_fv(const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString = "");
    HSqlBuilder& cond_fv(const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString operation,const QString optionsString = "");
    HSqlBuilder& cond_fe(const QString fieldName,const QString expression,const QString operation,const QString optionsString = "");
    HSqlBuilder& cond_fe(const QString fieldSpec[2],const QString expression,const QString operation,const QString optionsString = "");
    HSqlBuilder& cond_fb(const QString fieldName,const QString optionsString = "");
    HSqlBuilder& cond_fb(const QString fieldSpec[2],const QString optionsString = "");
    HSqlBuilder& cond_spec_f(const QString condSpec,const QString fieldName,const QString optionsString = "");
    HSqlBuilder& cond_spec_f(const QString condSpec,const QString fieldSpec[2],const QString optionsString = "");
    HSqlBuilder& cond_spec_v(const QString condSpec,const HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString = "");
    HSqlBuilder& cond_spec_fv(const QString condSpec,const QString fieldName,HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString = "");
    HSqlBuilder& cond_spec_fv(const QString condSpec,const QString fieldSpec[2],HSqlBuilder_FieldValueType vType,const QString value,const QString optionsString = "");
    HSqlBuilder& cond(HSqlBuilderCondition c);

    HSqlBuilder& sort(const QString fieldName,const QString optionsString = "");
    HSqlBuilder& sort(const QString fieldSpec[2],const QString optionsString = "");
    HSqlBuilder& sort(const QString tableName,const QString fieldName,const QString optionsString);
    HSqlBuilder& sort(HSqlBuilderSort s);

    HSqlBuilder& length(int limit);

    HSqlBuilder& setJsonExecutionMode(HSqlBuilder_JsonExecutionModeRequest exm);

    QString local_cmd(bool vmm = false,bool nice = false);
    QString json_string(void);

    QMap<QString,QVariant> bind_array(void);
    QString addValueForBind(QVariant v);

    QStringList query_field_list(void);

    static void add_special_cond_template(QString id,QString templ);
    static QString special_cond_template(QString id);
    static QMap<QString,QString> genOptions(QString options);

private:
    HSqlBuilder_QueryType type;
    QString base_table;
    QString base_alias;
    QString count_alias,count_field,count_table;
    HSqlBuilderCondition    condition;
    QList<HSqlBuilderField> field_list;
    QList<HSqlBuilderJoin>  join_list;
    QList<HSqlBuilderSort>  sort_list;
    int limitquery;
    HSqlBuilder_JsonExecutionModeRequest jsonExModeReq;

    QString local_cmd_Select(bool vmm = false,bool nice = false);
    QString local_cmd_Insert(bool vmm = false,bool nice = false);
    QString local_cmd_Update(bool vmm = false,bool nice = false);
    QString local_cmd_Delete(bool vmm = false,bool nice = false);
    QString local_cmd_JoinPart(bool vmm = false,bool nice = false);
    QString local_cmd_SortingPart(bool vmm = false,bool nice = false);

    QString json_string_Select(void);
    QString json_string_Insert(void);
    QString json_string_Update(void);
    QString json_string_Delete(void);
    QString json_string_FieldPart(void);
    QString json_string_JoinPart(void);
    QString json_string_SortingPart(void);
    QString json_string_ExecutionModeReqStr(void);

    QMap<QString,QVariant> forBind;
    static QMap<QString,QString> spec_conds;
};

HSqlBuilderCondition cond(HSqlBuilder_ConditionRelation r = And);
HSqlBuilder db_query (QString tablename,QString alias = "");
HSqlBuilder db_insert(QString tablename,QString alias = "");
HSqlBuilder db_update(QString tablename,QString alias = "");
HSqlBuilder db_delete(QString tablename,QString alias = "");

#endif // QUERY_BUILDER_H
