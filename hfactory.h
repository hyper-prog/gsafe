/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    hfactory.h
*/

#ifndef GSAFE__HFACTORY_LIB_HEADER_FILE_X_
#define GSAFE__HFACTORY_LIB_HEADER_FILE_X_

#include <QtCore>
#include <QtXml>

/** \defgroup hfactory hfactory */
/*  @{  */

#define TABLE  0
#define LIST   1
#define FIELDS 2

class HTable;
class HList;
class HTableBase;
class HDataField;

/* NOT DOCUMENTED: Helper class of HFactory. Do not use directly! */
class Hff_node
{
public:
    QString name;
    HDataField *f;

    Hff_node(QString namep,HDataField *fp);
    ~Hff_node(void);

};

/** HFactory is a meta-data/meta-object generator class.\n
 *  The gSAFE works with meta-data object which are in datalib. This base meta data objects are
 *  HTableBase and the two descendats HTable and HList.\n
 *  You can build these meta-data objects by api functions, or you can use the HFactory to build this object from an XML data.
 *  In the XML data yu can describe almost all information about the meta-data groups,
 *  so the HFactory can build the necessary data objects from it.\n\n
 *  The format of the XML data is described here: http://hyperprog.com/gsafe/doc/TEMPLATEDOC.txt\n
 *  You can find the XML Schema (XSD) file here: http://hyperprog.com/gsafe/doc/gsafemetadata.xsd\n
 *  \image  html pictures_doc/hfactory.png
 *  You can define this meta data objects, with api functions of HTableBase like this:
 \code
    ...
    HTable *table = new HTable("excerc");
    table->addField(new HKey("id","Identifier","Id",""));
    table->addField(new HLargeText("excer","Task to do","Task","")->asColored(200,100,100));
    table->addField(new HCheck("finished","Finished","Finished","",
                                "false","Yes, It is done!","No, it is unfinished."));
    table->addField(new HNumber("price","Cost","Cost","usd",""));
    table->addField(new HDate("deadline","Deadline","Deadline","NOW",false,false));
    ...
 \endcode
 *  You can use HFactory to generate this meta data object from XML file.
 *  The XML definition which describe the code above look like this:
 \code
 <templates>
  ...
   <element name="excercises">
     <table sqln="excerc">
       <extrafeatures/>
       <tabletitle>Tasks</tabletitle>
       <key sqln="id">
         <ex>Identifier</ex>
         <ti>Id</ti>
       </key>
       <largetext sqln="excer">
         <color r="200" g="100" b="100" />
         <ex>Task to do</ex>
         <ti>Task</ti>
       </largetext>
       <check sqln="finished">
         <ex>Finished</ex>
         <ti>Finished</ti>
         <true>Yes, It is done!</true>
         <false>No, it is unfinished.</false>
             <de>false</de>
       </check>
       <number sqln="price">
         <ex>Cost</ex>
         <ti>Cost</ti>
             <ta>usd</ta>
       </number>
       <date sqln="deadline">
         <ex>Deadline</ex>
         <ti>Deadline</ti>
             <de>NOW</de>
       </date>
     </table>
   </element>
   ...
 <templates>
 \endcode
 After you create a HFactory object you have to fill with the XML data. ( appendTemplateData() function)
 Every time when you need a meta data object you can generate it with genHTable() and genHList()    */
class HFactory : public  QObject , QXmlDefaultHandler //, QXmlErrorHandler
{
    Q_OBJECT

    public:
        /** Creates a HFactory object
         *  @param rdata you can set an initially XML data in this parameter. */
        HFactory(QString rdata="");
        /** Destructor */
        ~HFactory(void);

        /** Append XML meta data to the current. This XML will be concatenated to the end of the current data */
        void appendTemplateData(QString rdata);
        /** Clears the XML meta data */
        void clearTemplateData(void);

        /** Reads the XML meta data from a file.
         *  @param f the file to read
         *  @return 0 if success, 0!= if error occured. */
        int readFromFile(QFile *f);
        /** Reads the XML meta data from a file. Because the name is passed to QFile the name
         *  can be a valid file or a qt resource name too.
         *  @param f the file or resource to read
         *  @return 0 if success, 0!= if error occured. */
        int readFromFile(QString fn);

        /** Generate a HTableBase meta-data object according to the current XML meta data.
         *  @param part the name of the XML meta data element which will describe of the generated meta-data.
         *  @param mode Can be TABLE or LIST. The function generate HTable or HList */
        HTableBase *genTable (QString part,int mode=TABLE);

        /** Generate a HTable meta-data object according to the current XML meta data.
         *  If you specify the second "role" parameter the table returned in the specified role.
         *  @param part the name of the XML meta data element which will descibe of the generated meta-data.
         *  @param role the (optional) role which the table returned. */
        HTable     *genHTable(QString part,QString inRole="");

        /** Generate a HList meta-data object according to the current XML meta data.
         *  If you specify the second "role" parameter the list returned in the specified role.
         *  @param part the name of the XML meta data element which will descibe of the generated meta-data.
         *  @param role the (optional) role which the list returned. */
        HList      *genHList (QString part,QString inRole="");

        /** Returns the sql create string of the part.
         *  @see HTableBase::sqlCreateString() */
        QString sqlCreateString(QString part);

        /** Returns the gSafe 2 yaml string of the part. */
        QString yamlString(QString part);

    private:
        int ttype; //HTable || HList = TABLE || LIST
        int level; //deep in XML
        bool fields_readed;
        bool mainblock,read_on,fieldread_on; //Where I am, main block or in the requested part
        QString pname; //the name of the requested part
        QString fname; //the name of the currently read element
        QString fpar_att,fpar_def,fpar_sql;
        QString errstr;

        QString rawdata; //The raw XML data

        HTableBase *t; //I build this
        HDataField *f; //..with these.

        QList<Hff_node *> *fieldlist;

        //buffer:
        QString buffer,
                buffer_sql,
                buffer_exp,
                buffer_tit,
                buffer_tail,
                buffer_def,
                buffer_func,
                buffer_valid,
                buffer_hashf,
                buffer_specsort,
                buffer_popuptitle,
                buffer_minimum,
                buffer_maximum,
                buffer_1,
                buffer_2,
                buffer_3,
                buffer_4,
                buffer_5,
                buffer_6,
                buffer_7;

        QStringList buffer_k,
                    buffer_v,
                    buffer_notvalid;

        QStringList markers;

        QStringList rolename;
        QStringList rolemod;
        QStringList roleval;

        bool    default_readonly,
                default_hide,
                default_logging,
                default_color;
        int     default_color_r,default_color_g,default_color_b;


        bool    easy,
                editable,readonly,
                show,hide,
                logging,nologging,
                color,
                meldwithnext,
                nomiddlestretch,
                showprogress;
        int     guieditmaxwidth;
        int     r,g,b;
        int     sortc;
        int     fmin,fmax,fgroup;

        void insertDF(HDataField *df);

    public:
        /** XML parsing function. Do not use, it works automatically */
        bool startDocument(void);
        /** XML parsing function. Do not use, it works automatically */
        bool endDocument(void);
        /** XML parsing function. Do not use, it works automatically */
        bool startElement( const QString& ns, const QString& ln, const QString& name,
                       const QXmlAttributes& atts);
        /** XML parsing function. Do not use, it works automatically */
        bool endElement( const QString& ns, const QString& ln, const QString& name);
        /** XML parsing function. Do not use, it works automatically */
        bool characters(const QString& ch);
        /** XML parsing function. Do not use, it works automatically */
        QString errorString(void);

        /** XML parsing function. Do not use, it works automatically */
        bool warning(const QXmlParseException& exception);
        /** XML parsing function. Do not use, it works automatically */
        bool error(const QXmlParseException& exception);
        /** XML parsing function. Do not use, it works automatically */
        bool fatalError(const QXmlParseException& exception);

    private:
        void setHDataFieldAttributes(HDataField * d);
        void emptyBuffer(void);

    signals:
        void errorSignal(QString err);

};

/* @} */

#endif
