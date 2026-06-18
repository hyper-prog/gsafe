/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   po.h
*/

#ifndef GSAFE__PO_HEADER_FILE_X_
#define GSAFE__PO_HEADER_FILE_X_

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#ifndef GSAFE_DISABLE_PRINTERMODULE
#include <QPrinter>
#endif

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

    void moveCursorRelative(const QString& x,const QString& y);
    void moveCursorAbsolute(const QString& x,const QString& y);
    void addSpace(const QString& width,const QString& height);
    void addRect(const QString& width,const QString& height);
    void addText(const QString& width,const QString& text,HPageTileRenderer_TextType type = HTextType_Html);
    void addImage(const QString& width,const QImage& image);
    void drawRect(const QString& xpos,const QString& ypos,const QString& width,const QString& height);
    void drawGrid(const QString& xpos,const QString& ypos,const QString& width,const QString& height);
    void drawText(const QString& xpos,const QString& ypos,const QString& width,const QString& text,HPageTileRenderer_TextType type = HTextType_Html);
    void drawImage(const QString& xpos,const QString& ypos,const QString& width,const QImage& image);
    void drawImage4(const QString& xpos,const QString& ypos,const QString& width,const QString& height,const QImage& image);
    void newLine();
    void newPage();
    void newPageIfRequired(const QString& requiredHeight);
    void newPageUntilCount(const QString& pageCount);

    void enterArea(const QString& width,const QString& height);
    void enterArea(const QString& xpos,const QString& ypos,const QString& width,const QString& height);
    void returnArea();

    int  calcTextHeight(const QString& width,const QString& text,HPageTileRenderer_TextType type = HTextType_Html);
    int  calcImageHeight(const QString& width,const QImage& image);
    void incrementMinLineHeightToTextHeight(const QString& width,const QString& text,HPageTileRenderer_TextType type = HTextType_Html);
    void incrementMinLineHeightToImageHeight(const QString& width,const QImage& image);
    void incrementMinLineHeightToValue(const QString& height);

    void storePositionOfNextAddElement(const QString& withName);
    HPageTileRendererPosition storedPosition(const QString& withName);
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
    void setMargins(QString top,QString right,QString bottom,QString left);
    QColor getFontColor();

    void setUnknownCommandWarning(bool showWarnings);

    /**
     * Line oriented renderer function.
     * Every line can holds one of the following instructions
     * The command name and arguments are separated by # hashmarks
     *
     * Note: The relative drawings (which does not have start position) positioned to the cursor
     *       and moves the cursor position onward. (Eg: text#20%#Relative posotioned text )
     *       In contrast the absolute drawing elements (which has a start position) does not modifies
     *       the position of cursor!  (Eg: text#10%,1em,80%#Absolute posotioned text )
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
     *   fixh - Enables calculated fix line height for the following items until the line end
     *          fixh
     *          This command will enable buffering the subsequent commands until the line end.
     *          Therefore, this instruction always must be followed by "newl" instruction!
     *          When the line end is received the line height is calculated on the buffered elements
     *          so the rendering is done with the derived maximum of minimum line height on all item.
     *
     *   spac - Adds an empty space (box) at the cursor position
     *          spac#<sX>,<sY>
     *          <sX> and <sY> are POSITION STRINGS (see below)
     *          It does not have border or fill regardless the current settings
     *          but moves the cursor or increase line height if necessary
     *
     *   rect - Adds an empty rectangle at the cursor position or absolute position
     *          rect#<sX>,<sY>
     *          rect#<pX>,<pY>,<sX>
     *          <sX> and <sY> are POSITION STRINGS (see below)
     *          The rectangle can have frame and fill color but no content
     *          If relative then moves the cursor or increase line height if necessary
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
    *          imgr#<pX>,<pY>,<sX>,<sY>#<filename>
     *          <sX> is POSITION STRING specify the width of the image
    *          For 3-parameter absolute form (<pX>,<pY>,<sX>) the height is automatically calculated
    *          For 4-parameter absolute form (<pX>,<pY>,<sX>,<sY>) the image keeps aspect ratio and fits into the box
     *          The filename can be Qt resource too, and box can have frame
     *
     *   imgb - Adds an image at he cursor position or absolute position from base64 string
     *          imgb#<sX>#<base64 encoded image>
    *          imgb#<pX>,<pY>,<sX>#<base64 encoded image>
    *          imgb#<pX>,<pY>,<sX>,<sY>#<base64 encoded image>
     *          <sX> is POSITION STRING specify the width of the image
    *          For 3-parameter absolute form (<pX>,<pY>,<sX>) the height is automatically calculated
    *          For 4-parameter absolute form (<pX>,<pY>,<sX>,<sY>) the image keeps aspect ratio and fits into the box
     *
     *   smhr - Increment minimum line height to the calculated heigt of the image passed by filename
     *          smhf#<sX>#<filename>
     *          <sX> is POSITION STRING specify the width of the image
     *
     *   smhi - Increment minimum line height to the calculated heigt of the image passed as base64
     *          smhi#<sX>#<base64 encoded image>
     *          <sX> is POSITION STRING specify the width of the image
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
     *   area - Restrict the drawing to a specified area
     *          area#<sX>,<sY>
     *          area#<pX>,<pY>,<sX>,<sY>
     *          <sX>,<sY> is the size of the restricted drawing area
     *                    (If only the size is specified the start pont will be the cursor position)
     *          <pX>,<pY> is the absolute start point (left upper corner) of the
     *                    restricted drawing area
     *
     *   reta - Return from the restricted drawing area which entered by "area" command
     *          reta
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
     *          alig#<left|center|right|just>
     *
     *   setf - Set current font family and size
     *          setf#<FontName>,<PointSize>
     *
     *   npif - New page if the required height is not enough
     *          npif#<rH>
     *          <rH> is POSITION STRING specify the height
     *
     *   npuc - Start new pages until the whole document page count is reached the parameter value
     *          npuc#<pC>
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
     *   marg - Set margins in pixels (top,right,bottom,left)
     *          marg#<sT>,<sR>,<sB>,<sL>
     *          <sX> is POSITION STRING specify the height
     *          Sample: marg#25mm,25mm,25mm,25mm
     *
     *   sizc - Size correction value, change ratio between the experinced and calculated phisical size
     *          (only affects "mm" and "cm" size classes)
     *          sizc#1.075
     *          Default is 1.000
     *
     *   grid - Draw a grid into the specified cell
     *         grid#<sX>,<sY>,<w>,<h>
     *
     *   EVERYPAGE_START - Start a section which is re-played on every new page
     *          EVERYPAGE_START
     *
     *   EVERYPAGE_END - End the section which is re-played on every new page
     *          EVERYPAGE_END
     *
     *
     *  The POSITION STRING can be:
     *   Type             String  Means
     *   Simple string    120     120 pixel
     *   Percent size     20%     20% of the page width or height depending of the position
     *   Letter size      2em     2letter width or height
     *   Physical size    1cm     1 centimeter
     *   Physical size    5mm     5 millimeter
     *   Reverse size     -10%    =90% which 10% back of the page width or height
     *                    -80     80pixels less then the page width or height
     *   Dinamic size     >50%    Calculate the width or height from the current position
     *                            to the half (50%) of the page width or height
     *                    >-2em   Caclulate the width until 2 letter less than page width or height
     *
     *   Although the renderFromInstructions interprets one instruction per line you can write
     *   multiline instructions if the last characters of the line is #{
     *   If you write so, the following lines are concatenated together until a line is received which only ontains a }
     *   Sample:
     *
     *    html#100%#{
     *        This is a <strong>multiline</strong> text,
     *        which show you how to write
     *        multiline texts!
     *    }
     */
    void renderFromInstructions(const QString& txtintr);

    int currentPageIndex();

protected:
    void renderFromInstructionLineHL(const QStringList& parts);
    void renderFromInstructionLineLL(const QStringList& parts);

    int sizeStrToInt(QString str,const QString& xy);
    void drawBorders(int w,int h);
    void storePos(int w,int h);
    QString strSubstTokens(QString in);
    int millimeterToPixel(double mm);

    int areaWidth();
    int areaHeight();

    int playPageBoot();

signals:
    void startNewPage(void);
    void startNewLine(void);

protected:
    class AreaData
    {
    public:
        int sizeW;
        int sizeH;
        int cursorX;
        int cursorY;
        int currentLineHeight;
        int currentPage;
        int pageFilter;
        int margint,marginr,marginb,marginl;
    };

    QPainter *p;

    int currentPage,pageFilter;
    int cursorX,cursorY,virtualCursorXincr,currentLineHeight,minLineHeight;
    QFont defaultFont,font;
    Qt::Alignment alignment;
    QFont fs_defaultFont,fs_font;
    Qt::Alignment fs_alignment;
    int marginTop,marginRight,marginBottom,marginLeft;
    QColor fontColor;
    QPen pen;
    QBrush brush;
    HBorderFlag border;
    QString storePosOfNext;
    QMap<QString,HPageTileRendererPosition> storedPos;
    QList<AreaData> areastack;
    QList<QStringList> instruction_buffer;
    QList<QStringList> pageboot_buffer;
    bool inPageBoot;
    bool unknownCommandWarning;
    double sizeCorrectionValue;

public:
    int resolutionDpi;
    int physicalWidthMillimeter;
    int physicalHeightMillimeter;
};

/** Text preprocessor for HPageTileRenderer's renderFromInstructions method
 *  The following tokens are supported in the text:
 *
 *    // The lines started with // are comments and ignored and does not passed
 *          to the renderer.
 *
 *    You can place named annotations in comments.
 *    These annotations starts with @ and followed by the annotation name
 *    and colon and the annotation content.
 *    // @TITLE:This is the TITLE annotation
 *
 *    {{TOKEN}} : Replaced by the value of the token.
 *
 *      TOKEN can be the following:
 *          .container.name
 *              The token value is searched in the value maps and lists which
 *              are added by addValueMap, addValueList and addValueMapPtr methods.
 *              Sample: {{.values.text}}
 *
 *          ..point  the value is .
 *          ..colon  the value is :
 *          ..semicolon  the value is ;
 *          ..question  the value is ?
 *          ..openbrackets  the value is {
 *          ..closebrackets  the value is }
 *
 *          .container1.name1|.container2.name2
 *              Rerurns the first token value which is not empty.
 *              Sample: {{.values.text|.defaults.text}}
 *              Sample2: {{.values.one|.values.two|Nothing found}}
 *
 *          string1=string2?value1:value2
 *             If string1 and string2 are equals then value1 is returned otherwise value2 is returned.
 *             If the string1 or string2 are starts with . they are interpreted as tokens
 *             so the value is replaced according to the maps or lists.
 *             The value1 and value2 can be string or token but instead the {{ and }} you shoud use
 *             the [[ and ]] to avoid the too early processing.
 *             Sample: {{.values.check=ok?[[.values.text]]:Not ok}}
 *
 *    COND#<value1>#<operator>#<value2>
 *        Conditional instructions
 *    ENDC
 *        The conditional instruction is used to conditionally render some part of the text.
 *        The <value1> and <value2> can be either a token or a simple string.
 *        The operator can be one of the following: =  !=  <  >  <=  >=
 *
 *    FUNC#<functionName>
 *        Function definition and calling instructions
 *    ENDF
 *        The function definition and calling instructions are used
 *        to define a function which can be called later in the text.
 *
 *    CAll#<functionName>
 *        The function call instruction is used to call a previously defined function.
 *
 *    ALLVARIABLES
 *        Print all variables into a table
*/
class HTextProcessor
{
public:
    HTextProcessor();
    ~HTextProcessor();

    /** Process a whole document */
    QString processDoc(const QString& in);

    QString processLine(QString in);
    QString processToken(QString in);

    void addValueMap(QString name,const QMap<QString,QString>& m);
    void addValueList(QString name,const QList<QString>& l);
    void addValueMapPtr(QString name,QMap<QString,QString>* m);
    void clearValueMaps();
    const QMap<QString,QString>& valueMap(QString name);
    const QList<QString>& valueList(QString name);
    QMap<QString,QString>* valueMapPtr(QString name);

    QMap<QString,QString> extractValuesFiltered(QString prefix);

    bool isMapKeyExists(const QString& name,const QString& key);
    QString valueOfMapKey(const QString& name,const QString& key);
    bool setValueOfMapKey(const QString& name,const QString& key,const QString& value);

    QMap<QString,QStringList> annotations();

protected:
    QMap<QString, QMap<QString,QString> > smaps;
    QMap<QString, QMap<QString,QString> * > dmaps;
    QMap<QString, QList<QString> > slist;
    QString definingFunctionName;
    QString definingFunctionBody;
    QMap<QString,QString> functions;

    QMap<QString,QStringList> annot;
};

/**
 * Preview surface for page-based instruction rendering.
 *
 * HPdfPreviewFrame is a lightweight QWidget/QFrame target that paints one
 * logical page of the text-instruction document generated by
 * HPageTileRenderer. It is designed to be embedded into a scroll area and used
 * as the visual page canvas inside HPdfPreviewDialog.
 *
 * Responsibilities:
 * - Keeps current preview state:
 *   - showPageIndex: currently displayed page (0-based).
 *   - rawContent: source instruction text rendered by HPageTileRenderer.
 *   - maxPage: highest page index discovered during render pass.
 * - Maintains logical page geometry (default A4 at 200 DPI: 1652x2338).
 * - Applies zoom by resizing the widget while preserving the logical render
 *   coordinate system.
 * - Renders only the selected page via renderer page filtering.
 *
 * Rendering model:
 * - paintEvent creates a QPainter on this frame.
 * - Painter window is set to logical page size, so drawing coordinates stay
 *   stable independent from widget pixel size.
 * - HPageTileRenderer renders rawContent with setPageFilter(showPageIndex).
 * - maxPage is updated from renderer.currentPageIndex().
 *
 * Zoom model:
 * - setZoomPercent updates widget size (minimum/maximum/resize) according to
 *   logical size * zoomPercent / 100.
 * - This class does not perform scrolling itself; scrolling is handled by the
 *   container (HPdfPreviewDialog via QScrollArea).
 *
 * Notes:
 * - This widget is paint-only preview state holder; it does not handle input
 *   policy (wheel, touch, key). Those behaviors are managed in
 *   HPdfPreviewDialog.
 */
class HPdfPreviewFrame : public QFrame
{
    Q_OBJECT

public:
    HPdfPreviewFrame(QWidget *parent);
    ~HPdfPreviewFrame();

    void setLogicalPageSize(int w,int h);
    void setZoomPercent(int zp);
    int getZoomPercent() const;
    int getLogicalPageWidth() const;
    int getLogicalPageHeight() const;

    int showPageIndex;
    QString rawContent;
    int maxPage;

protected:
    int logicalPageWidth;
    int logicalPageHeight;
    int zoomPercent;

protected:
    void paintEvent(QPaintEvent *e);
};

/**
 * Interactive PDF-style preview dialog for instruction-rendered documents.
 *
 * HPdfPreviewDialog is the high-level controller and UI container around
 * HPdfPreviewFrame + HPageTileRenderer. It provides page navigation, zoom,
 * fit-to-page behavior, printing, PDF file generation, and multi-input preview
 * interaction (mouse wheel, keyboard, touch, pinch).
 *
 * Main responsibilities:
 * - Hosts a scrollable preview area with dark-grey boundary background and a
 *   page canvas (HPdfPreviewFrame).
 * - Builds toolbar/status UI and mode selectors for preview interaction.
 * - Maintains runtime preview state (page index, zoom level, wheel mode,
 *   touch-scrolling state, temporary Ctrl-to-zoom override).
 * - Routes input events through a unified boundary-aware scroll/page-change
 *   policy.
 * - Supports both preview rendering and output rendering:
 *   - Preview: via HPdfPreviewFrame paint path.
 *   - Output: via generatePdfFile/print using HPageTileRenderer.
 *
 * Rendering and output flow:
 * - setRawContent updates source instruction text used by preview and output.
 * - generatePdfFile configures QPdfWriter (A4 portrait, 200 DPI), attaches
 *   files, renders instructions, and stores renderer element positions.
 * - print configures QPrinter and reuses the same renderer instruction flow.
 * - startNewPage is the renderer callback target used to advance physical
 *   output pages on PDF/printer devices.
 *
 * Interaction model:
 * - WheelMode_Zoom: wheel controls zoom.
 * - WheelMode_Scroll: wheel/keys/touch scroll the page; if scrolling reaches
 *   limits, behavior falls back to page turn with boundary semantics.
 * - Ctrl-hold temporary mode: while Ctrl is pressed in Scroll mode, mode
 *   temporarily switches to Zoom and restores on release.
 * - Keyboard navigation shortcuts are handled centrally in preview key
 *   processing helpers.
 * - Touch behavior includes one-finger scrolling with jitter stabilization and
 *   two-finger pinch zoom.
 *
 * Stability/UX notes:
 * - Auto-fit can remain active across dialog resize until a manual zoom action
 *   disables it.
 * - Unified scroll delta processing keeps wheel, arrow, and touch boundary
 *   transitions consistent.
 * - One-finger boundary page flip can be constrained to require finger lift
 *   before further movement on the new page.
 *
 * Translation/UI note:
 * - User-facing strings are emitted through tr() to allow host application
 *   localization.
 */
class HPdfPreviewDialog : public QDialog
{
    Q_OBJECT

public:
    enum WheelMode
    {
        WheelMode_Zoom = 1,
        WheelMode_Scroll = 2,
    };

public:
    HPdfPreviewDialog(QWidget *parent,QString buttons = "print,generate,close");
    ~HPdfPreviewDialog();

    void setRawContent(const QString& c);
    void addAttachmentFile(const QString& name,const QString& content);

    int generatePdfFile(QString filename);

public slots:
    int generatePdf(void);
    int startNewPage();
    int changePage(int p);
    int nextPage();
    int prevPage();
    int firstPage();
    int lastPage();
    int zoomIn();
    int zoomOut();
    int fitPage();
    int setWheelMode(int wm);
    int setZoomMode();
    int setScrollMode();
    int print();
    int editorTextChanged();

protected:
    bool event(QEvent *e);
    bool eventFilter(QObject *watched,QEvent *event);
    void resizeEvent(QResizeEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void wheelEvent(QWheelEvent *e);

    bool processWheelEvent(QWheelEvent *e);
    bool processGestureEvent(QGestureEvent *e);
    /**
     * Unified scroll/page routing for wheel, keyboard and one-finger touch deltas.
     *
     * The method first tries vertical scrolling inside the current page.
     * If no vertical scrolling is possible (fit-page) or a boundary is reached,
     * it falls back to thresholded page changes via applyPageFallbackDelta().
     *
     * @param deltaY Signed vertical delta in "scroll up/down" convention used by inputs.
     * @param pageFlipThreshold Absolute accumulator threshold to trigger page turn.
     * @param pageDeltaAccumulator Optional external accumulator.
     *        If null, wheelPageDeltaAccumulator is used.
     * @param didPageFlip Optional out-flag, set true when a page turn happened.
     * @param singlePageFlipPerCall If true, stop after first page turn in this call.
     */
    bool processScrollDelta(int deltaY,int pageFlipThreshold,int *pageDeltaAccumulator,bool *didPageFlip,bool singlePageFlipPerCall);
    /**
     * Applies boundary fallback by accumulating delta and turning pages on threshold.
     *
     * Negative accumulator turns to next page, positive to previous page.
     * After a successful page turn, vertical scrollbar is snapped to the matching
     * reading boundary (top for next page, bottom for previous page).
     */
    bool applyPageFallbackDelta(int sourceDeltaY,int pageFlipThreshold,int *accumulator,bool singlePageFlipPerCall,QScrollBar *vs);
    bool processPreviewTouch(QTouchEvent *e);
    void resetOneFingerTouchState();
    bool processPreviewKeyPress(QKeyEvent *e);
    bool processPreviewKeyRelease(QKeyEvent *e);
    void beginTemporaryCtrlZoom();
    void endTemporaryCtrlZoom();
    int applyZoomPercent(int newZoomPercent,const QPoint& viewportAnchorPoint = QPoint(-1,-1),bool manualZoom = true);
    void updatePageShow();

    QMap<QString,QString> attachmentFiles;
    int wheelMode;                    // Active wheel behavior (Zoom or Scroll).
    int wheelPageDeltaAccumulator;    // Threshold accumulator used for wheel-driven page fallback.
    QLabel *pageShow;
    QPdfWriter *pdfWriter;
#ifndef GSAFE_DISABLE_PRINTERMODULE
    QPrinter *printer;
#endif
    QTextEdit *rawEditor;
    HPdfPreviewFrame *ppf;
    QWidget *previewContainer;
    QScrollArea *previewScrollArea;
    QButtonGroup *wheelModeGroup;
    QToolButton *modeZoomButton;
    QToolButton *modeScrollButton;
    QHBoxLayout *main_horizontal_layout;
    QVBoxLayout *main_vertical_layout;
    QHBoxLayout *toplay;
    QHBoxLayout *statuslay;
    bool autoFitToViewport;           // Keeps fit-page mode sticky across resize until manual zoom.
    int previewBoundaryPadding;       // Visual dark-grey margin around white page.
    bool ctrlTemporaryZoomActive;     // True while Ctrl-hold temporary zoom override is active.
    int ctrlTemporaryPreviousMode;    // Mode restored on Ctrl release.
    bool oneFingerScrollActive;       // True between one-finger touch begin/end.
    QPointF oneFingerLastPos;         // Last touch point for delta computation.
    qreal oneFingerRemainderY;        // Sub-pixel Y accumulator for smooth slow touch drags.
    int oneFingerDirection;           // Last accepted touch direction (+1 down, -1 up, 0 unknown).
    int oneFingerPageDeltaAccumulator;// Threshold accumulator for one-finger boundary page turns.
    int oneFingerDeadZonePx;          // Ignores tiny touch noise below this delta.
    int oneFingerReverseHysteresisPx; // Required opposite-direction movement before reversing direction.
    int oneFingerPageFlipThreshold;   // Touch-specific page-turn threshold (larger than wheel).
    bool oneFingerLockAfterPageFlip;  // After touch page turn, require finger lift before next action.

public:
    bool enable_render_warnings;
    QMap<QString,HPageTileRendererPosition> lastRenderStoredPositions;
};

/* @} */
#endif

//End of gSAFE po.h
