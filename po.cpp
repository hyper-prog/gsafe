/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    po.cpp
*/

#include <QtCore>
#include <QtGui>

#include "po.h"

HPageTileRenderer::HPageTileRenderer(QPainter *configuredPainter)
{
    p = configuredPainter;
    cursorX = 0;
    cursorY = 0;
    currentLineHeight = 0;
    minLineHeight = 0;
    currentPage = 0;
    pageFilter = -1;
    defaultFont = QFont("Arial",18);

    p->setPen(QPen(QColor(0,0,0),5));
    p->setPen(Qt::SolidLine);
    alignment = Qt::AlignLeft;
    pen = QPen(QColor(255,255,255));
    brush = QBrush(QColor(100,100,100),Qt::SolidPattern);
    border = HBorderFlag_None;
}

HPageTileRenderer::~HPageTileRenderer()
{

}

void HPageTileRenderer::setPageFilter(int pf)
{
    pageFilter = pf;
}

void HPageTileRenderer::setMinimumLineHeight(int mlh)
{
    minLineHeight = mlh;
}

void HPageTileRenderer::setBorder(HBorderFlag b)
{
    border = b;
}

void HPageTileRenderer::setTextAlignment(Qt::Alignment textAlignment)
{
    alignment = textAlignment;
}

void HPageTileRenderer::setPen(QPen npen)
{
    pen = npen;
    p->setPen(pen);
}

void HPageTileRenderer::setBrush(QBrush b)
{
    brush = b;
    p->setBrush(brush);
}

void HPageTileRenderer::setFont(QFont f)
{
    defaultFont = f;
}

int HPageTileRenderer::sizeStrToInt(QString str,QString xy)
{
    if(str.isEmpty())
        return 0;
    if(str.endsWith("%"))
    {
        int percent = str.mid(0,str.length()-1).toInt();
        if(xy == "x")
            return ((p->window().width()) * percent) / 100;
        if(xy == "y")
            return ((p->window().height()) * percent) / 100;
    }
    if(str.endsWith("em"))
    {
        int mul = str.mid(0,str.length()-2).toInt();
        QFontMetrics fm(defaultFont);
        if(xy == "x")
            return fm.averageCharWidth() * mul;
        if(xy == "y")
            return fm.height() * mul;
    }
    return str.toInt();
}

void HPageTileRenderer::moveCursorRelative(QString x,QString y)
{
    cursorX += sizeStrToInt(x,"x");
    cursorY += sizeStrToInt(y,"y");
}

void HPageTileRenderer::moveCursorAbsolute(QString x,QString y)
{
    cursorX = sizeStrToInt(x,"x");
    cursorY = sizeStrToInt(y,"y");
}

void HPageTileRenderer::addText(QString width,QString text,HPageTileRenderer_TextType type)
{
    int w_px = sizeStrToInt(width,"x");

    if(cursorX + w_px > p->window().width())
        newLine();

    QRect r(cursorX,cursorY,w_px,1000);
    QTextDocument td;
    if(type == HTextType_Plain)
        td.setPlainText(text);
    if(type == HTextType_Html)
        td.setHtml(text);
    if(type == HTextType_Markdown)
        td.setMarkdown(text);
    td.setDefaultFont(defaultFont);
    td.setTextWidth(r.width());
    td.setDefaultTextOption(QTextOption(alignment));

    if(td.size().height() + cursorY > p->window().height())
    {
        newPage();
        r = QRect(cursorX,cursorY,w_px,1000);
    }

    if(pageFilter < 0 || pageFilter == currentPage)
    {
        p->save();
        p->translate(r.topLeft());
        if(flagOn(border,HBorderFlag_Fill))
            p->fillRect(0,0,td.size().width(),minLineHeight > td.size().height() ? minLineHeight : td.size().height(),brush);
        td.drawContents(p);
        drawBorders(td.size().width(),minLineHeight > td.size().height() ? minLineHeight : td.size().height());
        p->restore();
    }

    if(currentLineHeight < minLineHeight)
        currentLineHeight = minLineHeight;
    if(currentLineHeight < td.size().height())
        currentLineHeight = td.size().height();

    cursorX += td.size().width();
}

void HPageTileRenderer::drawText(QString xpos,QString ypos,QString width,QString text,HPageTileRenderer_TextType type)
{
    int x = sizeStrToInt(xpos,"x");
    int y = sizeStrToInt(ypos,"y");
    int w_px = sizeStrToInt(width,"x");
    QRect r(x,y,w_px,1000);
    QTextDocument td;
    if(type == HTextType_Plain)
        td.setPlainText(text);
    if(type == HTextType_Html)
        td.setHtml(text);
    if(type == HTextType_Markdown)
        td.setMarkdown(text);
    td.setDefaultFont(defaultFont);
    td.setTextWidth(r.width());
    td.setDefaultTextOption(QTextOption(alignment));

    if(pageFilter < 0 || pageFilter == currentPage)
    {
        p->save();
        p->translate(r.topLeft());
        if(flagOn(border,HBorderFlag_Fill))
            p->fillRect(0,0,td.size().width(),td.size().height(),brush);
        td.drawContents(p);
        drawBorders(td.size().width(),td.size().height());
        p->restore();
    }
}

void HPageTileRenderer::drawBorders(int w,int h)
{
    if(flagOn(border,HBorderFlag_Top))
        p->drawLine(0,0,w,0);
    if(flagOn(border,HBorderFlag_Right))
        p->drawLine(w,0,w,h);
    if(flagOn(border,HBorderFlag_Bottom))
        p->drawLine(0,h,w,h);
    if(flagOn(border,HBorderFlag_Left))
        p->drawLine(0,0,0,h);
}

void HPageTileRenderer::addImage(QString width,QImage image)
{
    int w_px = sizeStrToInt(width,"x");

    if(cursorX + w_px > p->window().width())
        newLine();

    int height = (int)(((double)w_px / (double)image.width()) * (double)image.height());

    QRect r(cursorX,cursorY,w_px,height);

    if(height + cursorY > p->window().height())
    {
        newPage();
        r = QRect(cursorX,cursorY,w_px,height);
    }

    if(pageFilter < 0 || pageFilter == currentPage)
    {
        p->drawImage(r,image);
    }

    if(currentLineHeight < minLineHeight)
        currentLineHeight = minLineHeight;
    if(currentLineHeight < height)
        currentLineHeight = height;

    cursorX += w_px;
}

void HPageTileRenderer::drawImage(QString xpos,QString ypos,QString width,QImage image)
{
    int x = sizeStrToInt(xpos,"x");
    int y = sizeStrToInt(ypos,"y");
    int w_px = sizeStrToInt(width,"x");

    int height = (int)(((double)w_px / (double)image.width()) * (double)image.height());

    QRect r(x,y,w_px,height);

    if(pageFilter < 0 || pageFilter == currentPage)
    {
        p->drawImage(r,image);
    }
}

int  HPageTileRenderer::calcTextHeight(QString width,QString text,HPageTileRenderer_TextType type)
{
    int w_px = sizeStrToInt(width,"x");

    QRect r(cursorX,cursorY,w_px,1000);
    QTextDocument td;
    if(type == HTextType_Plain)
        td.setPlainText(text);
    if(type == HTextType_Html)
        td.setHtml(text);
    if(type == HTextType_Markdown)
        td.setMarkdown(text);
    td.setDefaultFont(defaultFont);
    td.setTextWidth(r.width());
    td.setDefaultTextOption(QTextOption(alignment));

    return td.size().height();
}

void HPageTileRenderer::newLine()
{
    if(currentLineHeight > 0)
    {
        cursorX = 0;
        cursorY += currentLineHeight;
        currentLineHeight = 0;
    }
    else
    {
        QFontMetrics fm(defaultFont);
        cursorY += fm.height();
        cursorX = 0;
        currentLineHeight = 0;
    }
    emit startNewLine();
}

void HPageTileRenderer::newPage()
{
    ++currentPage;
    cursorX = 0;
    cursorY = 0;
    currentLineHeight = 0;
    emit startNewPage();
}

int HPageTileRenderer::currentPageIndex()
{
    return currentPage;
}

void HPageTileRenderer::renderFromInstructions(QString txtintr)
{
    QList<QString> lines = txtintr.split("\n");
    QList<QString>::Iterator li;
    for(li = lines.begin() ; li != lines.end() ; ++li)
    {
        if(li->isEmpty())
            continue;
        QList<QString> parts = li->split("#",Qt::KeepEmptyParts);

        if(parts.at(0) == "movr")
        {
            QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
            moveCursorRelative(pp[0],pp[1]);
        }
        if(parts.at(0) == "mova")
        {
            QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
            moveCursorRelative(pp[0],pp[1]);
        }

        if(parts.at(0) == "newl")
            newLine();
        if(parts.at(0) == "newp")
            newPage();

        if(parts.at(0) == "text")
            addText(parts.at(1),parts.at(2),HTextType_Plain);
        if(parts.at(0) == "html")
            addText(parts.at(1),parts.at(2),HTextType_Html);
        if(parts.at(0) == "mark")
            addText(parts.at(1),parts.at(2),HTextType_Markdown);

        if(parts.at(0) == "fram")
        {
            HBorderFlag b = HBorderFlag_None;
            QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
            if(pp.contains("all"))
                b = b | HBorderFlag_All;
            if(pp.contains("top"))
                b = b | HBorderFlag_Top;
            if(pp.contains("right"))
                b = b | HBorderFlag_Right;
            if(pp.contains("bottom"))
                b = b | HBorderFlag_Bottom;
            if(pp.contains("left"))
                b = b | HBorderFlag_Left;
            if(pp.contains("fill"))
                b = b | HBorderFlag_Fill;
            setBorder(b);
        }
        if(parts.at(0) == "alig")
        {
            if(parts.at(1) == "left")
                setTextAlignment(Qt::AlignLeft);
            if(parts.at(1) == "right")
                setTextAlignment(Qt::AlignRight);
            if(parts.at(1) == "center")
                setTextAlignment(Qt::AlignCenter);
        }
        if(parts.at(0) == "font")
        {
            QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
            setFont(QFont(pp[0],pp[1].toInt()));
        }
    }
}

// ////////////////////////////////////////////////////////////////////////////////// //

HPdfPreviewFrame::HPdfPreviewFrame(QWidget *parent)
 : QFrame(parent)
{
    showPageIndex = 0;
}

HPdfPreviewFrame::~HPdfPreviewFrame()
{

}

void HPdfPreviewFrame::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)
    QPainter pp(this);
    pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi
    HPageTileRenderer renderer(&pp);
    renderer.setPageFilter(showPageIndex);
    renderer.renderFromInstructions(rawContent);
    if(renderer.currentPageIndex() != maxPage)
        maxPage = renderer.currentPageIndex();
}

HPdfPreviewDialog::HPdfPreviewDialog(QWidget *parent,bool generate_button)
 : QDialog(parent)
{
    pdfWriter = NULL;
    attachmentFiles.clear();
    QVBoxLayout *mlay = new QVBoxLayout(this);
    QHBoxLayout *toplay = new QHBoxLayout(0);

    ppf = new HPdfPreviewFrame(this);
    ppf->maxPage = 0;
    if(generate_button)
    {
        QPushButton *generateButton = new QPushButton(tr("Generate Pdf"),this);
        connect(generateButton,SIGNAL(clicked()),this,SLOT(generatePdf()));
        toplay->addWidget(generateButton);
    }

    QPushButton *closeButton = new QPushButton(tr("Close"),this);
    QPushButton *prevpButton = new QPushButton("<",this);
    QPushButton *nextpButton = new QPushButton(">",this);
    pageShow = new QLabel("1",this);

    connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));
    connect(prevpButton,SIGNAL(clicked()),this,SLOT(prevPage()));
    connect(nextpButton,SIGNAL(clicked()),this,SLOT(nextPage()));

    toplay->addStretch();
    toplay->addWidget(prevpButton);
    toplay->addWidget(pageShow);
    toplay->addWidget(nextpButton);
    toplay->addStretch();
    toplay->addWidget(closeButton);
    mlay->addLayout(toplay);
    mlay->addWidget(ppf);
}

void HPdfPreviewDialog::setRawContent(QString c)
{
    ppf->rawContent = c;
}

void HPdfPreviewDialog::addAttachmentFile(QString name,QString content)
{
    attachmentFiles[name] = content;
}

int HPdfPreviewDialog::generatePdfFile(QString filename)
{
    QPdfWriter pw(filename);
    pdfWriter = &pw;

    pw.setResolution(200);
    pw.setPageSize(QPageSize(QPageSize::A4));  /*8.26 x 11.69  -> *200 -> 1652 x 2338*/
    pw.setPageOrientation(QPageLayout::Portrait);

    QMap<QString,QString>::Iterator i;
    for(i = attachmentFiles.begin() ; i != attachmentFiles.end() ; ++i )
        pw.addFileAttachment(i.key(),i.value().toUtf8());

    QPainter pp(pdfWriter);
    pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi (Set elsewhere...)
    HPageTileRenderer renderer(&pp);
    connect(&renderer,SIGNAL(startNewPage()),this,SLOT(startNewPage()));
    renderer.renderFromInstructions(ppf->rawContent);
    pdfWriter = NULL;
    return 0;
}

int HPdfPreviewDialog::generatePdf(void)
{
    QString fn = QFileDialog::getSaveFileName(this,tr("Pdf file to save"),"","*.pdf");
    if(fn.isEmpty())
        return 0;
    return generatePdfFile(fn);
}

int HPdfPreviewDialog::changePage(int p)
{
    ppf->showPageIndex = p;
    pageShow->setText(QString("%1").arg(ppf->showPageIndex + 1));
    update();
    return 0;
}

int HPdfPreviewDialog::nextPage()
{
    ppf->showPageIndex++;
    if(ppf->showPageIndex > ppf->maxPage)
        ppf->showPageIndex = ppf->maxPage;
    pageShow->setText(QString("%1").arg(ppf->showPageIndex + 1));
    update();
    return 0;
}

int HPdfPreviewDialog::prevPage()
{
    ppf->showPageIndex--;
    if(ppf->showPageIndex < 0)
        ppf->showPageIndex = 0;
    pageShow->setText(QString("%1").arg(ppf->showPageIndex + 1));
    update();
    return 0;
}

int HPdfPreviewDialog::startNewPage()
{
    if(pdfWriter != NULL)
        pdfWriter->newPage();
    return 0;
}

HPdfPreviewDialog::~HPdfPreviewDialog()
{

}


//End of gSAFE po.cpp
