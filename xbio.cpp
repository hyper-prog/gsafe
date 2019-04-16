/*
    gSAFE - LIB
    general Sql dAtabase FrontEnd

   (C) 2011-2019 Peter Deak  (hyper80@gmail.com)

    License: GPLv2  http://www.gnu.org/licenses/gpl-2.0.html

        XBase Import/Export lib
    xbio.cpp
*/

/* WARNING: This file is depends on XBase library (http://linux.techass.com/projects/xdb/)
 * If you don't want to use the XBase support just remove
 * the xbio* files from your project, or undefine (remove) the ENABLE_XBASE_SUPPORT macro!
 */

#include "xbio.h"

#ifdef ENABLE_XBASE_SUPPORT

#include <xbase.h>

int cp852from128idx_utf16value[] =
{199 ,252 ,233 ,226 ,228 ,367 ,263 ,231 ,322 ,235 ,336 ,337 ,238 ,377 ,196 ,262 ,201 ,313 ,314 ,244 ,
 246 ,317 ,318 ,346 ,347 ,214 ,220 ,356 ,357 ,321 ,215 ,269 ,225 ,237 ,243 ,250 ,260 ,261 ,381 ,382 ,
 280 ,281 ,172 ,378 ,268 ,351 ,171 ,187 ,9617,9618,9619,9474,9508,193 ,194 ,282 ,350 ,9571,9553,9559,
 9565,379 ,380 ,9488,9492,9524,9516,9500,9472,9532,258 ,259 ,9562,9556,9577,9574,9568,9552,9580,164 ,
 273 ,272 ,270 ,203 ,271 ,327 ,205 ,206 ,283 ,9496,9484,9608,9604,354 ,366 ,9600,211 ,223 ,212 ,323 ,
 324 ,328 ,352 ,353 ,340 ,218 ,341 ,368 ,253 ,221 ,355 ,180 ,173 ,733 ,731 ,711 ,728 ,167 ,247 ,184 ,
 176 ,168 ,729 ,369 ,344 ,345 ,9632,160 };

int cp850from128idx_utf16value[] =
{199 ,252 ,233 ,226 ,228 ,224 ,229 ,231 ,234 ,235 ,232 ,239 ,238 ,236 ,196 ,197 ,201 ,230 ,198 ,244 ,
 246 ,242 ,251 ,249 ,255 ,214 ,220 ,248 ,163 ,216 ,215 ,402 ,225 ,237 ,243 ,250 ,241 ,209 ,170 ,186 ,
 191 ,174 ,172 ,189 ,188 ,161 ,171 ,187 ,9617,9618,9619,9474,9508,193 ,194 ,192 ,169 ,9571,9553,9559,
 9565,162 ,165 ,9488,9492,9524,9516,9500,9472,9532,227 ,195 ,9562,9556,9577,9574,9568,9552,9580,164 ,
 240 ,208 ,202 ,203 ,200 ,305 ,205 ,206 ,207 ,9496,9484,9608,9604,166 ,204 ,9600,211 ,223 ,212 ,210 ,
 245 ,213 ,181 ,254 ,222 ,218 ,219 ,217 ,253 ,221 ,175 ,180 ,173 ,177 ,8215,190 ,182 ,167 ,247 ,184 ,
 176 ,168 ,183 ,185 ,179 ,178 ,9632,160 };

HCodepageConvert::HCodepageConvert(void) {}
HCodepageConvert::~HCodepageConvert() {}

QString HCodepage852::conv_from(char * from)
{
    int i,l=strlen(from);
    QString f;
    f.fill(' ',l);
    for(i=0;i<l;++i)
        if((unsigned int)(from[i]) < 128u) //lower ascii not converted
            f[i] = QChar((unsigned int)from[i]);
        else
            f[i] = QChar(cp852from128idx_utf16value[(unsigned char)from[i]-128u]);
    return f;
}

void HCodepage852::conv_to(QString from,char *buffer,int bufflen)
{
    int i,ii,l=from.length();
    for(i=0;i<l && i<bufflen;++i)
        if((unsigned int)(from[i].unicode()) < 128u) //lower ascii not converted
            buffer[i] = (unsigned char)from[i].unicode();
        else
        {
            for(ii=0;ii<128;++ii)
                if((int)cp852from128idx_utf16value[ii] == (int)from[i].unicode())
                    break;
            buffer[i] = ii + 128;
        }
     if(i<bufflen)
        buffer[i]='\0';
}

QString HCodepage850::conv_from(char * from)
{
    int i,l=strlen(from);
    QString f;
    f.fill(' ',l);
    for(i=0;i<l;++i)
        if((unsigned int)(from[i]) < 128u) //lower ascii not converted
            f[i] = QChar((unsigned int)from[i]);
        else
            f[i] = QChar(cp850from128idx_utf16value[(unsigned char)from[i]-128u]);
    return f;
}

void HCodepage850::conv_to(QString from,char *buffer,int bufflen)
{
    int i,ii,l=from.length();
    for(i=0;i<l && i<bufflen;++i)
        if((unsigned int)(from[i].unicode()) < 128u) //lower ascii not converted
            buffer[i] = (unsigned char)from[i].unicode();
        else
        {
            for(ii=0;ii<128;++ii)
                if((int)cp850from128idx_utf16value[ii] == (int)from[i].unicode())
                    break;
            buffer[i] = ii + 128;
        }
     if(i<bufflen)
        buffer[i]='\0';
}

// ///////////////////////////////////////////////////////////////////////////////////// //

HXBaseFileHandler::HXBaseFileHandler(HCodepageConvert *conv,bool deletecobj)
{
    c = conv;
    xbase = new xbXBase();
    dbf   = new xbDbf(xbase);
    opened  = false;
    buffer= new char[CPCONV_BUFFERLENGTH];
    deleteconvertobject = deletecobj;
}

HXBaseFileHandler::~HXBaseFileHandler(void)
{
    delete dbf;
    delete xbase;
    delete []buffer;
    if(deleteconvertobject)
        delete c;
}

int HXBaseFileHandler::fieldCount(void)
{
    if(!opened)
        return -1;

    return dbf->FieldCount();
}

long HXBaseFileHandler::recordCount(void)
{
    if(!opened)
        return -1;

    xbULong recs;
    recs = dbf->NoOfRecords();
    return (long)recs;
}


QString HXBaseFileHandler::fieldName(int n)
{
    if(!opened)
        return "";
    return dbf->GetFieldName((xbShort)n);
}

HXBaseFieldType HXBaseFileHandler::fieldType(int n)
{
    if(!opened)
        return Error;

    char t;
    t = dbf->GetFieldType((xbShort)n);
    switch(t)
    {
        case 'C': return Char;
        case 'D': return Date;
        case 'L': return Logical;
        case 'N': return Numeric;
        case 'F': return Float;
        case 'M': return Memo;
        default:  return Error;
    }
    return Error;
}

// ///////////////////////////////////////////////////////////////////////////////////// //

HXBaseFileReader::HXBaseFileReader(HCodepageConvert *conv, bool deletecobj)
:HXBaseFileHandler(conv,deletecobj)
{
}

HXBaseFileReader::~HXBaseFileReader(void)
{
    if(opened)
        close();
}

int HXBaseFileReader::open(QString name)
{
    xbShort rc;

    if(opened)
    {
        emit errorSignal(QString("HXBaseFileReader::open : A database file already opened!"));
        return 1;
    }

    if(( rc = dbf->OpenDatabase(name.toLocal8Bit().constData())) != XB_NO_ERROR )
    {
       emit errorSignal(QString("HXBaseFileReader::open : Error opening \"%1\" file!").arg(name));
       return 1;
    }
    opened = true;
    return 0;
}

int HXBaseFileReader::close(void)
{
    if(!opened)
    {
        emit errorSignal(QString("HXBaseFileReader::open : There is no opened database!"));
        return 1;
    }
    dbf->CloseDatabase();
    opened = false;
    return 0;
}

int HXBaseFileReader::toRecord(long idx)
{
    xbShort rc;
    if(!opened)
        return 1;
    rc = dbf->GetRecord(idx);
    if(rc != XB_NO_ERROR)
    {
        emit errorSignal(QString("HXBaseFileReader::toRecord: Cannot seek to record: !").arg(idx));
        return 1;
    }
    return 0;
}

int HXBaseFileReader::firstRecord(void)
{
    if(!opened)
        return -1;

    xbShort rc;
    rc = dbf->GetFirstRecord();
    if( rc == XB_NO_ERROR )
        return 0;
    return 1;
}

int HXBaseFileReader::lastRecord(void)
{
    if(!opened)
        return -1;

    xbShort rc;
    rc = dbf->GetLastRecord();
    if( rc == XB_NO_ERROR )
        return 0;
    return 1;
}

int HXBaseFileReader::nextRecord(void)
{
    if(!opened)
        return -1;

    xbShort rc;
    rc = dbf->GetNextRecord();
    if(rc == XB_NO_ERROR)
        return 0;
    return 1;
}

int HXBaseFileReader::prevRecord(void)
{
    if(!opened)
        return -1;

    xbShort rc;
    rc = dbf->GetPrevRecord();
    if(rc == XB_NO_ERROR)
        return 0;
    return 1;
}

long HXBaseFileReader::getCurrentRecordIdx(void)
{
    return dbf->GetCurRecNo();
}

QString HXBaseFileReader::getFieldStr(QString fname)
{
    if(!opened)
        return "";
    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    //buffer  = dbf->GetFieldType(no);
    dbf->GetField(no,buffer);
    return c->conv_from(buffer).trimmed();
}

long HXBaseFileReader::getFieldDecimal(QString fname)
{
    if(!opened)
        return 0;
    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    //buffer  = dbf->GetFieldType(no);
    return dbf->GetLongField(no);
}

double HXBaseFileReader::getFieldFloat(QString fname)
{
    if(!opened)
        return 0;
    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    //buffer  = dbf->GetFieldType(no);
    return dbf->GetDoubleField(no);
}

bool HXBaseFileReader::getFieldBool (QString fname)
{
    if(!opened)
        return 0;
    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    //buffer  = dbf->GetFieldType(no);
    return (bool)(dbf->GetLogicalField(no));
}

QDate HXBaseFileReader::getFieldDate(QString fname)
{
    QDate date;

    if(!opened)
        return QDate();

    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    //buffer  = dbf->GetFieldType(no);
    dbf->GetField(no,buffer);

    date = QDate::fromString(c->conv_from(buffer),"yyyyMMdd");
    if(date.isValid())
        return date;

    date = QDate::fromString(c->conv_from(buffer),"yyyy.MM.dd");
    if(date.isValid())
        return date;

    date = QDate::fromString(c->conv_from(buffer),"yyyy-MM-dd");
    if(date.isValid())
        return date;

    return QDate();
}

QString HXBaseFileReader::getCellStr(long rec,QString fname)
{
    if(toRecord(rec))
        return QString();
    return getFieldStr(fname);
}

long HXBaseFileReader::getCellDecimal(long rec,QString fname)
{
    if(toRecord(rec))
        return 0;
    return getFieldDecimal(fname);
}

double HXBaseFileReader::getCellFloat(long rec,QString fname)
{
    if(toRecord(rec))
        return 0;
    return getFieldFloat(fname);
}

bool HXBaseFileReader::getCellBool(long rec,QString fname)
{
    if(toRecord(rec))
        return false;
    return getFieldBool(fname);
}

QDate HXBaseFileReader::getCellDate(long rec,QString fname)
{
    if(toRecord(rec))
        return QDate();
    return getFieldDate(fname);
}


// ///////////////////////////////////////////////////////////////////////////////////// //

HXBaseFileWriter::HXBaseFileWriter(HCodepageConvert *conv, bool deletecobj)
:HXBaseFileHandler(conv,deletecobj)
{
    defined_fnum = 0;
    schema = new xbSchema[XBASESCHEMA_MAXFIELD];
    setLastDefinedField(defined_fnum);
}

HXBaseFileWriter::~HXBaseFileWriter(void)
{
    delete schema;
}

void HXBaseFileWriter::setLastDefinedField(int idx)
{
    strncpy(schema[idx].FieldName,"",10);
    schema[idx].Type       = 0;
    schema[idx].FieldLen   = 0;
    schema[idx].NoOfDecs   = 0;
}

int HXBaseFileWriter::open(QString name)
{
    xbShort rc;

    if(opened)
    {
        emit errorSignal(QString("HXBaseFileWriter::open : A database file already opened!"));
        return 1;
    }

    if(( rc = dbf->OpenDatabase(name.toLocal8Bit().constData())) != XB_NO_ERROR )
    {
       emit errorSignal(QString("HXBaseFileWriter::open : Error opening \"%1\" file!").arg(name));
       return 1;
    }
    opened = true;
    return 0;
}

int HXBaseFileWriter::close(void)
{
    if(!opened)
    {
        emit errorSignal(QString("HXBaseFileWriter::open : There is no opened database!"));
        return 1;
    }
    dbf->CloseDatabase();
    opened = false;
    return 0;
}

void HXBaseFileWriter::defineField(QString name,HXBaseFieldType type,int length,int nofdecimals)
{
    if(opened)
    {
        emit errorSignal(QString("HXBaseFileWriter::defineField : A database file already opened, You can ONLY define field before creation!"));
        return;
    }

    int localtype;
    int locallength=length;
    if(type == Numeric)     { localtype = XB_NUMERIC_FLD;                       }
    if(type == Char)        { localtype = XB_CHAR_FLD;                          }
    if(type == Date)        { localtype = XB_DATE_FLD;      locallength = 8;    }
    if(type == Float)       { localtype = XB_FLOAT_FLD;                         }
    if(type == Logical)     { localtype = XB_LOGICAL_FLD;   locallength = 1;    }
    if(type == Memo)
    {
        emit errorSignal(QString("HXBaseFileWriter::defineField : The field has \"Memo\" type! I don't support it, sorry :-("));
        return;
    }
    if(type == Error)
    {
        emit errorSignal(QString("HXBaseFileWriter::defineField : The field has \"Error\" type!"));
        return;
    }

    strncpy(schema[defined_fnum].FieldName,name.toLocal8Bit().constData(),10);
    schema[defined_fnum].Type       = localtype;
    schema[defined_fnum].FieldLen   = locallength;
    schema[defined_fnum].NoOfDecs   = nofdecimals;

    setLastDefinedField(++defined_fnum);
}

int HXBaseFileWriter::create(QString name,int version)
{
    xbShort rc;

    if(opened)
    {
        emit errorSignal(QString("HXBaseFileWriter::open : A database file already opened!"));
        return 1;
    }
    dbf->SetVersion( version );
    if(( rc = dbf->CreateDatabase( name.toLocal8Bit().constData() , schema , XB_OVERLAY ))  != XB_NO_ERROR )
    {
        emit errorSignal(QString("HXBaseFileWriter::open : Error opening \"%1\" file!").arg(name));
        return 1;
    }
    opened = true;
    resetRecord();
    return 0;
}


int HXBaseFileWriter::resetRecord(void)
{
    if(!opened)
        return 1;
    dbf->BlankRecord();
    return 0;
}

int HXBaseFileWriter::appendRecord(void)
{
    if(!opened)
        return 1;

    xbShort rc;
    if(( rc = dbf->AppendRecord()) != XB_NO_ERROR )
    {
        emit errorSignal(QString("HXBaseFileWriter::appendRecord : Append record is unsuccessful!"));
        return 1;

    }
    resetRecord();
    return 0;
}

int HXBaseFileWriter::setFieldStr(QString fname,QString str)
{
    if(!opened)
        return 1;

    xbShort no;
    int max=CPCONV_BUFFERLENGTH;

    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    if(dbf->GetFieldLen(no) < CPCONV_BUFFERLENGTH)
        max = dbf->GetFieldLen(no);
    c->conv_to(str,buffer,max);
    dbf->PutField(no,buffer);
    return 0;
}

int HXBaseFileWriter::setFieldDecimal(QString fname,long decimal)
{
    if(!opened)
        return 1;
    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    dbf->PutLongField(no,decimal);
    return 0;
}

int HXBaseFileWriter::setFieldFloat(QString fname,double val)
{
    if(!opened)
        return 1;

    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    dbf->PutDoubleField(no,val);
    return 0;
}

int HXBaseFileWriter::setFieldBool(QString fname,bool logical)
{
    if(!opened)
        return 1;

    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    dbf->PutField(no,logical ? "T" : "F");
    return 0;
}

int HXBaseFileWriter::setFieldDate(QString fname,QDate date)
{
    if(!opened)
        return 1;

    xbShort no;
    no = dbf->GetFieldNo(fname.toLocal8Bit().constData());
    c->conv_to(date.toString("yyyyMMdd"),buffer,8);
    dbf->PutField(no,buffer);
    return 0;
}

#endif // ENABLE_XBASE_SUPPORT

//end code.
