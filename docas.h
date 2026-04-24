/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   docas.h
*/

#ifndef GSAFE__DOCASSEMBLER_H
#define GSAFE__DOCASSEMBLER_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "po.h"

/*  @{  */

/** Helper class to generate gSafe HPageTileRenderer documents
 *  Manage some annotations, underlay pdf merging, data requesting, etc */
class DocAssembler : public QObject
{
    Q_OBJECT

public:
    DocAssembler(QString documentSource);
    ~DocAssembler();

    int  askRequestedData(QWidget *widgetParent, QSize dialogSize = QSize(500, 400));
    void generatePdfDocument(QString outputFile);

    void setTitleValues(QString selectedTitle);

    void addValueMap(QString name,const QMap<QString,QString>& m);
    void addValueList(QString name,const QList<QString>& l);
    void addValueMapPtr(QString name,QMap<QString,QString>* m);
    void clearValueMaps();

    void setMonthNames(QStringList customMonthNames);

    void setWorkingDirectory(QString dir);
    void setSourceDocDirectory(QString dir);

    QString processLineByTextProcessor(QString line);
    QString processTokenByTextProcessor(QString in);

    QMap<QString,QStringList> getAnnotations();

    static QMap<QString,QString> extractDate(QString isodate,QString keyprefix);

protected:
    bool isMapKeyExists(QString fullname);
    QString valueOfMapKey(QString fullname);
    bool setValueOfMapKey(QString fullname,QString value);

    QString rawDocumentSource;
    HTextProcessor *textProcessor;
    int minumimGenPageCount;

    QString workingDirectory;
    QString sourceDocDirectory;
    
    QString preprocessedDoc;
    QMap<QString,QString> filenames;
    QMap<QString,QStringList> read_annotations;

    static QStringList monthNames;
    
    int preprocessDocument();
    int generateFilenames();
    int generateBasePdf();
    int finishingPdf();

    int getPageCountOfPdf(QString filename);
    int deleteWorkfileIfExists(QString filename);

public:
    bool enable_render_warnings;

    QMap<QString,HPageTileRendererPosition> lastRenderStoredPositions;
};

QMap<QString,QString>     getTitleFilenamePairsFromFolder(QString folder,QMap<QString,QString> restrict_annot_values = QMap<QString,QString>());
QMap<QString,QStringList> getAnnotationValuesFromText(QString documentSource);
QMap<QString,QStringList> getAnnotationValuesFromFile(QString filename);
QList<QString>            getAnnotationLinesFromText(QString documentSource);

QMap<QString, QString> merge_maps(const QMap<QString, QString> &base, const QMap<QString, QString> &overrides);
QMap< QString, QMap<QString,QString> > split_stringmaps(QMap<QString,QString> input);

/* @} */
#endif // GSAFE__DOCASSEMBLER_H

//End of gSAFE docas.h
