/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    dm.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore>

#include "xo.h"
#include "dmext.h"

HExcelXmlDocument::HExcelXmlDocument(QString docName)
{
    document_name = docName;
    style_counter = 10;

    colcount = 0;
    rowcount = 0;
    r_open = false;
    r_empty = true;
    in_header_row = false;
    c_index = 1;
    c_emptycellbefore = false;

    row_body = "";
    row_opts.clear();
    row_height = 0;

    orientation = "";
    doc_body = "";
    hc_cc = 0;
}

HExcelXmlDocument::~HExcelXmlDocument()
{

}

HExcelXmlDocument* HExcelXmlDocument::setDocumentName(QString docName)
{
    document_name = docName;
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::setTitle(QString title)
{
    document_name = title;
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::opts(QString options)
{
    table_opts = optionStringToMap(options);
    return this;
}

QMap<QString,QString> HExcelXmlDocument::optionStringToMap(QString optionString,QString mainSeparator)
{
    QMap<QString,QString> rmap;
    QList<QString> opts = optionString.split(mainSeparator,Qt::SkipEmptyParts);
    int i,c = opts.count();
    for(i = 0 ; i < c ; ++i)
    {
        QList<QString> opt = opts[i].split("=",Qt::SkipEmptyParts);
        if(opt.count() == 2 &&  !opt[0].trimmed().isEmpty() && !opt[1].trimmed().isEmpty())
            rmap[opt[0].trimmed()] = opt[1].trimmed();
    }
    return rmap;
}

void HExcelXmlDocument::mergeMapIntoMap(QMap<QString,QString>& mergeInto ,const QMap<QString,QString>& toMerge)
{
    QMap<QString,QString>::const_iterator i;
    for(i = toMerge.begin() ; i != toMerge.end() ; ++i)
        mergeInto[i.key()] = i.value();
}

HExcelXmlDocument* HExcelXmlDocument::nrow(QString options)
{
    if(r_open)
        endRow();
    beginRow(options);
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::nrows(int count,QString options)
{
    if(count <= 0)
        return this;
    int i;
    for(i = 0 ; i < count ; i++)
        nrow(options);
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::beginRow(QString options)
{
    if(r_open)
        return this;
    r_empty = true;
    r_open = true;
    rowcount++;
    row_opts = optionStringToMap(options);
    row_body = "";
    row_height = 0;
    c_index = 1;
    c_emptycellbefore = false;
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::endRow()
{
    if(!r_open)
        return this;
    if(r_empty)
    {
        doc_body.append("<Row><Cell/></Row>\n");
    }
    else
    {
        if(row_height != 0)
            doc_body.append(QString("<Row ss:AutoFitHeight=\"0\" ss:Height=\"%1\">\n").arg(row_height));
        else
            doc_body.append("<Row>\n");
        doc_body.append(row_body);
        doc_body.append("</Row>\n");
    }
    row_body = "";
    row_height = 0;
    r_open = false;
    row_opts.clear();
    in_header_row = false;
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::cell(QString c,QString options)
{
    if(!r_open)
        beginRow();

    QMap<QString,QString> opts = optionStringToMap(options);


    if(opts.contains("ashead") && opts["ashead"] == "true")
        in_header_row = true;

    if(hc_cc == 1 && (!opts.contains("ashead") || opts["ashead"] != "true"))
    {
        hc_cc = 2;
        if(in_header_row) //We only need to start a new row when already in header
            nrow();
    }

    mergeMapIntoMap(opts,table_opts);
    mergeMapIntoMap(opts,row_opts);

    if(opts.contains("xheight") && !opts["xheight"].isEmpty())
        opts["height"] = opts["xheight"];
    if(opts.contains("xwidth") && !opts["xwidth"].isEmpty())
        opts["width"] = opts["xwidth"];

    if(opts.contains("height") && !opts["height"].isEmpty())
        if(row_height < opts["height"].toInt())
            row_height = opts["height"].toInt();
    if(opts.contains("width") && !opts["width"].isEmpty())
        if(!column_width.contains(c_index) || column_width[c_index] < opts["width"].toInt())
            column_width[c_index] = opts["width"].toInt();

    QString sId = styleId(opts);
    if(!c.isEmpty() || !sId.isEmpty() || opts.contains("formula"))
    {
        QString ssMerge = "";
        QString ssStyle = "";
        QString ssIndex = "";
        QString ssFormula = "";
        if(c_emptycellbefore)
            ssIndex = QString(" ss:Index=\"%1\"").arg(c_index);
        if(!sId.isEmpty())
            ssStyle = QString(" ss:StyleID=\"%1\"").arg(sId);
        if(opts.contains("formula") && !opts["formula"].isEmpty())
            ssFormula = QString(" ss:Formula=\"=%1\"").arg(opts["formula"]);
        if(opts.contains("xcolspan") && !opts["xcolspan"].isEmpty() && opts["xcolspan"].toInt() > 1)
            opts["colspan"] = opts["xcolspan"];
        if(opts.contains("colspan") && !opts["colspan"].isEmpty() && opts["colspan"].toInt() > 1)
            ssMerge = QString(" ss:MergeAcross=\"%1\"").arg(opts["colspan"].toInt() - 1);

        if(!c.isEmpty())
        {
            QString type = "String";
            if(opts.contains("t"))
            {
                if(opts["t"] == "num")
                {
                    type = "Number";
                    if(c.indexOf(",") >= 0 && c.indexOf(",") == c.lastIndexOf(",")) //only one found
                        c = c.replace(",",".");
                    if(c.indexOf(",") >= 0 && c.indexOf(",") < c.lastIndexOf(",")) //more than one found
                        c = c.replace(",","");
                }
                if(opts["t"] == "dat")
                    type = "DateTime";
            }
            QString value = c;
            if(opts.contains("prefix") && !opts["prefix"].isEmpty())
                value.prepend(opts["prefix"]);
            if(opts.contains("suffix") && !opts["suffix"].isEmpty())
                value.append(opts["suffix"]);
            row_body.append(QString(" <Cell%1%2%3%4><Data ss:Type=\"%5\">%6</Data></Cell>\n")
                                .arg(ssIndex)
                                .arg(ssMerge)
                                .arg(ssStyle)
                                .arg(ssFormula)
                                .arg(type)
                                .arg(value));
        }
        else
        {
            row_body.append(QString(" <Cell%1%2%3%4/>\n")
                                .arg(ssIndex)
                                .arg(ssMerge)
                                .arg(ssStyle)
                                .arg(ssFormula));
        }
        c_emptycellbefore = false;
        r_empty = false;
    }
    else
    {
        c_emptycellbefore = true;
    }

    c_index++;

    if(opts.contains("colspan") && !opts["colspan"].isEmpty() && opts["colspan"].toInt() > 1)
        c_index += (opts["colspan"].toInt() - 1);

    if(colcount < c_index)
        colcount = c_index;

    return this;
}

HExcelXmlDocument* HExcelXmlDocument::cells(QList<QString> cs,QString options)
{
    int i,c = cs.count();
    for(i = 0 ; i < c ; ++i)
        cell(cs[i],options);
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::head(QString h,QString options)
{
    hc_cc = 1;
    if(!options.isEmpty())
        options.append(";");
    options.append("ashead=true");

    cell(h,options);
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::heads(QList<QString> hs,QString options)
{
    hc_cc = 1;

    if(!options.isEmpty())
        options.append(";");
    options.append("ashead=true");

    int i,c = hs.count();
    for(i = 0 ; i < c ; ++i)
        head(hs[i],options);
    return this;
}

QString HExcelXmlDocument::styleId(QString options)
{
    QMap<QString,QString> opts = optionStringToMap(options);
    return styleId(opts);
}

QString HExcelXmlDocument::styleId(const QMap<QString,QString>& opts)
{
    QString sStr = "";

    //wrap & vertical & horizontal
    if(opts.contains("wrap") || opts.contains("vertical") || opts.contains("horizontal"))
    {
        QString wrap = "1";
        if(opts.contains("wrap") && opts["wrap"] == "off")
            wrap = "0";
        QString vertical = "Top";
        if(opts.contains("vertical"))
        {
            if(opts["vertical"] == "top"   ) vertical = "Top";
            if(opts["vertical"] == "center") vertical = "Center";
            if(opts["vertical"] == "bottom") vertical = "Bottom";
        }
        QString horizontal = "Left";
        if(opts.contains("horizontal"))
        {
            if(opts["horizontal"] == "left"  ) horizontal = "Left";
            if(opts["horizontal"] == "center") horizontal = "Center";
            if(opts["horizontal"] == "right" ) horizontal = "Right";
        }
        sStr.append(QString("    <Alignment ss:Horizontal=\"%1\" ss:Vertical=\"%2\" ss:WrapText=\"%3\"/>\n")
                                .arg(horizontal)
                                .arg(vertical)
                                .arg(wrap));
    }

    //border & borderweight
    if(opts.contains("border"))
    {
        QStringList borders = {"Bottom","Left","Right","Top"};
        QStringList bordersLower = {"bottom","left","right","top"};
        int bw = 1;
        if(opts.contains("borderweight"))
            bw = opts["borderweight"].toInt();
        if(bw < 0 || bw > 3)
            bw = 1;
        sStr.append("    <Borders>\n");
        QList<QString> bs = opts["border"].toLower().split(",",Qt::SkipEmptyParts);

        if(bs.count() == 1)
        {
            for(int b = 0 ; b < 4 ; ++b)
            {
                if(opts["border"].toLower() == bordersLower[b] || opts["border"] == "all")
                    sStr.append(QString("      <Border ss:Position=\"%1\" ss:LineStyle=\"Continuous\" ss:Weight=\"%2\"/>\n")
                                            .arg(borders[b])
                                            .arg(bw));
            }
        }
        else
        {
            for(int b = 0 ; b < 4 ; ++b)
            {
                if(bs.contains(bordersLower[b]) || bs.contains("all"))
                    sStr.append(QString("      <Border ss:Position=\"%1\" ss:LineStyle=\"Continuous\" ss:Weight=\"%2\"/>\n")
                                            .arg(borders[b])
                                            .arg(bw));
            }
        }
        sStr.append("    </Borders>\n");
    }

    //background-color
    if(opts.contains("background-color"))
    {
        QString bgcolor = "";
        if(!opts["background-color"].isEmpty())
            bgcolor = opts["background-color"];
        sStr.append(QString("    <Interior ss:Color=\"%1\" ss:Pattern=\"Solid\"/>\n").arg(bgcolor));
    }

    //strong & italic & size & color
    if(opts.contains("strong") ||
       opts.contains("italic") ||
       opts.contains("underline") ||
       opts.contains("size") ||
       opts.contains("xsize") ||
       opts.contains("color") )
    {
        QString strong = "";
        if(opts.contains("strong") && opts["strong"] == "yes")
            strong = " ss:Bold=\"1\"";

        QString italic = "";
        if(opts.contains("italic") && opts["italic"] == "yes")
            italic = " ss:Italic=\"1\"";

        QString underline = "";
        if(opts.contains("underline") && opts["underline"] == "yes")
            underline = " ss:Underline=\"Single\"";

        QString size = "";
        if(opts.contains("size") && !opts["size"].isEmpty())
            size = QString(" ss:Size=\"%1\"").arg(opts["size"]);
        if(opts.contains("xsize") && !opts["xsize"].isEmpty())
            size = QString(" ss:Size=\"%1\"").arg(opts["xsize"]);

        QString color = "";
        if(opts.contains("color") && !opts["color"].isEmpty())
            color = QString(" ss:Color=\"%1\"").arg(opts["color"]);

        sStr.append(QString("    <Font ss:FontName=\"Arial\" x:CharSet=\"238\" x:Family=\"Swiss\"%1%2%3%4%5/>\n")
                            .arg(size).arg(strong).arg(italic).arg(underline).arg(color));
    }

    //numberformat
    if(opts.contains("numberformat") && !opts["numberformat"].isEmpty())
    {
        sStr.append(QString("    <NumberFormat ss:Format=\"%1\"/>\n")
                            .arg(opts["numberformat"]));
    }

    // - End of building style -
    if(sStr.isEmpty())
        return "";

    QMap<QString,QString>::Iterator i = processedStyles.begin();
    while(i != processedStyles.end())
    {
        if(i.value() == sStr)
            return i.key();
        ++i;
    }

    QString newStyleId = QString("s%1").arg(style_counter);
    ++style_counter;

    processedStyles[newStyleId] = sStr;
    return newStyleId;
}

QString HExcelXmlDocument::get()
{
    if(r_open)
        endRow();

    QString fdoc = "";

    fdoc.append(
        "<?xml version=\"1.0\"?>\n"
        "<?mso-application progid=\"Excel.Sheet\"?>\n"
        "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\n"
        " xmlns:o=\"urn:schemas-microsoft-com:office:office\"\n"
        " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\n"
        " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\n"
        " xmlns:html=\"http://www.w3.org/TR/REC-html40\">\n\n");

    QString un = "gSAFE generated";

    fdoc.append(QString("<DocumentProperties xmlns=\"urn:schemas-microsoft-com:office:office\">\n"
                        "  <Author>%1</Author>\n"
                        "  <LastAuthor>%2</LastAuthor>\n"
                        "  <Created>%3</Created>\n"
                        "</DocumentProperties>\n")
                            .arg(un)
                            .arg(un)
                            .arg(QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ssZ")));

    fdoc.append("<Styles>\n");
    fdoc.append("  <Style ss:ID=\"Default\" ss:Name=\"Normal\">\n"
                "    <Alignment ss:Vertical=\"Top\" ss:WrapText=\"1\"/>\n"
                "    <Borders/>\n"
                "    <Font ss:FontName=\"Arial\" x:CharSet=\"238\"/>\n"
                "    <Interior/>\n"
                "    <NumberFormat/>\n"
                "    <Protection/>\n"
                "  </Style>\n");

    QMap<QString,QString>::iterator si;
    for(si = processedStyles.begin() ; si != processedStyles.end() ; ++si)
    {
        fdoc.append(QString("  <Style ss:ID=\"%1\">\n").arg(si.key()));
        fdoc.append(si.value());
        fdoc.append("  </Style>\n");
    }
    fdoc.append("</Styles>\n\n");

    if(document_name.isEmpty())
        document_name = "Generated";
    fdoc.append(QString("<Worksheet ss:Name=\"%1\">\n").arg(document_name));
    fdoc.append(QString("<Table ss:ExpandedColumnCount=\"%1\" "
                               "ss:ExpandedRowCount=\"%2\" x:FullColumns=\"1\" x:FullRows=\"1\">\n")
                                    .arg(colcount)
                                    .arg(rowcount));
    int ci;
    for(ci = 1 ; ci <= colcount ; ++ci)
        if(column_width.contains(ci))
            fdoc.append(QString("<Column ss:Index=\"%1\" ss:AutoFitWidth=\"0\" ss:Width=\"%2\"/>\n")
                                    .arg(ci)
                                    .arg(column_width[ci]));

    fdoc.append(doc_body);
    fdoc.append("</Table>\n");

    fdoc.append("<WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">\n"
                "  <PageSetup>\n");

    if(!orientation.isEmpty())
        fdoc.append(QString("    <Layout x:Orientation=\"%1\"/>\n").arg(orientation));

    fdoc.append(
          "    <Header x:Margin=\"0.3\"/>\n"
          "    <Footer x:Margin=\"0.3\"/>\n"
          "    <PageMargins x:Bottom=\"0.75\" x:Left=\"0.25\" x:Right=\"0.25\" x:Top=\"0.75\"/>\n"
          "  </PageSetup>\n"
          "  <ProtectObjects>False</ProtectObjects>\n"
          "  <ProtectScenarios>False</ProtectScenarios>\n"
          "</WorksheetOptions>\n");

    fdoc.append("</Worksheet>\n");
    fdoc.append("</Workbook>\n"); //Root element
    return fdoc;
}

HExcelXmlDocument* HExcelXmlDocument::setOrientationLandscape()
{
    orientation = "Landscape";
    return this;
}

HExcelXmlDocument* HExcelXmlDocument::setOrientationPortrait()
{
    orientation = "";
    return this;
}

bool HExcelXmlDocument::writeFile(QString filename)
{
    QFile f(filename);
    if(f.open(QIODeviceBase::WriteOnly))
    {
        f.write(get().toUtf8());
        f.close();
        return true;
    }
    return false;
}

void HExcelXmlDocument::addDataMatrix(HDataMatrix *dm,bool controlRowAsOptions,QString options)
{
    int ri,rc = dm->rowCount();
    for(ri = 0 ; ri < rc ; ++ri)
    {
        QString o = options;
        if(controlRowAsOptions)
        {
            if(!o.isEmpty())
                o.append(";");
            o.append(dm->getRowControl(ri));
        }
        cells(dm->getRowStr(ri),o);
        nrow();
    }
}

//End of gSAFE xo.cpp
