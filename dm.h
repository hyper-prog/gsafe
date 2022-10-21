/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    dm.h
*/

#ifndef GSAFE__DM_HEADER_FILE_X_
#define GSAFE__DM_HEADER_FILE_X_

#include <QtCore>

#include <data.h>
#include <builder.h>
#include <database.h>

/*  @{  */

#define H_DECLARE_OPERATORS_FOR_FLAGS(FlagsForDefine) \
  inline FlagsForDefine operator|(FlagsForDefine f1, FlagsForDefine f2) \
  { return static_cast<FlagsForDefine>(static_cast<int>(f1) | static_cast<int>(f2)); } \
  inline FlagsForDefine operator&(FlagsForDefine f1, FlagsForDefine f2) \
  { return static_cast<FlagsForDefine>(static_cast<int>(f1) & static_cast<int>(f2)); } \
  inline FlagsForDefine operator~(FlagsForDefine f) \
  { return static_cast<FlagsForDefine>(~static_cast<int>(f)); } \
  inline bool flagOn(FlagsForDefine f, FlagsForDefine ftc) \
  { return (static_cast<int>(f) & static_cast<int>(ftc)) == static_cast<int>(ftc); }

enum HField_GenOptsForWhat {
    SqlBuilderInvalid  = 0,
    SqlBuilderGet      = 1,
    SqlBuilderSet      = 2,
};

enum HField_Status {
    HFieldStatus_Init       = 0,
    HFieldStatus_Default    = 1,
    HFieldStatus_Database   = 2,
    HFieldStatus_GuiChanged = 3,
    HFieldStatus_Changed    = 4,
};

enum HFieldEditType
{
    HFieldEdit_DefaultEditable = 0,
    HFieldEdit_Invisible       = 1,
    HFieldEdit_Readonly        = 2,
    HFieldEdit_ShowReadonly    = 3,
};

enum HJsonFlag
{
    HJsonFlag_Default            = 0,
    HJsonFlag_NoValues           = 1 << 0,
    HJsonFlag_NoStructure        = 1 << 1,
    HJsonFlag_KeyValueStyle      = 1 << 2,
    HJsonFlag_Compacted          = 1 << 3,
    HJsonFlag_DontGenerateObject = 1 << 4,
};

enum SqlOperationFlags
{
    SqlOpFlag_Default                    = 0,
    SqlOpFlag_TransactionDisabled        = 1 << 0,
    SqlOpFlag_DataUpdateSignalDisabled   = 1 << 1,
    SqlOpFlag_ForExtendendedItems        = 1 << 2,
    SqlOpFlag_LeaveStatusesUntouched     = 1 << 3,
    SqlOpFlag_ValidationOmit             = 1 << 4,
};

enum HDispObjectFlags
{
    HDispFlag_Default                           = 0,
    HDispFlag_DeleteDataObjectOnDestroy         = 1 << 0,
    HDispFlag_CleanDataOnStart                  = 1 << 1, // 2
    HDispFlag_CleanDataOnDestroy                = 1 << 2, // 4
    HDispFlag_NoMergeFlagsWithHField            = 1 << 3, // 8
    HDispFlag_NoInheritFlagsFromHRecordDisplay  = 1 << 4, // 16
    HDispFlag_Readonly                          = 1 << 5, // 32
    HDispFlag_ShowOnly                          = 1 << 6, // 64
    HDispFlag_Invisible                         = 1 << 7, // 128
    HDispFlag_DisableStretchInMiddle            = 1 << 8, // 256
    HDispFlag_AddStretchToLineEnd               = 1 << 9, // 512
    HDispFlag_AddStretchAfterFields             = 1 << 10, //1024
};

enum HBorderFlag
{
    HBorderFlag_None        = 0,
    HBorderFlag_Top         = 1 << 0,
    HBorderFlag_Right       = 1 << 1, // 2
    HBorderFlag_Bottom      = 1 << 2, // 4
    HBorderFlag_Left        = 1 << 3, // 8
    HBorderFlag_All         = 15,
    HBorderFlag_Fill        = 1 << 4, // 16
};

H_DECLARE_OPERATORS_FOR_FLAGS(HJsonFlag)

H_DECLARE_OPERATORS_FOR_FLAGS(HDispObjectFlags)

H_DECLARE_OPERATORS_FOR_FLAGS(SqlOperationFlags)

H_DECLARE_OPERATORS_FOR_FLAGS(HBorderFlag)

QString statusToString(HField_Status s);
QString getOptionValue(QString options,QString name,QString defval);

class HCheckField;
class HDataMatrix;
class HField;

/** The base and empty field validator class, every validator class extends this. */
class HBaseValidator
{
public:
    HBaseValidator(QString failMessage);
    virtual ~HBaseValidator();

    virtual QString validate(QString strValue);

    QString attribute(QString name);
    QStringList allDefinedAttributes();
    HBaseValidator* setAttribute(QString name,QString value);
    HBaseValidator* setAttribute(QString name,int ivalue);
    HBaseValidator* setAttribute(QString name,double dvalue);

    QString validatorType();
    QString failMessage();

    /** Sets the connected field of the validator. It does not need to call this, the
     *  HField::addValidator function automatically call this. */
    void setConnectedHField(HField *c);

protected:
    HField *connected;
    QString vType;
    QString fMessage;
    QMap<QString,QString> attributeMap;
};

/** Simple field validator which raise validation error if the field is empty */
class HNotEmptyValidator : public HBaseValidator
{
public:
    HNotEmptyValidator(QString failMessage);
    virtual ~HNotEmptyValidator();

    virtual QString validate(QString strValue);
};

/** A field validator which raise validation error if the field match (or not match) against a regex
 *  attributes:
 *     "valid_regex": "REGEX"
 *     "notvalid_regex": "REGEX" */
class HRegexValidator : public HBaseValidator
{
public:
    HRegexValidator(QString failMessage);
    virtual ~HRegexValidator();

    virtual QString validate(QString strValue);
};

/** A field validator which raise validation error if the field value is not match to a mathematical range.
 *  attributes:
 *     "minimum": "VALUE"
 *     "maximum": "VALUE" */
class HRangeValidator : public HBaseValidator
{
public:
    HRangeValidator(QString failMessage);
    virtual ~HRangeValidator();

    virtual QString validate(QString strValue);
};

/** A field validator which raise validation error if the field value is in (or not in) a set.
 *  attributes:
 *     "separator": "CHAR"  (default is ; )
 *     "valid_set": "STRING"  (sample: "red;blue;green" )
 *     "notvalid_set": "STRING" */
class HSetValidator : public HBaseValidator
{
public:
    HSetValidator(QString failMessage);
    virtual ~HSetValidator();

    virtual QString validate(QString strValue);
};

/** HField is the base class of all typed Sql fields.
 *  It holds the common settings/operations which useable in all derived types. */
class HField :  public QObject , public HSqlRelated, public HNamed
{
    Q_OBJECT

public:
    HField(QString sqlname,QString description,QString title);
    ~HField();
    virtual QString className();

    QString sqlName();
    QString alias();
    QString derivedSqlFieldName();
    QString derivedAlias();
    HField* setOverrideSqlName(QString oName);
    HField* setAlias(QString alias);

    virtual HSqlBuilder_FieldValueType fvType();
    virtual bool isKey();
    HField_Status status();
    void statusToDatabaseAfterStore();

    QString title();
    QString description();
    HField* setTitle(QString title);
    HField* setDescription(QString descr);
    HField* setFieldEditType(HFieldEditType feType);
    HFieldEditType fieldEditType();
    HField* setVisibleEditable();
    HField* setHidden();
    bool isVisible();
    HField* setVisibleReadonly();
    HField* setShowOnlyReadonly();
    HField* setBeforeText(QString bText);
    HField* setAfterText(QString aText);
    QString beforeText();
    QString afterText();
    HField* setNoSql(bool ns);
    bool noSql();
    HField* setSqlCreateType(QString sct);

    bool isEditable();
    HField* setDisplayFlags(HDispObjectFlags flags);
    HDispObjectFlags displayFlags();
    HField* setColor(int r,int g,int b);

    QString attribute(QString name);
    QStringList allDefinedAttributes();
    HField* setAttribute(QString name,QString value);
    HField* setAttribute(QString name,int ivalue);
    HField* setAttribute(QString name,double dvalue);

    void clearTags();
    HField* addTag(QString t);
    HField* addTags(QString tags,QString selectBy = ",");
    bool hasTag(QString t);
    const QList<QString> tags();

    virtual void putsOnSetter(HSqlBuilder *b,QString tableName = QString());
    virtual void putsOnGetter(HSqlBuilder *b,QString tableName = QString());
    virtual QString genBuilderOptions(HField_GenOptsForWhat forWhat);
    virtual void initialize(void);

protected:
    virtual void setStrValue_NoStatusChange(QString value);

    virtual void toJson_inWork_spec(QJsonObject& fo);
    virtual void applyJson_inWork_spec(QJsonObject& fo);

public:
    virtual QString strValue();
    virtual HField* setStrValue(QString value);
    virtual void setStrValue_Database(QString value);
    virtual void setStrValue_Gui(QString value);

    virtual QString strDefault();
    virtual HField* setStrDefault(QString value,bool setValue = false);
    virtual void restoreDefaultValue();
    virtual QString defaultValueCalc(QString def);
    virtual bool isUpdateRequired(bool forRealUpdate = true);
    virtual void refreshRelatedDatabaseData();

    HField *addValidator(HBaseValidator* v);
    const QList<HBaseValidator *> validators();
    void clearValidators();
    bool getLastValidatorCheckStatus();
    void setLastValidatorCheckToFail();
    void resetLastValidatorStatus();

    virtual QString validate();

    virtual QString convertToDisplay(QString fv);
    virtual QString displayValue();

    virtual QString sqlCreateType(QString options = QString(""));

    HField& operator=(QString val);
    operator const QString();

    static HField* fromJson(QString jsonData);
    static HField* fromJsonFile(QString jsonFileName);
    virtual bool applyJson(QString jsonData);
    virtual bool applyJsonFile(QString jsonFileName);
    virtual QString toJson(HJsonFlag flags = HJsonFlag_Default);

    static HField* fromJson_inWork(QJsonValue jsonValue);
    virtual bool applyJson_inWork(QJsonObject jsonObject);
    virtual bool applyJson_inWork_valueOnly(QJsonObject jsonObject);
    virtual QJsonValue toJson_inWork(HJsonFlag flags);

protected:
    QString def,v;
    QString fSqlName,fTitle,fDescription;
    HDispObjectFlags dispFlags;
    QMap<QString,QString> attributeMap;
    HFieldEditType fet;
    bool fNoSql;
    QList<QString> mytags;

    HField_Status sts;
    bool fDatabaseAndMemoryRepDiffers;

    QMap<QString,QString> defaultCreateTypes;
    QList<HBaseValidator *> vValidators;
    bool lastValidatorCheckFailed;
    bool inConfigure;

public slots:
    int changeDatabaseTo(QString toName);

signals:
    void dataChanged(void);
    void featureChanged(void);
};

/** This class a special template Key-Value store class. */
template <typename T>
class HKeyValueStore
{

public:
    HKeyValueStore();
    ~HKeyValueStore();

    void clear();
    void addItem(T key,QString val);
    void setItems(QList<T>& keys,QList<QString>& values);
    void addUnlistedItem(T key,QString val);
    void setUnlistedItems(QList<T>& keys,QList<QString>& values);

    const QList<T>& keys();
    const QList<QString>& values();
    const QList<T>& unlistedKeys();
    const QList<QString>& unlistedValues();

    bool hasKey(T key,bool unlistedIncluded = false);
    QString value(T key,bool unlistedIncluded = false);

    T keyByIdx(int index);
    QString valueByIdx(int index);
    int indexOfKey(T key);
    int indexOfValue(QString val);

protected:
    QList<T> vKeys;
    QList<QString> vValues;
    QList<T> uKeys;
    QList<QString> uValues;

    QList<T> sItemsSort;
    QMap<T,QString> sItems;
    QMap<T,QString> uNsItems;
};

/** This class is a base of Key-Value typed HField descendats.
 *  The Key-Value managing operations works from here.
 *  It holds a HKeyValueStore instance inside */
template <typename T>
class HFieldWithValueStore
{
public:
    HFieldWithValueStore();
    ~HFieldWithValueStore();

    const QList<T> selectableKeys();
    const QList<QString> selectableValues();
    T selectableKey(int index);
    QString selactableValue(int index);
    int indexOfSelectableKey(T key);
    int indexOfSelectableValue(QString value);

protected:
    void jsonApplyHelper(QJsonObject& job);
    void jsonGenerateHelper(QJsonObject& job);

protected:
    HKeyValueStore<T> selectables;
};

class HRecord;
class HRecordLines;

/** This class implements the base Sql table managing operations
 *  of HRecord and HRecordLines classes. */
class HTableSkel : public QObject , public HSqlRelated, public HNamed
{
    Q_OBJECT

public:
    HTableSkel(QString tableName,QString title = QString());
    ~HTableSkel();

    virtual QString className();

    QString tableName();
    QString tableTitle();
    HTableSkel* setTableName(QString name);
    HTableSkel* setTableTitle(QString title);

protected:
    QString tblName,tblTitle;
};

/** This class implements the field managing operations of HRecord and HRecordLines classes. */
class HRecordSkel : public HTableSkel
{
    Q_OBJECT

public:
    HRecordSkel(QString tableName,QString title = QString());
    ~HRecordSkel();
    virtual QString className();

    virtual HRecordSkel& db(QString databaseName = "");

    HRecordSkel* addField(HField *nf);
    void clearFields();
    int fieldCount();

    HField* fieldByIndex(int index);
    HField* fieldByName(QString sqlname);
    int fieldIndexByName(QString sqlname);

    virtual int keyIndex();
    virtual QString keySqlName();
    virtual HSqlBuilder_FieldValueType keyFvType();

    void tagSetFieldEditType(QString t,HFieldEditType feType);
    void tagSetNoSql(QString t,bool ns);
    void tagSetDisplayFlags(QString t,HDispObjectFlags flags);
    void tagSetAttribute(QString t,QString name,QString value);
    void tagSetAttribute(QString t,QString name,int ivalue);
    void tagSetAttribute(QString t,QString name,double dvalue);
    void tagSetColor(QString t,int r,int g,int b);

    virtual void putsOnGetter(HSqlBuilder *b);

    virtual QString generateString(int verbose = 0);

    QString toJson(HJsonFlag flags = HJsonFlag_Default);
    bool applyJson(QString jsonData);
    bool applyJsonFile(QString jsonFileName);
    QJsonValue toJson_embedded(HJsonFlag flags);

    HRecord * toHRecord();
    HRecordLines * toHRecordLines();

protected:
    bool applyJson_inWork(QJsonObject jsonObject);
    bool applyAddFieldsJson_inWork(QJsonObject jsonObject);
    QJsonValue toJson_inWork(HJsonFlag flags);
    static QString readJsonFile_inWork(QString jsonFileName);
    virtual void subspec_toJson_inWork(QJsonObject *top);
    virtual void subspec_applyJson_inWork(QJsonObject *top);

    QList<HField *> fields;

signals:
    void databaseChanged(QString toName);
    void dataChanged();
};

/** It represents a database record which built from HField descendat fields. */
class HRecord : public HRecordSkel
{
    Q_OBJECT

public:
    HRecord(QString tableName,QString title = QString());
    ~HRecord();
    virtual QString className();

    HField& operator[](int index);
    HField& operator[](QString sqlname);

    bool isUpdateRequired(bool forRealUpdate = true);
    bool updateGuardAlert();

    virtual QString keyStrValue();
    QString strValue(QString sqlname);
    QString strValue(int index);
    HRecord* setStrValue(QString sqlname,QString value);
    HRecord* setStrValue(int index,QString value);

    static HRecord* fromJson(QString jsonData,QString inDatabase = "");
    static HRecord* fromJsonFile(QString jsonFileName,QString inDatabase = "");

    virtual QString generateString(int verbose = 0);

    void tagSetStrValue(QString t,QString value);
    void tagRestoreDefaultValue(QString t);
    void tagSetStrDefault(QString t,QString value,bool setValue = false);

    virtual void putsOnSetter(HSqlBuilder *b);

    static HRecord* fromJson_embedded(QJsonObject jsonObject);

    QString validate();

    QMap<QString,QString> getValueMap(bool sqlonly = false);
    QMap<QString,QString> getDisplayValueMap(bool sqlonly = false);

protected:
    static HRecord* fromJson_inWork(QJsonObject jsonObject,QString inDatabase = "");
    virtual void subspec_toJson_inWork(QJsonObject *top);
    virtual void subspec_applyJson_inWork(QJsonObject *top);

public slots:
    int returnToDefault(SqlOperationFlags flags = SqlOpFlag_Default);
    int readWithKey(QString keyValue,SqlOperationFlags flags = SqlOpFlag_Default);
    int update(SqlOperationFlags flags = SqlOpFlag_Default);
    int reloadTimestamps(bool tr_disabled = false);
    int insert(SqlOperationFlags flags = SqlOpFlag_Default);
    void resetFieldStsAfterDbStore();
    int refreshRelatedDatabaseData();

signals:
    void validationFailed(QString message);
};

/** It holds a set of Sql records of same table. */
class HRecordLines : public HRecordSkel
{
    Q_OBJECT

public:
    HRecordLines(QString tableName,QString title = QString());
    ~HRecordLines();

    virtual QString className();

    virtual bool readLines(bool tdisabled = false);

    void setConditions(HSqlBuilderCondition c);
    void setJoins(HSqlBuilderJoin j);
    void setSorts(HSqlBuilderSort s);

    void addOnItemAction(QString actionName,QString displayText);
    const QMap<QString,QString> allOnItemActions();
    const QStringList& readedFields();

    virtual void putsOnGetter(HSqlBuilder *b);

    HDataMatrix *dataMatrix();
    const QStringList& keyValueArray();

    static HRecordLines* fromJson(QString jsonData);
    static HRecordLines* fromJsonFile(QString jsonFileName);

    static HRecordLines* fromJson_embedded(QJsonObject jsonObject);

protected:
    static HRecordLines* fromJson_inWork(QJsonObject jsonObject);
    virtual void subspec_toJson_inWork(QJsonObject *top);
    virtual void subspec_applyJson_inWork(QJsonObject *top);
    void setMatrixHeaders(QStringList fields);

protected:
    HDataMatrix *matrix;
    QStringList keyValues;

    HSqlBuilder sQuery;
    HSqlBuilderJoin bJoins;
    HSqlBuilderCondition bCond;
    HSqlBuilderSort bSort;

    QMap<QString,QString> onItemActions;
    QStringList readed_fields;

protected slots:
    int actionOnRecordSlot(QString key);

public slots:
    int alternateActionOnRecordSlot(QString key,QString eventText);
    int captureNotify(QString tblname);

public:
    QString soft_current_key;

signals:
    void actionOnRecord(QString key);
    void alternateAactionOnRecord(QString key,QString eventText);
};

QString genereteSqlCreate(HRecordSkel *sk,QString forDatabase);
HField* newHFieldObjByName(QString type,QString sqlname,QString desc,QString title);
QStringList displayFlagsToStringList(HDispObjectFlags f);
HDispObjectFlags stringListToDisplayFlags(QStringList l);


/* @} */
#endif

//End of gSAFE dm.h
