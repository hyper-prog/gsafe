/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    xmlolib.cpp
*/

#include <QtCore>
#include <QtGui>

#include "dconsole.h"
#include "xmlolib.h"


HExcelXmlGenerator::HExcelXmlGenerator(void)
{

}

HExcelXmlGenerator::~HExcelXmlGenerator(void)
{

}

void HExcelXmlGenerator::generate(HBase *d, QTextStream *s, QString params)
{
    int c;
    int rowCount=0;
    int columnCount=0;
    bool datareaded=false;
    HXmlWriter xml;
    QString tname = "Generated table";
    QString tnameshort;

    bool bordereddata = false;
    bool putcolumnwidthsett = false;
    bool putheader = true;
    bool puttitle = true;

    if(params.contains("bordered_data"))
        bordereddata = true;
    if(params.contains("set_columnwidth"))
        putcolumnwidthsett = true;
    if(params.contains("without_header"))
        putheader = false;
    if(params.contains("notitle"))
        puttitle = false;

    if(d->getWhoami() == "HTable")
    {
        if( !((HTableBase *)d)->tableTitle().isEmpty() )
            tname = ((HTableBase *)d)->tableTitle();
        rowCount = ((HTable *)d)->recordCount();
        columnCount = ((HTable *)d)->fieldCount();

    }
    if(d->getWhoami() == "HList")
    {
        if( !((HTableBase *)d)->tableTitle().isEmpty() )
            tname = ((HTableBase *)d)->tableTitle();
        rowCount = ((HList *)d)->recordCount();
        columnCount = ((HList *)d)->fieldCount();
    }
    if(d->getWhoami() == "HPlainDataMatrix")
    {
        if( !((HPlainDataMatrix *)d)->getTitle().isEmpty() )
            tname = ((HPlainDataMatrix *)d)->getTitle();
        rowCount = ((HPlainDataMatrix *)d)->rowCount();
        columnCount = ((HPlainDataMatrix *)d)->columnCount();
    }

    if(tname.length() > 18)
        tnameshort = tname.left(15).append("...");
    else
        tnameshort = tname;

    if(puttitle)
    {
        if(columnCount<3)
            columnCount = 3;
        rowCount += 3;
    }

    if(putheader)
    {
        rowCount += 1;
    }


    //Begin building table

    s->setCodec("UTF-8");
    xml.setStream(s);
    xml.putRawData("<?xml version=\"1.0\"?>\n<?mso-application progid=\"Excel.Sheet\"?>\n");
    xml.beginNode("Workbook","xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n"
                             " xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n"
                             " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\n"
                             " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\n"
                             " xmlns:html=\"http://www.w3.org/TR/REC-html40\"");
     xml.beginNode("DocumentProperties","xmlns=\"urn:schemas-microsoft-com:office:office\"");
      xml.putCData("Author","gSAFE");
      xml.putCData("LastAuthor","gSAFE");
      //xml.putCData("Created","2011-03-29T11:30:06Z");
      xml.putCData("Company","gSAFE");
      xml.putCData("Version","12.00");
     xml.endNode();

     xml.beginNode("ExcelWorkbook","xmlns=\"urn:schemas-microsoft-com:office:excel\"");
      xml.putCData("WindowHeight","8775");
      xml.putCData("WindowWidth","14160");
      xml.putCData("WindowTopX","360");
      xml.putCData("WindowTopY","75");
      xml.putCData("ProtectStructure","False");
      xml.putCData("ProtectWindows","False");
     xml.endNode();

     xml.beginNode("Styles");
      xml.beginNode("Style","ss:ID=\"Default\" ss:Name=\"Normal\"");
       xml.putEmptyNode("Alignment","ss:Vertical=\"Top\"");
       xml.putEmptyNode("Borders");
       xml.putEmptyNode("Font","x:CharSet=\"238\" ");
       xml.putEmptyNode("Interior");
       xml.putEmptyNode("NumberFormat");
       xml.putEmptyNode("Protection");
      xml.endNode();

      // --------- OPTIONAL STYLES -----------
      //s21 Bold font style
      xml.beginNode("Style","ss:ID=\"s21\"");
       xml.putEmptyNode("Font","x:CharSet=\"238\" x:Family=\"Swiss\" ss:Bold=\"1\"");
      xml.endNode();
      //Bigger sized bold font style. Not wrapped.
      xml.beginNode("Style","ss:ID=\"s22\"");
       xml.putEmptyNode("Font","x:CharSet=\"238\" x:Family=\"Swiss\" ss:Size=\"14\" ss:Bold=\"1\"");
       xml.putEmptyNode("Alignment","ss:Vertical=\"Top\" ss:WrapText=\"0\"");
      xml.endNode();
      // Bordered normal font style.
      xml.beginNode("Style","ss:ID=\"s23\"");
       xml.beginNode("Borders");
        xml.putEmptyNode("Border","ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
        xml.putEmptyNode("Border","ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
        xml.putEmptyNode("Border","ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
        xml.putEmptyNode("Border","ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
       xml.endNode();
      xml.endNode();
      // Bordered and Bold font style.
      xml.beginNode("Style","ss:ID=\"s24\"");
       xml.putEmptyNode("Font","x:CharSet=\"238\" x:Family=\"Swiss\" ss:Bold=\"1\"");
       xml.beginNode("Borders");
        xml.putEmptyNode("Border","ss:Position=\"Bottom\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
        xml.putEmptyNode("Border","ss:Position=\"Left\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
        xml.putEmptyNode("Border","ss:Position=\"Right\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
        xml.putEmptyNode("Border","ss:Position=\"Top\" ss:LineStyle=\"Continuous\" ss:Weight=\"1\"");
       xml.endNode();
      xml.endNode();
      // --------- OPTIONAL STYLES -----------

     xml.endNode();//Styles

     xml.beginNode("Worksheet",QString("ss:Name=\"%1\"").arg(tnameshort));
      xml.beginNode("Table",QString("ss:ExpandedColumnCount=\"%1\" ss:ExpandedRowCount=\"%2\""
                                    " x:FullColumns=\"1\" x:FullRows=\"1\" ss:DefaultRowHeight=\"15\"")
                                .arg(columnCount)
                                .arg(rowCount)
                                );

      //Set column width if necessary
      if(putcolumnwidthsett)
          for(c=0;c<columnCount;++c)
              xml.putEmptyNode("Column",QString("ss:Index=\"%1\" ss:AutoFitWidth=\"1\" ss:Width=\"%2\"")
                               .arg(c+1)
                               .arg(80));


      if(puttitle)
      {
          xml.beginNode("Row"); xml.putEmptyNode("Cell"); xml.endNode();

          xml.beginNode("Row","ss:StyleID=\"s22\"");
           xml.beginNode("Cell","ss:Index=\"1\"");
            xml.putCData("Data",tname,"ss:Type=\"String\"");
           xml.endNode();
          xml.endNode();

          xml.beginNode("Row"); xml.putEmptyNode("Cell"); xml.endNode();
      }

      //let's rock!
      if(d->getWhoami() == "HTable")
      {
          datareaded = true;
          HTable *data = (HTable *)d;

          HDataField *df=NULL;
          data->firstField();
          while((df=data->nextFieldAll()) != NULL)
          {
              if(!df->isShow())
                  continue;
              xml.beginNode("Row","ss:StyleID=\"s24\"");
               xml.beginNode("Cell");
                xml.putCData("Data",df->getExplainText(),"ss:Type=\"String\"");
               xml.endNode();
               xml.beginNode("Cell",(bordereddata ? "ss:StyleID=\"s23\"" : ""));
                xml.putCData("Data",df->dbValueToDispValue(df->getValue()).toString(),"ss:Type=\"String\"");
               xml.endNode();
              xml.endNode();
          }
      }

      if(d->getWhoami() == "HList")
      {
          datareaded = true;
          HList *data = (HList *)d;

          int i;
          QStringList sl;

          //Header!
          if(putheader)
          {
              sl = data->getShortTitleHeads();
              xml.beginNode("Row","ss:StyleID=\"s24\"");
              for(i =0;i < (int)sl.size();++i)
              {
                  xml.beginNode("Cell");
                   xml.putCData("Data",sl[i],"ss:Type=\"String\"");
                  xml.endNode();
              }
              xml.endNode();
          }

          QList<QStringList *>::Iterator iv = data->getValues()->begin();
          while(iv != data->getValues()->end())
          {
                  sl = **iv;
                  xml.beginNode("Row",(bordereddata ? "ss:StyleID=\"s23\"" : ""));

                  for(i =0;i < (int)sl.size();++i)
                  {
                      xml.beginNode("Cell");
                       xml.putCData("Data",sl[i],"ss:Type=\"String\"");
                      xml.endNode();
                  }
                  xml.endNode();
                  ++iv;
          }
      }

      if(d->getWhoami() == "HPlainDataMatrix")
      {
          datareaded = true;
          HPlainDataMatrix *data = (HPlainDataMatrix *)d;

          int i;
          QStringList sl;

          //Header!
          if(putheader)
          {
              sl = data->getHeader();
              xml.beginNode("Row","ss:StyleID=\"s24\"");
              for(i=0;i < (int)sl.size();++i)
              {
                  xml.beginNode("Cell");
                   xml.putCData("Data",sl[i],"ss:Type=\"String\"");
                  xml.endNode();
              }
              xml.endNode();
          }

          if(data->rowCount() > 0)
          {
              data->firstRow();
              do
              {
                  sl = data->currentRowStr();
                  xml.beginNode("Row",(bordereddata ? "ss:StyleID=\"s23\"" : ""));
                  for(i =0;i < (int)sl.size();++i)
                  {
                      xml.beginNode("Cell");
                       xml.putCData("Data",sl[i],"ss:Type=\"String\"");
                      xml.endNode();
                  }
                  xml.endNode();
              }
              while(data->nextRow());
          }
      }

      if(!datareaded)
          error(QString("HExcelXmlGenerator::generate: Cannot handle item: Not supported type (%1)!")
                    .arg(d->getWhoami()));

      xml.endNode();//Table

      xml.beginNode("WorksheetOptions","xmlns=\"urn:schemas-microsoft-com:office:excel\"");
       xml.beginNode("PageSetup");
        xml.putEmptyNode("Header","x:Margin=\"0.3\"");
        xml.putEmptyNode("Footer","x:Margin=\"0.3\"");
        xml.putEmptyNode("PageMargins","x:Bottom=\"0.75\" x:Left=\"0.7\" x:Right=\"0.7\" x:Top=\"0.75\"");
       xml.endNode();

       xml.beginNode("Panes");
        xml.beginNode("Pane");
         xml.putCData("Number","3");
         xml.putCData("ActiveRow",QString("%1").arg(rowCount));
         xml.putCData("ActiveCol",QString("%2").arg(columnCount));
        xml.endNode();
       xml.endNode();

       xml.putCData("ProtectObjects","False");
       xml.putCData("ProtectScenarios","False");
      xml.endNode();
     xml.endNode();
    xml.endNode();
}

QString HExcelXmlGenerator::generateToString(HBase *d, QString params)
{
    QString xml;
    QTextStream *s = new QTextStream(&xml);
    generate(d,s,params);
    delete s;
    return xml;
}

void HExcelXmlGenerator::generateToFile(HBase *d, QString filename, QString params)
{
    QFile xmlfile(filename);
    if (!xmlfile.open(QIODevice::WriteOnly))
    {
        error(QString(GSAFETEXT_CANNOT_CREATE_FILE).arg(filename));
        return;
    }

    QTextStream *s = new QTextStream(&xmlfile);
    generate(d,s,params);
    xmlfile.close();
    delete s;
}

// ////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* const image_browse_data[] = {
"20 20 13 1",
"c c #000000",
"e c #000080",
"k c #0000ff",
"j c #008080",
"i c #00ff00",
"h c #00ffff",
"g c #800000",
"a c #808080",
"d c #c0c0c0",
". c #dbd8d1",
"f c #ff0000",
"# c #ffff00",
"b c #ffffff",
"....................",
"..#..a#.............",
"..a#.ab..a..........",
"...a#a#.a#..........",
"....a#ba#...........",
"..aaab##aaaaaaaaaa..",
"....#abbbbbbbbbbbc..",
"..abda#bbbefbbghbc..",
"..#.aac#bbijbbfjbc..",
"....abddbbddbbddbc..",
"....abbbbbbbbbbbbc..",
"....abfjbbikbbfibc..",
"....abghbbafbbaebc..",
"....abddbbddbbddbc..",
"....abbbbbbbbbbbbc..",
"....accccccccccccc..",
"....................",
"....................",
"....................",
"...................."};

HXmloTable::HXmloTable(QWidget *parent,HBase *d)
:QDialog(parent)

{
    data = d;

    setWindowTitle(GSAFETEXT_XMLO_WINTITLE);

    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    QHBoxLayout *toplayout  = new QHBoxLayout(0);
    QHBoxLayout *midlayout  = new QHBoxLayout(0);
    QHBoxLayout *buttlayout = new QHBoxLayout(0);

    QLabel      *label1          = new QLabel(GSAFETEXT_XMLO_GETLABEL,this);
    QPushButton *savePushButton  = new QPushButton(this);
                 savePushButton->setText(GSAFETEXT_SAVE);
    QPushButton *closePushButton = new QPushButton(this);
                 closePushButton->setText(GSAFETEXT_CLOSE);
                 fnedit          = new QLineEdit(this);
                 fnedit->setText("output.xml");
    QToolButton *fcToolButton    = new QToolButton(this);
                 fcToolButton->setIcon(QIcon(  QPixmap( (const char **) image_browse_data )    ));

    toplayout->addStretch();
    toplayout->addWidget(label1);
    toplayout->addStretch();

    midlayout->addStretch();
    midlayout->addWidget(fnedit);
    midlayout->addSpacing(10);
    midlayout->addWidget(fcToolButton);
    midlayout->addStretch();

    buttlayout->addStretch();
    buttlayout->addWidget(savePushButton);
    buttlayout->addSpacing(10);
    buttlayout->addWidget(closePushButton);
    buttlayout->addStretch();

    mainlayout->addSpacing(15);
    mainlayout->addLayout(toplayout);
    mainlayout->addSpacing(10);
    mainlayout->addLayout(midlayout);
    mainlayout->addStretch();
    mainlayout->addLayout(buttlayout);
    mainlayout->addSpacing(20);

    setSizeGripEnabled(true);

    connect(fcToolButton,SIGNAL(clicked()),this,SLOT(cf()));
    connect(savePushButton,SIGNAL(clicked()),this,SLOT(saveButton()));
    connect(closePushButton,SIGNAL(clicked()),this,SLOT(closeButton()));
    resize(300,160);
}

HXmloTable::~HXmloTable(void)
{
}

int HXmloTable::cf(void)
{
    QString o;
    o = QFileDialog::getSaveFileName(this,GSAFETEXT_XMLO_OUTFILETITLE,"","*.xml");
    if(!o.isEmpty())
      fnedit->setText(o);
    return 0;
}

int HXmloTable::saveButton(void)
{
    HExcelXmlGenerator xmlgen;
    sdebug("saveB");

    if(data == NULL)
        return 0;
    if(fnedit->text().isEmpty())
        return 0;

    QFile f(fnedit->text());
    if (f.open(QIODevice::WriteOnly))
     {
      QTextStream *s = new QTextStream(&f);    // we will serialize the data into file f
      s->setCodec(QTextCodec::codecForName("UTF-8"));
      xmlgen.generate(data,s,"bordered_data set_columnwidth");
      f.close();
     }
    else
    {
        error(QString(GSAFETEXT_CANNOT_CREATE_FILE).arg(fnedit->text()));
        return 0;
    }
    close();
    return 0;
}

int HXmloTable::closeButton(void)
{
    close();
    return 0;
}



//end code
