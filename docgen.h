/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    docgen.h
*/

#ifndef GSAFE__DOCGEN_HEADER_FILE_X_
#define GSAFE__DOCGEN_HEADER_FILE_X_

#include <QtCore>
#include <QtXml>

/** \defgroup docgen docgen */
/*  @{  */

#include "res_p.h"

class HTable;
class HFloatTables;

/** HyperDoc is a template document class.\n
 *  This class is store a html based document template, which references some (datalib) data.
 *  The template document can contains XML control elements which replaced to data fields when the processed document is generated.
 *  Just like in circular letters. The referenced data can came from HTable, HList or Resource provider classes (HResourceProvider).
 *  You can use [ ] | signs in the document which are converted to < > " signs in the final document.
 *  (We use this sings above that's why the template codes should be irrelevant for the xml parser)
 *  Useable control elements:
 *  - [DATA BEGIN=|| END=|| NOTFOUND=||]htable_name.connectedname:datafield_name[/DATA]
 *  - [RES BEGIN=|| END=|| NOTFOUND=||]resource_name[/RES] 
 *  - [COND DATA=|htable_name:datafield_name| IS=|eq/ne| TO=|constant|]conditional text[/COND]
 *  - [COND RES=||]conditional text[/COND] where the res=|true| then show otherwise not show
 *  - [ITER DATA=|hlist_name| NOTFOUND=||] Here you can use elements but the table names have to signed with * char [/ITER]
 *
 *  If the requested data not found nothing appear, unless you fill the notfound attribute 
 *  @see HTable @see HList @see HResourceProvider   */
class HyperDoc : public  QObject , QXmlDefaultHandler
{
    Q_OBJECT 

public:
    /** Creates an empty HyperDoc template document object 
     *  @param n the name of the template. */
    HyperDoc(QString n);
    /** Destructor */
    ~HyperDoc();

    /** Clears the current template document */
    void clear(); 
    /** Sets the text of the template document */
    void set(QString docstr);
    /** Append a text to the current template document (concatenated to the end) */
    void add(QString docstr);

    /** Clears all hyperdoc data source and resource providers (Including ITER,DATA,RES providers)     
     *  @see addDataSource() @see addResourceProvider() @see addIterDataTables() */
    void clearResources();

    /** Add a new data source object (HTable) This is the data provider of DATA tag.
     *  @param ht the data source pointer
     *  @param redefinedName the redefined data source name. 
     *      If this parameter missing the default name will be the name of the HTable. 
     *  @see clearResources()   */
    void addDataSource(HTable *ht,QString redefinedName = "");
    /** Add a new resource provider object (HResourceProvider) This is the data provider of RES tag.
     *  @param rp the resource provider object    
     *  @see clearResources()   */
    void addResourceProvider(HResourceProvider *rp);
    /** Add a new list item provider object (HFloatTables) This is the data provider of ITER tag.
     *  @param ft the list item provider object     
     *  @see clearResources()   */
    void addIterDataTables(HFloatTables *ft);


    /** Returns the raw untouched template document */
    QString getRawDoc();
    /** Returns the processed document. */ 
    QString generateDoc();

    /** Returns the template doc name */
    QString getName(void) { return name; }
    /** Sets the template doc name */
    void setName(QString n) { name=n; }

private:
    bool startDocument(void);
    bool endDocument(void);
    bool startElement( const QString& ns, const QString& ln, const QString& name, 
                       const QXmlAttributes& atts);
    bool endElement( const QString& ns, const QString& ln, const QString& name);
    bool processingInstruction(const QString &target,const QString &data);
    bool skippedEntity(const QString &name);
    bool characters(const QString& ch);
    bool warning(const QXmlParseException& exception);
    bool error(const QXmlParseException& exception);
    bool fatalError(const QXmlParseException& exception);
    QString errorString(void) { return errstr; }

    HTable* getDataTable(QString data);
    HFloatTables* getFloatDataTable(QString data) { return getHFT(data); }

protected:
    void clearBuffer();

    QString getData(QString data);

    QString getRes(QString res);
    HFloatTables* getHFT(QString res);

    void out(QString o);

protected:
    QString name;
    QString body; //Base text body
    QString preprocessed;
    QString errstr;

    bool ignore;
    bool itermode;
    QString buffer,iterbuffer,iter_hft_name,iter_notfound;

    QString att_begin,att_end,att_notfound;
    HTable                     *insideIter;
    int skip_cond_deep;

public:
    QList<HResourceProvider *> *resourceProviders;
    QList<HTable *>           *dtsp;
    QList<QString>            *dtsn;
    QList<HFloatTables *>     *fts;

signals:
    /** This signal is emitted if error occured. */
    void errorSignal(QString err);

};

/* @} */

#endif
