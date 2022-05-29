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
}

HPageTileRenderer::~HPageTileRenderer()
{

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

int HPageTileRenderer::areaWidth()
{
    if(!areastack.isEmpty())
        return areastack.first().sizeW;
    return p->window().width();
}

int HPageTileRenderer::areaHeight()
{
    if(!areastack.isEmpty())
        return areastack.first().sizeH;
    return p->window().height();
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

    if(!str.endsWith("%") && !str.endsWith("em"))
        iv = str.toInt();

    if(str.endsWith("%"))
    {
        double percent = str.mid(0,str.length()-1).toDouble();
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

void HPageTileRenderer::storePos(int w,int h)
{
    HPageTileRendererPosition sp;
    sp.valid = true;
    sp.page = currentPage + 1;
    sp.pixel_x = cursorX;
    sp.pixel_y = cursorY;
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

    QRect r(cursorX,cursorY,w_px,1000);
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
        r = QRect(cursorX,cursorY,w_px,1000);
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
    QRect r(x,y,w_px,1000);
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

    QRect r(cursorX,cursorY,w_px,h_eff);

    if(h_eff + cursorY > areaHeight())
    {
        newPage();
        r = QRect(cursorX,cursorY,w_px,h_eff);
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

    QRect r(cursorX,cursorY,w_px,h_eff);

    if(h_eff + cursorY > areaHeight())
    {
        newPage();
        r = QRect(cursorX,cursorY,w_px,h_eff);
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

    QRect r(cursorX,cursorY,w_px,height);

    if(height + cursorY > areaHeight())
    {
        newPage();
        r = QRect(cursorX,cursorY,w_px,height);
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

    QRect r(x,y,w_px,height);

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

    QRect r(x,y,w_px,h_px);
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

    QRect r(cursorX,cursorY,w_px,1000);
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

    QRect r(cursorX,cursorY,w_px,h_px);

    if(h_px + cursorY > areaHeight())
    {
        newPage();
        r = QRect(cursorX,cursorY,w_px,h_px);
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

    areastack.push_front(area);
    p->save();
    p->translate(r.topLeft());

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

    QRect r(x,y,w_px,h_px);

    AreaData area;
    area.sizeW = w_px;
    area.sizeH = h_px;
    area.cursorX = cursorX;
    area.cursorY = cursorY;
    area.currentLineHeight = currentLineHeight;
    area.currentPage = currentPage;
    area.pageFilter = pageFilter;

    areastack.push_front(area);
    p->save();
    p->translate(r.topLeft());

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
        inPageBoot = false;
    if(inPageBoot)
        pageboot_buffer.push_back(parts);
    if(cmd == "EVERYPAGE_START")
        inPageBoot = true;

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
        if(cmd == "text")
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
                incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Plain);
        if(cmd == "html")
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
                incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Html);
        if(cmd == "mark")
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
                incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Markdown);

        if(cmd == "imgr")
            if(parts.at(1).split(",",Qt::KeepEmptyParts).count() < 3)
            {
                QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
                if(pp.count() < 3)
                    incrementMinLineHeightToImageHeight(parts.at(1),QImage(parts.at(2)));
            }
        if(cmd == "imgb")
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

    if(cmd == "fixh")
    {
    }
    if(cmd == "movr")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() >= 2)
            moveCursorRelative(pp[0],pp[1]);
    }
    if(cmd == "mova")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() >= 2)
            moveCursorAbsolute(pp[0],pp[1]);
    }

    if(cmd == "newl")
        newLine();
    if(cmd == "newp")
        newPage();

    if(cmd == "spac")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() == 1)
            addSpace(pp[0],"1");
        if(pp.count() == 2)
            addSpace(pp[0],pp[1]);
    }
    if(cmd == "rect")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() == 2)
            addRect(pp[0],pp[1]);
        if(pp.count() == 4)
            drawRect(pp[0],pp[1],pp[2],pp[3]);
    }
    if(cmd == "text")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() > 2)
            drawText(pp[0],pp[1],pp[2],parts.at(2),HTextType_Plain);
        else
            addText(parts.at(1),parts.at(2),HTextType_Plain);
    }
    if(cmd == "html")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() > 2)
            drawText(pp[0],pp[1],pp[2],parts.at(2),HTextType_Html);
        else
            addText(parts.at(1),parts.at(2),HTextType_Html);
    }
    if(cmd == "mark")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() > 2)
            drawText(pp[0],pp[1],pp[2],parts.at(2),HTextType_Markdown);
        else
            addText(parts.at(1),parts.at(2),HTextType_Markdown);
    }

    if(cmd == "imgr")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() > 2)
            drawImage(pp[0],pp[1],pp[2],QImage(parts.at(2)));
        else
            addImage(parts.at(1),QImage(parts.at(2)));
    }
    if(cmd == "imgb")
    {
        QImage img = QImage::fromData(QByteArray::fromBase64(parts.at(2).toLocal8Bit()));
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() > 2)
            drawImage(pp[0],pp[1],pp[2],img);
        else
            addImage(parts.at(1),img);
    }

    if(cmd == "smhr")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() < 3)
            incrementMinLineHeightToImageHeight(parts.at(1),QImage(parts.at(2)));
    }
    if(cmd == "smhi")
    {
        QImage img = QImage::fromData(QByteArray::fromBase64(parts.at(2).toLocal8Bit()));
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() < 3)
            incrementMinLineHeightToImageHeight(parts.at(1),img);
    }
    if(cmd == "smht")
        incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Plain);
    if(cmd == "smhh")
        incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Html);
    if(cmd == "smhm")
        incrementMinLineHeightToTextHeight(parts.at(1),parts.at(2),HTextType_Markdown);
    if(cmd == "smhz")
        setMinimumLineHeight(0);
    if(cmd == "smhv")
        setMinimumLineHeight(parts.at(1));

    if(cmd == "colf")
        setFontColor(html6HexColor(parts.at(1)));
    if(cmd == "coll")
        setPen(QPen(html6HexColor(parts.at(1))));
    if(cmd == "colb")
        setBrush(QBrush(html6HexColor(parts.at(1))));

    if(cmd == "fram")
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
    if(cmd == "alig")
    {
        if(parts.at(1) == "left")
            setTextAlignment(Qt::AlignLeft);
        if(parts.at(1) == "right")
            setTextAlignment(Qt::AlignRight);
        if(parts.at(1) == "center")
            setTextAlignment(Qt::AlignCenter);
    }

    if(cmd == "setf")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() >= 2)
            setFont(QFont(pp[0],pp[1].toInt()));
    }
    if(cmd == "setd")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() >= 2)
            setDefaultFont(QFont(pp[0],pp[1].toInt()));
    }
    if(cmd == "deff")
    {
        resetToDefaultFont();
    }

    if(cmd == "getp")
        storePositionOfNextAddElement(parts.at(1));

    if(cmd == "area")
    {
        QStringList pp = parts.at(1).split(",",Qt::KeepEmptyParts);
        if(pp.count() == 2)
            enterArea(pp[0],pp[1]);
        if(pp.count() == 4)
            enterArea(pp[0],pp[1],pp[2],pp[3]);
    }
    if(cmd == "reta")
        returnArea();

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

        if(cmd == "COND")
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

        if(cmd == "FUNC")
        {
            definingFunctionName = parts.at(1).trimmed();
            definingFunctionBody = "";
            continue;
        }

        if(cmd == "CALL")
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
                            to = processToken(rp[0]);
                        else
                            to = processToken(rp[1]);
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
    attachmentFiles.clear();
    lastRenderStoredPositions.clear();
    QVBoxLayout *mlay = new QVBoxLayout(this);
    QHBoxLayout *toplay = new QHBoxLayout(0);
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
    mlay->addLayout(toplay);
    mlay->addWidget(ppf);

    pageShow->setText(QString("%1").arg(ppf->showPageIndex + 1));
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
    printer = new QPrinter();
    printer->setResolution(200);
    printer->setPageSize(QPageSize(QPageSize::A4));  /*8.26 x 11.69  -> *200 -> 1652 x 2338*/
    printer->setPageOrientation(QPageLayout::Portrait);

    QPrintDialog *pd = new QPrintDialog(printer,this);
    if(pd->exec())
    {
        QPainter pp(printer);
        pp.setWindow(0,0,1652,2338); // PageSite A4 on 200 dpi (Set elsewhere...)
        HPageTileRenderer *renderer = new HPageTileRenderer(&pp);
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
