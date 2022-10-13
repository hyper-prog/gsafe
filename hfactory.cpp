/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2021 Peter Deak  (hyper80@gmail.com)

    License: LGPLv2.1

    hfactory.cpp
*/

#include "datalib.h"
#include "hfactory.h"
#include "dconsole.h"

//#define PARSER_DEBUG

Hff_node::Hff_node(QString namep,HDataField *fp)
{
    f=fp;
    name=namep;
}

Hff_node::~Hff_node(void)
{
    if(f != NULL)
        delete f;
}

HFactory::HFactory(QString rdata)
{
    errstr = "";
    rawdata = rdata;
    fieldlist = new QList<Hff_node *>();
    fields_readed = false;

    default_readonly=false;
    default_hide=false;
    default_logging=false;
    default_color=false;
    default_color_r=0;
    default_color_g=0;
    default_color_b=0;
}

HFactory::~HFactory(void)
{
    Hff_node *n;
    while(!fieldlist->isEmpty())
    {
        n = *fieldlist->begin();
        fieldlist->removeAll(n);
        delete n;
    }
    delete fieldlist;
    fieldlist = NULL;
    fields_readed = false;

    fpar_att = "";
    fpar_def = "";
    fpar_sql = "";
}

HTable* HFactory::genHTable(QString part,QString inRole)
{
    HTable *t;
    t = (HTable *)genTable(part,TABLE);
    if(!inRole.isEmpty())
        t->useRole(inRole);
    return t;
}

HList* HFactory::genHList (QString part,QString inRole)
{
    HList *l;
    l = (HList *)genTable(part,LIST);
    if(!inRole.isEmpty())
        l->useRole(inRole);
    return l;
}

QString HFactory::sqlCreateString(QString part)
{
    QString cs="";
    HTable *t=NULL;

    t = genHTable(part);
    if(t != NULL)
        cs = t->sqlCreateString();
    delete t;
    return cs;
}

int HFactory::readFromFile(QFile *f)
{
    sdebug("==Reading from file==");
     if (!f->open(QIODevice::ReadOnly | QIODevice::Text))
         return 1;
     QTextStream in(f);
     rawdata = in.readAll();
    sdebug("==Done==");
     return 0;
}

int HFactory::readFromFile(QString fn)
{
    QFile f(fn);
    return readFromFile(&f);
}

void HFactory::appendTemplateData(QString rdata)
{
    rawdata.append(rdata);

}

void HFactory::clearTemplateData(void)
{
    rawdata = "";
}

QString HFactory::errorString(void)
{
    return errstr;
}

bool HFactory::warning(const QXmlParseException& exception)
{
    QString perror = errstr;
    errstr = QString("XML parse error!\nLine:%1 Col:%2\n(pubID:%3 sysID:%4)\n\nHFactory message:%6\nSorry :-(")
        .arg(exception.lineNumber())
        .arg(exception.columnNumber())
        .arg(exception.publicId())
        .arg(exception.systemId())
        .arg(exception.message())
        .arg(perror);
    return false;
}

bool HFactory::error(const QXmlParseException& exception)
{
    QString perror = errstr;
    errstr = QString("XML parse error!\nLine:%1 Col:%2\n(pubID:%3 sysID:%4)\n\nHFactory message:%6\nSorry :-(")
        .arg(exception.lineNumber())
        .arg(exception.columnNumber())
        .arg(exception.publicId())
        .arg(exception.systemId())
        .arg(exception.message())
        .arg(perror);
    return false;
}

bool HFactory::fatalError(const QXmlParseException& exception)
{
    QString perror = errstr;
    errstr = QString("XML parse error!\nLine:%1 Col:%2\n(pubID:%3 sysID:%4)\nMessage:%5\n\nHFactory message:%6\nSorry :-(")
        .arg(exception.lineNumber())
        .arg(exception.columnNumber())
        .arg(exception.publicId())
        .arg(exception.systemId())
        .arg(exception.message())
        .arg(perror);
    return false;
}

HTableBase *HFactory::genTable(QString part,int mode)
{
    sdebug("*** HFactory::genTable ***");
    t = NULL;
    ttype = mode;
    level = 0;

    if(ttype != FIELDS)
    {
        sdebug("== Begin reading fields ==");
        if(!fields_readed)
        {
            sdebug("Parsing...");
            genTable("",FIELDS);
        }
        fields_readed = true;
        ttype = mode;
        sdebug("== End reading fields ==");
    }

    mainblock=false;
    read_on=false;
    fieldread_on=false;
    pname=part;

    emptyBuffer();

    QXmlInputSource *source =new QXmlInputSource();
    source->setData(rawdata);

    QXmlSimpleReader reader;
    reader.setContentHandler( this );
    reader.setErrorHandler( this );
    if(!reader.parse( *source ))
    {
        emit errorSignal("Error during the XML parsing! \n: " + errorString());
        sdebug("!!!!!!!!!!!!! THE XML PARSER FOUND ERROR:");
        sdebug("--begin error string--------------------");
        sdebug(errorString());
        sdebug("--end error string----------------------");
        delete source;
        sdebug("*** HFactory::genTable *** END");
        return NULL;
    }
    if(t==NULL && mode != FIELDS)
    {
        emit errorSignal(QString("XML Warinig: The requested element not found! (%1)").arg(part));
    }
    delete source;
    source = NULL;
    sdebug("*** HFactory::genTable *** END");
    return t;
}

bool HFactory::startDocument(void)
{
    sdebug("///HFactory///-> start-document");
    return true;
}

bool HFactory::endDocument(void)
{
    sdebug("///HFactory///-> end-document");
    return true;
}

bool HFactory::startElement( const QString& ls, const QString& ln, const QString& name,
                       const QXmlAttributes& atts)
{
    Q_UNUSED(ls);
    Q_UNUSED(ln);
#ifdef PARSER_DEBUG
    sdebug("XML-PARSER start-element: "+name);
#endif

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Main containers /////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////

    if(name == "templates")
    {
        mainblock = true;
        return true;
    }

    if(!mainblock) return true;

    if(ttype != FIELDS && name == "element" && atts.value("name") == pname)
    {
        read_on = true;
        return true;
    }

    if(ttype == FIELDS && !read_on && name == "field") //we are'nt in the required section or there is a field definition
    {
        fieldread_on = true;
        f = NULL;
        fname = atts.value("name");
        if(fname.isEmpty())
        {
            errstr="The name of the field node is cannot empty!";
            return false;
        }
        return true;
    }

    if(!read_on && !fieldread_on)
        return true;

    if(read_on && fieldread_on)
    {
        errstr = "Place the \"field\" nodes out of the element \"nodes\" !";
        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Main targets ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "table")
    {
        if(!read_on && fieldread_on)
        {
            errstr = "Systax error: Cannot place \"table\" node in \"field\" node!";
            return false;
        }

        if(atts.value("sqln").isEmpty())
        {
            errstr = "Missig or empty \"sqln\" parameter of node \"table\" !";
            return false;
        }
        if(ttype == TABLE)
            t = new HTable(atts.value("sqln"));
        else if(ttype == LIST)
            t = new HList (atts.value("sqln"));

        default_readonly=false;
        default_hide=false;
        default_logging=false;
        default_color=false;
        default_color_r=0;
        default_color_g=0;
        default_color_b=0;

        level = 1;

        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    if(read_on && t == NULL)
    {
        errstr = "Want to read table element to uninitialized Table !";
        return false;
    }

    if(read_on && name == "usefield")
    {
        if(!fields_readed)
        {
            errstr = "Internal error: Fields not readed yet!";
            return false;
        }
        QString match;
        HDataField *clone;
        match = atts.value("name");

        fpar_att = "";
        fpar_def = "";
        fpar_sql = "";
        fpar_att = atts.value("mode");
        fpar_def = atts.value("default");
        fpar_sql = atts.value("sqln");

        QList<Hff_node *>::iterator i = fieldlist->begin();
        while(i != fieldlist->end())
        {
            if((*i)->name == match)
            {
                sdebug("==== Begin - INSERT a CLONE ====");
                clone = (*i)->f->get_clone();

                if(!fpar_def.isEmpty())
                {
                    sdebug("Set a NEW default VALUE!");
                    clone->setDefval(fpar_def);
                }

                if(!fpar_sql.isEmpty())
                {
                    sdebug("Set a NEW default SQLN!");
                    clone->setDefval(fpar_sql);
                }

                if(fpar_att.contains("rw"))
                    clone->setEditable();
                if(fpar_att.contains("ro"))
                    clone->setReadonly();
                if(fpar_att.contains("hide"))
                    clone->setHide();
                if(fpar_att.contains("show"))
                    clone->setShow();

                insertDF(clone);

                sdebug("==== End   - INSERT a CLONE ====");
            }
            ++i;
        }
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Type modifier parts /////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "color")
    {
        if(level == 1 && !fieldread_on)
        {
            default_color=true;
            default_color_r=atts.value("r").toUInt();
            default_color_g=atts.value("g").toUInt();
            default_color_b=atts.value("b").toUInt();
        }
        else if(level == 2 || fieldread_on)
        {
            color = true;
            r = atts.value("r").toUInt();
            g = atts.value("g").toUInt();
            b = atts.value("b").toUInt();
        }
        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "readonly")
    {
        if(level == 1 && !fieldread_on)
            default_readonly=true;
        else if(level == 2 || fieldread_on)
            readonly=true;
        return true;
    }
    if(name == "editable")
    {
        if(level == 1 && !fieldread_on)
            ; //not necessary because the default is editable
        else if(level == 2 || fieldread_on)
            editable=true;
        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "logging")
    {
        if(level == 1 && !fieldread_on)
            default_logging=true;
        else if(level == 2 || fieldread_on)
            logging=true;
        return true;
    }
    if(name == "nologging")
    {
        if(level == 1 && !fieldread_on)
            ; //not necessary because the default is nologging
        else if(level == 2 || fieldread_on)
            nologging=true;
        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "show")
    {
        if(level == 2 || fieldread_on)
            show=true;
        return true;
    }
    if(name == "hide")
    {
        if(level == 2 || fieldread_on)
            hide=true;
        return true;
    }

    if(name == "role")
    {
        if(level == 1 && !fieldread_on)
        {
            errstr="Not allowed \"role\" tag here...";
            return false;
        }
        else if(level == 2 || fieldread_on)
        {
            if(atts.value("name").isEmpty())
            {
                errstr="You must specify valid (not empty) role name!";
                return false;
            }

            rolename.push_back( atts.value("name") );
            rolemod.push_back( atts.value("mod") );
            roleval.push_back( atts.value("val") );
        }
        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "meldwithnext")
    {
        if(level == 2 || fieldread_on)
            meldwithnext=true;
        return true;
    }

    if(name == "nomiddlestretch")
    {
        if(level == 2 || fieldread_on)
            nomiddlestretch=true;
        return true;
    }

    if(name == "displayformat")
    {
        bool ok;

        fmin = atts.value("minfrag").toInt(&ok);
        if(!ok)
        {
            errstr = "Cannot parse minfrag attribute!";
            return false;
        }

        fmax = atts.value("maxfrag").toInt(&ok);
        if(!ok)
        {
            errstr = "Cannot parse minfrag attribute!";
            return false;
        }
        fgroup = atts.value("group").toInt(&ok);
        if(!ok)
        {
            errstr = "Cannot parse minfrag attribute!";
            return false;
        }

        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    // in Filends //////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "hash")
    {

        buffer_k.push_back(atts.value("key"));
        buffer_v.push_back(atts.value("value"));

        return true;
    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    // Fields //////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////

    if(name == "key"       ||
       name == "smalltext" ||
       name == "largetext" ||
       name == "charhash"  ||
       name == "numhash"   ||
       name == "number"    ||
       name == "check"     ||
       name == "floating"  ||
       name == "static"    ||
       name == "date"      ||
       name == "dateu"     ||
       name == "timestamp" ||
       name == "connect"   ||
       name == "sqlchoose"    )
    {
        level++;
        emptyBuffer();
        buffer_sql = atts.value("sqln");
        return true;
    }

    if(name == "explain" || name == "ex" ||
       name == "title"   || name == "ti" ||
       name == "tail"    || name == "ta" ||
       name == "default" || name == "de" ||
       name == "true"    || name == "tr" ||
       name == "false"   || name == "fa" ||
       name == "controll"|| name == "co" ||
       name == "mark"    || name == "m"  ||
       name == "connected_table" ||
       name == "keyfield"        ||
       name == "showfield"       ||
       name == "showfieldeasy"   ||
       name == "showfieldhead"   ||
       name == "filter_part"     ||
       name == "tabletitle"      ||
       name == "hashfunc"        ||
       name == "extended"        ||
       name == "valid"           ||
       name == "notvalid"        ||
       name == "function"        ||
       name == "sortc"           ||
       name == "extrafeatures"   ||
       name == "showtitle"       ||
       name == "orderby_part"    ||
       name == "popuptitle"      ||
       name == "specialsort"     ||
       name == "progress"        ||
       name == "depend"          ||
       name == "minimum"         ||
       name == "maximum"         ||
       name == "meldwithnext"    ||
       name == "nomiddlestretch" ||
       name == "editmaxwidth"   )
    {
        //action in endElement
        return true;
    }

    errstr = "Unknown (start)element found: \""+name+"\" !";
    return false;
}

void HFactory::emptyBuffer(void)
{
    fpar_att = "";
    fpar_def = "";
    fpar_sql = "";

    buffer      = "";
    buffer_sql  = "";
    buffer_exp  = "";
    buffer_tit  = "";
    buffer_tail = "";
    buffer_def  = "";
    buffer_valid= "";
    buffer_func = "";
    buffer_hashf= "";
    buffer_specsort="";
    buffer_popuptitle="";
    buffer_minimum ="";
    buffer_maximum ="";
    buffer_1    = "";
    buffer_2    = "";
    buffer_3    = "";
    buffer_4    = "";
    buffer_5    = "";
    buffer_6    = "";
    buffer_7    = "";

    nomiddlestretch = false;
    guieditmaxwidth = 0;

    editable = false;
    readonly = false;

    logging  = false;
    nologging= false;

    show     = false;
    hide     = false;

    color    = false;

    easy     = true;
    meldwithnext = false;
    showprogress = false;
    sortc    = -1; //no sort
    r=0; g=0; b=0;
    buffer_k.clear();
    buffer_v.clear();
    buffer_notvalid.clear();
    fmin=0;
    fmax=4;
    fgroup=0;

    markers.clear();

    rolename.clear();
    rolemod.clear();
    roleval.clear();
}

bool HFactory::characters(const QString& ch)
{
    if(!mainblock) return true;
    if(!read_on && !fieldread_on) return true;

    buffer.append(QString(ch).trimmed());
    return true;
}

void HFactory::setHDataFieldAttributes(HDataField * d)
{
        if(default_color && !color)   d->setColor(default_color_r,default_color_g,default_color_b);
        if(color)           d->setColor(r,g,b);

        if(readonly || default_readonly)    d->setReadonly();
        if(editable)                        d->setEditable();

        if(logging || default_logging)      d->setLogging();
        if(nologging)                       d->resetLogging();

        if(show)     d->setShow();
        if(hide)     d->setHide();

        if(meldwithnext) d->meldWithNext();
        if(nomiddlestretch) d->noMiddleStretch();
        if(guieditmaxwidth != 0) d->setEditBoxMaxWidth(guieditmaxwidth);

        if(!buffer_specsort.isEmpty())
            d->setSpecDisplaySortMode(buffer_specsort);
        if(!buffer_func.isEmpty())
            d->setFunction(buffer_func);
        if(!markers.isEmpty())
            d->addMark(markers);

        if(rolename.count() > 0)
        {
            int i,c=rolename.count();
            for(i=0;i<c;++i)
                d->addRole( HDataFieldRole(rolename[i],rolemod[i],roleval[i]) );
        }
}


void HFactory::insertDF(HDataField *df)
{
    if(read_on)
        t->addField(df);
    if(fieldread_on)
    {
        fieldlist->push_back(new Hff_node(fname,df));
        fname = "";
    }
}

bool HFactory::endElement( const QString& ls, const QString&ln, const QString& name)
{
    Q_UNUSED(ls);
    Q_UNUSED(ln);

#ifdef PARSER_DEBUG
    sdebug("XML-PARSER end-element: "+name);
#endif

    if(name == "templates")
    {
        mainblock = false;
        return true;
    }

    if(!mainblock) return true;

    if(name == "element")
    {
        read_on = false;
        return true;
    }

    if(name == "field")
    {
        fieldread_on = false;
        return true;
    }

    if(!read_on && !fieldread_on) return true;

    /////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////
    if(read_on && t == NULL)
    {
        errstr = "Want to read table element to uninitialized Table (end)!";
        return false;
    }

    if(read_on && name == "tabletitle")
    {
        if(t != NULL)
        {
            t->setTableTitle(buffer);
        }
        buffer = "";
        return true;
    }
    /////////////////////////////////////
    if(name == "explain" || name == "ex")
    {
        buffer_exp = buffer;
        buffer = "";
        return true;
    }
    if(name == "title" || name == "ti")
    {
        buffer_tit = buffer;
        buffer = "";
        return true;
    }
    if(name == "tail" || name == "ta")
    {
        buffer_tail = buffer;
        buffer = "";
        return true;
    }
    if(name == "default" || name == "de")
    {
        buffer_def = buffer;
        buffer = "";
        return true;
    }
    if(name == "true" || name == "tr")
    {
        buffer_1 = buffer;
        buffer = "";
        return true;
    }
    if(name == "false" || name == "fa")
    {
        buffer_2 = buffer;
        buffer = "";
        return true;
    }

    if(name == "controll" || name == "co")
    {
        buffer_1 = "controll";
        return true;
    }

    if(name == "connected_table")
    {
        buffer_1 = buffer;
        buffer = "";
        return true;
    }
    if(name == "keyfield")
    {
        buffer_2 = buffer;
        buffer = "";
        return true;
    }
    if(name == "showfield")
    {
        buffer_3 = buffer;
        buffer = "";
        return true;
    }
    if(name == "showfieldeasy")
    {
        buffer_7 = buffer;
        buffer = "";
        return true;
    }
    if(name == "showfieldhead")
    {
        buffer_4 = buffer;
        buffer = "";
        return true;
    }
    if(name == "filter_part")
    {
        buffer_5 = buffer;
        buffer = "";
        return true;
    }
    if(name == "orderby_part")
    {
        buffer_6 = buffer;
        buffer = "";
        return true;
    }
    if(name == "popuptitle")
    {
        buffer_popuptitle = buffer;
        buffer = "";
        return true;
    }
    if(name == "sortc")
    {
        bool ok;
        sortc = buffer.toInt(&ok) - 1;
        if(!ok)
        {
            errstr = QString("Bad value in sortc!");
            return false;
        }
        buffer = "";
        return true;
    }

    if(name == "extended")
    {
        easy = false;
        return true;
    }

    if(name == "progress")
    {
        showprogress = true;
        return true;
    }

    if(name == "extrafeatures")
    {
        t->extrafeatures = true;
        return true;
    }

    if(name == "showtitle")
    {
        t->gui_showtabletitle = true;
        return true;
    }

    if(name == "valid")
    {
        buffer_valid = buffer;
        buffer = "";
        return true;
    }
    if(name == "notvalid")
    {
        buffer_notvalid.push_back(buffer);
        buffer = "";
        return true;
    }
    if(name == "function")
    {
        buffer_func = buffer;
        buffer = "";
        return true;
    }

    if(name == "mark" || name == "m")
    {
        markers.push_back(buffer);
        buffer = "";
        return true;
    }

    if(name == "hashfunc")
    {
        buffer_hashf = buffer;
        buffer = "";
        return true;
    }

    if(name == "specialsort")
    {
        buffer_specsort = buffer;
        buffer = "";
        return true;
    }

    if(name == "minimum")
    {
        buffer_minimum = buffer;
        buffer = "";
        return true;
    }
    if(name == "maximum")
    {
        buffer_maximum = buffer;
        buffer = "";
        return true;
    }
    if(name == "editmaxwidth")
    {
        int i=0;
        bool ok;
        i = buffer.toInt(&ok);
        if(ok)
            guieditmaxwidth = i;
        buffer = "";
        return true;
    }

    ///////////////////////////////////////////////////////////////////
    if(read_on && name == "depend")
    {
        if(!buffer.isEmpty())
            t->addDependTableName(buffer);
        buffer = "";
        return true;
    }


    ////////////////////////////////////////////////////////////////////////////////////////////
    // Fields //////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "key")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }

        HKey *d = new HKey(buffer_sql,buffer_exp,buffer_tit,buffer_def);
        setHDataFieldAttributes(d);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "connect")
    {
        level--;

        if(buffer_sql.isEmpty() || buffer_1.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }

        HConnect *d = new HConnect(buffer_sql,buffer_1);
        setHDataFieldAttributes(d);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "smalltext")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }

        HSmallText *d = new HSmallText(buffer_sql,buffer_exp,buffer_tit,buffer_tail,buffer_def);
        setHDataFieldAttributes(d);
        d->setValidator(buffer_valid);
        d->setNotValidValues(buffer_notvalid);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "largetext")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }

        HLargeText *d = new HLargeText(buffer_sql,buffer_exp,buffer_tit,buffer_def);
        setHDataFieldAttributes(d);
        d->setValidator(buffer_valid);
        d->setNotValidValues(buffer_notvalid);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "charhash")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }
        HCharHash *d = new HCharHash(buffer_sql,buffer_exp,buffer_tit,buffer_tail,
                                     buffer_def,buffer_k,buffer_v);
        setHDataFieldAttributes(d);
        d->setValidator(buffer_valid);
        d->setNotValidValues(buffer_notvalid);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "numhash")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }
        HNumHash *d = new HNumHash(buffer_sql,buffer_exp,buffer_tit,buffer_tail,
                                     buffer_def,buffer_k,buffer_v);
        setHDataFieldAttributes(d);
        d->setValidator(buffer_valid);
        d->setNotValidValues(buffer_notvalid);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "number")
    {
        bool ok=true;
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }
        HNumber *d = new HNumber(buffer_sql,buffer_exp,buffer_tit,buffer_tail,
                                     buffer_def);
        setHDataFieldAttributes(d);
        d->setValidator(buffer_valid);
        d->setNotValidValues(buffer_notvalid);

        if(!buffer_maximum.isEmpty())
        {
            d->setMaximum(buffer_maximum.toInt(&ok));
            if(!ok)
                sdebug(QString("Error conversion HNumber..maximum field data to int: \"%1\"").arg(buffer_maximum));
        }
        if(!buffer_minimum.isEmpty())
        {
            d->setMinimum(buffer_minimum.toInt(&ok));
            if(!ok)
                sdebug(QString("Error conversion HNumber..minimum field data to int: \"%1\"").arg(buffer_minimum));
        }

        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "static")
    {
        level--;

        HStatic *d = new HStatic(buffer_exp,buffer_tit,buffer_def);
        setHDataFieldAttributes(d);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "check")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }
        HCheck *d = new HCheck(buffer_sql,buffer_exp,buffer_tit,buffer_tail,
                                     buffer_def,buffer_1,buffer_2);
        setHDataFieldAttributes(d);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "floating")
    {
        bool ok=true;
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }
        HFloating *d = new HFloating(buffer_sql,buffer_exp,buffer_tit,buffer_tail,
                                     buffer_def);
        setHDataFieldAttributes(d);
        d->setValidator(buffer_valid);
        d->setNotValidValues(buffer_notvalid);
        d->setDisplayMode(fmin,fmax,fgroup);

        if(!buffer_maximum.isEmpty())
        {
            d->setMaximum(buffer_maximum.toDouble(&ok));
            if(!ok)
                sdebug(QString("Error conversion HFloating..maximum field data to double: \"%1\"").arg(buffer_maximum));
        }
        if(!buffer_minimum.isEmpty())
        {
            d->setMinimum(buffer_minimum.toDouble(&ok));
            if(!ok)
                sdebug(QString("Error conversion HFloating..minimum field data to double: \"%1\"").arg(buffer_minimum));
        }

        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "date")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }

        HDate *d = new HDate(buffer_sql,buffer_exp,buffer_tit,buffer_def,
                                        false,false);
        setHDataFieldAttributes(d);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "dateu")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }

        HDate *d = new HDate(buffer_sql,buffer_exp,buffer_tit,buffer_def,
                                        true);
        setHDataFieldAttributes(d);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "timestamp")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }
        HTimestamp *d = new HTimestamp(buffer_sql,buffer_exp,buffer_tit,buffer_def,
                                            buffer_1 == "controll" ? true : false);
        setHDataFieldAttributes(d);
        insertDF(d);
        emptyBuffer();
        return true;
    }

    if(name == "sqlchoose")
    {
        level--;

        if(buffer_sql.isEmpty())
        {
            errstr = QString("Missing parameter (%1) !").arg(name);
            return false;
        }

        if(buffer_3.isEmpty() && buffer_7.isEmpty())
        {
            errstr = QString("SqlChoose: Have to define showfield or showfieldeasy attribute!");
            return false;
        }
        if(!buffer_3.isEmpty() && !buffer_7.isEmpty())
        {
            errstr = QString("SqlChoose: You have to define showfield OR showfieldeasy attribute but only one! (not both)");
            return false;
        }
        if(buffer_3.isEmpty() && !buffer_7.isEmpty())
        {
            buffer_7.replace("|","||\'|\'||");
            buffer_3 = buffer_7;
        }

        HSqlChoose *d = new HSqlChoose(buffer_sql,buffer_exp,buffer_tit,buffer_def,
                                       buffer_1,buffer_2,buffer_3,buffer_4,buffer_5,buffer_6,buffer_hashf,easy);

        if(easy)
            d->progress_on_read = false;
        else
            d->progress_on_read = showprogress;


        if(!buffer_popuptitle.isEmpty())
            d->setPopupDialogTitle(buffer_popuptitle);

        d->setGuiSortIndicator(sortc);
        setHDataFieldAttributes(d);

        d->setValidator(buffer_valid);
        d->setNotValidValues(buffer_notvalid);

        insertDF(d);
        emptyBuffer();
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Not affected elements ///////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////
    if(name == "table"          ||
       name == "usefield"       ||
       name == "color"          ||
       name == "role"           ||
       name == "readonly"       ||
       name == "editable"       ||
       name == "show"           ||
       name == "hide"           ||
       name == "logging"        ||
       name == "nologging"      ||
       name == "meldwithnext"   ||
       name == "nomiddlestretch"||
       name == "displayformat"  ||
       name == "hash"       )
    {
        buffer = "";
        return true;
    }


    sdebug("end-element: "+name);

    errstr = "Unknown (end)element found: \""+name+"\" !";
    return false;
}

QString HFactory::yamlString(QString part)
{
    QString cs="";
    HTable *t=NULL;

    t = genHTable(part);
    if(t != NULL)
    {
        cs.append(QString("name: %1\n").arg(t->sqlTableName()));
        cs.append(QString("title: %1\n").arg(t->tableTitle()));
        cs.append(QString("fields:\n"));
        HDataField *f;
        t->firstField();
        while((f = t->nextField()) != NULL)
        {
            cs.append(QString("- sqlname: %1\n").arg(f->sqlSelectHead()));

            if(f->getWhoami() == "HKey"           ) cs.append("  type: SKey\n");
            //else if(f->getWhoami() == "SKey"    ) cs.append("  type: NKey\n");
            else if(f->getWhoami() == "HSmallText") cs.append("  type: SmallText\n");
            else if(f->getWhoami() == "HLargeText") cs.append("  type: LargeText\n");
            else if(f->getWhoami() == "HNumber"   ) cs.append("  type: Number\n");
            else if(f->getWhoami() == "HFloating" ) cs.append("  type: Floating\n");
            else if(f->getWhoami() == "HDate"     ) cs.append("  type: Date\n");
            else if(f->getWhoami() == "HTimestamp") cs.append("  type: Timestamp\n");
            else if(f->getWhoami() == "HCheck"    ) cs.append("  type: Check\n");
            else if(f->getWhoami() == "HNumHash"  ) cs.append("  type: NumSelect\n");
            else if(f->getWhoami() == "HCharHash" ) cs.append("  type: TxtSelect\n");
            //else if(f->getWhoami() == "SKey"    ) cs.append("  type: SqlNChoose\n");
            else if(f->getWhoami() == "HSqlChoose") cs.append("  type: SqlSChoose\n");
            else if(f->getWhoami() == "HStatic"   ) cs.append("  type: Static\n");
            else                                    cs.append("  type: WARNING_NOT_CONVERTED_TYPE\n");

            cs.append(QString("  description: \"%1\"\n").arg(f->getExplainText()));
            cs.append(QString("  title: \"%1\"\n").arg(f->getTitleText()));
            cs.append(QString("  default: \"%1\"\n").arg(f->getDefaultValue()));

            if(f->getWhoami() == "HNumHash")
            {
                HNumHash *sf = (HNumHash *)f;
                int i,c = sf->keys.count();
                if(c > 0)
                {
                    cs.append("  selectables:\n");
                    for(i = 0 ; i < c ; ++i)
                        cs.append(QString("  - %1: \"%2\"\n").arg(sf->keys[i]).arg(sf->values[i]));
                }
            }
            if(f->getWhoami() == "HCharHash" )
            {
                HCharHash *sf = (HCharHash *)f;
                int i,c = sf->keys.count();
                if(c > 0)
                {
                    cs.append("  selectables:\n");
                    for(i = 0 ; i < c ; ++i)
                        cs.append(QString("  - %1: \"%2\"\n").arg(sf->keys[i]).arg(sf->values[i]));
                }
            }


            bool hasAtt = false;
            QString extraAttributes = "";

            if(f->getRColor() != 180 || f->getGColor() != 180 || f->getBColor() != 180)
            {
                hasAtt = true;
                QString hexcolor;
                hexcolor = QString("%1%2%3")
                            .arg(QString::asprintf("%2x",f->getRColor()))
                            .arg(QString::asprintf("%2x",f->getGColor()))
                            .arg(QString::asprintf("%2x",f->getBColor()));
                extraAttributes.append(QString("  - color: %1\n").arg(hexcolor));
            }

            if(!f->getTailText().isEmpty())
            {
                hasAtt = true;
                extraAttributes.append(QString("  - txt_after: %1\n").arg(f->getTailText()));
            }

            QStringList displayflags;

            if(!f->isShow())
                displayflags.push_back("Invisible");
            if(!f->isEditable())
                displayflags.push_back("Readonly");

            if(displayflags.count() > 0)
                cs.append(QString("  displayflags: \"%1\"\n").arg(displayflags.join("|")));

            if(f->getWhoami() == "HDate")
            {
                HDate *sf = (HDate *)f;
                if(sf->is_unknown_alive())
                    cs.append("  unknownallowed: yes\n");
            }
            if(f->getWhoami() == "HFloating")
            {
                HFloating *sf = (HFloating *)f;
                if(sf->getDisplayModeMinDf() != 0 || sf->getDisplayModeMaxDf() != 4 || sf->getDisplayModeGroup() != 0)
                {
                    hasAtt = true;
                    extraAttributes.append(QString("  - display_min_decimals: %1\n").arg(sf->getDisplayModeMinDf()));
                    extraAttributes.append(QString("  - display_max_decimals: %1\n").arg(sf->getDisplayModeMaxDf()));
                    extraAttributes.append(QString("  - display_group_thousands: %1\n").arg(sf->getDisplayModeGroup()));
                }
            }

            if(hasAtt)
            {
                cs.append("  attributes:\n");
                cs.append(extraAttributes);
            }
        }
    }
    delete t;
    return cs;
}


//end code
