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

//End of gSAFE po.cpp
