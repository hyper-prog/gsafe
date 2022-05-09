/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    po.h
*/

#ifndef GSAFE__PO_HEADER_FILE_X_
#define GSAFE__PO_HEADER_FILE_X_

#include <QtCore>
#include <QtGui>

#include "dm.h"

/*  @{  */

enum HPageTileRenderer_TextType {
    HTextType_Plain     = 0,
    HTextType_Html      = 1,
    HTextType_Markdown  = 2,
};

/** This class can render a QPainter device by tiled text and image fields.
 *  Useable to generate screen, pdf or printer output.
 *  The text fields can receive html or markdown texts while the borders and alignments are also configurable.
 *  The line and page wraps are automatically calculated.
 *  In case of some device output the new page signal have to be connected to the device slot to handle page change. */
class HPageTileRenderer : public QObject
{
    Q_OBJECT

public:
    HPageTileRenderer(QPainter *configuredPainter);
    ~HPageTileRenderer();

    void moveCursorRelative(QString x,QString y);
    void moveCursorAbsolute(QString x,QString y);
    void addText(QString width,QString text,HPageTileRenderer_TextType type = HTextType_Html);
    void addImage(QString width,QImage image);
    void drawText(QString xpos,QString ypos,QString width,QString text,HPageTileRenderer_TextType type = HTextType_Html);
    void drawImage(QString xpos,QString ypos,QString width,QImage image);
    void newLine();
    void newPage();

    int  calcTextHeight(QString width,QString text,HPageTileRenderer_TextType type = HTextType_Html);

    void setPageFilter(int pf);

    void setMinimumLineHeight(int mlh);
    void setBorder(HBorderFlag b);
    void setTextAlignment(Qt::Alignment textAlignment);
    void setPen(QPen npen);
    void setBrush(QBrush b);
    void setFont(QFont f);

protected:
    int sizeStrToInt(QString str,QString xy);
    void drawBorders(int w,int h);

signals:
    void startNewPage(void);
    void startNewLine(void);

protected:
    QPainter *p;

    int currentPage,pageFilter;
    int cursorX,cursorY,currentLineHeight,minLineHeight;
    QFont defaultFont;
    Qt::Alignment alignment;
    QPen pen;
    QBrush brush;
    HBorderFlag border;
};

/* @} */
#endif

//End of gSAFE po.h
