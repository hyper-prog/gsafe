/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    dmext.h
*/

#ifndef GSAFE__DMEXT_HEADER_FILE_X_
#define GSAFE__DMEXT_HEADER_FILE_X_

#include <QtCore>

#include <data.h>
#include <dm.h>
#include <builder.h>
#include <database.h>

/*  @{  */

/** HDataMatrix is a class store a table (X rows and Y columns). However it is usally contains sql tables
 *  this class is not an sql related class. It is usally used as a result of a complex sql query.
 *  With this class you can manipulate simple data tables, can make reports, printigs, xml tables or do anything with the data.
 *  @see HDispPlainDataMatrix
 */
class HDataMatrix : public QObject , public HNamed
{
    Q_OBJECT

public:
    /** Created an empty table */
    HDataMatrix();
    /** Destructor */
    ~HDataMatrix();

    virtual QString className();

    /** Returns the title of the table */
    QString getTitle() { return title; }
    /** Sets the title of the table */
    void setTitle(QString t) { title = t; }
    /** Sets the extended title of the table */
    void setExtendedTitle(QString xt) { exp_title = xt; }

    /** Clears all cell data from HDataMatrix. It keeps the title data. */
    void clear();
    /** Clears all data from HDataMatrix. Even the title data. */
    void clearAll();

    /** Returns the number of coumns */
    int columnCount();
    /** Returns the number of rows */
    int rowCount();
    /** Returns the number of records in the table. Same as rowCount() */
    int recordCount() { return rowCount(); }
    /** Returns the number of fields. Same as rowCount() */
    int fieldCount() { return columnCount(); }

    /** Sets a specified table header cell text*/
    void setHeaderCell(int col,QString strdata);

    /** Sets the whole table header text with a QString list */
    void setHeader(QList<QString> strlistdata);
    /** Clears the whole table header texts */
    void clearHeader();

    /** Returns the specified header text */
    QString getHeaderItem(int col);
    /** Returns the whole table header */
    const QList<QString> getHeader();
    /** Returns the column index of the passed header text.
     *  If the header text not found it returns -1. */
    int getHeaderColIndex(QString headertext);

    /** Sets wrap settings for the specified column which is needed for printing */
    void setColumnPrintWrap(int col,bool wrap);
    /** Returns wrap settings for the specified column which is needed for printing */
    bool getColumnPrintWrap(int col);
    /** Sets the maximum width of the specified column which is needed for printing */
    void setColumnPrintMaxWidth(int col,int m);
    /** Returns the maximum width of the specified column which is needed for printing */
    int  getColumnPrintMaxWidth(int col);

    /** Do an ordinary find & replace process in the texts of cells but only in the specified column.
     *  If the parts parameter is false it is only replace in case of full cell matching. */
    void replaceTextInColumn(int col,QString find,QString replace,bool parts=false);
    /** Run down on the specified column row by row, and if found more cells with same text, it's only keep the first occurance.
     *  The other cells contains same text will be replaced an empty string */
    void replaceSameUnderCellToEmpty(int col);

    /** Sets the first row as current */
    void firstRow();
    /** Makes the next row as current */
    bool nextRow();
    /** Check if there is next row */
    bool isEnded();
    /** Removes the current row from the table */
    void removeCurrentRow();
    /** Returns the current row by returning a list of HValue */
    QList<HValue> currentRow();
    /** Returns the current row by returning a list of QStrings (All cells are converted to QString) */
    QList<QString> currentRowStr();
    /** Returns the current row by returning only one concatenated string with the specified separator
     *  (All cells are converted to QString) */
    QString currentRowStr(QString separator);
    /** Returns the control string of the current row (Control string is needed for making fancy report)
     *  @see getContentAsHtml() */
    QString currentRowControl();

    /** Adds a new row to the end of the table. */
    void addRow(QList<HValue> listdata,QString ctrl="");
    /** Adds a new empty row to the end of the table. */
    void addRowEmpty(QString ctrl="");

    /** Adds a new row to the end of the table. */
    void addRowStr(QList<QString> strlistdata,QString ctrl="");

    /** Appends an external HDataMatrix to this table. */
    void appendHPlainDataMatrix(HDataMatrix *tail);

    /** Returns the specified row */
    QList<HValue> getRow(int row);
    /** Returns the specified row */
    QList<QString> getRowStr(int row);
    /** Returns the specified row in one concatenated string with the specified separator
     *  (All cells are converted to QString) */
    QString getRowStr(int row,QString separator);

    /** Returns the content of the specified cell */
    HValue getCell(int row,int col);
    /** Returns the content of the specified cell */
    QString getCellStr(int row,int col);
    /** Returns the control string of the specified row */
    QString getRowControl(int row);
    /** Sets the content of the specified cell */
    void setCell(int row,int col,HValue vdata,bool autoCreateRows = false);
    /** Sets the content of the specified cell */
    void setCellStr(int row,int col,QString strdata,bool autoCreateRows = false);

    /** Returns the content of the cell, the column is specified by the header text.
        If the headers are not set or not found this function returns empty value. */
    HValue getCellByHead(int row,QString colheader);
    /** Returns the content of the cell, the column is specified by the header text.
        If the headers are not set or not found this function returns empty value. */
    QString getCellByHeadStr(int row,QString colheader);
    /** Sets the content of the cell, the column is specified by the header text.
        If the headers are not set or not found this function does nothing. */
    void setCellByHead(int row,QString colheader,HValue vdata,bool autoCreateRows = false);
    /** Sets the content of the cell the column is specified by the header text.
        If the headers are not set or not found this function does nothing.  */
    void setCellByHeadStr(int row,QString colheader,QString strdata,bool autoCreateRows = false);

    /** Sets the control string of the specified row */
    void setRowControl(int row,QString ctrl);

    /** Appends and prepends the specified string to every cell of the specified column */
    void setAppendPrependColumn(int col,QString prepend,QString append);

    /** Removes every html tags <*> from the data cells if the control string contains : "optionalhtmltags".
     *  If the force_all is true it removes every even if the "optionalhtmltags" is missing. */
    void removeHTMLTags(bool force_all=false);

    /** Calculates the sum of the values of the specified column. (The cells have to be convertable to integer) */
    int sumCoulmnAsInt(int col);
    /** Calculates the sum of the values of the specified column. (The cells have to be convertable to float) */
    double sumCoulmnAsDouble(int col);
    /** Returns a string which contains the concatenated values of the specified column with the specified separator */
    QString concatenateCoulmn(int col,QString separator);

    /** It generates a CSV output from the object. Fields are separated by semicolon. */
    QString getContentAsCSV();

    /** Returns a string which contains the concatenated values of the specified column with the specified separator */
    QString getColumnConcatenated(int col,QString separator);

    /** Set embedded property to true. It does not affect the behavior of HDataMatrix. Its only a sign for outer callers.
     *  This only set if the HDataMatrix is embedded to a HRecordLines. It works autmatically so don't use the function! */
    void setEmbedded();
    /** Retruns true if this HDataMatrix is embedded to another data structure like HRecordLines. */
    bool isEmbedded();

protected:
    int col_count;
    QString title;
    QString exp_title;

    QList<QString> headerTexts;
    QList<HValue *> data;
    QList<QString> control;
    QList<HValue *>::iterator iter;
    QList<QString>::iterator iter_ctrl;

    QList<bool> printCellWraps;
    QList<int> printMaxCellWidths;
    bool embedded;

    void checkAndGrow(int to);

public:
    /** The default keyfield. Used when you put this object in HDispPlainDataMatrix.
     *  With keyfield you can specify the key of the HTableBrowser elements.
     *      This is important when the user clicks on an element. That case the "activateItem" signal will be emitted which
     *      tell the key of the activated item. This key value came from:
     *      -   If the keyfield = -2 (default) not specified. The HDispPlainDataMatrix tell the keyfield (in parameter).
     *      -   If the keyfield = -1 a new key will be generated, which will be the index number of the rows.
     *      -   If the keyfield >= 0 means the keyfield indexed column will be the index. */
    int keyfield;

    QString soft_current_key;

public slots:
    /** Emits the actionOnRecord signal with the given parametered key.
     *  Usally the gui call this slot to emit the signal. */
    int actLine(QString key);
    /** Emits the alternateActionOnRecord signal with the given parametered key.
     *  Usally the gui call this slot to emit the signal. */
    int alternateActLine(QString key);
    /** Send data changed signal */
    int sendDataChanged();

signals:
    /** This signal is emitted when the user (left) clicked on a record
     *  (Can be emitted directly by slot actLine() )  */
    void actionOnRecord(QString key);
    /** This signal is emitted when the user right clicked on a record
     *  (Can be emitted directly by slot alternateActLine() ) */
    void alternateActionOnRecord(QString key);
    /** This signal is emitted when the data is changed */
    void dataChanged();

};

// ///////////////////////////////////////////////////////////////////////////////
// / Classes of HDynTable (and related) //////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

enum HDynTableCellType {
    HDynCellType_String  = 0,
    HDynCellType_Double  = 1,
    HDynCellType_Undefined  = 9,
};

/** An element (An individual cell) of the dynamically builded HDynTable.
 *  You don't need to use this class directly use HDynTable's "defN" or "defS" methods instead this class.
 *  @see HDynTable */
class HDynTableElement
{
    friend class HDynTable;

private:
    QString name;
    QString rowName,colName;
    QString sqlname;
    QList<QString> labels;

    HDynTableCellType type;
    QString textual_value;
    double  double_value;

private:
    /** Creates a HDynTableElement. Use HDynTable's defN or defS methods instead.  */
    HDynTableElement(HDynTableCellType t,QString name,QString sqlname,QString rowName,QString colName,QString labels="");

    HDynTableCellType getType();
    bool isNumeric();
    bool isTextual();

    void    setDoubleValue(double d);
    void    setTextualValue(QString s);
    double  getDoubleValue();
    QString getTextualValue();

    void addLabel(QString l);
    bool hasLabel(QString l);
};

/** The HDynTable contains the collection of numeric or textual typed fileds referenced by names.\n
 *  After you define the elements, you can easely set/get elements value referenced by it's name.
 *  The HDynTable has many batch mathematical function to easely work with the all element together.
 *  The mathematical functions only works on numeric elements, they don't affect the textual elements.
 \code
 HDynTable t;

 t.defN("a");
 t.defN("b");
 t.defS("t");


 t["a"] = 10.2;
 t["b"] = 3 * t["a"];
 t.divide(t.sum());
 t["b"] += 100;

 t["t"] = "Texual data";

 \endcode
 *
 *  The HDynTable can dynamically build a QTableWidget from the elements you defined. (See HDynTableDispConnector)\n
 *  You can define element with different column/row names, and the HDynTable will be generate the appropirate QTableWiget,
 *  It means: The same row named elements will appear in same row, if an element have unique rowname it will appear in separated row.
 *  The classes will automatically handles the refreshing the data in the memory.
 *  It means that the user changes will automatically apply on HDynTable's values.
 *  It also can handle the SQL query/update of the fileds. (See HDynTableSqlConnector)
 *
\code
    ...
    //Create an empty dyntable and define the fields
    HDynTable *dt = new HDynTable();
          //name    sqlf    rown    coln
    dt->defN("appg" ,"appg" ,"Apple","Good");
    dt->defN("appb" ,"appb" ,"Apple","Bad");
    dt->defN("pearg","pearg","Pear" ,"Good");
    dt->defN("pearb","pearb","Pear" ,"Bad");
    dt->defN("plumg","plumg","Plum" ,"Good");
    dt->defN("plumb","plumb","Plum" ,"Bad");
    dt->defN("nut"  ,"nut"  ,"Nut"  ,"Good");

    HSqlHandler *sql=new HSqlHandler();

    //Connects the dyntable to the sql table
    HDynTableSqlConnector *sql_conn = new HDynTableSqlConnector(dt,sql,"mytable");
    //Initialize the ui's table with this dyntable
    HDynTableDispConnector *disp_conn = new HDynTableDispConnector(dt,ui->table);

    //We reads the values from sql. The gui will refresh automatically.
    sql_conn->readSql();

    //We want the sql table to be updated every time, when the user change a data on gui
    connect(disp_conn,SIGNAL(dataChangedByGui()),sql_conn,SLOT(updateSql()));
    ...
 \endcode
*   The code above is generates a QTableWidget look like this:
*    \image  html pictures_doc/hdyntable.png
*   @see HDynTableDispConnector
*   @see HDynTableSqlConnector        */
class HDynTable : public QObject, public HNamed
{
    Q_OBJECT

private:
    QList<HDynTableElement> elements;
    QList<HDynTableElement>::iterator internal_iterator;

    /** Adds a HDynTableElement
     *  @see HDynTableElement */
    void add(HDynTableElement e);
public:
    /** Creates an empty HDynTable */
    HDynTable();
    /** Creates a copy of the given HDynTable */
    HDynTable(const HDynTable& t);
    ~HDynTable();

    virtual QString className();

    /** Define a new numeric typed element (Handles double data)
     *  @param name The name to identify the field. Must be unique. You can set/get values by this name (t["name"] = 0.1;)
     *  @param sqlname The sql filed name assigned to this element
     *  @param rowName This element will be show in this named row in the dynamically builded HDynTable
     *  @param colName This element will be show in this named column in the dynamically builded HDynTable
     *  @param labels This parameters contains the "|" separated collections of text labels. You can filter many mathematical   */
    void defN(QString name,QString sqlname="",QString rowName="",QString colName="",QString labels="")
    { add(HDynTableElement(HDynCellType_Double,name,sqlname,rowName,colName,labels));  }

    /** Define a new string typed element (Handles textual data)
     *  @param name The name to identify the field. Must be unique. You can set/get values by this name (t["name"] = 0.1;)
     *  @param sqlname The sql filed name assigned to this element
     *  @param rowName This element will be show in this named row in the dynamically builded HDynTable
     *  @param colName This element will be show in this named column in the dynamically builded HDynTable
     *  @param labels This parameters contains the "|" separated collections of text labels. You can filter many mathematical */
    void defS(QString name,QString sqlname="",QString rowName="",QString colName="",QString labels="")
    { add(HDynTableElement(HDynCellType_String,name,sqlname,rowName,colName,labels));  }

    /** Define a new typed element (You can give the type in parameter)
     *  @param type The type of the field. Can be HDYNELEMENT_DOUBLE or HDYNELEMENT_STRING
     *  @param ... other parameters are same as defN,defS
     *  @see defN
     *  @see defS  */
    void defT(HDynTableCellType type,QString name,QString sqlname="",QString rowName="",QString colName="",QString labels="")
    { add(HDynTableElement(type,name,sqlname,rowName,colName,labels));  }

    /** Clears all element from the HDynTable */
    void clear() { elements.clear(); }

    /** Dump out all elements name and value in one concatenated string */
    QString dumpElements(bool newline = false);

    /** Retrun the element names in a list */
    QStringList elementNames(QString limitTo = "");
    QStringList elementSqlNames(QString limitTo = "");

    /** Sets the given named element's value */
    void setElementValue(QString name,double value);
    /** Sets the given named element's value */
    void setElementValue(QString name,QString value);

    /** Queries the given named element's double value */
    double  getElementValueDouble(QString name,bool *found=NULL);
    /** Queries the given named element's string value */
    QString getElementValueString(QString name,bool *found=NULL);

    /** Sets the specified row and column crossed element's double value */
    void setElementValueByRowColName(QString r,QString c,double value);
    /** Sets the specified row and column crossed element's string value */
    void setElementValueByRowColName(QString r,QString c,QString value);

    /** Queries the specified row and column crossed element's double value */
    double getElementValueDoubleByRowColName(QString r,QString c,bool *found=NULL);
    /** Queries the specified row and column crossed element's string value */
    QString getElementValueStringByRowColName(QString r,QString c,bool *found=NULL);

    /** Queries the given named element's sql name */
    QString getElementSqlName(QString name);
    /** Queries the given named element's type */
    HDynTableCellType getElementType(QString name);
    /** Queries the given named element's row string */
    QString getElementRowString(QString name);
    /** Queries the given named element's column string */
    QString getElementColString(QString name);
    /** Queries the given named element's labels */
    const QList<QString> getElementLabels(QString name);

    /** Set the internal intetator to the first element.
     *  You can walk throught the elements with this functions
     \code
     HDynTable t;
     ...
     t.firstElement();
     while(!t.isEnded())
     {
        foo( t.currentElementName() + "=" + t.currentElementValueString() );

        t.nextElement();
     }
     \endcode
     *  @see isEnded()
     *  @see nextElement() */
    void firstElement();
    /** Check if the internal iterator reach the end of the fields.
     *  @see firstElement() */
    bool isEnded();
    /** Steps the internal iterator to the next element.
     *  @see firstElement() */
    void nextElement();

    /** Returns the name of the element in the position of internal iterator */
    QString currentElementName();
    /** Returns the row name of the element in the position of internal iterator */
    QString currentElementRowName();
    /** Returns the column name of the element in the position of internal iterator */
    QString currentElementColumnName();
    /** Returns the sql name of the element in the position of internal iterator */
    QString currentElementSqlName();
    /** Returns the labels of the element in the position of internal iterator */
    QStringList currentElementLabels();

    /** Returns true if the element in the position of internal iterator is numeric */
    bool    currentElementIsNumeric();

    /** Returns the double value of the element in the position of internal iterator */
    double  currentElementValueDouble();
    /** Returns the string name of the element in the position of internal iterator */
    QString currentElementValueString();

    /** Sets the value of the element in the position of internal iterator */
    void setCurrentElementValue(double d);
    /** Sets the value of the element in the position of internal iterator */
    void setCurrentElementValue(QString s);


    /** Sets all elements value to zero and empty string.
     *  @param limitTo if you give this parameter only the given labelled element's are zeroed */
    void   emptyall(QString limitTo = "");
    /** Sets all element's value to zero.
     *  @param limitTo if you give this parameter only the given labelled element's are zeroed */
    void   zeroall(QString limitTo = "");
    /** Returns the maximum valued element.
     *  @param limitTo if you give this parameter only the given labelled element's are examined */
    double max(QString limitTo = "");
    /** Returns the minimum valued element.
     *  @param limitTo if you give this parameter only the given labelled element's are examined */
    double min(QString limitTo = "");
    /** Returns the sum of element's value.
     *  @param limitTo if you give this parameter only the given labelled element's are examined */
    double sum(QString limitTo = "");

    /** Substract a double value from all double cell */
    void sustraction(double d,QString limitTo = "");
    /** Field to field substraction of the tables (The two tables field's must be same) */
    void sustraction(HDynTable dt,QString limitTo = "");
    /** Add a double value to all double cell */
    void addition(double d,QString limitTo = "");
    /** Field to field addition of the tables (The two tables field's must be same) */
    void addition(HDynTable dt,QString limitTo = "");
    /** Multiplicate all double cell with a double value  */
    void multiplicate(double d,QString limitTo = "");
    /** Field to field multiplication of the tables (The two tables field's must be same) */
    void multiplicate(HDynTable dt,QString limitTo = "");
    /** Divide all double cell with a double value  */
    void divide(double d,QString limitTo = "");
    /** Field to field divide of the tables (The two tables field's must be same) */
    void divide(HDynTable dt,QString limitTo = "");

    /** Returns the number of all defined elements(with no argument) or a specified labelled elements. (if nonempty argument passed) */
    int countElements(QString limitTo="");

    /** Returns the first element's index matching the name passed in parameter. Returns -1 if the element did not found. */
    int indexByElementName(QString name);
    /** Returns the first element's index matching the sqlname passed in parameter. Returns -1 if the element did not found. */
    int indexByElementSqlName(QString sqlname);
    /** Returns the name of the index'th element */
    QString indexedElementName(int index);
    /** Returns the sql name of the index'th element */
    QString indexedElementSqlName(int index);
    /** Returns the row name of the index'th element */
    QString indexedElementRowName(int index);
    /** Returns the column name of the index'th element */
    QString indexedElementColName(int index);
    /** Returns the value name of the index'th element
     *  @param If you give this bool pointer the pointer will be set to true any cases the element has been found */
    double getIndexedValueDouble(int index,bool *found=NULL);
    /** Returns the value name of the index'th element
     *  @param If you give this bool pointer the pointer will be set to true any cases the element has been found */
    QString getIndexedValueString(int index,bool *found=NULL);

    /** Sets the index'th elements value */
    void setIndexedElementValue(int index,double d);
    /** Sets the index'th elements value */
    void setIndexedElementValue(int index,QString s);

    /** Returns true if the given named element has a given label. Otherwise returns false.
     *  @param name The examided element's name
     *  @param label The label name to check
     *  @param If you give this bool pointer the pointer will be set to true any cases the element has been found */
    bool elementHasLabel(QString name,QString label,bool *found=NULL);

    static HDynTable* fromJson(QString jsonData,HDynTableCellType forceType = HDynCellType_Undefined);
    static HDynTable* fromJsonFile(QString jsonFileName,HDynTableCellType forceType = HDynCellType_Undefined);
    virtual bool applyJson(QString jsonData,HDynTableCellType forceType = HDynCellType_Undefined);
    virtual bool applyJsonFile(QString jsonFileName,HDynTableCellType forceType = HDynCellType_Undefined);
    virtual QString toJson(HJsonFlag flags = HJsonFlag_Default);

    static HDynTable* fromJson_inWork(QJsonArray jsonArray,HDynTableCellType forceType = HDynCellType_Undefined);
    virtual bool applyJson_inWork(QJsonArray jsonArray,HDynTableCellType forceType = HDynCellType_Undefined);
    virtual bool applyJson_inWork_valueOnly(QJsonArray jsonArray);
    virtual QJsonValue toJson_inWork(HJsonFlag flags);

private:

    class HDynTableElementNameReferencer
    {
        private:
            HDynTable* ref;
            QString name;

            friend class HDynTable;
            HDynTableElementNameReferencer(HDynTable *ref,QString name)
            {   this->ref = ref;	this->name = name;   }
        public:
            HDynTableElementNameReferencer operator=(double value)
                    { ref->setElementValue(name,value); return *this; }
            HDynTableElementNameReferencer operator=(QString value)
                    { ref->setElementValue(name,value); return *this; }
            operator double()
                    { return ref->getElementValueDouble(name); }
            operator QString()
                    { return ref->getElementValueString(name); }

            HDynTableElementNameReferencer operator+=(double value)
                    { ref->setElementValue(name,ref->getElementValueDouble(name)+value); return *this; }
            HDynTableElementNameReferencer operator-=(double value)
                    { ref->setElementValue(name,ref->getElementValueDouble(name)-value); return *this; }
            HDynTableElementNameReferencer operator*=(double value)
                    { ref->setElementValue(name,ref->getElementValueDouble(name)*value); return *this; }
            HDynTableElementNameReferencer operator/=(double value)
                    { ref->setElementValue(name,ref->getElementValueDouble(name)/value); return *this; }
    };

public:

    /** You can set/get element's value by name with this operator */
    HDynTableElementNameReferencer operator[](QString name) { return HDynTableElementNameReferencer(this,name); }

    HDynTable& operator=(HDynTable s);

signals:
    /** Emitted when the data changed in the memory.
     *  It does not emits when the element's structure changed (added/cleared) */
    void dataChangedNongui();

};

/** This class can connect a HDynTable object to the sql database.
  * It can read/update/insert the data according the emelent's sqlname attribute and the given parameters.
  * (You can see an example in HDynTable describe)
  * @see HDynTable */
class HDynTableSqlConnector : public QObject, public HSqlRelated
{
    Q_OBJECT

public:
    /** If this attribute is true, the class doesn't start a new transaction when run an sql query
     *  Default is false. */
    bool tdisabled;

    /** It creates a connection to the database
      * @param dynt The HDynTable to connect.
      * @param tablename The name of the sql table contains the data
      * @param filter */
    HDynTableSqlConnector(HDynTable *dynt,QString tablename,HSqlBuilderCondition filter);
    ~HDynTableSqlConnector();


    /** Generates the sql create (DDL) string of the meta table. (CREATE TABLE ...)
     *  @return The sql create command
     *  @param switches string can contain modifiers. Currently there is not implemented any modifier. */
    QString sqlCreateString(QString options = "");

public slots:
    /** Inserts the content of table to the database according the parameters given to the constructor.
     *  Retrun value: 0: success read, 1: error occured */
    int insertSql(QList<HSqlBuilderField> additional_fields = QList<HSqlBuilderField>());
    /** Reads the data from sql.
     *  Retrun value: 0: success read, 1: no data read, 2: error occured */
    int readSql();
    /** Updates the data to the sql.
     *  Retrun value: 0: success read, 1: error occured */
    int updateSql();
signals:
    /** Emitted when the data just readed from the sql database */
    void justReaded();
    /** Emitted when the data just updated to the sql database */
    void justUpdated();

private:
    HDynTable *dynt;
    QString tablename;
    HSqlBuilderCondition filter;

};

/* @} */
#endif

//End of gSAFE dmext.h
