/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    xmlolib.h
*/
#ifndef GSAFE__XML_OUTPUT_LIB_HEADER_FILE_X_
#define GSAFE__XML_OUTPUT_LIB_HEADER_FILE_X_

#include <QtCore>
#include <QtGui>

#ifndef COMPILED_WITH_QT4X
#include <QtWidgets>
#endif

/** \defgroup xmlolib xmlolib */
/*  @{  */

#include "datalib.h"

/** This class can generate an Excel 2003 XML table output from a HPlainDataMatrix/HList/HTable objects. */
class HExcelXmlGenerator
{
public:
    /** Creates a HExcelXmlGenerator object */
    HExcelXmlGenerator(void);
    /** Destructor */
    ~HExcelXmlGenerator(void);

    /** Generate the Excel 2003 Xml table output from the HPlainDataMatrix/HList/HTable and the output is
     *  written to the "s" stream.
     *  @param d the HTable/HList/HPlainDataMatrix class which holds the data to export
     *  @param s the Output strem to the exported xml data
     *  @param params you can set some parameter in this text. It can contains:
                    - bordered_data : Put border/frame around the data cells.
                    - set_columnwidth : Set auto resized wider columns.
                    - without_header : Don't export the column header cells.
                    - notitle : Don't export the title to the table.
     */
    void generate(HBase *d,QTextStream *s,QString params="");

    /** Generates the output to QString. (It calls the generate() )
     *  @see generate() */
    QString generateToString(HBase *d,QString params="");
    /** Generates the output to a file. (It calls the generate() )
     *  @see generate() */
    void generateToFile(HBase *d,QString filename,QString params="");
};

/** HXmloTable class can generate a Microsoft Excel 2003 XML table format output from
 *  HTable/HList/HPlainDataMatrix objects.
 *  If you create this class a dialog will appear which ask the file name to save.
 *  If the process is success the file can be opened with excel.\n
 *  This class will generate the document according the meta data class (HTable/HList/HPlainDataMatrix)
 *  and the data will came from these class too.
  \code
    HTable *mytable=...
    ...
    HXmloTable *d = new HXmloTable(this,mytable);
    d->exec();
    delete d;
    ...
 \endcode
 * or you can print a result of a query
 \code
    HSqlHandler h;
    HPlainDataMatrix *pdm = h.submitNResultQuery(3,"SELECT name,age,address FROM peoples;","Error occured");
    HXmloTable *d = new HXmloTable(this,pdm);
    d->exec();
    delete d;
    delete pdm;
 \endcode
 It uses the HExcelXmlGenerator class to generate the output
 *  @see HExcelXmlGenerator
 *  @see HTable @see HList @see HPlainDataMatrix    */
class HXmloTable : public QDialog
{
    Q_OBJECT

    private:
        QLineEdit  *fnedit;
        HBase *data;

        void genXml(QTextStream *out);

    public:
        /** Creates a HXmloTable object
         *  @param parent the QWidget descendant parent
         *  @param d the meta-data class    */
        HXmloTable(QWidget *parent,HBase *d);
        /** Destructor */
        ~HXmloTable(void);

    public slots:
        int cf(void);
        int saveButton(void);
        int closeButton(void);
};

/* @} */

#endif
