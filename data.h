﻿/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    data.h
*/

#ifndef GSAFE__DATA_HEADER_FILE_X_
#define GSAFE__DATA_HEADER_FILE_X_

#include <QtCore>

/** \mainpage
 *  gSAFE is a General Sql dAtabase Front End multiplatform library written in C++ based on Qt (https://www.qt.io/).
 *  It is a complex system which define meta objects (Tables / Lists), which can autogenerate the graphical interfaces to manipulate the data.
 *	These meta objects can work together with Sql databases (PostgreSQL,MySQL,Sqlite,Odbc) handles timestamps,
 *	concurence writing etc.
 *	The meta objects contains all information about the data which need to manupulete, shows, store,
 *	print or even make a excel table from its, including the attributes for the gui to edit the fields.
 *	You can define this meta object from program code or Json data. After you defined the meta object they can be
 *	attached to gui or make a table. The gui is generated by the attributes of meta data table.
 *	This library enables you to develop sql based application really quick time.
 *	The base library is developed with Qt so the generated gui is a Qt based gui, work on all platform where the Qt runs.
 *	\n\n
 *	The base modules of gSAFE:
 *  \image  html pictures_doc/modules.png
 */

/** \defgroup data data */
/*  @{  */

/** The version of gsafe */
#define GSAFE_VERSION   "2.0.27"

/** Converts a double value to char * string
 *  @param v the double value to convert
 *  @param buffer the buffer wehre the string is written
 *  @param bufflen the size of the buffer. (limits the max length of converted string)
 *  @param min specified the minimum displayed decimal digit fragments.
 *  @param max specified the maximum displayed decimal digit fragments.
 *  @param group the displayed digits will be groupped by three digit.
 *      (if group==1 means "2334455.34" displayed "2 334 455.45" )
 *  @return 1 if the buffer is smaller size than needed, 0 otherwise */
int my_dtoa(double v,char *buffer,int bufflen,int min,int max,int group);

/** Converts a double value to QString. It uses the my_dtoa function inside.
 *  @see my_dtoa */
QString doubleToQString(double val,int min,int max,int group);

#define ROUND_MATH  0
#define ROUND_FLOOR 1
/** Round a double value.
*   @param val the value to round
*   @param mode the round mode. Can be ROUND_MATH or ROUND_FLOOR */
double dRound(double val,int mode=ROUND_MATH);

size_t h_strlcpy(char *dest, const char *src, size_t size);

/** The gSAFE base class.
 *  Holds the name of the class, so it's enable the dynamic query of the object type through the className() method. */
class HNamed
{
public:
    /** Empty constructor. It does nothing. */
    HNamed() {}

    /** This methos returns the typename of the object.
     *  It soubld redefine in every gSAFE class. */
    virtual QString className();
};

/** This class represents a single Sql value */
class HValue
{
public:
    enum HValueType {
        Null    = 0,
        Boolean = 1,
        String  = 2,
        Number  = 3,
    };

    HValue();
    HValue(const char* sv);
    HValue(QString     sv);
    HValue(QVariant    vv);
    HValue(QJsonValue  jv);
    HValue(int         iv);
    HValue(bool        bv);
    HValue(double      dv);

    void set();
    void set(const char* sv);
    void set(QString     sv);
    void set(QVariant    vv);
    void set(QJsonValue  jv);
    void set(int         iv);
    void set(bool        bv);
    void set(double      dv);

    QString toString();
    HValueType type();

    operator const QString();

    HValue& operator=(const char *sv);
    HValue& operator=(QString     sv);
    HValue& operator=(QVariant    vv);
    HValue& operator=(QJsonValue  jv);
    HValue& operator=(int         iv);
    HValue& operator=(bool        bv);
    HValue& operator=(double      dv);

protected:
    QString v;
    HValueType ot;

};

class HSql;

/** This is the base class of Sql related classes.
 *  It can set/query/change between Qt native or gSAFE rest databases.
 *  It does not have Sql related functions.
 *  The only function of this object is to set/switch between databases. */
class HSqlRelated
{
public:
    HSqlRelated();
    virtual QString currentDb();
    virtual HSqlRelated& db(QString databaseName = "");
    virtual HSql getSql();

private:
    QString overwrittenDbName;

};

/* @} */
#endif

//End of gSAFE data.h
