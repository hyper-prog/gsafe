/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   docas.cpp
*/

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "dm.h"
#include "docas.h"
#include "dconsole.h"
#include "dialog.h"

DocAssembler::DocAssembler(QString documentSource)
{
    rawDocumentSource = documentSource;
    textProcessor = new HTextProcessor();
    workingDirectory = QDir::currentPath() + QDir::separator() + "work";
    sourceDocDirectory = QDir::currentPath() + QDir::separator() + "documents";
    enable_render_warnings = false;
    monthNames = QStringList() << tr("error")
                    << tr("january") << tr("february") << tr("march")
                    << tr("april") << tr("may") << tr("june")
                    << tr("july") << tr("august") << tr("september")
                    << tr("october") << tr("november") << tr("december");
}

DocAssembler::~DocAssembler()
{
    delete textProcessor;
}

void DocAssembler::setMonthNames(QStringList customMonthNames)
{
    monthNames = customMonthNames;
}

void DocAssembler::setWorkingDirectory(QString dir)
{
    workingDirectory = dir;
}

void DocAssembler::setSourceDocDirectory(QString dir)
{
    sourceDocDirectory = dir;
}

void DocAssembler::generatePdfDocument(QString outputFile)
{
    minumimGenPageCount = 0;
    if(QFile(outputFile).exists())
        if(!QFile::remove(outputFile))
        {
            throw GSafeException(tr("Error, Cannot remove existing output file: %1").arg(outputFile));
        }

    preprocessDocument();
    generateFilenames();
    generateBasePdf();
    finishingPdf();
    
    deleteWorkfileIfExists(outputFile);
    if(!QFile::copy(filenames["result"],outputFile))
    {
        throw GSafeException(tr("Error, Cannot copy result file to output file: %1").arg(outputFile));
    } 
    
    deleteWorkfileIfExists(filenames["base"]);
    deleteWorkfileIfExists(filenames["underlay"]);
    deleteWorkfileIfExists(filenames["result"]);
}

int DocAssembler::getPageCountOfPdf(QString filename)
{
    filenames["qpdf"] = QDir::currentPath() + QDir::separator() + "qpdf" + QDir::separator() + "qpdf.exe";
    if(!QFile::exists(filenames["qpdf"]))
    {
        sdebug(QString("Error, qpdf.exe not found..."));
        throw GSafeException(tr("Error, the qpdf.exe not found in qpdf directory.\nPlease download the qpdf binary from https://github.com/qpdf/qpdf"));
        return -1;
    }

    QProcess qpdf;
    QStringList arguments;
    arguments << "--show-npages" << filename;
    qpdf.setReadChannel(QProcess::StandardOutput);
    qpdf.start(filenames["qpdf"],arguments);
    if(!qpdf.waitForFinished(-1))
    {
        throw GSafeException(tr("Error, Cannot run qpdf --show-npages command to the underlay pdf!"));
        return -1;
    }
    
    QString output = qpdf.readAllStandardOutput().trimmed();
    bool ok;
    int pageCount = output.toInt(&ok);
    if (!ok) {
        throw GSafeException(tr("Error, Cannot parse output of qpdf --show-npages command on the underlay pdf!"));
        return -1;
    }
    return pageCount;
    if(qpdf.exitStatus() != QProcess::NormalExit || qpdf.exitCode() != 0)
    {
        throw GSafeException(tr("Error, The qpdf --show-npages command to failed to run on the underlay pdf!"));
        return -1;
    }
    return pageCount;
}

int DocAssembler::deleteWorkfileIfExists(QString filename)
{
    if(QFile::exists(filename))
    {
        if(!QFile::remove(filename))
        {
            sdebug(QString("Error deleting file: %1").arg(filename));
            return 1;
        }
    }
    return 0;
}

int DocAssembler::preprocessDocument()
{
    preprocessedDoc = "";
    read_annotations.clear();

    preprocessedDoc = textProcessor->processDoc(rawDocumentSource);
    read_annotations = textProcessor->annotations();
    return 0;
}

int DocAssembler::generateFilenames()
{
    QDir currdir = QDir::current();
    if(!currdir.mkpath(workingDirectory))
    {
        throw GSafeException(tr("Error, Cannot create working directory: %1").arg(workingDirectory));
    }

    if(!QDir(workingDirectory).exists())
    {
        throw GSafeException(tr("Error, Cannot create working directory: %1").arg(workingDirectory));
    }

    filenames["base"] = workingDirectory + QDir::separator() + "gen.pdf";
    filenames["result"] = filenames["base"];
    if(read_annotations.contains("UnderlayPdf"))
    {
        filenames["original_underlay"] = sourceDocDirectory + QDir::separator() + read_annotations["UnderlayPdf"].last();
        if(QFile(filenames["original_underlay"]).exists())
        {
            filenames["underlay"] = workingDirectory + QDir::separator() + "underlay.pdf";
            filenames["result"] = workingDirectory + QDir::separator() + "result.pdf";
            
            if(QFile(filenames["underlay"]).exists())
                if(!QFile::remove(filenames["underlay"]))
                    throw GSafeException(tr("Error, Cannot remove previous work file: %1").arg(filenames["underlay"]));

            if(!QFile::copy(filenames["original_underlay"],filenames["underlay"]))
                throw GSafeException(tr("Error, Cannot copy original underlay file to working directory: %1").arg(filenames["underlay"]));
            minumimGenPageCount = getPageCountOfPdf(filenames["underlay"]);
            return 0;
        }
        else
        {
            throw GSafeException(tr("Error, The underlay pdf file specified in the document annotation not found: %1").arg(filenames["original_underlay"]));
        }
    }
    return 0;
}

int DocAssembler::generateBasePdf()
{
    int dpi = 200;

    if(QFile(filenames["base"]).exists())
        if(!QFile::remove(filenames["base"]))
            throw GSafeException(tr("Error, Cannot remove previous work file: %1").arg(filenames["base"]));

    QPdfWriter pw(filenames["base"]);

    pw.setResolution(dpi);
    pw.setPageSize(QPageSize(QPageSize::A4));  /*8.26 x 11.69  -> *200 -> 1652 x 2338*/
    pw.setPageOrientation(QPageLayout::Portrait);

    QPainter pp(&pw);
    pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi (Set elsewhere...)
    HPageTileRenderer renderer(&pp);
    renderer.resolutionDpi = dpi;
    if(enable_render_warnings)
        renderer.setUnknownCommandWarning(true);

    connect(&renderer, &HPageTileRenderer::startNewPage,
            this,[&pw]() { pw.newPage(); });

    if(minumimGenPageCount > 0)
        preprocessedDoc += QString("\nnpuc#%1").arg(minumimGenPageCount);
    renderer.renderFromInstructions(preprocessedDoc);

    lastRenderStoredPositions = renderer.storedPositions();
    pp.end();
    return 0;
}

int DocAssembler::finishingPdf()
{
    if(!filenames.contains("underlay") || filenames["underlay"].isEmpty())
        return 0;

    //It should be checked earlier
    if(!filenames.contains("qpdf") || filenames["qpdf"].isEmpty())
        return 0;
    
    if(QFile::exists(filenames["result"]))
        if(!QFile::remove(filenames["result"]))
            throw GSafeException(tr("Error, Cannot remove previous work file: %1").arg(filenames["result"]));

    QProcess qpdf;
    QStringList arguments;
    arguments << "--overlay" << filenames["base"] << "--" << filenames["underlay"] << filenames["result"];
    qpdf.start(filenames["qpdf"],arguments);
    if(!qpdf.waitForFinished(-1))
    {
        throw GSafeException(tr("Error, Cannot run qpdf to merge the underlay and base pdf files!"));
        return 1;
    }

    if(qpdf.exitStatus() != QProcess::NormalExit || qpdf.exitCode() != 0)
    {
        throw GSafeException(tr("Error, Qpdf returned error: %1 %2").arg(qpdf.exitCode()).arg(qpdf.readAllStandardError()));
        return 1;
    }
    return 0;
}

void DocAssembler::addValueMap(QString name,const QMap<QString,QString>& m)
{
    textProcessor->addValueMap(name,m);
}

void DocAssembler::addValueList(QString name,const QList<QString>& l)
{
    textProcessor->addValueList(name,l);
}

void DocAssembler::addValueMapPtr(QString name,QMap<QString,QString>* m)
{
    textProcessor->addValueMapPtr(name,m);
}

void DocAssembler::clearValueMaps()
{
    textProcessor->clearValueMaps();
}

void DocAssembler::setTitleValues(QString selectedTitle)
{
    QMap<QString,QStringList> temp_annots = getAnnotationValuesFromText(rawDocumentSource);
    QStringList titles = temp_annots["Title"];
    for(const QString& title : std::as_const(titles))
    {
        QStringList tpt = title.split("|");
        if(tpt.count() == 1 && title == selectedTitle)
        {
            setValueOfMapKey("document.title",title);
            setValueOfMapKey("document.titlecode","");
        }
        if(tpt.count() > 1 && tpt.first() == selectedTitle)
        {
            setValueOfMapKey("document.title",tpt.first());
            setValueOfMapKey("document.titlecode",tpt[1]);
        }
    }
}

QMap<QString,QString> getTitleFilenamePairsFromFolder(QString folder,QMap<QString,QString> restrict_annot_values)
{
    QMap<QString,QString> result;
    QDir dir(folder);
    if(!dir.exists())
        return result;

    QStringList nameFilters;
    nameFilters << "*.pot" << "*.POT" << "*.Pot";
    QFileInfoList fileList = dir.entryInfoList(nameFilters, QDir::Files | QDir::NoSymLinks);
    foreach (const QFileInfo &fileInfo, fileList)
    {
        QString filename = fileInfo.fileName();
        QMap<QString, QStringList> annotations = getAnnotationValuesFromFile(fileInfo.absoluteFilePath());
        QStringList titles = annotations.value("Title", QStringList(fileInfo.baseName()));

        bool skip_by_restrict = false;
        QMap<QString, QString>::Iterator i;
        for(i = restrict_annot_values.begin() ; i != restrict_annot_values.end() ; ++i )
        {
            if(!annotations.value(i.key()).contains(i.value()))
            {
                skip_by_restrict = true;
                break;
            }
        }

        if(!skip_by_restrict)
            for(const QString& title : std::as_const(titles))
            {
                QStringList tpt = title.split("|");
                if(tpt.count() == 1)
                    result[title] = filename;
                if(tpt.count() > 1)
                    result[tpt.first()] = filename;
            }
    }
    return result;
}

QMap<QString,QStringList> getAnnotationValuesFromFile(QString filename)
{
    QMap<QString,QStringList> result;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return result;

    QTextStream in(&file);
    result = getAnnotationValuesFromText(in.readAll());
    file.close();
    return result;
}

QMap<QString,QStringList> getAnnotationValuesFromText(QString documentSource)
{
    QMap<QString,QStringList> result;
    QTextStream in(&documentSource);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.startsWith("//") && line.contains("@") && line.contains(":"))
        {
            QString commentsubline = line.mid(2).trimmed();
            if(commentsubline.startsWith("@"))
            {
                QStringList parts = commentsubline.mid(1).split(":");
                if(parts.count() == 2)
                    result[parts[0].trimmed()].push_back(parts[1].trimmed());
            }
            continue;
        }
    }
    return result;
}

QList<QString> getAnnotationLinesFromText(QString documentSource)
{
    QList<QString> result;
    QTextStream in(&documentSource);
    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.startsWith("//") && line.contains("@") && line.contains(":"))
        {
            QString commentsubline = line.mid(2).trimmed();
            if(commentsubline.startsWith("@"))
                result.push_back(commentsubline.mid(1).trimmed());
            continue;
        }
    }
    return result;
}

bool DocAssembler::isMapKeyExists(QString fullname)
{
    if(fullname.startsWith("."))
        fullname = fullname.mid(1);
    QStringList p = fullname.split(".",Qt::SkipEmptyParts);
    if(p.count() == 1)
        return textProcessor->isMapKeyExists("values",p[0]);
    if(p.count() > 1)
    {
        QString mapname = p[0];
        QString keyname = fullname.mid(mapname.length() + 1);
        return textProcessor->isMapKeyExists(mapname,keyname);
    }
    return false;
}

QString DocAssembler::valueOfMapKey(QString fullname)
{
    if(fullname.startsWith("."))
        fullname = fullname.mid(1);
    QStringList p = fullname.split(".",Qt::SkipEmptyParts);
    if(p.count() == 1)
        return textProcessor->valueOfMapKey("values",p[0]);
    if(p.count() > 1)
    {
        QString mapname = p[0];
        QString keyname = fullname.mid(mapname.length() + 1);
        return textProcessor->valueOfMapKey(mapname,keyname);
    }
    return QString();
}

bool DocAssembler::setValueOfMapKey(QString fullname,QString value)
{
    if(fullname.startsWith("."))
        fullname = fullname.mid(1);
    QStringList p = fullname.split(".",Qt::SkipEmptyParts);
    if(p.count() == 1)
        return textProcessor->setValueOfMapKey("values",p[0],value);
    if(p.count() > 1)
    {
        QString mapname = p[0];
        QString keyname = fullname.mid(mapname.length() + 1);
        return textProcessor->setValueOfMapKey(mapname,keyname,value);
    }
    return false;
}

int DocAssembler::askRequestedData(QWidget *widgetParent, QSize dialogSize)
{
    QList<QString> annLin;
    QList<QString> setValKeys;

    HRecord *rec = new HRecord("get_data");
    annLin = getAnnotationLinesFromText(textProcessor->processLine(rawDocumentSource));
    int fileld_number_to_get = 0;
    for(auto it = annLin.constBegin(); it != annLin.constEnd(); ++it)
    {
        if(it->startsWith("GET:") || it->startsWith("GETNE:"))
        {
            bool ne = false;
            QString cmdline;
            if(it->startsWith("GET:"))
            {
                cmdline = it->mid(4).trimmed();
                ne = false;
            }
            if(it->startsWith("GETNE:"))
            {
                cmdline = it->mid(6).trimmed();
                ne = true;
            }

            QStringList cparts = cmdline.split("#",Qt::SkipEmptyParts);
            if(cparts.count() == 3 || cparts.count() == 4)
            {
                if(cparts[0] == "string")
                {
                    if(!ne || !isMapKeyExists(cparts[1]))
                    {
                        HField *f = new HSmallTextField(cparts[1],cparts[2],"title");
                        f->setColor(170,170,255);
                        rec->addField(f);
                        if(cparts.count() == 4)
                            rec->setStrValue(cparts[1],cparts[3]);
                        if(isMapKeyExists(cparts[1]))
                            rec->setStrValue(cparts[1],valueOfMapKey(cparts[1]));
                        setValKeys.push_back(cparts[1]);
                        fileld_number_to_get++;
                    }
                }
                if(cparts[0] == "date")
                {
                    if(!ne || !isMapKeyExists(cparts[1]))
                    {
                        HField *f = new HDateField(cparts[1],cparts[2],"title");
                        f->setColor(170,170,255);
                        rec->addField(f);
                        rec->setStrValue(cparts[1],QDate::currentDate().toString(Qt::ISODate));
                        if(cparts.count() == 4)
                        {
                            QDate defDate = QDate::fromString(cparts[3], Qt::ISODate);
                            if(defDate.isValid())
                                rec->setStrValue(cparts[1], defDate.toString(Qt::ISODate));
                            else
                                rec->setStrValue(cparts[1],QDate::currentDate().toString(Qt::ISODate));
                        }
                        if(isMapKeyExists(cparts[1]))
                        {
                            QDate getDate = QDate::fromString(valueOfMapKey(cparts[1]), Qt::ISODate);
                            if(getDate.isValid())
                                rec->setStrValue(cparts[1], getDate.toString(Qt::ISODate));
                            else
                                rec->setStrValue(cparts[1],QDate::currentDate().toString(Qt::ISODate));
                        }
                        setValKeys.push_back(cparts[1]);
                        fileld_number_to_get++;
                    }
                }
                if(cparts[0] == "bool")
                {
                    if(!ne || !isMapKeyExists(cparts[1]))
                    {
                        HField *f = new HCheckField(cparts[1],cparts[2],"title");
                        f->setColor(170,170,255);
                        rec->addField(f);
                        if(cparts.count() == 4)
                        {
                            QString val = cparts[3];
                            if(val == "true" || val == "1" || val == "yes" || val == "y" || val == "t")
                                rec->setStrValue(cparts[1], "1");
                            else
                                rec->setStrValue(cparts[1], "0");
                        }
                        if(isMapKeyExists(cparts[1]))
                        {
                            QString val = valueOfMapKey(cparts[1]).toLower();
                            if(val == "true" || val == "1" || val == "yes" || val == "y" || val == "t")
                                rec->setStrValue(cparts[1], "1");
                            else
                                rec->setStrValue(cparts[1], "0");
                        }
                        setValKeys.push_back(cparts[1]);
                        fileld_number_to_get++;
                    }
                }
            }
        }
    }

    //No need to ask data
    if(fileld_number_to_get == 0)
    {
        delete rec;
        return 0;
    }

    HDialog *dlg = new HDialog(widgetParent);
    dlg->setAttribute("window_title", tr("Input required data"));
    dlg->setAttribute("window_resizebuttons","all");
    dlg->setAttribute("stretch_before_bottom_buttons","yes");
    dlg->setAttribute("button_1_text",tr("Ok"));
    dlg->setAttribute("button_1_action","accept");
    dlg->add(rec);
    dlg->resize(dialogSize);
    int modcount = 0;
    if(dlg->exec() == QDialog::Accepted)
    {
        foreach(const QString &key, setValKeys)
        {
            if(rec->fieldByName(key)->className() == "HDateField")
            {
                QDate date = QDate::fromString(rec->strValue(key), Qt::ISODate);
                if(date.isValid())
                {
                    setValueOfMapKey(key,date.toString(Qt::ISODate));
                    setValueOfMapKey(key + "_fulldate",date.toString(Qt::ISODate));
                    setValueOfMapKey(key + "_year",date.toString("yyyy"));
                    setValueOfMapKey(key + "_year2",date.toString("yy"));
                    setValueOfMapKey(key + "_month",date.toString("MM"));
                    setValueOfMapKey(key + "_monthname",monthNames.value(date.month(), "Error"));
                    setValueOfMapKey(key + "_day",date.toString("dd"));
                    setValueOfMapKey(key + "_fulldetailedname",QString("%1 %2 %3")
                                            .arg(date.toString("yyyy"))
                                            .arg(monthNames.value(date.month(), "Error"))
                                            .arg(date.toString("dd")));
                    ++modcount;
                }
            }
            if(rec->fieldByName(key)->className() == "HSmallTextField")
            {
                setValueOfMapKey(key,rec->strValue(key));
                ++modcount;
            }
            if(rec->fieldByName(key)->className() == "HCheckField")
            {
                setValueOfMapKey(key,rec->strValue(key));
                ++modcount;
            }
        }
    }
    delete dlg;
    return modcount;
}

QMap<QString, QString> merge_maps(const QMap<QString, QString> &base, const QMap<QString, QString> &overrides)
{
    QMap<QString, QString> result = base;
    for (auto it = overrides.constBegin(); it != overrides.constEnd(); ++it)
    {
        result[it.key()] = it.value();
    }
    return result;
}

/* This function receives a QMap<QString,QString> and splits it into a
   more QMap<QString, QString> maps according to the keys prefixes which
   are separated by a dot ('.') character.
   For example, a key "user.name" will be placed into a map with the name "user"
   The result is a QMap where the keys are the prefixes and the values are the corresponding QMap<QString,QString> */
QMap< QString, QMap<QString,QString> > split_stringmaps(QMap<QString,QString> input)
{
    QMap< QString, QMap<QString,QString> > result;

    for (auto it = input.constBegin(); it != input.constEnd(); ++it)
    {
        const QString &fullKey = it.key();
        QString value = it.value();

        int dotIndex = fullKey.indexOf('.');
        if (dotIndex == -1)
        {
            // No prefix, put in a default map
            result["values"].insert(fullKey, value);
        }
        else
        {
            QString prefix = fullKey.left(dotIndex);
            QString subKey = fullKey.mid(dotIndex + 1);
            result[prefix].insert(subKey, value);
        }
    }

    return result;
}

