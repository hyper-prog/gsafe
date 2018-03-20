/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2018 Peter Deak  (hyper80@gmail.com)

    License: GPLv2  http://www.gnu.org/licenses/gpl-2.0.html

    docgen.cpp
*/

#include <QtCore>
#include <QtXml>

#include "datalib.h"
#include "dconsole.h"
#include "docgen.h"

HyperDoc::HyperDoc(QString n)
{
    sdebug("*** HyperDoc::HyperDoc ***");
    name = n;
    ignore = false;
    itermode = false;
    resourceProviders = new QList<HResourceProvider *>();
    dtsp = new QList<HTable *>();
    dtsn = new QList<QString>();
    fts  = new QList<HFloatTables *>();
    insideIter = NULL;
    iter_hft_name = "";
    iter_notfound = "";
    skip_cond_deep = 0;
    clear();
    clearResources();
    sdebug("*** HyperDoc::HyperDoc ***END");
}

HyperDoc::~HyperDoc() 
{
    sdebug("*** HyperDoc::~HyperDoc ***");
    clear();
    clearResources();
    delete resourceProviders;
    delete dtsp;
    delete dtsn;
    delete fts;
    sdebug("*** HyperDoc::~HyperDoc ***END");
}

void HyperDoc::clear()
{
    body = "";
}

void HyperDoc::set(QString docstr)
{
    body = docstr;
}

void HyperDoc::add(QString docstr)
{
    body.append(docstr);
}

void HyperDoc::clearResources()
{
    resourceProviders->clear();
    dtsp->clear();
    dtsn->clear();
    fts->clear();
}

void HyperDoc::addDataSource(HTable *ht,QString redefinedName)
{
    if(ht == NULL)
        return;
    dtsp->push_back(ht);
    if(redefinedName.isEmpty())
        dtsn->push_back(ht->sqlTableName());
    else
        dtsn->push_back(redefinedName);
}

void HyperDoc::addResourceProvider(HResourceProvider *rp)
{
    resourceProviders->push_back(rp);
}

void HyperDoc::addIterDataTables(HFloatTables *ft)
{
    fts->push_back(ft);
}

QString HyperDoc::getRawDoc()
{
    return body;
}

QString HyperDoc::getData(QString data)
{
    if(data.isEmpty())
        return QString("");
    
    HTable *req_t;
    HDataField *req_f;
    QString tname,fname;
    QList<QString>::iterator namei;
    QList<HTable *>::iterator datai;

    tname = data.section(':',0,0);
    fname = data.section(':',1,1);
    

    if(tname == "*")
    {
        sdebug(QString("Data requested from table \"%1\" with name \"%2\"").arg(tname).arg(fname));
        if(insideIter != NULL)
        {
            req_f = insideIter->fieldBySqlName(fname,false);
            if(req_f != NULL)
                return req_f->dbValueToDispValue(req_f->getValue()).toString();
        }
    }
    else
    {
        QString mainname,subname;

        mainname = tname.section('.',0,0);
        subname = tname.section('.',1,1);

        if(subname.isEmpty())
            subname = mainname;

        sdebug(QString("Data requested from table \"%1\" (subnamed:\"%2\") with name \"%3\"")
                            .arg(mainname).arg(subname).arg(fname));
        req_t = getDataTable(mainname);
        if(req_t != NULL)
        {
            req_f = req_t->fieldBySqlName(fname,true,subname);
            if(req_f != NULL)
                    return  req_f->dbValueToDispValue(req_f->getValue()).toString();
        }
    }
    return QString("");
}

HTable* HyperDoc::getDataTable(QString data)
{
    QList<QString>::iterator namei;
    QList<HTable *>::iterator datai;
    
    namei = dtsn->begin();
    datai = dtsp->begin();
    while(namei != dtsn->end())
    {   
        if(*namei == data)
        {
            return *datai;
        }
        ++namei;
        ++datai;
    }
    return NULL;
}

QString HyperDoc::getRes(QString res)
{
    if(res.isEmpty())
        return QString("");

    QList<HResourceProvider *>::iterator i;

    i = resourceProviders->begin();
    while(i != resourceProviders->end())
    {
        if((*i)->hasResource(res))
            return (*i)->getResource(res);
        ++i;
    }
    return QString("");
}

HFloatTables* HyperDoc::getHFT(QString res)
{
    if(res.isEmpty())
        return NULL;

    QList<HFloatTables *>::iterator i;

    i = fts->begin();
    while(i != fts->end())
    {
        if((*i)->sqlTableName() == res)
            return (*i);
        ++i;
    }
    return NULL;
}

QString HyperDoc::generateDoc()
{
    QString parserdata;

    sdebug("*** HyperDoc::generateDoc ***");

    parserdata = body;
    parserdata.replace("[","<");
    parserdata.replace("]",">");
    parserdata.replace("|","\"");
    QXmlInputSource *source =new QXmlInputSource();
    source->setData(parserdata);

    errstr="";
    QXmlSimpleReader reader;
    reader.setContentHandler( this );
    reader.setErrorHandler( this );
    preprocessed = "";
    ignore = false;
    itermode = false;
    iterbuffer = "";
    iter_hft_name = "";
    iter_notfound = "";
    if(!reader.parse( *source ))
    {
        emit errorSignal("Error during the XML parsing! \n: " + errorString());
        delete source;
        sdebug("*** HyperDoc::generateDoc ***ERROR");
        sdebug(errstr);
        return "";
    }
    delete source;
    source = NULL;
    sdebug("*** HyperDoc::generateDoc ***");
    return preprocessed;
}

////////////////////////////////////////////////////////////////////////////////
void HyperDoc::out(QString o)
{
    if(!itermode)
        preprocessed.append(o);
    else
        iterbuffer.append(o);
}

bool HyperDoc::startDocument(void)
{
    out("");
    return true;
}

bool HyperDoc::endDocument(void)
{
    out("");
    return true;
}

bool HyperDoc::startElement( const QString& ns, const QString& ln, const QString& name,const QXmlAttributes& atts)
{
    Q_UNUSED(ns);
    Q_UNUSED(ln);

    int i;

    if(!buffer.isEmpty())
    {
        out(buffer);
        clearBuffer();
    }

    if(ignore && name.toLower() == "cond") //we already in an ignored part
    {
        ++skip_cond_deep;
        sdebug("------------- Increment skip_cond_deep");
        return true;
    }

    if(ignore)
        return true;

    if(!itermode)
     if(name.toLower() == "data" || name.toLower() == "res")
     {
         att_begin   = atts.value("begin");
         att_end     = atts.value("end");
         att_notfound= atts.value("notfound");
         return true;
     }

    if(name.toLower() == "iterinside")
        return true;
    
    if(name.toLower() == "iter")
    {   
        if(itermode)
            emit errorSignal("ERROR:Cannot paste an ITER tag into an another ITER!");

        iter_hft_name = atts.value("data");
        iter_notfound = atts.value("notfound");
        iterbuffer = "<iterinside>";
        itermode=true;
        return true;
    }

    if(!itermode)
     if(name.toLower() == "cond")
     {
         QString a,b;
    
         if(!atts.value("res").isEmpty())
             a = getRes(atts.value("res"));
         if(!atts.value("data").isEmpty())
             a = getData(atts.value("data"));
    
         b = atts.value("to");

         sdebug(QString("HyperDoc-COND \"%1\"  with  \"%2\"  ;").arg(a).arg(b));

         if(atts.value("is") == "eq")
         {
             if(a == b)
             {
                 sdebug(" - (eq) result:true ");
                 ignore = false;
             }
             else
             {
                 sdebug(" - (eq) result:false ");
                 ignore = true;
             }
         }
         else if(atts.value("is") == "ne")
         {
             if(a != b)
             {
                 sdebug(" - (ne) result:true ");
                 ignore = false;
             }
             else
             {
                 sdebug(" - (ne) result:false ");
                 ignore = true;
             }
         }
         else
             ignore = true;
         return true;
     }

    //// XML copy part
    buffer = "";
    out("<"+name);
    for(i=0;i < atts.count();++i)
    {
        out(QString(" %1=\"%2\"").arg(atts.qName(i)).arg(atts.value(i)));
    }

    out(">");
    return true;
}

bool HyperDoc::endElement( const QString& ns, const QString& ln, const QString& name)
{
    Q_UNUSED(ns);
    Q_UNUSED(ln);
    //The data and res tags have to arrive before the emptiyng of the buffer because
    //they works from buffer. The other not.
    if(!ignore && !itermode)
     if(name.toLower() == "data" || name.toLower() == "res")
     {
         QString data;

         if(name.toLower() == "data")  data=getData(buffer);
         if(name.toLower() == "res")   data=getRes (buffer);

         if(data.isEmpty())
             out(att_notfound);
         else
             out(att_begin+data+att_end);
         clearBuffer();
         return true;
     }

    if(!buffer.isEmpty())
    {
        out(buffer);
        clearBuffer();
    }

    if(!itermode)
     if(name.toLower() == "cond")
     {
         if(ignore && skip_cond_deep > 0)
         {
             --skip_cond_deep;
             sdebug("------------- Decrement skip_cond_deep");
             return true;
         }
         sdebug("Reached end of cond: set ingnore to false");
         ignore = false;
         return true;
     }

    if(name.toLower() == "iterinside")
        return true;
        
    if(name.toLower() == "iter")
    {
        int i;
        HFloatTables *flott;
        HyperDoc *subdoc;

        iterbuffer.append("</iterinside>");
        itermode=false;
        subdoc =new HyperDoc(name);
        //copying resources:
        *(subdoc->resourceProviders) = *resourceProviders;
        *(subdoc->dtsp)              = *dtsp;
        *(subdoc->dtsn)              = *dtsn;
        *(subdoc->fts)               = *fts;
        
        subdoc->set(iterbuffer);

        flott = getHFT(iter_hft_name);
        if(flott == NULL || flott->count() == 0)
            out(iter_notfound);
        else
        {
            for(i=0;i<flott->count();++i)
            {
                subdoc->insideIter = flott->getTByNum(i);
                out(subdoc->generateDoc());
            }
        }
        iter_hft_name = "";
        iter_notfound = "";
        iterbuffer = "";
        delete subdoc;
    }

    if(ignore)
        return true;

    ///// XML copy part
    buffer = "";
    out(QString("</%1>").arg(name));
    return true;
}

bool HyperDoc::characters(const QString& ch)
{
    if(!ignore)
        buffer.append(ch);
    return true;
}

bool HyperDoc::processingInstruction(const QString &target,const QString &data)
{
    sdebug("OOOOOOOOOOOOOOOOOOOOOOOOOO"+target+","+data);
    return true;
}

bool HyperDoc::skippedEntity(const QString &name)
{
    sdebug("IIIIIIIIIIII Skipped entry:"+name);
    return true;
}

void HyperDoc::clearBuffer()
{
    buffer = "";
    att_begin = "";
    att_end = "";
    att_notfound = "";
}

bool HyperDoc::warning(const QXmlParseException& exception)
{
    errstr = QString("WARNING!\nLIN:%1 COL:%2\npubID:%3 sysID:%4\nMessage:%5\nSorry :-(")
        .arg(exception.lineNumber())
        .arg(exception.columnNumber())
        .arg(exception.publicId())
        .arg(exception.systemId())
        .arg(exception.message());
    return false;
}

bool HyperDoc::error(const QXmlParseException& exception)
{
    errstr = QString("WARNING!\nLIN:%1 COL:%2\npubID:%3 sysID:%4\nMessage:%5\nSorry :-(")
        .arg(exception.lineNumber())
        .arg(exception.columnNumber())
        .arg(exception.publicId())
        .arg(exception.systemId())
        .arg(exception.message());
    return false;
}

bool HyperDoc::fatalError(const QXmlParseException& exception)
{
    errstr = QString("WARNING!\nLIN:%1 COL:%2\npubID:%3 sysID:%4\nMessage:%5\nSorry :-(")
        .arg(exception.lineNumber())
        .arg(exception.columnNumber())
        .arg(exception.publicId())
        .arg(exception.systemId())
        .arg(exception.message());
    return false;
}
//end code.
