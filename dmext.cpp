/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2025 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   dmext.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore>

#include "data.h"
#include "dm.h"
#include "dmext.h"
#include "dconsole.h"
#include "builder.h"

HDataMatrix::HDataMatrix()
{
    col_count = 0;

    title = "";
    exp_title = "";

    headerTexts.clear();
    printCellWraps.clear();
    printMaxCellWidths.clear();

    keyfield = -2; //default: The HDispPlainDataMatrix can set the key
    soft_current_key = "";
    embedded = "";
}

QString HDataMatrix::className()
{
    return "HDataMatrix";
}

void HDataMatrix::checkAndGrow(int to)
{
    int r_run,c_run,r_cnt;
    if(to <= col_count)
        return;
    sdebug(QString("< HDataMatrix > grow %1 -> %2").arg(col_count).arg(to));
    for(c_run = col_count ; c_run < to ; ++c_run)
    {
        headerTexts.push_back("");
        printCellWraps.push_back(false);
        printMaxCellWidths.push_back(0);
    }

    //Grow data rows
    r_cnt = data.count();
    for(r_run = 0 ; r_run < r_cnt ; ++r_run)
    {
        HValue *new_row = new HValue[to];
        for(c_run = 0 ; c_run < to ; ++c_run)
            new_row[c_run] = c_run < col_count ? (data[r_run])[c_run] : HValue("");
        HValue *to_del = data[r_run];
        data[r_run] = new_row;
        delete [] to_del;
    }

    col_count = to;
}

HDataMatrix::~HDataMatrix(void)
{
    clear();
}

void HDataMatrix::setEmbedded(QString to)
{
    embedded = to;
}

bool HDataMatrix::isEmbedded()
{
    return !embedded.isEmpty();
}

QString HDataMatrix::embeddedInto()
{
    return embedded;
}

void HDataMatrix::clear()
{
    //Clear data fields
    while(!data.isEmpty())
    {
        delete[] data.first();
        data.pop_front();
    }
    data.clear();

    //Clear control data
    control.clear();

    //Reset iterators
    iter = data.begin();
    iter_ctrl = control.begin();

    //Reset other...
    headerTexts.clear();
    printCellWraps.clear();
    printMaxCellWidths.clear();
    col_count = 0;

    keyfield = -2;

    soft_current_key = "";
}

void HDataMatrix::clearAll()
{
    title = "";
    exp_title = "";
    clear();
}

int HDataMatrix::columnCount()
{
    return col_count;
}

int  HDataMatrix::rowCount()
{
    return data.count();
}

void HDataMatrix::setHeaderCell(int col,QString strdata)
{
    checkAndGrow(col+1);
    headerTexts[col] = strdata;
}

void HDataMatrix::setHeader(QList<QString> strlistdata)
{
    checkAndGrow(strlistdata.count());
    int i,c = strlistdata.count();
    for(i = 0 ; i < c ; ++i)
        headerTexts[i] = strlistdata[i];
}

void HDataMatrix::clearHeader(void)
{
    int i;
    for(i = 0; i < col_count;++i)
        headerTexts[i] = "";
}

int HDataMatrix::getHeaderColIndex(QString headertext)
{
    int i;
    for(i = 0; i < col_count;++i)
        if(headerTexts[i] == headertext)
            return i;
    return -1;
}

QString HDataMatrix::getHeaderItem(int col)
{
    return headerTexts[col];
}

const QList<QString> HDataMatrix::getHeader(void)
{
    return headerTexts;
}

void HDataMatrix::addRow(QList<HValue> listdata,QString ctrl)
{
    int i,c = listdata.count();
    checkAndGrow(c);

    HValue *row;
    row = new HValue[col_count];
    for(i = 0; i < col_count;++i)
        row[i] = i < c ? listdata[i] : HValue("");

    data.push_back(row);
    control.push_back(ctrl);
}

void HDataMatrix::addRowEmpty(QString ctrl)
{
    int i;
    HValue *row;
    row = new HValue[col_count];
    for(i = 0; i < col_count;++i)
        row[i] = HValue("");

    data.push_back(row);
    control.push_back(ctrl);
}

void HDataMatrix::appendHPlainDataMatrix(HDataMatrix *tail)
{
    checkAndGrow(tail->columnCount());

    if(tail->rowCount() > 0)
    {
        tail->firstRow();
        do
            addRow( tail->currentRow(),tail->currentRowControl() );
        while(tail->nextRow());
    }
}

void HDataMatrix::addRowStr(QList<QString> strlistdata,QString ctrl)
{
    int i,c = strlistdata.count();
    checkAndGrow(c);

    HValue *row;
    row = new HValue[col_count];
    for(i = 0; i < col_count;++i)
        row[i] = i < c ? HValue(strlistdata[i]) : HValue("");

    data.push_back(row);
    control.push_back(ctrl);
}

HValue HDataMatrix::getCell(int row,int col)
{
    if(row < 0 || col < 0)
        return HValue("ERROR");
    if(row > data.count() || col > col_count)
        return HValue("ERROR");

    return (data.at(row))[col];
}

QString HDataMatrix::getCellStr(int row,int col)
{
    if(row < 0 || col < 0)
        return "ERROR";
    if(row > data.count() || col > col_count)
        return "ERROR";

    return (data.at(row))[col].toString();
}

QString HDataMatrix::getRowControl(int row)
{
    return control[row];
}

QString HDataMatrix::getColumnConcatenated(int col,QString separator)
{
    bool first;
    QString r;

    r="";
    if(data.count() <= 0)
        return r;
    firstRow();
    first = true;
    do
    {
        if(!first)
            r.append(separator);
        else
            first = false;
        r.append(currentRowStr()[col]);
    }
    while(nextRow());
    return r;
}


void HDataMatrix::setCell(int row,int col,HValue vdata,bool autoCreateRows)
{
    checkAndGrow(col+1);

    if(row < 0 || col < 0)
        return;
    if((row + 1) > data.count())
    {
        if(!autoCreateRows)
            return;
        while((row + 1) > data.count())
            addRowEmpty("");
    }

    (data.at(row))[col] = vdata;
}

void HDataMatrix::setCellStr(int row,int col,QString strdata,bool autoCreateRows)
{
    checkAndGrow(col+1);

    if(row < 0 || col < 0)
        return;
    if((row + 1) > data.count())
    {
        if(!autoCreateRows)
            return;
        while((row + 1) > data.count())
            addRowEmpty("");
    }

    (data.at(row))[col] = HValue(strdata);
}

HValue HDataMatrix::getCellByHead(int row,QString colheader)
{
    int colIdx = getHeaderColIndex(colheader);
    if(colIdx < 0)
    {
        throw GSafeException("Called HDataMatrix::getCellByHead with a not existing header text");
        return HValue();
    }
    return getCell(row,colIdx);
}

QString HDataMatrix::getCellByHeadStr(int row,QString colheader)
{
    int colIdx = getHeaderColIndex(colheader);
    if(colIdx < 0)
    {
        throw GSafeException("Called HDataMatrix::getCellByHead with a not existing header text");
        return QString();
    }
    return getCellStr(row,colIdx);
}

void HDataMatrix::setCellByHead(int row,QString colheader,HValue vdata,bool autoCreateRows)
{
    int colIdx = getHeaderColIndex(colheader);
    if(colIdx >= 0)
        setCell(row,colIdx,vdata,autoCreateRows);
}

void HDataMatrix::setCellByHeadStr(int row,QString colheader,QString strdata,bool autoCreateRows)
{
    int colIdx = getHeaderColIndex(colheader);
    if(colIdx >= 0)
        setCellStr(row,colIdx,strdata,autoCreateRows);
}

void HDataMatrix::setRowControl(int row,QString ctrl)
{
    if(row < 0 )
        return;
    if(row > data.count())
        return;

    control[row] = ctrl;
}

QList<HValue> HDataMatrix::getRow(int row)
{
    int i;
    QList<HValue> list;
    list.clear();
    HValue *rowdata=NULL;
    rowdata = data.at(row);
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i]);
        }
    return list;
}

QList<QString> HDataMatrix::getRowStr(int row)
{
    int i;
    QList<QString> list;
    list.clear();
    HValue *rowdata=NULL;
    rowdata = data.at(row);
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i].toString());
        }
    return list;
}

QString HDataMatrix::getRowStr(int row,QString separator)
{
    int i;
    QString list="";
    HValue *rowdata=NULL;
    rowdata = data.at(row);
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.append(rowdata[i].toString() + separator);
        }
    return list;
}

void HDataMatrix::firstRow(void)
{
    iter = data.begin();
    iter_ctrl = control.begin();
}

bool HDataMatrix::nextRow(void)
{
    if(iter == data.end())
        return false;

    ++iter;
    ++iter_ctrl;

    if(iter == data.end())
        return false;

    return true;
}

bool HDataMatrix::isEnded(void)
{
    if(iter == data.end() || iter_ctrl == control.end())
        return true;
    return false;
}

void HDataMatrix::removeCurrentRow(void)
{
    if(iter == data.end() || iter_ctrl == control.end())
        return;

    HValue *to_delete = *iter;
    iter = data.erase(iter);
    iter_ctrl = control.erase(iter_ctrl);
    delete [] to_delete;
}

QList<HValue> HDataMatrix::currentRow(void)
{
    int i;
    QList<HValue> list;
    list.clear();
    HValue *rowdata=NULL;
    rowdata = *iter;
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i]);
        }
    return list;
}

QString HDataMatrix::currentRowControl(void)
{
    return *iter_ctrl;
}

QList<QString> HDataMatrix::currentRowStr(void)
{
    int i;
    QList<QString> list;
    list.clear();
    HValue *rowdata=NULL;
    rowdata = *iter;
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.push_back(rowdata[i].toString());
        }
    return list;
}

QString HDataMatrix::currentRowStr(QString separator)
{
    int i;
    QString list="";
    HValue *rowdata=NULL;
    rowdata = *iter;
    if(rowdata != NULL)
        for(i = 0; i < col_count;++i)
        {
            list.append(rowdata[i].toString() + separator);
        }
    return list;
}

void HDataMatrix::replaceTextInColumn(int col,QString find,QString replace,bool parts)
{
    QList<HValue *>::iterator iter;
    HValue *in=NULL;

    iter = data.begin();
    while(iter != data.end())
    {
        in = *iter;
        if(parts)
        {

            if(in[col].toString().contains(find))
            {
                QString str;

                str = in[col].toString();
                str.replace(find,replace);
                in[col] = str;
            }
        }
        else
        {
            if(in[col].toString() == find)
                in[col] = replace;
        }
        ++iter;
    }
}

void HDataMatrix::replaceSameUnderCellToEmpty(int col)
{
    QString laststr;
    QList<HValue *>::iterator iter;
    HValue *in=NULL;

    laststr = "__NOTHING_WAS_BEFORE__";
    iter = data.begin();
    while(iter != data.end())
    {
        in = *iter;
        if(in[col].toString() == laststr)
            in[col] = "";
        else
            laststr = in[col].toString();
        ++iter;
    }
}

void HDataMatrix::setAppendPrependColumn(int col,QString prepend,QString append)
{
    QString str;
    QList<HValue *>::iterator iter;
    HValue *in=NULL;

    iter = data.begin();
    while(iter != data.end())
    {
        in = *iter;
        str = in[col].toString();
        str.prepend(prepend);
        str.append(append);
        in[col] = str;
        ++iter;
    }
}

int HDataMatrix::sumCoulmnAsInt(int col)
{
    int sum;
    QList<HValue *>::iterator iter;
    HValue *in=NULL;

    iter = data.begin();
    sum = 0;
    while(iter != data.end())
    {
        in = *iter;
        sum += in[col].toString().toInt();
        ++iter;
    }
    return sum;
}

double HDataMatrix::sumCoulmnAsDouble(int col)
{
    double sum;
    QList<HValue *>::iterator iter;
    HValue *in=NULL;

    iter = data.begin();
    sum = 0;
    while(iter != data.end())
    {
        in = *iter;
        sum += in[col].toString().toDouble();
        ++iter;
    }
    return sum;
}

QString HDataMatrix::concatenateCoulmn(int col,QString separator)
{
    QString sum;
    QList<HValue *>::iterator iter;
    HValue *in=NULL;

    iter = data.begin();
    sum = "";
    while(iter != data.end())
    {
        in = *iter;
        if(!sum.isEmpty())
            sum += separator;
        sum += in[col].toString();
        ++iter;
    }
    return sum;
}

void HDataMatrix::removeHTMLTags(bool force_all)
{
    int i;
    QString str,str2;
    QList<HValue *>::iterator iter;
    QList<QString>::iterator ctrl_iter;
    HValue *in=NULL;

    iter = data.begin();
    ctrl_iter = control.begin();
    while(iter != data.end())
    {
        if(ctrl_iter == control.end())
            sdebug("Error: HDataMatrix::removeHTMLTags: Control and data count mismatch!");

        in = *iter;
        if(force_all || (*ctrl_iter).contains("optionalhtmltags") )
        {
            for(i=0;i<col_count;++i)
            {
                str = in[i].toString();

                str2 = str.replace(QRegularExpression("<[^<>]*>"),"");
                sdebug("str:"+str+","+str2);
                in[i] = str2;
            }
        }
        ++iter;
        ++ctrl_iter;
    }
}

QString HDataMatrix::getContentAsCSV(void)
{
    QString out;
    QList<QString> lst;
    int i;

    out = "";

    if(rowCount() > 0)
    {
        for(i = 0 ; i < col_count ; ++i)
            out += (i > 0 ? ";" : "") + headerTexts[i];
        out += "\n";

        firstRow();
        do
        {
            lst = currentRowStr();
            for(i = 0 ; i < col_count ; ++i)
            {
                out += (i > 0 ? ";" : "") + lst[i];
            }
            out += "\n";
        }
        while(nextRow());
    }
    return out;
}

void HDataMatrix::setColumnPrintWrap(int col,bool wrap)
{
    checkAndGrow(col+1);
    printCellWraps[col] = wrap;
}

void HDataMatrix::setColumnPrintMaxWidth(int col,int m)
{
    checkAndGrow(col+1);
    printMaxCellWidths[col] = m;
}

bool HDataMatrix::getColumnPrintWrap(int col)
{
    if(col <= col_count)
        return printCellWraps[col];
    else
        return false;
}

int HDataMatrix::getColumnPrintMaxWidth(int col)
{
    if(col <= col_count)
        return printMaxCellWidths[col];
    else
        return 0;
}

int HDataMatrix::actLine(QString key)
{
    emit actionOnRecord(key);
    return 0;
}

int HDataMatrix::alternateActLine(QString key)
{
    emit alternateActionOnRecord(key);
    return 0;
}

int HDataMatrix::sendDataChanged()
{
    emit dataChanged();
    return 0;
}

// ///////////////////////////////////////////////////////////////////////////////
// / Classes of HDynTable (and related) //////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////

HDynTableElement::HDynTableElement(HDynTableCellType t,QString name,QString sqlname,QString rowName,QString colName,QString labels)
{
    if(type != HDynCellType_String && type != HDynCellType_Double)
        type = HDynCellType_String;

    this->type    = t;
    this->name    = name;
    this->rowName = rowName;
    this->colName = colName;
    this->sqlname = sqlname;
    this->labels.clear();
    this->labels = labels.split("|",Qt::SkipEmptyParts);

    textual_value = "";
    double_value  = 0.0;
}

HDynTableCellType HDynTableElement::getType(void)
{
    return type;
}

bool HDynTableElement::isNumeric(void)
{
    if(type == HDynCellType_Double)
        return true;
    return false;
}

bool HDynTableElement::isTextual(void)
{
    return !isNumeric();
}

void HDynTableElement::setDoubleValue(double d)
{
    if(type == HDynCellType_String)
        textual_value = QString("%1").arg(d,0,'f',QLocale::FloatingPointShortest);

    if(type == HDynCellType_Double)
        double_value = d;
}

void HDynTableElement::setTextualValue(QString s)
{
    if(type == HDynCellType_String)
        textual_value = s;

    if(type == HDynCellType_Double)
    {
        double d;
        bool ok;
        d = s.toDouble(&ok);
        if(ok)
            double_value = d;
    }
}

double HDynTableElement::getDoubleValue(void)
{
    if(type == HDynCellType_String)
    {
        double d;
        bool ok;
        d = textual_value.toDouble(&ok);
        if(ok)
            return d;
    }

    if(type == HDynCellType_Double)
         return double_value;

    return 0.0;
}

QString HDynTableElement::getTextualValue(void)
{
    if(type == HDynCellType_String)
        return textual_value;

    if(type == HDynCellType_Double)
        return QString("%1").arg(double_value,0,'f',QLocale::FloatingPointShortest);

    return "";
}

void HDynTableElement::addLabel(QString l)
{
    if(!l.isEmpty())
        labels.push_back(l);
}

bool HDynTableElement::hasLabel(QString l)
{
    if(l.isEmpty())
        return true;
    QList<QString>::iterator i=labels.begin();
    while(i != labels.end())
    {
        if(*i == l)
            return true;
        ++i;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////

HDynTable::HDynTable(void)
 : QObject()
{
    elements.clear();
}

HDynTable::HDynTable(const HDynTable& t)
 : QObject()
{
    elements.clear();

    QList<HDynTableElement>::const_iterator i = t.elements.begin();
    while(i != t.elements.end())
    {
        elements.push_back(HDynTableElement(*i));
        ++i;
    }
}

HDynTable::~HDynTable(void)
{
    elements.clear();
}

QString HDynTable::className()
{
    return "HDynTable";
}

HDynTable& HDynTable::operator=(HDynTable s)
{
    elements.clear();

    QList<HDynTableElement>::const_iterator i = s.elements.begin();
    while(i != s.elements.end())
    {
        elements.push_back(HDynTableElement(*i));
        ++i;
    }
    return *this;
}

void HDynTable::add(HDynTableElement e)
{
    elements.push_back(e);
}

void HDynTable::deleteElement(QString name)
{
    for(QList<HDynTableElement>::iterator i = elements.begin(); i != elements.end() ; ++i )
        if(i->name == name)
        {
            elements.erase(i);
            firstElement();
            return;
        }
}

void HDynTable::deleteAllElementWithLabel(QString label)
{
    for(QList<HDynTableElement>::iterator i = elements.begin(); i != elements.end() ; )
    {
        if(i->hasLabel(label))
            i = elements.erase(i);
        else
            ++i;
    }
    firstElement();
}

QString HDynTable::dumpElements(bool newline)
{
    QString s="";
    bool first=true;
    firstElement();
    while(!isEnded())
    {
        s.append(QString("%1%2=%3%4")
                 .arg(first || newline ? "" : ",")
                 .arg(currentElementName())
                 .arg(currentElementValueString())
                 .arg(newline ? "\n" : ""));
        nextElement();
        first = false;
    }
    return s;
}

QStringList HDynTable::elementNames(QString limitTo)
{
    QStringList rlst;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(limitTo.isEmpty() || i->hasLabel(limitTo))
            rlst.push_back(i->name);
        ++i;
    }
    return rlst;
}

QStringList HDynTable::elementSqlNames(QString limitTo)
{
    QStringList rlst;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(limitTo.isEmpty() || i->hasLabel(limitTo))
            rlst.push_back(i->sqlname);
        ++i;
    }
    return rlst;
}

QString HDynTable::getElementSqlName(QString name)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return i->sqlname;
        ++i;
    }
    return "";
}


HDynTableCellType HDynTable::getElementType(QString name)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return i->type;
        ++i;
    }
    return HDynCellType_Undefined;
}

QString HDynTable::getElementRowString(QString name)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return i->rowName;
        ++i;
    }
    return "";
}

QString HDynTable::getElementColString(QString name)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return i->colName;
        ++i;
    }
    return "";
}

const QList<QString> HDynTable::getElementLabels(QString name)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return i->labels;
        ++i;
    }
    return QList<QString>();
}

void HDynTable::setElementValue(QString name,double value)
{

    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            i->setDoubleValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::setElementValue(QString name,QString value)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            i->setTextualValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

double HDynTable::getElementValueDouble(QString name,bool *found)
{
    if(found != NULL)
        *found = false;

    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
        {
            if(found != NULL)
                *found = true;
            return i->getDoubleValue();
        }
        ++i;
    }
    return 0.0;
 }

QString HDynTable::getElementValueString(QString name,bool *found)
{
    if(found != NULL)
        *found = false;

    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
        {
            if(found != NULL)
                *found = true;
            return i->getTextualValue();
        }
        ++i;
    }
    return "";
}

void HDynTable::setElementValueByRowColName(QString r,QString c,double value)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
            i->setDoubleValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::setElementValueByRowColName(QString r,QString c,QString value)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
            i->setTextualValue(value);
        ++i;
    }
    emit dataChangedNongui();
}

double HDynTable::getElementValueDoubleByRowColName(QString r,QString c,bool *found)
{
    if(found != NULL)
        *found = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
        {
            if(found != NULL)
                *found = true;
            return i->getDoubleValue();
        }
        ++i;
    }
    return 0.0;
}

QString HDynTable::getElementValueStringByRowColName(QString r,QString c,bool *found)
{
    if(found != NULL)
        *found = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->rowName == r && i->colName == c)
        {
            if(found != NULL)
                *found = true;
            return i->getTextualValue();
        }
        ++i;
    }
    return "";
}

void HDynTable::firstElement(void)
{
    internal_iterator = elements.begin();

}

bool HDynTable::isEnded(void)
{
    if(internal_iterator == elements.end())
        return true;
    return false;
}

void HDynTable::nextElement(void)
{
    ++internal_iterator;
}

QString HDynTable::currentElementName(void)
{
    return internal_iterator->name;
}

QString HDynTable::currentElementRowName(void)
{
    return internal_iterator->rowName;
}

QString HDynTable::currentElementColumnName(void)
{
    return internal_iterator->colName;
}

QString HDynTable::currentElementSqlName(void)
{
    return internal_iterator->sqlname;
}

QStringList HDynTable::currentElementLabels(void)
{
    return internal_iterator->labels;
}

bool HDynTable::currentElementIsNumeric(void)
{
    return internal_iterator->isNumeric();
}

double HDynTable::currentElementValueDouble(void)
{
    return internal_iterator->getDoubleValue();
}

QString HDynTable::currentElementValueString(void)
{
    return internal_iterator->getTextualValue();
}

void HDynTable::setCurrentElementValue(double d)
{
    internal_iterator->setDoubleValue(d);
    emit dataChangedNongui();
}

void HDynTable::setCurrentElementValue(QString s)
{
    internal_iterator->setTextualValue(s);
    emit dataChangedNongui();
}

void HDynTable::emptyall(QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo))
        {
            if(i->isNumeric())
                i->setDoubleValue(0.0);
            else
                i->setTextualValue("");
        }
        ++i;
    }
    emit dataChangedNongui();

}

void HDynTable::zeroall(QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo))
            if(i->isNumeric())
                i->setDoubleValue(0.0);

        ++i;
    }
    emit dataChangedNongui();
}

double HDynTable::max(QString limitTo)
{
    double max = 0;
    bool set = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric() && !set)
        {
            max = i->getDoubleValue();
            set = true;
        }
        if(i->hasLabel(limitTo) && i->isNumeric() && i->getDoubleValue() > max)
            max = i->getDoubleValue();
        ++i;
    }
    return max;
}

double HDynTable::min(QString limitTo)
{
    double min = 0;
    bool set = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric() && !set)
        {
            min = i->getDoubleValue();
            set = true;
        }
        if(i->hasLabel(limitTo) && i->isNumeric() && i->getDoubleValue() < min)
            min = i->getDoubleValue();
        ++i;
    }
    return min;
}

double HDynTable::sum(QString limitTo)
{
    double sum = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            sum += i->getDoubleValue();
        ++i;
    }
    return sum;
}

void HDynTable::sustraction(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() - d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::sustraction(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() - dt.getElementValueDouble(i->name) );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::addition(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() + d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::addition(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() + dt.getElementValueDouble(i->name) );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::multiplicate(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() * d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::multiplicate(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            i->setDoubleValue( i->getDoubleValue() * dt.getElementValueDouble(i->name) );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::divide(double d,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
            if(d != 0.0)
                i->setDoubleValue( i->getDoubleValue() / d );
        ++i;
    }
    emit dataChangedNongui();
}

void HDynTable::divide(HDynTable dt,QString limitTo)
{
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo) && i->isNumeric())
        {
            double d;
            d = dt.getElementValueDouble(i->name);
            if(d != 0.0)
                i->setDoubleValue( i->getDoubleValue() / d );
        }
        ++i;
    }
    emit dataChangedNongui();
}

int HDynTable::countElements(QString limitTo)
{
    int count = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->hasLabel(limitTo))
            ++count;
        ++i;
    }
    return count;
}

int HDynTable::indexByElementName(QString name)
{
    int idx=0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
            return idx;

        ++idx;
        ++i;
    }
    return -1;
}

int HDynTable::indexByElementSqlName(QString sqlname)
{
    int idx=0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->sqlname == sqlname)
            return idx;

        ++idx;
        ++i;
    }
    return -1;
}

QString HDynTable::indexedElementName(int index)
{
    int run = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(run == index)
            return i->name;

        ++run;
        ++i;
    }
    return "";
}

QString HDynTable::indexedElementRowName(int index)
{
    int run = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(run == index)
            return i->rowName;

        ++run;
        ++i;
    }
    return "";
}

QString HDynTable::indexedElementColName(int index)
{
    int run = 0;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(run == index)
            return i->colName;

        ++run;
        ++i;
    }
    return "";
}

QString HDynTable::indexedElementSqlName(int index)
{
    return getElementSqlName( indexedElementName(index) );
}

double HDynTable::getIndexedValueDouble(int index,bool *found)
{
    return getElementValueDouble( indexedElementName(index) , found );
}

QString HDynTable::getIndexedValueString(int index,bool *found)
{
    return getElementValueString( indexedElementName(index) , found );
}

void HDynTable::setIndexedElementValue(int index,double d)
{
    setElementValue( indexedElementName(index) , d );
}

void HDynTable::setIndexedElementValue(int index,QString s)
{
    setElementValue( indexedElementName(index) , s );
}

bool HDynTable::elementHasLabel(QString name,QString label,bool *found)
{
    if(found != NULL)
        *found = false;
    QList<HDynTableElement>::iterator i = elements.begin();
    while(i != elements.end())
    {
        if(i->name == name)
        {
            if(found != NULL)
                *found = true;
            return i->hasLabel(label);
        }
        ++i;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////

HDynTableSqlConnector::HDynTableSqlConnector(HDynTable *dynt,QString tablename,HSqlBuilderCondition filter)
 : QObject()
{
    this->dynt       = dynt;
    this->tablename  = tablename;
    this->filter     = filter;
    tdisabled = false;
}

HDynTableSqlConnector::~HDynTableSqlConnector()
{
    this->dynt       = NULL;
    this->tablename  = "";
}

int HDynTableSqlConnector::insertSql(QList<HSqlBuilderField> additional_fields)
{
    if(dynt == NULL)
        return 1;
    HSql sql = getSql();

    HSqlBuilder b(Insert,tablename);
    if(!additional_fields.isEmpty())
    {
        int i,c = additional_fields.count();
        for( i = 0 ; i < c ; ++i )
            b.set(additional_fields[i]);
    }
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        b.set_fv(dynt->currentElementSqlName(),Quoted,dynt->currentElementValueString());
        dynt->nextElement();
    }

    sql.exec(b,"Error in HDynTableSqlConnector::insertSql",tdisabled);
    if(sql.errorStatus())
        return 1;
    return 0;
}

int HDynTableSqlConnector::readSql()
{
    if(dynt == NULL)
        return 2;

    HSql sql = getSql();
    HSqlBuilder b(Select,tablename);

    dynt->firstElement();
    while(!dynt->isEnded())
    {
        b.get(dynt->currentElementSqlName());
        dynt->nextElement();
    }

    if(!filter.isEmpty())
        b.cond(filter);

    auto r = sql.execMulti(b,"Error in HDynTableSqlConnector::readSql",tdisabled);

    if(sql.errorStatus())
        return 2;

    if(!r->nextRecord())
        return 1;

    int idx = 0;
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        dynt->setCurrentElementValue(r->value(idx));
        dynt->nextElement();
        ++idx;
    }
    emit justReaded();
    return 0;
}

int HDynTableSqlConnector::updateSql(void)
{
    if(dynt == NULL)
        return 1;

    HSql sql = getSql();
    HSqlBuilder b(Update,tablename);
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        b.set_fv(dynt->currentElementSqlName(),Quoted,dynt->currentElementValueString());
        dynt->nextElement();
    }

    if(!filter.isEmpty())
        b.cond(filter);

    sql.exec(b,"Error in HDynTableSqlConnector::updateSql",tdisabled);
    if(sql.errorStatus())
        return 1;
    return 0;
}

QString HDynTableSqlConnector::sqlCreateString(QString options)
{
    QString cs;
    QString varcharlength = "64",numericsize = "22,6";
    QStringList opts = options.split(";");

    int i,c = opts.count();
    for(i = 0 ; i < c ; ++i)
    {
        if(opts.at(i).startsWith("varchar_length="))
            varcharlength = opts.at(i).mid(15);
        if(opts.at(i).startsWith("numeric_size="))
            numericsize = opts.at(i).mid(13);
    }

    cs = QString("create table %1 (").arg(tablename);
    int idx = 0;
    dynt->firstElement();
    while(!dynt->isEnded())
    {
        if(idx != 0)
            cs.append(",\n\t");
        else
            cs.append("\n\t");
        cs.append( dynt->currentElementSqlName() );
        cs.append(" ");
        cs.append( dynt->currentElementIsNumeric() ?
                       QString("numeric(%1)").arg(numericsize) :
                       QString("varchar(%1)").arg(varcharlength) );

        ++idx;
        dynt->nextElement();
    }
    cs.append("\n);");
    return cs;
}

//End of gSAFE dmext.cpp
