/*
    gSAFE - LIB
    general Sql dAtabase FrontEnd

   (C) 2011-2022 Peter Deak  (hyper80@gmail.com)

    License: LGPLv2.1

        XBase Import/Export lib
    xbio.h
*/

/* This module is not depends from gSAFE
 * Can be used separatedly (Just add xbio.h/xbio.cpp to your project)
 *
 * WARNING: This file's .cpp is depends on XBase library (http://linux.techass.com/projects/xdb/)
 * If you don't want to use the XBase support just remove
 * the xbio* files from your project, or undefine (remove) the ENABLE_XBASE_SUPPORT macro!
 */

#ifdef ENABLE_XBASE_SUPPORT

#ifndef GSAFE_XBASE_IO_H
#define GSAFE_XBASE_IO_H

#include <QtCore>

/** \defgroup xbaseio xbaseio */
/* @{ */

#define CPCONV_BUFFERLENGTH     256
#define XBASESCHEMA_MAXFIELD    64

class xbXBase;
class xbDbf;
class xbSchema;

enum HXBaseFieldType    {
                           Error,
                           Numeric,
                           Char,
                           Date,
                           Float,
                           Logical,
                           Memo
                        };

/** The codepage converter abstract class.
 *  The HXBaseFileHandler class use it to set the coding of the dbf file. */
class HCodepageConvert
{

public:
    HCodepageConvert(void);

    virtual QString conv_from(char * from) = 0;
    virtual void    conv_to(QString from,char * buffer,int bufflen) = 0;
    virtual ~HCodepageConvert();
};

/** The codpage converter class for IBM 852 codepage. */
class HCodepage852 : public HCodepageConvert
{

public:
    HCodepage852(void) {}

    virtual QString conv_from(char * from);
    virtual void    conv_to(QString from,char * buffer,int bufflen);
};

/** The codpage converter class for IBM 850 codepage. */
class HCodepage850 : public HCodepageConvert
{

public:
    HCodepage850(void) {}

    virtual QString conv_from(char * from);
    virtual void    conv_to(QString from,char * buffer,int bufflen);
};

/** The XBase interface base class. Makes a interface to XBase lib, make possible to
 *  create/read/write .DBF files.
 *  @see HXBaseFileReader
 *  @see HXBaseFileWriter */
class HXBaseFileHandler : public QObject
{
    Q_OBJECT

protected:
    HCodepageConvert *c;
    char *buffer;
    xbXBase *xbase;
    xbDbf   *dbf;
    bool opened;
    bool deleteconvertobject;

public:
    /** Creates a standard XBase file handler object.
     *  Use HXBaseFileReader or HXBaseFileWriter instead! */
    HXBaseFileHandler(HCodepageConvert *conv,bool deletecobj=false);
    /** Destructor */
    ~HXBaseFileHandler(void);

    /** Returns the number of fields */
    int  fieldCount(void);
    /** Returns the name of the specified field */
    QString fieldName(int n);
    /** Returns the type of the specified field */
    HXBaseFieldType fieldType(int n);
    /** Returns the total record number of the opened file */
    long recordCount(void);

signals:
    /** This signal is emitted, when error occured. The "error" string contains the error message. */
    void errorSignal(QString error);
};

/** You can read one XBase files with this class.
 *  \code
    HXBaseFileReader reader(new HCodepage852(),true);
    connect(&reader,SIGNAL(errorSignal(QString)),this,SLOT(slotError(QString)));

    reader.open("DATA.DBF");
    r = reader.firstRecord();
    while(r == 0)
    {
        toDo( reader.getFieldStr("STREET") );
        r = reader.nextRecord();
    }
    reader.close();
    \endcode
    @see HXBaseFileWriter */
class HXBaseFileReader : public HXBaseFileHandler
{
    Q_OBJECT

public:
    /** Creates a XBase file reader object
     *  @param conv The codepage converter of the class
     *      (put a HCodepageConvert descendant object according to the coding of the dbf file)
     *  @param deletecobj if this parameter true the "conv" object is deleted if not used. */
    HXBaseFileReader(HCodepageConvert *conv,bool deletecobj=false);
    /** Destructor */
    ~HXBaseFileReader(void);

    /** Opens a XBase file.
    *   @param name the path and name of the file
    *   @return 0 if success */
    int open(QString name);
    /** Close the currently opened file */
    int close(void);

    /** Jump to the specified record, and makes it current
     *  @param idx the index of the necessary record
     *  @return 0 if success */
    int toRecord(long idx);
    /** Jump to the first record, and makes it current
      \code
        int r = reader.firstRecord();
        while(r == 0)
        {
            toDo( reader.getFieldStr("FIELD1") );
            r = reader.nextRecord();
        }
      \endcode
     *  @return 0 if success
     *  @see nextRecord() */
    int firstRecord(void);
    /** Jump to the last record, and makes it current
     *  @return 0 if success
     *  @see firstRecord() */
    int lastRecord(void);
    /** Jump to the next record, and makes it current
     *  @return 0 if success (if we reach the end got 1)
     *  @see firstRecord()*/
    int nextRecord(void);
    /** Jump to the next record, and makes it current
     *  @return 0 if success (if we reach the end got 1)
     *  @see nextRecord()*/
    int prevRecord(void);
    /** Returns the index of the current record */
    long getCurrentRecordIdx(void);

    /** Get a value of a named field from the current record */
    QString getFieldStr    (QString fname);
    /** Get a value of a named field from the current record */
    long    getFieldDecimal(QString fname);
    /** Get a value of a named field from the current record */
    double  getFieldFloat  (QString fname);
    /** Get a value of a named field from the current record */
    bool    getFieldBool   (QString fname);
    /** Get a value of a named field from the current record */
    QDate   getFieldDate   (QString fname);


    QString getCellStr    (long rec,QString fname);
    long    getCellDecimal(long rec,QString fname);
    double  getCellFloat  (long rec,QString fname);
    bool    getCellBool   (long rec,QString fname);
    QDate   getCellDate   (long rec,QString fname);

};

/** You can create and write one XBase files with this class.
 *  - Define the scheme
 *  - Create the file
 *  - Write the records
 *  - Close the file
 *
 *  \code
    HXBaseFileWriter writer(new HCodepage852(),true);
    writer.defineField("NAME",Char,20);
    writer.defineField("MONEY",Numeric,10);

    writer.create("OUTPUT.DBF",3); //Opens the file

    writer.setFieldStr("NAME","Big Joe");
    writer.setFieldDecimal("MONEY",176);
    writer.appendRecord();

    writer.setFieldStr("NAME","Alice Newmann");
    writer.setFieldDecimal("MONEY",3169);
    writer.appendRecord();

    writer.close();
\endcode
 *  @see HXBaseFileReader*/
class HXBaseFileWriter : public HXBaseFileHandler
{
    Q_OBJECT

protected:
    xbSchema *schema;
    int       defined_fnum;

    void setLastDefinedField(int idx);

public:
    /** Creates a XBase file writer object
     *  @param conv The codepage converter of the class
     *      (put a HCodepageConvert descendant object according to the coding of the dbf file)
     *  @param deletecobj if this parameter true the "conv" object is deleted if not used. */
    HXBaseFileWriter(HCodepageConvert *conv,bool deletecobj=false);
    /** Destructor */
    ~HXBaseFileWriter(void);

public:
    /** Define a field in the schema of XBase file.
     *  You can only use this function before create() !
     *  The field specified with this function will be appended to the existing field list.
     *  @param name the name of the field (limited to 10 char. XBase limit)
     *  @param type the type of the field. See HXBaseFieldType enum
     *  @param length the length of the field.
     *  @param nofdecimals the number of decimals */
    void defineField(QString name,HXBaseFieldType type,int length=0,int nofdecimals=0);

    /** Creates the XBase file. You have to specify the schema before with the defineField() function.
     *  After calling this function the dbf is opened. (Do not call the open()!)
     *  @param name the path and name of the file
     *  @param version the version of the file (eg: 3 or 4)*/
    int create(QString name,int version);
    /** Opens a XBase file to write. Do not call create() if you call this func!
    *   @param name the path and name of the file
    *   @return 0 if success */
    int open(QString name);
    /** Closes the currently opened XBase file */
    int close(void);

    /** Empty the current record buffer. */
    int resetRecord(void);
    /** Append the current record buffer to the end of the file, and empty the buffer. */
    int appendRecord(void);

    /** Set a value of a named field in the current record buffer */
    int setFieldStr    (QString fname,QString str);
    /** Set a value of a named field in the current record buffer */
    int setFieldDecimal(QString fname,long decimal);
    /** Set a value of a named field in the current record buffer */
    int setFieldFloat  (QString fname,double val);
    /** Set a value of a named field in the current record buffer */
    int setFieldBool   (QString fname,bool logical);
    /** Set a value of a named field in the current record buffer */
    int setFieldDate   (QString fname,QDate date);

};

/* @} */

#endif // GSAFE_XBASE_IO_H

#endif // ENABLE_XBASE_SUPPORT
