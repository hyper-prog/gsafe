/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   xo.h
*/

#ifndef GSAFE__XO_HEADER_FILE_X_
#define GSAFE__XO_HEADER_FILE_X_

#include <QtCore>

/*  @{  */

enum HDMtrxAdd_TypeHint {
    HDMtrxAdd_Auto      = 0,
    HDMtrxAdd_String    = 1,
    HDMtrxAdd_Numeric   = 2,
};

class HDataMatrix;
/**
 * Class ExcelXmlDocument generate an Excel XML Spreadsheet
 * cell,cells and nrow function can be used to build a formatted table
 * The cell,cells,head,heads,nrow functions can receive an options string which is tell the formatting of the cell(s).
 * This option string is built from options which separated by ;
 *
 *  height=NUMBER - The height of the cell's row
 *  width=NUMBER - The width of the cell's row
 *  formula=STRING - Specify a formula to the cell
 *      Examples: "RC[-1]*2" - Same row 1 column less * 2
 *                "R[-1]C*2" - Same column 1 row less * 2
 *                "R3C3*2" - Absolute row 3 column 3 * 2
 *  t=str|num|dat - The type of the cell
 *      str - String
 *      num - Number
 *      dat - Date, also specify the numberformat => "Short Date" and give the data in iso date yyyy-MM-dd!
 *  wrap=on|off - Cell wrapping on or off
 *  vertical=top|center|bottom - Vertical align of the cell
 *  horizontal=left|center|right - Horizontal align of the cell
 *  border=[none|all|top|bottom|left|right] - Borders of the cell. Can be a simple text value or more value separated by ,
 *       Examples: "border=all"
 *                 "border=top,left"
 *                 "border=none"
 *  borderweight=0|1|2|3 - Border width
 *  background-color=#RRGGBB - The background color of the cell
 *  strong=yes - Bold font
 *  italic=yes - Italic font
 *  size=XX - Point size of the font
 *  underline=yes - The font will be underlined
 *  color=#RRGGBB - The color of the font
 *  numberformat=STRING - The format of the numbers "#,##0\ &quot;Ft&quot;;[Red]\-#,##0\ &quot;Ft&quot;"
 *  prefix=STRING
 *  suffix=STRING
 *
 *  Examples for complete option string:
 *     "t=num;border=all;size=14;formula= "
 *     "border=all ; italic=yes ; size=18 "
 *     "background-color=#ff4455 ; strong=yes ; size=18 ; width=140"
 */
class HExcelXmlDocument
{
public:
    HExcelXmlDocument(QString docName = "Generated document");
    ~HExcelXmlDocument();

    QString name(void);
    HExcelXmlDocument* setDocumentName(QString docName);
    HExcelXmlDocument* setTitle(QString title);

    HExcelXmlDocument* opts(QString options = "");
    HExcelXmlDocument* nrow(QString options = "");
    HExcelXmlDocument* nrows(int count = 1,QString options = "");
    void addDataMatrix(HDataMatrix *dm,HDMtrxAdd_TypeHint typeHint = HDMtrxAdd_Auto,bool controlRowAsOptions = false,QString options = "");

    HExcelXmlDocument* cell(QString c,QString options = "");
    HExcelXmlDocument* cells(QList<QString> cs,QString options = "");

    HExcelXmlDocument* head(QString h,QString options = "");
    HExcelXmlDocument* heads(QList<QString> hs,QString options = "");

    HExcelXmlDocument* setOrientationLandscape();
    HExcelXmlDocument* setOrientationPortrait();

    bool writeFile(QString filename);
    QString get();

    static QMap<QString,QString> optionStringToMap(QString optionString,QString mainSeparator = ";");
    static void mergeMapIntoMap(QMap<QString,QString>& mergeInto ,const QMap<QString,QString>& toMerge);

protected:
    HExcelXmlDocument* beginRow(QString options = "");
    HExcelXmlDocument* endRow();
    QString styleId(QString options);
    QString styleId(const QMap<QString,QString>& opts);


protected:
    QString doc_body;
    int colcount,rowcount;

    bool r_open,r_empty,in_header_row;
    int hc_cc; // header|cell, 0-no 1-header 2-cell
    QString row_body;
    QMap<QString,QString> table_opts;
    QMap<QString,QString> row_opts;
    int row_height;
    int c_index;
    bool c_emptycellbefore;

    QMap<int,int> column_width;

    QString document_name;
    QString orientation;
    int style_counter;
    QMap<QString,QString> processedStyles;
};

/* @} */
#endif

//End of gSAFE xo.h
