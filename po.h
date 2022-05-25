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
#include <QtWidgets>

#include "dm.h"

/*  @{  */

enum HPageTileRenderer_TextType {
    HTextType_Plain     = 0,
    HTextType_Html      = 1,
    HTextType_Markdown  = 2,
};

/** A minimal modified version of QTextDocument.
 *  Its able to draw content with different colors by drawColorContents method */
class HTextDocument : public QTextDocument
{
public:
    HTextDocument();
    ~HTextDocument();

    void drawColorContents(QPainter *p, const QColor& color,const QRectF &rect = QRectF());
};

class HPageTileRendererPosition
{
public:
    HPageTileRendererPosition();

    bool valid;
    int page;
    int pixel_x,pixel_y,pixel_w,pixel_h;
    double percent_x,percent_y,percent_w,percent_h;

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
    void addRect(QString width,QString height);
    void addText(QString width,QString text,HPageTileRenderer_TextType type = HTextType_Html);
    void addImage(QString width,QImage image);
    void drawRect(QString xpos,QString ypos,QString width,QString height);
    void drawText(QString xpos,QString ypos,QString width,QString text,HPageTileRenderer_TextType type = HTextType_Html);
    void drawImage(QString xpos,QString ypos,QString width,QImage image);
    void newLine();
    void newPage();

    int  calcTextHeight(QString width,QString text,HPageTileRenderer_TextType type = HTextType_Html);
    void incrementMinLineHeightToTextHeight(QString width,QString text,HPageTileRenderer_TextType type = HTextType_Html);

    void storePositionOfNextAddElement(QString withName);
    HPageTileRendererPosition storedPosition(QString withName);
    QMap<QString,HPageTileRendererPosition> storedPositions();

    /** The numbering starts from 0
     *  -1 means no page filter */
    void setPageFilter(int pf);

    void setMinimumLineHeight(int mlh);
    void setMinimumLineHeight(QString mlh);
    void setBorder(HBorderFlag b);
    void setTextAlignment(Qt::Alignment textAlignment);
    void setPen(QPen npen);
    void setBrush(QBrush b);
    void setFontColor(QColor c);
    void setFont(QFont f);
    void setDefaultFont(QFont f);
    void resetToDefaultFont(void);
    QColor getFontColor();

    /**
     * Line oriented renderer function.
     * Every line can holds one of the following instructions
     * The command name and arguments are separated by # hashmarks
     *
     *   mova - Move cursor to absolute position
     *          mova#<pX>,<pY>
     *          <pX> and <pY> are POSITION STRINGS (see below)
     *
     *   movr - Move cursor relative from the current position
     *          movr#<dX>,<dY>
     *          <dX> and <dY> are POSITION STRINGS (see below)
     *
     *   newl - Start a new line
     *
     *   newp - Start a new page
     *
     *   rect - Adds an empty rectangle at the cursor position or absolute position
     *          rect#<sX>,<sY>
     *          rect#<pX>,<pY>,<sX>
     *          <sX> and <sY> are POSITION STRINGS (see below)
     *          The rectangle can have frame and fill color but no content
     *
     *   text - Adds a plain text at the cursor position or absolute position
     *          text#<sX>#<Text until the line end>
     *          text#<pX>,<pY>,<sX>#<Text until the line end>
     *          <sX> is POSITION STRING specify the width of the text box where the text wrapped in.
     *          The height is automatically calculated except if minimum line height is set.
     *          The box can have frame and fill color and use the alignment settings
     *
     *   html - Adds a html text at the cursor position or absolute position
     *          html#<sX>#<Html until the line end>
     *          html#<pX>,<pY>,<sX>#<Html until the line end>
     *          <sX> is POSITION STRING specify the width of the html box where the html wrapped in.
     *          The height is automatically calculated except if minimum line height is set.
     *          The box can have frame and fill color and use the alignment settings
     *
     *   mark - Adds a markdown text at the cursor position or absolute position
     *          mark#<sX>#<Markdown until the line end>
     *          mark#<pX>,<pY>,<sX>#<Markdown until the line end>
     *          <sX> is POSITION STRING specify the width of the text box where the text wrapped in.
     *          The height is automatically calculated except if minimum line height is set.
     *          The box can have frame and fill color and use the alignment settings
     *
     *   imgr - Adds an image at he cursor position or absolute position from file (Or Qt resource)
     *          imgr#<sX>#<filename>
     *          imgr#<pX>,<pY>,<sX>#<filename>
     *          <sX> is POSITION STRING specify the width of the image
     *          The height is automatically calculated
     *          The filename can be Qt resource too, and box can have frame
     *
     *   imgb - Adds an image at he cursor position or absolute position from base64 string
     *          imgb#<sX>#<base64 encoded image>
     *          imgr#<pX>,<pY>,<sX>#<base64 encoded image>
     *          <sX> is POSITION STRING specify the width of the image
     *          The height is automatically calculated
     *
     *   smht - Increment minimum line height to the calculated heigt of the text fragment
     *          smht#<sX>#<Text until the line end>
     *          <sX> is POSITION STRING specify the width of the text box where the text measured in.
     *          If the calculated height is larger than the current minimum line height,
     *          the value is updated the this calculated value
     *
     *   smhh - Increment minimum line height to the calculated heigt of the html fragment
     *          smhh#<sX>#<Html until the line end>
     *          <sX> is POSITION STRING specify the width of the text box where the html measured in.
     *          If the calculated height is larger than the current minimum line height,
     *          the value is updated the this calculated value
     *
     *   smhm - Increment minimum line height to the calculated heigt of the markdown fragment
     *          smhm#<sX>#<Markdown until the line end>
     *          <sX> is POSITION STRING specify the width of the text box where the markdown measured in.
     *          If the calculated height is larger than the current minimum line height,
     *          the value is updated the this calculated value
     *
     *   smhz - Set minimum line height to zero
     *          smhz
     *
     *   smhv - Set minimum line height to a specified value
     *          smhv#<sY>
     *          <sY> is POSITION STRING specify the height
     *
     *   colf - Set color of the font (letters)
     *          colf#<rrggbb>
     *          <rrggbb> is the hexadecimal R G B codes, just like in html without # sign
     *
     *   coll - Set color of the lines & frames
     *          coll#<rrggbb>
     *          <rrggbb> is the hexadecimal R G B codes, just like in html without # sign
     *
     *   colb - Set color of the fills (Text backgrounds)
     *          colb#<rrggbb>
     *          <rrggbb> is the hexadecimal R G B codes, just like in html without # sign
     *
     *   fram - Set frames
     *          fram#<none|all|top|right|bottom|left|fill>,..
     *          Comma separated list of words above.
     *            fram#top,right,fill
     *            fram#none
     *
     *   alig - Set text alignment in the box
     *          alig#<left|center|right>
     *
     *   setf - Set current font family and size
     *          setf#<FontName>,<PointSize>
     *
     *   setd - Set current and the default font family and size
     *          setd#<FontName>,<PointSize>
     *
     *   deff - Return the current font to the default
     *          deff
     *
     *   getp - Store the redered position of the next "Add element" and hold under the specified name
     *          getp#<Name>
     *
     *  The POSITION STRING can be:
     *   Type             String  Means
     *   Simple string    120     120 pixel
     *   Percent size     20%     20% of the page width or height depending of the position
     *   Letter size      2em     2letter width or height
     *   Reverse size     -10%    =90% which 10% back of the page width or height
     *                    -80     80pixels less then the page width or height
     *   Dinamic size     >50%    Calculate the width or height from the current position
     *                            to the half (50%) of the page width or height
     *                    >-2em   Caclulate the width until 2 letter less than page width or height
     *
     */
    void renderFromInstructions(QString txtintr);

    int currentPageIndex();

protected:
    int sizeStrToInt(QString str,QString xy);
    void drawBorders(int w,int h);
    void storePos(int w,int h);

signals:
    void startNewPage(void);
    void startNewLine(void);

protected:
    QPainter *p;

    int currentPage,pageFilter;
    int cursorX,cursorY,currentLineHeight,minLineHeight;
    QFont defaultFont,font;
    QColor fontColor;
    Qt::Alignment alignment;
    QPen pen;
    QBrush brush;
    HBorderFlag border;
    QString storePosOfNext;
    QMap<QString,HPageTileRendererPosition> storedPos;
};

/** Text preprocessor for HPageTileRenderer's renderFromInstructions method */
class HTextProcessor
{
public:
    HTextProcessor();
    ~HTextProcessor();

    QString processDoc(QString in);
    QString processLine(QString in);
    QString processToken(QString in);

    void addValueMap(QString name,const QMap<QString,QString>& m);

protected:
    QMap<QString, QMap<QString,QString> > smaps;
};

/** Frame to be render the pdf content. Used by HPdfPreviewDialog. */
class HPdfPreviewFrame : public QFrame
{
    Q_OBJECT

public:
    HPdfPreviewFrame(QWidget *parent);
    ~HPdfPreviewFrame();

    int showPageIndex;
    QString rawContent;
    int maxPage;

protected:
    void paintEvent(QPaintEvent *e);
};

/** Pdf preview dialog with generator possibility */
class HPdfPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    HPdfPreviewDialog(QWidget *parent,bool generate_button = true);
    ~HPdfPreviewDialog();

    void setRawContent(QString c);
    void addAttachmentFile(QString name,QString content);

    int generatePdfFile(QString filename);

public slots:
    int generatePdf(void);
    int startNewPage();
    int changePage(int p);
    int nextPage();
    int prevPage();

protected:
    QMap<QString,QString> attachmentFiles;
    QLabel *pageShow;
    QPdfWriter *pdfWriter;
    HPdfPreviewFrame *ppf;

public:
    QMap<QString,HPageTileRendererPosition> lastRenderStoredPositions;
};

/* @} */
#endif

//End of gSAFE po.h
