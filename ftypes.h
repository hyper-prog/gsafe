/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    ftypes.h
*/

#ifndef GSAFE__FTYPES_HEADER_FILE_X_
#define GSAFE__FTYPES_HEADER_FILE_X_

#include <QtCore>

#include <builder.h>
#include <dm.h>

/*  @{  */

/** A static field type */
class HStaticField : public HField
{
    Q_OBJECT

public:
    HStaticField(QString sqlname,QString description,QString title);
    ~HStaticField();

    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();
    virtual bool isUpdateRequired(bool forRealUpdate = true);

    virtual void putsOnSetter(HSqlBuilder *b,QString tableName = QString());
    virtual void putsOnGetter(HSqlBuilder *b,QString tableName = QString());
};

/** A string record primary key type */
class HSKeyField : public HField
{
    Q_OBJECT

public:
    HSKeyField(QString sqlname,QString description,QString title);
    ~HSKeyField();

    virtual QString className();
    virtual bool isKey();
    virtual HSqlBuilder_FieldValueType fvType();
    virtual bool isUpdateRequired(bool forRealUpdate = true);

    virtual QString convertToDisplay(QString fv);
};

/** A numeric record primary key type */
class HNKeyField : public HField
{
    Q_OBJECT

public:
    HNKeyField(QString sqlname,QString description,QString title);
    ~HNKeyField();

    virtual QString className();
    virtual bool isKey();
    virtual HSqlBuilder_FieldValueType fvType();
    virtual bool isUpdateRequired(bool forRealUpdate = true);

    virtual QString convertToDisplay(QString fv);
};

/** A small textual field *
 *  Attributes:
 *   "password_mode" = "yes"
 *   "gui_minwidth"
 *   "gui_maxwidth"
 */
class HSmallTextField : public HField
{
    Q_OBJECT

public:
    HSmallTextField(QString sqlname,QString description,QString title);
    ~HSmallTextField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();
};

/** A largel textual field
 *  Attributes:
 *   "gui_minwidth"
 *   "gui_maxwidth"
 *   "gui_minheight"
 *   "gui_maxheight"
 *
 *   "csvtable" = "yes"
 *   "csvtable_columns" = "HEAD1;HEAD2;..."
 *   "csvtable_rows" = "ROWNAME1;ROWNAME2;..."
 *   "csvtable_emptyrows" = "keep"
 */
class HLargeTextField : public HField
{
    Q_OBJECT

public:
    HLargeTextField(QString sqlname,QString description,QString title);
    ~HLargeTextField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();
};

/** A static Key-Value selectable item with string key type
 *  Attributes:
 *   "radiobuttons" = "yes"
 *   "radiobutton_space"
 */
class HTxtSelectField : public HField , public HFieldWithValueStore<QString>
{
    Q_OBJECT

public:
    HTxtSelectField(QString sqlname,QString description,QString title);
    ~HTxtSelectField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

    HTxtSelectField* clearSelectables();
    HTxtSelectField* addSelectableItem(QString key,QString val);
    HTxtSelectField* setSelectableItems(QList<QString>& keys,QList<QString>& values);
    HTxtSelectField* addUnSelectableItem(QString key,QString val);
    HTxtSelectField* setUnSelectableItems(QList<QString>& keys,QList<QString>& values);

    virtual QString convertToDisplay(QString fv);

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    virtual void toJson_inWork_spec(QJsonObject& fo);
    virtual void applyJson_inWork_spec(QJsonObject& fo);

    static QString str2str(QString s,QList<QString> selectableKeys,QList<QString> unSelectableKeys,QString def);
};

/** A static Key-Value selectable item with numeric key type
 *  Attributes:
 *   "radiobuttons" = "yes"
 *   "radiobutton_space"
 */
class HNumSelectField : public HField , public HFieldWithValueStore<int>
{
    Q_OBJECT

public:
    HNumSelectField(QString sqlname,QString description,QString title);
    ~HNumSelectField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

    HNumSelectField* clearSelectables();
    HNumSelectField* addSelectableItem(int key,QString val);
    HNumSelectField* setSelectableItems(QList<int>& keys,QList<QString>& values);
    HNumSelectField* addUnSelectableItem(int key,QString val);
    HNumSelectField* setUnSelectableItems(QList<int>& keys,QList<QString>& values);

    virtual QString convertToDisplay(QString fv);

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    virtual void toJson_inWork_spec(QJsonObject& fo);
    virtual void applyJson_inWork_spec(QJsonObject& fo);

    static QString str2str(QString s,QList<int> selectableKeys,QList<int> unSelectableKeys,QString def);
};

/** A boolean/checkable field type */
class HCheckField : public HField
{
    Q_OBJECT

public:
    HCheckField(QString sqlname,QString description,QString title);
    ~HCheckField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    static QString str2str(QString s);
    static bool str2val(QString s);
    static QString val2str(bool b);

public:
    bool value();
    HCheckField* setValue(bool bv);
    void setValue_Gui(bool bv);

    virtual QString convertToDisplay(QString fv);
    virtual void putsOnSetter(HSqlBuilder *b,QString tableName = QString());

};

/** A date field type */
class HDateField : public HField
{
    Q_OBJECT

public:
    HDateField(QString sqlname,QString description,QString title);
    ~HDateField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    virtual void toJson_inWork_spec(QJsonObject& fo);
    virtual void applyJson_inWork_spec(QJsonObject& fo);

    static QString str2str(QString s,bool unknownAllowed);
    static QDate str2val(QString s,bool unknownAllowed);
    static QString val2str(QDate d,bool unknownAllowed);

public:
    QDate value();
    HDateField* setValue(QDate dv);
    void setValue_Gui(QDate dv);

    HDateField* setUnknownAllowed(bool ua,QString setDefaultStrValue = QString());
    bool unknownAllowed();

    virtual QString convertToDisplay(QString fv);
    virtual void putsOnSetter(HSqlBuilder *b,QString tableName = QString());
    virtual QString defaultValueCalc(QString def);

    QString lastValidValue;

protected:
    static QString fallbackDateValue;
    bool unkAllowed;

};

/** An integer field type
 *  Attributes:
 *   "gui_simpleedit" = "yes"
 *   "minimum"
 *   "maximum"
 *   "gui_maxwidth"
 *   "gui_minwidth"
 */
class HNumberField : public HField
{
    Q_OBJECT

public:
    HNumberField(QString sqlname,QString description,QString title);
    ~HNumberField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    static QString str2str(QString s,QString mins,QString maxs);
    static int str2val(QString s);
    static QString val2str(int i,QString mins,QString maxs);

public:
    int value();
    HNumberField* setValue(int iv);
    void setValue_Gui(int iv);

    virtual QString convertToDisplay(QString fv);
};

/** A floating number field type
 *  Attributes:
 *   "minimum"
 *   "maximum"
 *   "gui_maxwidth"
 *   "gui_minwidth"
 */
class HFloatingField : public HField
{
    Q_OBJECT

public:
    HFloatingField(QString sqlname,QString description,QString title);
    ~HFloatingField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    static QString str2str(QString s,QString mins,QString maxs);
    static double str2val(QString s);
    static QString val2str(double d,QString mins,QString maxs);

public:
    double value();
    HFloatingField* setValue(double dv);
    void setValue_Gui(double dv);

    virtual QString convertToDisplay(QString fv);
};

/** A timestamp type field
 *  Attributes:
 *   "guardupdate" = "yes"
 *   "autoupdate" => "none" , "always" , "insert" , "update"
 */
class HTimestampField : public HField
{
    Q_OBJECT

public:
    HTimestampField(QString sqlname,QString description,QString title);
    ~HTimestampField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

    virtual bool isUpdateRequired(bool forRealUpdate = true);
    virtual void putsOnSetter(HSqlBuilder *b,QString tableName = QString());

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    static QString str2str(QString s);
    static QDateTime str2val(QString s);
    static QString val2str(QDateTime dt);

public:
    QDateTime value();

    virtual QString convertToDisplay(QString fv);

};

/** This is the base class of dynamic Key-Value field
 *  where the Key-Value pairs comes from an another Sql table.
 *  Attributes:
 *   "easyselect" = "yes"
 *   "popuplist_title"
 *   "popuplist_sortbyidx"
 *   "popuplist_plusbutton"
 *   "popuplist_func_on_item_1"
 *   "popuplist_func_on_item_2" ...
 */
class HSqlXChooseField : public HField
{
    Q_OBJECT

protected:
    HSqlXChooseField(QString sqlname,QString description,QString title);
    ~HSqlXChooseField();

    void setUnknownAllowed_X(bool ua,QString setDefaultStrValue = QString());
    bool unknownAllowed_X();

    void refreshKVs_X();
    void setKVSource_X(QString tablename,QString keyfield,QString showfields,QString filter,QString sort);

    virtual void internal_clearKVs();
    virtual void internal_addKV(QString k,QString v);

    virtual void toJson_inWork_spec(QJsonObject& fo);
    virtual void applyJson_inWork_spec(QJsonObject& fo);

public:
    QString connectedTableName();

public slots:
    int customUserEventDispatcher(QString what,QString ckey);

protected:

    bool read;
    QString jTableName;
    QString jKeyFieldName;
    QString jShowFields;
    QString jFilter;
    QString jSort;

    bool unkAllowed;
public:

signals:
    void customUserEvent(QString what,QString ckey);
};

/** A dynamic Key-Value field with string Key type
 *  where the Key-Value pairs comes from an another Sql table.
 *  Attributes:
 *   "easyselect" = "yes"
 *   "popuplist_title"
 *   "popuplist_sortbyidx"
 *   "popuplist_plusbutton"
 *   "popuplist_func_on_item_1"
 *   "popuplist_func_on_item_2" ...
 */
class HSqlSChooseField : public HSqlXChooseField , public HFieldWithValueStore<QString>
{
    Q_OBJECT

public:
    HSqlSChooseField(QString sqlname,QString description,QString title);
    ~HSqlSChooseField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

    HSqlSChooseField* setUnknownAllowed(bool ua,QString setDefaultStrValue = QString());
    bool unknownAllowed();

    HSqlSChooseField* refreshKVs();
    HSqlSChooseField* setKVSource(QString tablename,QString keyfield,QString showfields,QString filter,QString sort);
    HSqlSChooseField* setAttributeRc(QString name,QString value);

    virtual QString convertToDisplay(QString fv);
    virtual void initialize(void);
    virtual void refreshRelatedDatabaseData();

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    static QString str2str(QString s,bool unknownAllowed,QList<QString> selectableKeys,QString def);

    virtual void internal_clearKVs();
    virtual void internal_addKV(QString k,QString v);
};

/** A dynamic Key-Value field with integer Key type
 *  where the Key-Value pairs comes from an another Sql table.
 *  Attributes:
 *   "easyselect" = "yes"
 *   "popuplist_title"
 *   "popuplist_sortbyidx"
 *   "popuplist_plusbutton"
 *   "popuplist_func_on_item_1"
 *   "popuplist_func_on_item_2" ...
 */
class HSqlNChooseField : public HSqlXChooseField , public HFieldWithValueStore<int>
{
    Q_OBJECT

public:
    HSqlNChooseField(QString sqlname,QString description,QString title);
    ~HSqlNChooseField();
    virtual QString className();
    virtual HSqlBuilder_FieldValueType fvType();

    HSqlNChooseField* setUnknownAllowed(bool ua,QString setDefaultStrValue = QString());
    bool unknownAllowed();

    HSqlNChooseField* refreshKVs();
    HSqlNChooseField* setKVSource(QString tablename,QString keyfield,QString showfields,QString filter,QString sort);
    HSqlNChooseField* setAttributeRc(QString name,QString value);

    virtual void putsOnSetter(HSqlBuilder *b,QString tableName = QString());

    virtual QString convertToDisplay(QString fv);
    virtual void initialize(void);
    virtual void refreshRelatedDatabaseData();

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    static QString str2str(QString s,bool unknownAllowed,QList<int> selectableKeys,QString def);

    virtual void internal_clearKVs();
    virtual void internal_addKV(QString k,QString v);
};


/* @} */
#endif

//End of gSAFE ftypes.h
