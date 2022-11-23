/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    data.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore>

#include "data.h"
#include "dconsole.h"
#include "builder.h"
#include "database.h"

using namespace std;

QString HNamed::className()
{
    return "HNamed";
}

// ///////////////////////////////////////////////////////////////////// //

GSafeException::GSafeException(const char* err)
 : std::exception()
{
    errMsg = strdup(err);
}

const char *GSafeException::what() const noexcept
{
    return errMsg;
}

// ///////////////////////////////////////////////////////////////////// //

HValue::HValue()
{
    set();
}

HValue::HValue(QString sv)
{
    set(sv);
}

HValue::HValue(const char *sv)
{
    set(sv);
}

HValue::HValue(QVariant vv)
{
    set(vv);
}

HValue::HValue(QJsonValue jv)
{
    set(jv);
}

HValue::HValue(int iv)
{
    set(iv);
}

HValue::HValue(bool bv)
{
    set(bv);
}

HValue::HValue(double dv)
{
    set(dv);
}


void HValue::set()
{
    ot = Null;
    v = "";
}

void HValue::set(QString sv)
{
    ot = String;
    v = sv;
}

void HValue::set(const char *sv)
{
    ot = String;
    v = sv;
}

void HValue::set(QVariant vv)
{
    ot = String;
    if(!strcmp(vv.typeName(),"qlonglong") || !strcmp(vv.typeName(),"double"))
        ot = Number;
    if(!strcmp(vv.typeName(),"bool"))
        ot = Boolean;
    v = vv.toString();
}

void HValue::set(QJsonValue jv)
{
    if(jv.isString())
    {
        ot = String;
        v = jv.toString();
    }

    if(jv.isBool())
    {
        ot = Boolean;
        v = jv.toBool() ? "true": "false";
    }

    if(jv.isDouble())
    {
        ot = Number;
        v = QString::asprintf("%f",jv.toDouble());
        if(v.indexOf(".") != -1)
        {
            while(v.startsWith("0"))
                v = v.mid(1);
            while(v.endsWith("0"))
            {
                int len = v.length();
                v = v.left(len-1);
            }
            if(v.endsWith("."))
            {
                int len = v.length();
                v = v.left(len-1);
            }
        }
    }

    if(jv.isNull())
    {
        ot = Null;
        v = "";
    }
}

void HValue::set(int iv)
{
    ot = Number;
    v = QString::asprintf("%d",iv);
}

void HValue::set(bool bv)
{
    ot = Boolean;
    v = bv ? "true" : "false";
}
void HValue::set(double dv)
{
    ot = Number;
    v = QString::asprintf("%f",dv);
}

QString HValue::toString()
{
    return v;
}

int HValue::toInt()
{
    return toIntDefExc(0,true);
}

int HValue::toIntForced(int fallbackValue)
{
    return toIntDefExc(fallbackValue,false);
}

int HValue::toIntDefExc(int fallbackValue, bool throwException)
{
    bool ok;
    if(ot != Number && ot != String)
    {
        if(throwException)
            throw GSafeException("Called HValue::toInt on a not mathing value type");
        return fallbackValue;
    }
    if(v.isEmpty())
        return fallbackValue;

    int iv = v.toInt(&ok);
    if(!ok)
    {
        if(throwException)
            throw GSafeException("Called HValue::toInt on a not integer value");
        return fallbackValue;
    }
    return iv;
}

double HValue::toDouble()
{
    return toDoubleDefExc(0.0,true);
}

double HValue::toDoubleForced(double fallbackValue)
{
    return toDoubleDefExc(fallbackValue,false);
}

double HValue::toDoubleDefExc(double fallbackValue, bool throwException)
{
    bool ok;
    if(ot != Number && ot != String)
    {
        if(throwException)
            throw GSafeException("Called HValue::toDouble on a not mathing value type");
        return fallbackValue;
    }
    if(v.isEmpty())
        return fallbackValue;
    double dv = v.toDouble(&ok);
    if(!ok)
    {
        if(throwException)
            throw GSafeException("Called HValue::toDouble on a not numeric value");
        return fallbackValue;
    }
    return dv;
}

bool HValue::toBool()
{
    return toBoolDefExc(false,true);
}

bool HValue::toBoolForced(bool fallbackValue)
{
    return toBoolDefExc(fallbackValue,false);
}

bool HValue::toBoolDefExc(bool fallbackValue, bool throwException)
{
    if(ot == Null)
        return false;
    if(v.isEmpty() || v == "0" || v.toLower() == "f" || v.toLower() == "false")
        return false;
    if(v == "1" || v.toLower() == "t" || v.toLower() == "true")
        return true;
    if(throwException)
        throw GSafeException("Called HValue::toBool on a corrupt value");
    return fallbackValue;
}

HValue::HValueType HValue::type()
{
    return ot;
}

HValue::operator const QString()
{
    return v;
}

HValue& HValue::operator=(QString sv)
{
    set(sv);
    return *this;
}

HValue& HValue::operator=(const char *sv)
{
    set(sv);
    return *this;
}

HValue& HValue::operator=(QVariant vv)
{
    set(vv);
    return *this;
}

HValue& HValue::operator=(QJsonValue jv)
{
    set(jv);
    return *this;
}

HValue& HValue::operator=(int iv)
{
    set(iv);
    return *this;
}

HValue& HValue::operator=(bool bv)
{
    set(bv);
    return *this;
}

HValue& HValue::operator=(double dv)
{
    set(dv);
    return *this;
}

// /////////////////////////////////////////////////////////////////////// //

HSqlRelated::HSqlRelated()
{
    overwrittenDbName = "";
}

QString HSqlRelated::currentDb()
{
    return overwrittenDbName;
}

HSqlRelated& HSqlRelated::db(QString databaseName)
{
    overwrittenDbName = databaseName;
    return *this;
}

HSql HSqlRelated::getSql()
{
    return HSql(overwrittenDbName);
}

// ////////////////////////////////////////////////////////////////////// //

size_t h_strlcpy(char *dest, const char *src, size_t size)
{
    size_t ret = strlen(src);

    if (size) {
        size_t len = (ret >= size) ? size - 1 : ret;
        memcpy(dest, src, len);
        dest[len] = '\0';
    }
    return ret;
}

int my_dtoa(double v,char *buffer,int bufflen,int min,int max,int group)
{
    int digitnum;
    int i,forlength;
    int length=0; //the currnt filled length of the buffer

    char digit;
    char *str = buffer;

    unsigned long int i_ip,i_fp,idigit_value;
    double ip,fp;

    bufflen -= 2; //decrease bufflen value, to avoid decreasing in every if

    if(isnan(v))
    {
        if(bufflen < 4)
            return 1;
        h_strlcpy(str,"NaN",bufflen);
        return 0;
    }
    if(isinf(v))
    {
        if(bufflen < 4)
            return 1;
        h_strlcpy(str,"Inf",bufflen);
        return 0;
    }

    //split the number to integer and fractional part.
    fp = fabs(modf(v,&ip));
    ip = fabs(ip);
    if(fp != 0.0)
    {
        fp *= pow(10.0,max);
        fp = floor(fp + 0.5);
    }
    i_ip=ip;
    i_fp=fp;

    //If the original (rounded) number is negative put the sign to front
    v *= pow(10.0,max);
    v = floor(v + 0.5);
    if (v < 0)
    {
        *(str++) = '-';
        ++length;
        v = -v;
    }

    //Generate integer part (from i_ip)
    idigit_value = 1;
    digitnum = 1;
    while(idigit_value*10 <= i_ip)
    {
        idigit_value *= 10;
        ++digitnum;
    }
    forlength=0;
    while(idigit_value >= 1)
    {
        //put grouping space if set
        if(group && forlength != 0 && digitnum % 3 == 0)
        {
            *(str++) = ' ';
            ++length;
            if(length >= bufflen)
            {
                *(str) = '\0';
                return 1;
            }
        }

        digit = static_cast<char>((i_ip - i_ip%idigit_value) / idigit_value);
        i_ip = i_ip%idigit_value;

        *(str++) = '0' + digit%10;
        ++length;
        --digitnum;
        ++forlength;
        idigit_value /= 10;

        if(length >= bufflen)
        {
            *(str) = '\0';
            return 1;
        }
    }

    //Generate fractional part (from i_fp)
    digitnum=0;
    if( i_fp > 0 )
    {
        *(str++) = '.';
        ++length;

        idigit_value = 1;
        for(i=0;i<max-1;++i)
            idigit_value *= 10;

        while (idigit_value >= 1)
        {
            if(group && digitnum && digitnum%3 == 0)
            {
                *(str++) = ' ';
                ++length;
                if(length >= bufflen)
                {
                    *(str) = '\0';
                    return 1;
                }
            }

            digit = static_cast<char>((i_fp - i_fp%idigit_value) / idigit_value);
            i_fp = i_fp%idigit_value;

            *(str++) = '0' + digit%10;
            ++length;
            ++digitnum;
            idigit_value /= 10;

            if(length >= bufflen)
            {
                *(str) = '\0';
                return 1;
            }

            if(digitnum >= min && i_fp == 0)
                break;
        }
    }
    else
    {   //the original number was an integer, so we fill the minimal fractional part with zeros
        if(min > 0)
        {
            *(str++) = '.';
            ++length;
            for(digitnum=0;digitnum<min;)
            {
                if(group && digitnum && digitnum%3 == 0)
                {
                    *(str++) = ' ';
                    ++length;
                    if(length >= bufflen)
                    {
                        *(str) = '\0';
                        return 1;
                    }
                }
                *(str++) = '0';
                ++length;
                ++digitnum;
                if(length >= bufflen)
                {
                    *(str) = '\0';
                    return 1;
                }
            }
        }
    }
    *str = '\0';
    return 0;
}

QString doubleToQString(double val,int min,int max,int group)
{

    //return QVariant(QString("%1%2").arg(v.toDouble(),0,'f',2).arg(tailstr.isEmpty() ? "" : (" "+tailstr)));
    //return QVariant(QString("").sprintf("%.2f",v.toDouble()) + (tailstr.isEmpty() ? "" : (" "+tailstr)));

    QString v;
    char buffer[128];
    my_dtoa(val,buffer,128,min,max,group);
    v = buffer;
    return v;
}

double dRound(double val,int mode)
{
    double intpart = floor(val);
    if(mode == ROUND_FLOOR)
        return intpart;
    if( (val-intpart) >= 0.5 )
        return (intpart + 1.0);
    return intpart;
}


//End of gSAFE data.cpp
