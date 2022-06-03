/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    po.cpp
*/

#include <QtCore>
#include <QtGui>

#ifndef GSAFE_DISABLE_PRINTERMODULE
#include <QPrinter>
#include <QPrintDialog>
#endif

#include "po.h"
#include "dconsole.h"
#include "guiext.h"

HTextDocument::HTextDocument()
    : QTextDocument()
{
}

HTextDocument::~HTextDocument()
{
}

void HTextDocument::drawColorContents(QPainter *p,const QColor& color, const QRectF &rect)
{
    p->save();
    QAbstractTextDocumentLayout::PaintContext ctx;
    if (rect.isValid()) {
        p->setClipRect(rect);
        ctx.clip = rect;
    }
    ctx.palette.setColor(QPalette::Text, color);
    documentLayout()->draw(p, ctx);
    p->restore();
}

HPageTileRendererPosition::HPageTileRendererPosition()
{
    valid = false;
    page = 0;
    pixel_x = 0;
    pixel_y = 0;
    pixel_w = 0;
    pixel_h = 0;
    percent_x = 0.0;
    percent_y = 0.0;
    percent_w = 0.0;
    percent_h = 0.0;
}

HPageTileRenderer::HPageTileRenderer(QPainter *configuredPainter)
{
    p = configuredPainter;
    cursorX = 0;
    cursorY = 0;
    currentLineHeight = 0;
    minLineHeight = 0;
    currentPage = 0;
    pageFilter = -1; //No filter
    defaultFont = QFont("Arial",18);
    font = QFont("Arial",18);
    fontColor = QColor(0,0,0);
    marginTop    = 20;
    marginRight  = 20;
    marginBottom = 20;
    marginLeft   = 20;

    p->setPen(QPen(QColor(0,0,0),5));
    p->setPen(Qt::SolidLine);
    alignment = Qt::AlignLeft;
    pen = QPen(QColor(255,255,255));
    brush = QBrush(QColor(100,100,100),Qt::SolidPattern);
    border = HBorderFlag_None;
    storePosOfNext = "";
    storedPos.clear();
    instruction_buffer.clear();
    pageboot_buffer.clear();
    inPageBoot = false;
    unknownCommandWarning = false;

    resolutionDpi = 200;
    physicalWidthMillimeter = 210; //Default A4 width in portrait on 200dpi
    physicalHeightMillimeter = 297; //Default A4 height in portrait on 200dpi
}

HPageTileRenderer::~HPageTileRenderer()
{
}

void HPageTileRenderer::setUnknownCommandWarning(bool showWarnings)
{
    unknownCommandWarning = showWarnings;
}

void HPageTileRenderer::storePositionOfNextAddElement(QString withName)
{
    storePosOfNext = withName;
}

HPageTileRendererPosition HPageTileRenderer::storedPosition(QString withName)
{
    if(storedPos.contains(withName))
        return storedPos[withName];
    return HPageTileRendererPosition();
}

QMap<QString,HPageTileRendererPosition> HPageTileRenderer::storedPositions()
{
    return storedPos;
}

void HPageTileRenderer::setPageFilter(int pf)
{
    pageFilter = pf;
}

void HPageTileRenderer::setMinimumLineHeight(int mlh)
{
    minLineHeight = mlh;
}

void HPageTileRenderer::setMinimumLineHeight(QString mlh)
{
    minLineHeight = sizeStrToInt(mlh,"y");
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
    font = f;
}

void HPageTileRenderer::setDefaultFont(QFont f)
{
    defaultFont = f;
    font = f;
}

void HPageTileRenderer::resetToDefaultFont()
{
    font = defaultFont;
}

int HPageTileRenderer::millimeterToPixel(int mm)
{
     return (int)((double)(mm  * resolutionDpi) / 25.4);
}

int HPageTileRenderer::areaWidth()
{
    if(!areastack.isEmpty())
        return areastack.first().sizeW - (marginLeft + marginRight);
    return p->window().width() - (marginLeft + marginRight);
}

int HPageTileRenderer::areaHeight()
{
    if(!areastack.isEmpty())
        return areastack.first().sizeH - (marginTop + marginBottom);
    return p->window().height() - (marginTop + marginBottom);
}

int HPageTileRenderer::sizeStrToInt(QString str,QString xy)
{
    if(str.isEmpty())
        return 0;
    int iv = 0;
    bool reverse = false;
    bool relative_to_pos = false;

    if(str.startsWith(">"))
    {
        relative_to_pos = true;
        str.remove(0,1);
    }
    if(str.startsWith("-"))
    {
        reverse = true;
        str.remove(0,1);
    }

    str = str.trimmed();

    if(!str[0].isDigit())
        return 0;

    if(str.endsWith("px"))
        str = str.mid(0,str.length() - 2);

    if(!str.endsWith("%") && !str.endsWith("em") && !str.endsWith("mm") && !str.endsWith("cm"))
        iv = str.toInt();

    if(str.endsWith("%"))
    {
        double percent = str.mid(0,str.length()-1).toDouble();
        if(percent > 100.0)
            percent = 100.0;
        if(xy == "x")
            iv = (areaWidth() * percent) / 100.0;
        if(xy == "y")
            iv = (areaHeight() * percent) / 100.0;
    }

    if(str.endsWith("em"))
    {
        double mul = str.mid(0,str.length()-2).toDouble();
        QFontMetrics fm(font);
        if(xy == "x")
            iv = fm.averageCharWidth() * mul;
        if(xy == "y")
            iv = fm.height() * mul;
    }

    if(str.endsWith("cm"))
    {
        double mul = str.mid(0,str.length()-2).toDouble();
        iv = millimeterToPixel(mul * 10);
    }

    if(str.endsWith("mm"))
    {
        int mul = str.mid(0,str.length()-2).toInt();
        iv = millimeterToPixel(mul);
    }

    if(reverse)
    {
        if(xy == "x")
            iv = areaWidth() - iv;
        if(xy == "y")
            iv = areaHeight() - iv;
    }

    if(relative_to_pos)
    {
        if(xy == "x" && iv > cursorX)
            iv = iv - cursorX;
        if(xy == "y" && iv > cursorY)
            iv = iv - cursorY;
    }

    return iv;
}

void HPageTileRenderer::setMargins(QString top,QString right,QString bottom,QString left)
{
    marginTop    = sizeStrToInt(top,"y");
    marginRight  = sizeStrToInt(right,"x");
    marginBottom = sizeStrToInt(bottom,"y");
    marginLeft   = sizeStrToInt(left,"x");
}

void HPageTileRenderer::storePos(int w,int h)
{
    HPageTileRendererPosition sp;
    sp.valid = true;
    sp.page = currentPage + 1;
    sp.pixel_x = marginLeft + cursorX;
    sp.pixel_y = marginRight + cursorY;
    sp.pixel_w = w;
    sp.pixel_h = h;
    sp.percent_x = ((double)sp.pixel_x * 100.0) / p->window().width();
    sp.percent_y = ((double)sp.pixel_y * 100.0) / p->window().height();
    sp.percent_w = ((double)sp.pixel_w * 100.0) / p->window().width();
    sp.percent_h = ((double)sp.pixel_h * 100.0) / p->window().height();

    storedPos[storePosOfNext] = sp;
    storePosOfNext = "";
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

QString HPageTileRenderer::strSubstTokens(QString in)
{
    int ps;
    if((ps = in.indexOf("@@{",0)) == -1)
        return in;
    int pe;
    if((pe = in.indexOf("}@@",ps)) == -1)
        return in;

    QString token;
    token = in.mid(ps+3,pe-ps-3);
    if(token == "pagenum")
        in.replace(ps,pe-ps+3,QString("%1").arg(currentPage + 1));
    if(token == "renderdate")
        in.replace(ps,pe-ps+3,QString("%1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd")));
    if(token == "rendertime")
        in.replace(ps,pe-ps+3,QString("%1").arg(QDateTime::currentDateTime().toString("hh:mm:ss")));

    return strSubstTokens(in);
}

void HPageTileRenderer::addText(QString width,QString text,HPageTileRenderer_TextType type)
{
    int w_px = sizeStrToInt(width,"x");

    if(cursorX + w_px > areaWidth())
    {
        newLine();
    }

    QRect r(marginLeft + cursorX,marginTop + cursorY,w_px,1000);
    HTextDocument td;
    if(type == HTextType_Plain)
        td.setPlainText(strSubstTokens(text));
    if(type == HTextType_Html)
        td.setHtml(strSubstTokens(text));
    if(type == HTextType_Markdown)
        td.setMarkdown(strSubstTokens(text));
    td.setDefaultFont(font);
    td.setTextWidth(r.width());

    td.setDefaultTextOption(QTextOption(alignment));

    int h_eff = td.size().height();
    if(minLineHeight > h_eff)
        h_eff = minLineHeight;

    if(h_eff + cursorY > areaHeight())
    {
        newPage();
        r = QRect(marginLeft + cursorX,marginTop + cursorY,w_px,1000);
    }

    if(pageFilter == -1 || pageFilter == currentPage)
    {
        p->save();
        p->translate(r.topLeft());
        if(flagOn(border,HBorderFlag_Fill))
            p->fillRect(0,0,td.size().width(),h_eff,brush);
        td.drawColorContents(p,fontColor);
        drawBorders(td.size().width(),h_eff);
        p->restore();
    }

    if(!storePosOfNext.isEmpty())
        storePos(td.size().width(),h_eff);

    if(currentLineHeight < minLineHeight)
        currentLineHeight = minLineHeight;
    if(currentLineHeight < h_eff)
        currentLineHeight = h_eff;

    cursorX += td.size().width();
}

void HPageTileRenderer::drawText(QString xpos,QString ypos,QString width,QString text,HPageTileRenderer_TextType type)
{
    int x = sizeStrToInt(xpos,"x");
    int y = sizeStrToInt(ypos,"y");
    int w_px = sizeStrToInt(width,"x");
    QRect r(marginLeft + x,marginTop + y,w_px,1000);
    HTextDocument td;
    if(type == HTextType_Plain)
        td.setPlainText(strSubstTokens(text));
    if(type == HTextType_Html)
        td.setHtml(strSubstTokens(text));
    if(type == HTextType_Markdown)
        td.setMarkdown(strSubstTokens(text));
    td.setDefaultFont(font);
    td.setTextWidth(r.width());
    td.setDefaultTextOption(QTextOption(alignment));

    if(pageFilter == -1 || pageFilter == currentPage)
    {
        p->save();
        p->translate(r.topLeft());
        if(flagOn(border,HBorderFlag_Fill))
            p->fillRect(0,0,td.size().width(),td.size().height(),brush);
        td.drawColorContents(p,fontColor);
        drawBorders(td.size().width(),td.size().height());
        p->restore();
    }
}

void HPageTileRenderer::addRect(QString width,QString height)
{
    int w_px = sizeStrToInt(width,"x");
    int h_px = sizeStrToInt(height,"y");
    int h_eff = h_px;

    if(minLineHeight > h_px)
        h_eff = minLineHeight;

    if(cursorX + w_px > areaWidth())
        newLine();

    QRect r(marginLeft + cursorX,marginTop + cursorY,w_px,h_eff);

    if(h_eff + cursorY > areaHeight())
    {
        newPage();
        r = QRect(marginLeft + cursorX,marginTop + cursorY,w_px,h_eff);
    }

    if(pageFilter == -1 || pageFilter == currentPage)
    {
        p->save();
        p->translate(r.topLeft());
        if(flagOn(border,HBorderFlag_Fill))
            p->fillRect(0,0,w_px,h_eff,brush);
        drawBorders(w_px,h_eff);
        p->restore();
    }

    if(!storePosOfNext.isEmpty())
        storePos(w_px,h_eff);

    if(currentLineHeight < minLineHeight)
        currentLineHeight = minLineHeight;
    if(currentLineHeight < h_eff)
        currentLineHeight = h_eff;

    cursorX += w_px;
}

void HPageTileRenderer::addSpace(QString width,QString height)
{
    int w_px = sizeStrToInt(width,"x");
    int h_px = sizeStrToInt(height,"y");
    int h_eff = h_px;

    if(minLineHeight > h_px)
        h_eff = minLineHeight;

    if(cursorX + w_px > areaWidth())
        newLine();

    QRect r(marginLeft + cursorX,marginTop + cursorY,w_px,h_eff);

    if(h_eff + cursorY > areaHeight())
    {
        newPage();
        r = QRect(marginLeft + cursorX,marginTop + cursorY,w_px,h_eff);
    }

    if(currentLineHeight < minLineHeight)
        currentLineHeight = minLineHeight;
    if(currentLineHeight < h_eff)
        currentLineHeight = h_eff;

    cursorX += w_px;
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

    if(cursorX + w_px > areaWidth())
        newLine();

    int height = (int)(((double)w_px / (double)image.width()) * (double)image.height());

    QRect r(marginLeft + cursorX,marginTop + cursorY,w_px,height);

    if(height + cursorY > areaHeight())
    {
        newPage();
        r = QRect(marginLeft + cursorX,marginTop + cursorY,w_px,height);
    }

    if(pageFilter == -1 || pageFilter == currentPage)
    {
        p->drawImage(r,image);
    }

    if(!storePosOfNext.isEmpty())
        storePos(w_px, height);

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

    QRect r(marginLeft + x,marginTop + y,w_px,height);

    if(pageFilter == -1 || pageFilter == currentPage)
    {
        p->drawImage(r,image);
    }
}

int HPageTileRenderer::calcImageHeight(QString width,QImage image)
{
    int w_px = sizeStrToInt(width,"x");
    return (int)(((double)w_px / (double)image.width()) * (double)image.height());
}

void HPageTileRenderer::drawRect(QString xpos,QString ypos,QString width,QString height)
{
    int x = sizeStrToInt(xpos,"x");
    int y = sizeStrToInt(ypos,"y");
    int w_px = sizeStrToInt(width,"x");
    int h_px = sizeStrToInt(height,"y");

    QRect r(marginLeft + x,marginTop + y,w_px,h_px);
    if(pageFilter == -1 || pageFilter == currentPage)
    {
        p->save();
        p->translate(r.topLeft());
        if(flagOn(border,HBorderFlag_Fill))
            p->fillRect(0,0,w_px,h_px,brush);
        drawBorders(w_px,h_px);
        p->restore();
    }
}

int  HPageTileRenderer::calcTextHeight(QString width,QString text,HPageTileRenderer_TextType type)
{
    int w_px = sizeStrToInt(width,"x");

    QRect r(0,0,w_px,1000);
    QTextDocument td;
    if(type == HTextType_Plain)
        td.setPlainText(strSubstTokens(text));
    if(type == HTextType_Html)
        td.setHtml(strSubstTokens(text));
    if(type == HTextType_Markdown)
        td.setMarkdown(strSubstTokens(text));
    td.setDefaultFont(font);
    td.setTextWidth(r.width());
    td.setDefaultTextOption(QTextOption(alignment));

    return td.size().height();
}

void HPageTileRenderer::incrementMinLineHeightToTextHeight(QString width,QString text,HPageTileRenderer_TextType type)
{
    int ch = calcTextHeight(width,text,type);
    if(minLineHeight < ch)
        minLineHeight = ch;
}

void HPageTileRenderer::incrementMinLineHeightToImageHeight(QString width,QImage image)
{
    int ch = calcImageHeight(width,image);
    if(minLineHeight < ch)
        minLineHeight = ch;
}

void HPageTileRenderer::incrementMinLineHeightToValue(QString height)
{
    int h = sizeStrToInt(height,"y");
    if(minLineHeight < h)
        minLineHeight = h;
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
        QFontMetrics fm(font);
        cursorY += fm.height();
        cursorX = 0;
        currentLineHeight = 0;
    }
    emit startNewLine();
}

void HPageTileRenderer::newPage()
{
    if(!areastack.isEmpty())
    {
        pageFilter = -2; //Complete disable drawing
        return;
    }
    ++currentPage;
    cursorX = 0;
    cursorY = 0;
    currentLineHeight = 0;
    emit startNewPage();
    playPageBoot();
}

int HPageTileRenderer::currentPageIndex()
{
    return currentPage;
}

void HPageTileRenderer::setFontColor(QColor c)
{
    fontColor = c;
}

QColor HPageTileRenderer::getFontColor()
{
    return fontColor;
}

void HPageTileRenderer::enterArea(QString width,QString height)
{
    int w_px = sizeStrToInt(width,"x");
    int h_px = sizeStrToInt(height,"y");

    if(cursorX + w_px > areaWidth())
        newLine();

    QRect r(marginLeft + cursorX,marginTop + cursorY,w_px,h_px);

    if(h_px + cursorY > areaHeight())
    {
        newPage();
        r = QRect(marginLeft + cursorX,marginTop + cursorY,w_px,h_px);
    }

    if(currentLineHeight < minLineHeight)
        currentLineHeight = minLineHeight;
    if(currentLineHeight < h_px)
        currentLineHeight = h_px;

    cursorX += w_px;

    AreaData area;
    area.sizeW = w_px;
    area.sizeH = h_px;
    area.cursorX = cursorX;
    area.cursorY = cursorY;
    area.currentLineHeight = currentLineHeight;
    area.currentPage = currentPage;
    area.pageFilter = pageFilter;
    area.margint = marginTop;
    area.marginr = marginRight;
    area.marginb = marginBottom;
    area.marginl = marginLeft;

    areastack.push_front(area);
    p->save();
    p->translate(r.topLeft());

    marginTop    = 0;
    marginRight  = 0;
    marginBottom = 0;
    marginLeft   = 0;
    cursorX = 0;
    cursorY = 0;
    currentLineHeight = 0;
}

void HPageTileRenderer::enterArea(QString xpos,QString ypos,QString width,QString height)
{
    int x = sizeStrToInt(xpos,"x");
    int y = sizeStrToInt(ypos,"y");
    int w_px = sizeStrToInt(width,"x");
    int h_px = sizeStrToInt(height,"y");

    QRect r(marginLeft + x, marginTop + y,w_px,h_px);

    AreaData area;
    area.sizeW = w_px;
    area.sizeH = h_px;
    area.cursorX = cursorX;
    area.cursorY = cursorY;
    area.currentLineHeight = currentLineHeight;
    area.currentPage = currentPage;
    area.pageFilter = pageFilter;
    area.margint = marginTop;
    area.marginr = marginRight;
    area.marginb = marginBottom;
    area.marginl = marginLeft;

    areastack.push_front(area);
    p->save();
    p->translate(r.topLeft());

    marginTop    = 0;
    marginRight  = 0;
    marginBottom = 0;
    marginLeft   = 0;
    cursorX = 0;
    cursorY = 0;
    currentLineHeight = 0;
}

void HPageTileRenderer::returnArea()
{
    if(areastack.isEmpty())
        return;
    cursorX = areastack.first().cursorX;
    cursorY = areastack.first().cursorY;

    marginTop    = areastack.first().margint;
    marginRight  = areastack.first().marginr;
    marginBottom = areastack.first().marginb;
    marginLeft   = areastack.first().marginl;

    currentLineHeight = areastack.first().currentLineHeight;
    currentPage = areastack.first().currentPage;
    pageFilter = areastack.first().pageFilter;
    areastack.pop_front();
    p->restore();
}

void HPageTileRenderer::renderFromInstructions(QString txtintr)
{
    QString concatenated = "";
    QList<QString> lines = txtintr.split("\n");
    QList<QString>::Iterator li;
    for(li = lines.begin() ; li != lines.end() ; ++li)
    {
        if(li->isEmpty())
            continue;

        if(!concatenated.isEmpty())
        {
            if(li->trimmed() == "}")
            {
                QList<QString> parts = concatenated.trimmed().split("#",Qt::KeepEmptyParts);
                renderFromInstructionLineHL(parts);
                concatenated = "";
                continue;
            }
            concatenated.append(li->trimmed());
            concatenated.append(" ");
            continue;
        }
        if(li->trimmed().endsWith("#{"))
        {
            concatenated = li->trimmed();
            concatenated.chop(1);
            continue;
        }
        QList<QString> parts = li->split("#",Qt::KeepEmptyParts);
        renderFromInstructionLineHL(parts);
    }
}

int HPageTileRenderer::playPageBoot()
{
    QList<QStringList>::Iterator i;
    for( i = pageboot_buffer.begin() ; i != pageboot_buffer.end() ; ++i )
        renderFromInstructionLineLL(*i);
    return 0;
}

void HPageTileRenderer::renderFromInstructionLineHL(const QStringList& parts)
{
    QString cmd = parts.at(0).trimmed();

    if(cmd == "EVERYPAGE_END")
    {
        inPageBoot = false;
        return;
    }
    if(inPageBoot)
        pageboot_buffer.push_back(parts);
    if(cmd == "EVERYPAGE_START")
    {
        inPageBoot = true;
        return;
    }

    if(cmd == "fixh")
    {
        instruction_buffer.push_back(parts);
        setMinimumLineHeight(0);
        fs_font = font;
        fs_defaultFont = defaultFont;
        fs_alignment = alignment;
        return;
    }

    if(!instruction_buffer.isEmpty())
    {
        if(cmd == "newl" || cmd == "newp")
        {
            instruction_buffer.push_back(QString("smhz").split("#",Qt::KeepEmptyParts));
            instruction_buffer.push_back(parts);

            font = fs_font;
            defaultFont = fs_defaultFont;
            alignment = fs_alignment;

            while(!instruction_buffer.isEmpty())
            {
                renderFromInstructionLineLL(instruction_buffer.first());
                instruction_buffer.pop_front();
            }
            return;
        }

        if(cmd == "text" && parts.count() > 2)
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
                incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Plain);
        if(cmd == "html" && parts.count() > 2)
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
                incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Html);
        if(cmd == "mark" && parts.count() > 2)
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
                incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Markdown);

        if(cmd == "imgr" && parts.count() > 2)
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
            {
                QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
                if(pp.count() < 3)
                    incrementMinLineHeightToImageHeight(parts.at(1),QImage(parts.at(2)));
            }
        if(cmd == "imgb" && parts.count() > 2)
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
            {
                QImage img = QImage::fromData(QByteArray::fromBase64(parts.at(2).toLocal8Bit()));
                QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
                if(pp.count() < 3)
                    incrementMinLineHeightToImageHeight(parts.at(1),img);
            }

        if(cmd == "rect" || cmd == "spac")
        {
            QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
            if(pp.count() == 2)
                incrementMinLineHeightToValue(pp[1]);
        }

        if(cmd == "alig" || cmd == "setf" || cmd == "setd" || cmd == "deff")
            renderFromInstructionLineLL(parts);

        instruction_buffer.push_back(parts);
        return;
    }

    renderFromInstructionLineLL(parts);
}

void HPageTileRenderer::renderFromInstructionLineLL(const QStringList& parts)
{
    QString cmd = parts.at(0).trimmed();

    //Commands without arguments
    if(cmd == "fixh")
    {
        return;
    }
    if(cmd == "newl")
    {
        newLine();
        return;
    }
    if(cmd == "newp")
    {
        newPage();
        return;
    }
    if(cmd == "smhz")
    {
        setMinimumLineHeight(0);
        return;
    }
    if(cmd == "deff")
    {
        resetToDefaultFont();
        return;
    }
    if(cmd == "reta")
    {
        returnArea();
        return;
    }

    if(parts.count() < 2)
    {
        if(unknownCommandWarning)
            sdebug(QString("HPageTileRenderer warning, unknown or not complete command: %1").arg(parts.join("#")));
        return;
    }

    //Commands with one arguments
    if(cmd == "alig")
    {
        if(parts.at(1) == "left")
            setTextAlignment(Qt::AlignLeft);
        if(parts.at(1) == "right")
            setTextAlignment(Qt::AlignRight);
        if(parts.at(1) == "center")
            setTextAlignment(Qt::AlignCenter);
        if(parts.at(1) == "just")
            setTextAlignment(Qt::AlignJustify);
        return;
    }
    if(cmd == "smhv")
    {
        setMinimumLineHeight(parts.at(1));
        return;
    }
    if(cmd == "colf")
    {
        setFontColor(html6HexColor(parts.at(1)));
        return;
    }
    if(cmd == "coll")
    {
        setPen(QPen(html6HexColor(parts.at(1))));
        return;
    }
    if(cmd == "colb")
    {
        setBrush(QBrush(html6HexColor(parts.at(1))));
        return;
    }
    if(cmd == "getp")
    {
        storePositionOfNextAddElement(parts.at(1));
        return;
    }

    QStringList fpp = parts.at(1).split(",",Qt::KeepEmptyParts);

    //Commands with (only) one splitted arguments
    if(cmd == "movr" && fpp.count() >= 2)
    {
        moveCursorRelative(fpp[0],fpp[1]);
        return;
    }
    if(cmd == "mova" && fpp.count() >= 2)
    {
        moveCursorAbsolute(fpp[0],fpp[1]);
        return;
    }
    if(cmd == "setf" && fpp.count() >= 2)
    {
        setFont(QFont(fpp[0],fpp[1].toInt()));
        return;
    }
    if(cmd == "setd" && fpp.count() >= 2)
    {
        setDefaultFont(QFont(fpp[0],fpp[1].toInt()));
        return;
    }
    if(cmd == "spac")
    {
        if(fpp.count() == 1)
            addSpace(fpp[0],"1");
        if(fpp.count() == 2)
            addSpace(fpp[0],fpp[1]);
        return;
    }
    if(cmd == "rect")
    {
        if(fpp.count() == 2)
            addRect(fpp[0],fpp[1]);
        if(fpp.count() == 4)
            drawRect(fpp[0],fpp[1],fpp[2],fpp[3]);
        return;
    }
    if(cmd == "fram")
    {
        HBorderFlag b = HBorderFlag_None;
        if(fpp.contains("all"))
            b = b | HBorderFlag_All;
        if(fpp.contains("top"))
            b = b | HBorderFlag_Top;
        if(fpp.contains("right"))
            b = b | HBorderFlag_Right;
        if(fpp.contains("bottom"))
            b = b | HBorderFlag_Bottom;
        if(fpp.contains("left"))
            b = b | HBorderFlag_Left;
        if(fpp.contains("fill"))
            b = b | HBorderFlag_Fill;
        setBorder(b);
        return;
    }
    if(cmd == "area")
    {
        if(fpp.count() == 2)
            enterArea(fpp[0],fpp[1]);
        if(fpp.count() == 4)
            enterArea(fpp[0],fpp[1],fpp[2],fpp[3]);
        return;
    }
    if(cmd == "marg")
    {
        if(fpp.count() == 4)
            setMargins(fpp[0],fpp[1],fpp[2],fpp[3]);
        return;
    }

    if(parts.count() < 3)
    {
        if(unknownCommandWarning)
            sdebug(QString("HPageTileRenderer warning, unknown or not complete command: %1").arg(parts.join("#")));
        return;
    }

    //Commands with two arguments
    if(cmd == "text")
    {
        if(fpp.count() > 2)
            drawText(fpp[0],fpp[1],fpp[2],parts.at(2),HTextType_Plain);
        else
            addText(parts.at(1),parts.at(2),HTextType_Plain);
        return;
    }
    if(cmd == "html")
    {
        if(fpp.count() > 2)
            drawText(fpp[0],fpp[1],fpp[2],parts.at(2),HTextType_Html);
        else
            addText(parts.at(1),parts.at(2),HTextType_Html);
        return;
    }
    if(cmd == "mark")
    {
        if(fpp.count() > 2)
            drawText(fpp[0],fpp[1],fpp[2],parts.at(2),HTextType_Markdown);
        else
            addText(parts.at(1),parts.at(2),HTextType_Markdown);
        return;
    }
    if(cmd == "imgr")
    {
        if(fpp.count() > 2)
            drawImage(fpp[0],fpp[1],fpp[2],QImage(parts.at(2)));
        else
            addImage(parts.at(1),QImage(parts.at(2)));
        return;
    }
    if(cmd == "imgb")
    {
        QImage img = QImage::fromData(QByteArray::fromBase64(parts.at(2).toLocal8Bit()));
        if(fpp.count() > 2)
            drawImage(fpp[0],fpp[1],fpp[2],img);
        else
            addImage(parts.at(1),img);
        return;
    }
    if(cmd == "smhr")
    {
        if(fpp.count() < 3)
            incrementMinLineHeightToImageHeight(parts.at(1),QImage(parts.at(2)));
        return;
    }
    if(cmd == "smhi")
    {
        QImage img = QImage::fromData(QByteArray::fromBase64(parts.at(2).toLocal8Bit()));
        if(fpp.count() < 3)
            incrementMinLineHeightToImageHeight(parts.at(1),img);
        return;
    }
    if(cmd == "smht")
    {
        incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Plain);
        return;
    }
    if(cmd == "smhh")
    {
        incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Html);
        return;
    }
    if(cmd == "smhm")
    {
        incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Markdown);
        return;
    }

    if(unknownCommandWarning)
        sdebug(QString("HPageTileRenderer warning, unknown command: %1").arg(parts.join("#")));
}

// ////////////////////////////////////////////////////////////////////////////////// //
HTextProcessor::HTextProcessor()
{
    clearValueMaps();
    definingFunctionName = "";
    definingFunctionBody = "";
    functions.clear();
}

HTextProcessor::~HTextProcessor()
{
}

void HTextProcessor::addValueMap(QString name,const QMap<QString,QString>& m)
{
    smaps[name] = m;
}

void HTextProcessor::addValueMapPtr(QString name,QMap<QString,QString>* m)
{
    dmaps[name] = m;
}

void HTextProcessor::clearValueMaps()
{
    smaps.clear();
    dmaps.clear();
}

QString HTextProcessor::processDoc(QString in)
{
    QString out = "";
    QList<QString> lines = in.split("\n");
    QList<QString>::Iterator li;
    QList<bool> conds;
    conds.clear();
    for(li = lines.begin() ; li != lines.end() ; ++li)
    {
        QList<QString> parts = li->split("#",Qt::KeepEmptyParts);
        QString cmd = parts.at(0).trimmed();

        if(cmd.startsWith("//"))
            continue;

        if(cmd == "ENDF")
        {
            functions[definingFunctionName] = definingFunctionBody;
            definingFunctionName = "";
            definingFunctionBody = "";
            continue;
        }

        if(!definingFunctionName.isEmpty())
        {
            definingFunctionBody.append(*li);
            definingFunctionBody.append("\n");
            continue;
        }

        if(cmd == "COND" && parts.count() > 3)
        {
            if(!conds.empty() && !conds.first())
            {
                conds.push_front(false);
                continue;
            }

            if(parts.count() < 4) //Badly formed condition
            {
                conds.push_front(false);
                continue;
            }

            if(parts.at(2) == "=")
            {
                if( processLine(parts.at(1).trimmed()).trimmed() == processLine(parts.at(3).trimmed()).trimmed() )
                    conds.push_front(true);
                else
                    conds.push_front(false);
                continue;
            }
            if(parts.at(2) == "!=")
            {
                if( processLine(parts.at(1).trimmed()).trimmed() == processLine(parts.at(3).trimmed()).trimmed() )
                    conds.push_front(false);
                else
                    conds.push_front(true);
                continue;
            }

            bool ok1,ok3;
            double p1dv = processLine(parts.at(1)).toDouble(&ok1);
            double p3dv =  processLine(parts.at(3)).toDouble(&ok3);

            if(!ok1 || !ok3)
            {
                conds.push_front(false);
                continue;
            }

            if(parts.at(2) == ">")
            {
                if(p1dv > p3dv)
                    conds.push_front(true);
                else
                    conds.push_front(false);
                continue;
            }
            if(parts.at(2) == "<")
            {
                if(p1dv < p3dv)
                    conds.push_front(true);
                else
                    conds.push_front(false);
                continue;
            }
            if(parts.at(2) == ">=")
            {
                if(p1dv >= p3dv)
                    conds.push_front(true);
                else
                    conds.push_front(false);
                continue;
            }
            if(parts.at(2) == "<=")
            {
                if(p1dv <= p3dv)
                    conds.push_front(true);
                else
                    conds.push_front(false);
                continue;
            }
            continue;
        }

        if(cmd == "ENDC")
        {
            conds.pop_front();
            continue;
        }

        if(cmd == "FUNC" && parts.count() > 1)
        {
            definingFunctionName = parts.at(1).trimmed();
            definingFunctionBody = "";
            continue;
        }

        if(cmd == "CALL" && parts.count() > 1)
        {
            QString fname = parts.at(1).trimmed();
            if(functions.contains(fname))
            {
                out.append(processDoc(functions[fname]));
                out.append("\n");
            }
            continue;
        }

        if(conds.empty() || conds.first())
        {
            out.append(processLine(*li));
            out.append("\n");
        }
    }
    return out;
}

QString HTextProcessor::processLine(QString in)
{
    int startsearch,ps,pe;
    startsearch = 0;
    while((ps = in.indexOf("{{",startsearch)) != -1)
    {
        pe = in.indexOf("}}",ps);
        if(pe > 0)
        {
            QString sam = in.mid(ps+2,pe-ps-2);
            QString to = "";

            if(sam.indexOf("|",0) > -1)
            {
                QStringList sams = sam.split("|",Qt::KeepEmptyParts);
                QList<QString>::iterator si = sams.begin();
                while(si != sams.end())
                {
                    to = processToken(*si);
                    if(!to.isEmpty())
                        break;
                    ++si;
                }
            }
            else if(sam.indexOf("?",0) > -1 && sam.indexOf(":",0) > -1)
            {
                QStringList t,cp,rp;
                t = sam.split("?",Qt::KeepEmptyParts);
                if(t.count() == 2)
                {
                    cp = t[0].split("=",Qt::KeepEmptyParts);
                    rp = t[1].split(":",Qt::KeepEmptyParts);
                    if(cp.count() == 2 && rp.count() == 2)
                    {
                        if(processToken(cp[0]) == processToken(cp[1]))
                            to = processLine(rp[0].replace("[[","{{").replace("]]","}}"));
                        else
                            to = processLine(rp[1].replace("[[","{{").replace("]]","}}"));
                    }
                }
            }
            else
            {
                to = processToken(sam);
            }

            in.replace(ps,pe-ps+2,to);
            startsearch = ps;
        }
        else
        {
            in.replace(ps,2,"");
            break;
        }
    }
    return in;
}

QString HTextProcessor::processToken(QString in)
{
    if(in.startsWith(".."))
    {
        if(in.mid(2) == "point")         return ".";
        if(in.mid(2) == "colon")         return ":";
        if(in.mid(2) == "semicolon")     return ";";
        if(in.mid(2) == "question")      return "?";
        if(in.mid(2) == "openbrackets")  return "{";
        if(in.mid(2) == "closebrackets") return "}";
        return "";
    }
    if(in.startsWith("."))
    {
        QStringList tp = in.mid(1).split(".",Qt::SkipEmptyParts);
        if(tp.count() == 2)
        {
            if(smaps.contains(tp[0]) && smaps[tp[0]].contains(tp[1]))
                return smaps[tp[0]].value(tp[1]);
            if(dmaps.contains(tp[0]) && dmaps[tp[0]] != NULL && dmaps[tp[0]]->contains(tp[1]))
                return dmaps[tp[0]]->value(tp[1]);
        }
        return "";
    }
    return in;
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
    QPainter pp(this);
    pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi
    HPageTileRenderer renderer(&pp);
    renderer.setPageFilter(showPageIndex);
    renderer.renderFromInstructions(rawContent);
    if(renderer.currentPageIndex() != maxPage)
        maxPage = renderer.currentPageIndex();
    QFrame::paintEvent(e);
}

HPdfPreviewDialog::HPdfPreviewDialog(QWidget *parent,QString buttons)
 : QDialog(parent)
{
    pdfWriter = NULL;
    rawEditor = NULL;
    enable_render_warnings = false;
    attachmentFiles.clear();
    lastRenderStoredPositions.clear();
    main_horizontal_layout = new QHBoxLayout(this);
    main_vertical_layout = new QVBoxLayout(0);
    main_horizontal_layout->addLayout(main_vertical_layout);
    toplay = new QHBoxLayout(0);
    QStringList btns = buttons.split(",",Qt::SkipEmptyParts); // "print,generate"

    ppf = new HPdfPreviewFrame(this);

    ppf->maxPage = 0;
    if(btns.contains("generate"))
    {
        QPushButton *generateButton = new QPushButton(tr("Generate Pdf"),this);
        connect(generateButton,SIGNAL(clicked()),this,SLOT(generatePdf()));
        toplay->addWidget(generateButton);
    }

#ifndef GSAFE_DISABLE_PRINTERMODULE
    printer = NULL;
    if(btns.contains("print"))
    {
        QPushButton *printButton = new QPushButton(tr("Print"),this);
        connect(printButton,SIGNAL(clicked()),this,SLOT(print()));
        toplay->addWidget(printButton);
    }
#endif

    if(btns.contains("editor"))
    {
        rawEditor = new QTextEdit(this);
        connect(rawEditor,SIGNAL(textChanged()),this,SLOT(editorTextChanged()));
        main_horizontal_layout->addWidget(rawEditor);
    }

    QPushButton *closeButton = new QPushButton(tr("Close"),this);
    QPushButton *prevpButton = new QPushButton("<",this);
    QPushButton *nextpButton = new QPushButton(">",this);
    pageShow = new QLabel("",this);

    connect(closeButton,SIGNAL(clicked()),this,SLOT(close()));

    connect(prevpButton,SIGNAL(clicked()),this,SLOT(prevPage()));
    connect(nextpButton,SIGNAL(clicked()),this,SLOT(nextPage()));

    toplay->addStretch();
    toplay->addWidget(prevpButton);
    toplay->addWidget(pageShow);
    toplay->addWidget(nextpButton);
    toplay->addStretch();
    toplay->addWidget(closeButton);
    main_vertical_layout->addLayout(toplay);
    main_vertical_layout->addWidget(ppf);

    pageShow->setText(QString("%1").arg(ppf->showPageIndex + 1));
}

void HPdfPreviewDialog::setRawContent(QString c)
{
    ppf->rawContent = c;
    if(rawEditor != NULL)
        rawEditor->setPlainText(c);
    update();
}

int HPdfPreviewDialog::editorTextChanged()
{
    ppf->rawContent = rawEditor->toPlainText();
    update();
    return 0;
}

void HPdfPreviewDialog::addAttachmentFile(QString name,QString content)
{
    attachmentFiles[name] = content;
}

int HPdfPreviewDialog::generatePdfFile(QString filename)
{
    int dpi = 200;
    QPdfWriter pw(filename);
    pdfWriter = &pw;

    pw.setResolution(dpi);
    pw.setPageSize(QPageSize(QPageSize::A4));  /*8.26 x 11.69  -> *200 -> 1652 x 2338*/
    pw.setPageOrientation(QPageLayout::Portrait);

    QMap<QString,QString>::Iterator i;
    for(i = attachmentFiles.begin() ; i != attachmentFiles.end() ; ++i )
        pw.addFileAttachment(i.key(),i.value().toUtf8());

    QPainter pp(pdfWriter);
    pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi (Set elsewhere...)
    HPageTileRenderer renderer(&pp);
    renderer.resolutionDpi = dpi;
    if(enable_render_warnings)
        renderer.setUnknownCommandWarning(true);
    connect(&renderer,SIGNAL(startNewPage()),this,SLOT(startNewPage()));
    renderer.renderFromInstructions(ppf->rawContent);
    lastRenderStoredPositions = renderer.storedPositions();
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

int HPdfPreviewDialog::print()
{
#ifndef GSAFE_DISABLE_PRINTERMODULE
    int dpi = 200;
    printer = new QPrinter();
    printer->setResolution(dpi);
    printer->setPageSize(QPageSize(QPageSize::A4));  /*8.26 x 11.69  -> *200 -> 1652 x 2338*/
    printer->setPageOrientation(QPageLayout::Portrait);

    QPrintDialog *pd = new QPrintDialog(printer,this);
    if(pd->exec())
    {
        QPainter pp(printer);
        pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi (Set elsewhere...)
        HPageTileRenderer *renderer = new HPageTileRenderer(&pp);
        renderer->resolutionDpi = dpi;
        if(enable_render_warnings)
            renderer->setUnknownCommandWarning(true);
        connect(renderer,SIGNAL(startNewPage()),this,SLOT(startNewPage()));
        renderer->renderFromInstructions(ppf->rawContent);
        delete renderer;
    }
    delete pd;
    delete printer;
    printer = NULL;
#endif
    return 0;
}

int HPdfPreviewDialog::nextPage()
{
    setUpdatesEnabled(false);
    ppf->showPageIndex++;
    if(ppf->showPageIndex > ppf->maxPage)
        ppf->showPageIndex = ppf->maxPage;
    pageShow->setText(QString("%1").arg(ppf->showPageIndex + 1));
    setUpdatesEnabled(true);
    update();
    return 0;
}

int HPdfPreviewDialog::prevPage()
{
    setUpdatesEnabled(false);
    ppf->showPageIndex--;
    if(ppf->showPageIndex < 0)
        ppf->showPageIndex = 0;
    pageShow->setText(QString("%1").arg(ppf->showPageIndex + 1));
    setUpdatesEnabled(true);
    update();
    return 0;
}

int HPdfPreviewDialog::startNewPage()
{
    if(pdfWriter != NULL)
        pdfWriter->newPage();
#ifndef GSAFE_DISABLE_PRINTERMODULE
    if(printer != NULL)
        printer->newPage();
#endif
    return 0;
}

void HPdfPreviewDialog::wheelEvent(QWheelEvent *e)
{
    if(e->angleDelta().y() < 0)
        nextPage();
    if(e->angleDelta().y() > 0)
        prevPage();
}

HPdfPreviewDialog::~HPdfPreviewDialog()
{

}

//End of gSAFE po.cpp
