/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    gui.cpp
*/

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#include <QtCore>
#include <QtGui>
#include <QtWidgets>

#include "data.h"
#include "dm.h"
#include "ftypes.h"
#include "dconsole.h"
#include "builder.h"

#include "gui.h"
#include "guiext.h"

HFieldDisplay * newDispObjForDataField(QWidget *parent,HField *df,HDispObjectFlags upperClassFlags)
{
    QString dfcn = df->className();

    if(dfcn == "HStaticField")     return new HStaticDisplay(parent,df,upperClassFlags);
    if(dfcn == "HNKeyField")       return new HNKeyDisplay(parent,df,upperClassFlags);
    if(dfcn == "HSKeyField")       return new HSKeyDisplay(parent,df,upperClassFlags);
    if(dfcn == "HSmallTextField")  return new HSmallTextDisplay(parent,df,upperClassFlags);
    if(dfcn == "HLargeTextField")  return new HLargeTextDisplay(parent,df,upperClassFlags);
    if(dfcn == "HNumberField")     return new HNumberDisplay(parent,df,upperClassFlags);
    if(dfcn == "HFloatingField")   return new HFloatingDisplay(parent,df,upperClassFlags);
    if(dfcn == "HDateField")       return new HDateDisplay(parent,df,upperClassFlags);
    if(dfcn == "HTimestampField")  return new HTimestampDisplay(parent,df,upperClassFlags);
    if(dfcn == "HCheckField")      return new HCheckDisplay(parent,df,upperClassFlags);
    if(dfcn == "HTxtSelectField")  return new HXSelectDisplay(parent,df,upperClassFlags);
    if(dfcn == "HNumSelectField")  return new HXSelectDisplay(parent,df,upperClassFlags);
    if(dfcn == "HSqlNChooseField") return new HSqlXChooseDisplay(parent,df,upperClassFlags);
    if(dfcn == "HSqlSChooseField") return new HSqlXChooseDisplay(parent,df,upperClassFlags);

    return NULL;
}

HRecordDisplay::HRecordDisplay(QWidget *parent,HRecord *data,HDispObjectFlags flags)
 : QFrame(parent)
{
    dLink = data;
    startflags = flags;

    if(flagOn(startflags,HDispFlag_CleanDataOnStart))
        dLink->returnToDefault();

    layout = new QVBoxLayout(this);
    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(2);

    HDispObjectFlags flagToPass = HDispFlag_Default;
    if(!flagOn(startflags,HDispFlag_NoInheritFlagsFromHRecordDisplay))
        flagToPass = startflags;

    QHBoxLayout *last = NULL;
    int i,fc = dLink->fieldCount();
    for(i = 0 ; i < fc ; i++ )
    {
        if(dLink->fieldByIndex(i)->isVisible())
        {
            int stretch = 0;
            if(!dLink->fieldByIndex(i)->attribute("gui_vert_stretch").isEmpty())
                stretch = dLink->fieldByIndex(i)->attribute("gui_vert_stretch").toInt();

            HFieldDisplay *ndf = newDispObjForDataField(this,dLink->fieldByIndex(i),flagToPass);
            connect(dLink->fieldByIndex(i),SIGNAL(dataChanged()),ndf,SLOT(valueUpdatedInMemory()));
            connect(dLink->fieldByIndex(i),SIGNAL(featureChanged()),ndf,SLOT(guiElementsNeedUpdateSlot()));
            if(dLink->fieldByIndex(i)->attribute("gui_inline_attached") == "yes" && last != NULL)
                last->addWidget(ndf);
            else
                layout->addWidget(ndf,stretch);

            last = ndf->layout;
        }
    }

    if(flagOn(startflags,HDispFlag_AddStretchAfterFields))
        layout->addStretch(1);
}

HRecordDisplay::~HRecordDisplay()
{
    if(flagOn(startflags,HDispFlag_CleanDataOnDestroy))
        dLink->returnToDefault();

    if(flagOn(startflags,HDispFlag_DeleteDataObjectOnDestroy))
        delete dLink;
}

HFieldDisplay::HFieldDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : QFrame(parent)
{
    dLink = data;
    descriptionLabel = NULL;
    beforeLabel = NULL;
    afterLabel = NULL;
    progressUpdatingData = false;
    blink = 0;

    layout = new QHBoxLayout(this);
    if(flagOn(flags,HDispFlag_NoMergeFlagsWithHField))
        startflags = flags;
    else
        startflags = flags | data->displayFlags();

    if(flagOn(startflags,HDispFlag_Readonly))
        data->setFieldEditType(HFieldEdit_Readonly);

    if(flagOn(startflags,HDispFlag_ShowOnly))
        data->setFieldEditType(HFieldEdit_ShowReadonly);

    if(flagOn(startflags,HDispFlag_Invisible))
        data->setFieldEditType(HFieldEdit_Invisible);

    if(!dLink->attribute("color").isEmpty())
    {
        QPalette palette;
        palette.setColor(QPalette::Window,html6HexColor(dLink->attribute("color")));
        setPalette(palette);
        setAutoFillBackground(true);
    }

    if(dLink->attribute("gui_inline_attached") == "yes")
        layout->setContentsMargins(0,0,0,0);
    else
        layout->setContentsMargins(2,2,2,2);

    layout->setSpacing(2);
}

void HFieldDisplay::generateGuiElementsBefore()
{
    if(dLink->attribute("gui_inline_attached") != "yes")
    {
        int stretch = 0;
        if(!dLink->attribute("gui_horiz_labelw_stretch").isEmpty())
            stretch = dLink->attribute("gui_horiz_labelw_stretch").toInt();

        descriptionLabel = new QLabel(this);
        descriptionLabel->setText(dLink->description());
        layout->addWidget(descriptionLabel,stretch);
        if(!flagOn(startflags,HDispFlag_DisableStretchInMiddle))
            layout->addStretch();
    }

    if(!dLink->beforeText().isEmpty())
    {
        int stretch = 0;
        if(!dLink->attribute("gui_horiz_beforlw_stretch").isEmpty())
            stretch = dLink->attribute("gui_horiz_beforlw_stretch").toInt();

        beforeLabel = new QLabel(this);
        beforeLabel->setText(dLink->beforeText());
        layout->addWidget(beforeLabel,stretch);
    }
}

void HFieldDisplay::generateGuiElementsAfter()
{
    if(!dLink->afterText().isEmpty())
    {
        int stretch = 0;
        if(!dLink->attribute("gui_horiz_afterlw_stretch").isEmpty())
            stretch = dLink->attribute("gui_horiz_afterlw_stretch").toInt();

        afterLabel = new QLabel(this);
        afterLabel->setText(dLink->afterText());
        layout->addWidget(afterLabel,stretch);
    }

    if(flagOn(startflags,HDispFlag_AddStretchToLineEnd))
    {
        QLabel *emptyLabel = new QLabel(this);
        emptyLabel->setText("");
        layout->addStretch();
        layout->addWidget(emptyLabel);
    }
}

int HFieldDisplay::guiElementsNeedUpdateSlot()
{
    return guiElementsNeedUpdate();
}

int HFieldDisplay::guiElementsNeedUpdate()
{
    if(beforeLabel != NULL)
        beforeLabel->setText(dLink->beforeText());
    if(descriptionLabel != NULL)
        descriptionLabel->setText(dLink->description());
    if(afterLabel != NULL)
        afterLabel->setText(dLink->afterText());

    if(!dLink->attribute("color").isEmpty())
    {
        QPalette palette;
        palette.setColor(QPalette::Window,html6HexColor(dLink->attribute("color")));

        setPalette(palette);
        setAutoFillBackground(true);
    }

    updateValueEditorRoStatus();
    return 0;
}

int HFieldDisplay::valueUpdatedInMemory()
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    valueSetOnGui_internal();
    progressUpdatingData = false;
    return 0;
}

void HFieldDisplay::valueSetOnGui_internal()
{
}

void HFieldDisplay::updateValueEditorRoStatus()
{
}

int HFieldDisplay::timedUpdateSlot(void)
{
    if(!dLink->getLastValidatorCheckStatus())
    {
        blink = 0;
        update();
    }

    if(blink > 0)
    {
        blink++;
            if(blink >= 3) blink = 1;

        update();
        QTimer::singleShot(500,this,SLOT(timedUpdateSlot()));
    }
    return 0;
}

void HFieldDisplay::paintEvent(QPaintEvent *e)
{
    if((blink == 0 && dLink->getLastValidatorCheckStatus()) )
    {
        blink = 1;
        timedUpdateSlot();
    }

    if(blink > 0)
    {
        QPainter p(this);
        p.setPen( blink%2 == 0 ? Qt::red : Qt::black);
        p.drawRect(0,0,width()-1,height()-1);
    }
    QFrame::paintEvent(e);
}

HFieldDisplay::~HFieldDisplay()
{
}

// /////////////////////////////////////////////////////////////////////////////////// //

HStaticDisplay::HStaticDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    valueShow = new QLabel(this);
    valueSetOnGui_internal();
    layout->addWidget(valueShow,stretch);

    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

void HStaticDisplay::valueSetOnGui_internal()
{
    if(valueShow != NULL)
        valueShow->setText(dLink->strValue());
}

HStaticField *HStaticDisplay::myHStaticField()
{
    return (HStaticField *)dLink;
}

HStaticDisplay::~HStaticDisplay()
{
}

HSKeyDisplay::HSKeyDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    valueShow = new QLabel(this);
    valueSetOnGui_internal();
    layout->addWidget(valueShow,stretch);

    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

void HSKeyDisplay::valueSetOnGui_internal()
{
    if(valueShow != NULL)
        valueShow->setText(QString("<strong>%1</strong>").arg(dLink->displayValue()));
}

HSKeyField *HSKeyDisplay::myHSKeyField()
{
    return (HSKeyField *)dLink;
}

HSKeyDisplay::~HSKeyDisplay()
{
}

HNKeyDisplay::HNKeyDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
: HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    valueShow = new QLabel(this);
    valueSetOnGui_internal();
    layout->addWidget(valueShow,stretch);

    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

void HNKeyDisplay::valueSetOnGui_internal()
{
    if(valueShow != NULL)
        valueShow->setText(QString("<strong>%1</strong>").arg(dLink->displayValue()));
}

HNKeyField *HNKeyDisplay::myHNKeyField()
{
    return (HNKeyField *)dLink;
}

HNKeyDisplay::~HNKeyDisplay()
{
}

HSmallTextDisplay::HSmallTextDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
: HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
    {
        valueEditor = new QLineEdit(this);
        if(!dLink->attribute("password_mode").isEmpty() && dLink->attribute("password_mode") == "yes")
            valueEditor->setEchoMode(QLineEdit::Password);
        if(!dLink->attribute("gui_maxwidth").isEmpty())
            valueEditor->setMaximumWidth(dLink->attribute("gui_maxwidth").toInt());
        if(!dLink->attribute("gui_minwidth").isEmpty())
            valueEditor->setMinimumWidth(dLink->attribute("gui_minwidth").toInt());
        valueSetOnGui_internal();
        connect(valueEditor,SIGNAL(textChanged(QString)),this,SLOT(valueUpdatedOnGui(QString)));
        layout->addWidget(valueEditor,stretch);
    }
    if(data->fieldEditType() == HFieldEdit_ShowReadonly)
    {
        valueShow = new QLabel(this);
        valueSetOnGui_internal();
        layout->addWidget(valueShow,stretch);
    }

    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

int HSmallTextDisplay::valueUpdatedOnGui(const QString& t)
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    dLink->setStrValue_Gui(t);
    progressUpdatingData = false;
    return 0;
}

void HSmallTextDisplay::valueSetOnGui_internal()
{
    if(valueEditor != NULL)
        valueEditor->setText(dLink->strValue());
    if(valueShow != NULL)
        valueShow->setText(dLink->strValue());
}

void HSmallTextDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
        valueEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
}

HSmallTextField* HSmallTextDisplay::myHSmallTextField()
{
    return (HSmallTextField*)dLink;
}

HSmallTextDisplay::~HSmallTextDisplay()
{
}

HLargeTextDisplay::HLargeTextDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
: HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;
    valueTableEditor = NULL;

    int stretch = 0;
    generateGuiElementsBefore();
    int minw=100,minh=50,maxw = 0,maxh = 0;

    if(!dLink->attribute("gui_minwidth").isEmpty())
        minw = dLink->attribute("gui_minwidth").toInt();
    if(!dLink->attribute("gui_minheight").isEmpty())
        minh = dLink->attribute("gui_minheight").toInt();
    if(!dLink->attribute("gui_maxwidth").isEmpty())
        maxw = dLink->attribute("gui_maxwidth").toInt();
    if(!dLink->attribute("gui_maxheight").isEmpty())
        maxh = dLink->attribute("gui_maxheight").toInt();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(dLink->attribute("csvtable") == "yes")
    {
        valueTableEditor = new QTableWidget(this);
        QStringList cols = dLink->attribute("csvtable_columns").split(";",Qt::KeepEmptyParts);
        QStringList rows = dLink->attribute("csvtable_rows").split(";",Qt::KeepEmptyParts);

        QStringList colcolors;
        QStringList rowcolors;
        colcolors.clear();
        rowcolors.clear();

        if(!dLink->attribute("csvtable_column_colors").isEmpty())
            colcolors = dLink->attribute("csvtable_column_colors").split(";",Qt::KeepEmptyParts);
        if(!dLink->attribute("csvtable_row_colors").isEmpty())
            rowcolors = dLink->attribute("csvtable_row_colors").split(";",Qt::KeepEmptyParts);

        valueTableEditor->setColumnCount(cols.count());
        valueTableEditor->setRowCount(rows.count());
        valueTableEditor->setHorizontalHeaderLabels(cols);
        valueTableEditor->setVerticalHeaderLabels(rows);
        valueTableEditor->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        valueTableEditor->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        valueTableEditor->setStyleSheet("QTableWidget::item { padding: 1px; }");

        int ri,ci;
        int rc = valueTableEditor->rowCount(), cc = valueTableEditor->columnCount();
        QString csv = "";
        for(ri = 0 ; ri < rc ; ++ri)
            for(ci = 0 ; ci < cc ; ++ci)
            {
                valueTableEditor->setItem(ri,ci,new QTableWidgetItem(""));
                if(!rowcolors.isEmpty() && !rowcolors.at(ri).isEmpty())
                    valueTableEditor->item(ri,ci)->setBackground(QBrush(html6HexColor(rowcolors.at(ri)),Qt::SolidPattern));
                if(!colcolors.isEmpty() && !colcolors.at(ci).isEmpty())
                    valueTableEditor->item(ri,ci)->setBackground(QBrush(html6HexColor(colcolors.at(ci)),Qt::SolidPattern));
            }

        valueSetOnGui_internal();
        connect(valueTableEditor,SIGNAL(itemChanged(QTableWidgetItem *)),this,SLOT(valueUpdatedOnGuiTbl(QTableWidgetItem *)));

        valueTableEditor->setMinimumSize(minw,minh);
        if(maxw > 0 && maxh > 0)
            valueTableEditor->setMaximumSize(maxw,maxh);

        layout->addWidget(valueTableEditor,stretch);
    }
    else
    {
        if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
        {
            valueEditor = new QTextEdit(this);
            valueEditor->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            valueSetOnGui_internal();
            valueEditor->setTabChangesFocus(true);
            valueEditor->setMinimumSize(minw,minh);
            if(maxw > 0 && maxh > 0)
                valueEditor->setMaximumSize(maxw,maxh);
            connect(valueEditor,SIGNAL(textChanged()),this,SLOT(valueUpdatedOnGui()));
            layout->addWidget(valueEditor,stretch);
        }
        if(data->fieldEditType() == HFieldEdit_ShowReadonly)
        {
            valueShow = new QTextEdit(this);
            valueShow->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
            valueSetOnGui_internal();
            valueShow->setReadOnly(true);
            valueShow->setMinimumSize(minw,minh);
            if(maxw > 0 && maxh > 0)
                valueEditor->setMaximumSize(maxw,maxh);

            QColor winColor = this->palette().color(QPalette::Window);
            QPalette palette;
            palette.setColor(QPalette::Base,winColor);
            valueShow->setPalette(palette);
            layout->addWidget(valueShow,stretch);
        }
    }
    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

int HLargeTextDisplay::valueUpdatedOnGui()
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    dLink->setStrValue_Gui(valueEditor->toPlainText());
    progressUpdatingData = false;
    return 0;
}

int HLargeTextDisplay::valueUpdatedOnGuiTbl(QTableWidgetItem *twi)
{
    Q_UNUSED(twi)
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    int ri,ci;
    int rc = valueTableEditor->rowCount(), cc = valueTableEditor->columnCount();
    QString csv = "";
    for(ri = 0 ; ri < rc ; ++ri)
    {
        bool has_content = false;
        QString rstr = "";
        for(ci = 0 ; ci < cc ; ++ci)
        {
            if(ci > 0)
                rstr.append(";");

            QString cell_str = valueTableEditor->item(ri,ci)->text();
            if(!cell_str.isEmpty())
                has_content = true;
            rstr.append(cell_str);
        }
        if(has_content || dLink->attribute("csvtable_emptyrows") == "keep")
        {
            csv.append(rstr);
            csv.append("\n");
        }
    }
    dLink->setStrValue_Gui(csv);
    progressUpdatingData = false;
    return 0;
}

void HLargeTextDisplay::valueSetOnGui_internal()
{
    if(valueEditor != NULL)
        valueEditor->setText(dLink->strValue());
    if(valueShow != NULL)
        valueShow->setText(dLink->strValue());
    if(valueTableEditor != NULL)
    {
        int r,c;
        for(r = 0 ; r < valueTableEditor->rowCount() ; r++)
            for(c = 0 ; c < valueTableEditor->columnCount() ; c++)
                valueTableEditor->item(r,c)->setText("");

        QString csv = dLink->strValue();
        QStringList rows = csv.split("\n",Qt::SkipEmptyParts);

        for(r = 0 ; r < rows.count() ; r++)
        {
            if(r > valueTableEditor->rowCount())
                break;
            QStringList cells = rows[r].split(";",Qt::KeepEmptyParts);
            for(c = 0 ; c < cells.count() ; c++)
            {
                if(c > valueTableEditor->columnCount())
                    break;
                QTableWidgetItem *item = valueTableEditor->item(r,c);
                if(item != nullptr)
                    item->setText(cells[c]);
            }
        }
    }
}

void HLargeTextDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
        valueEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
    if(valueTableEditor != NULL)
    {
        int r,c;
        for(r = 0 ; r < valueTableEditor->rowCount() ; r++)
            for(c = 0 ; c < valueTableEditor->columnCount() ; c++)
            {
                if(dLink->fieldEditType() == HFieldEdit_DefaultEditable)
                    valueTableEditor->item(r,c)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled|Qt::ItemIsEditable);
                else
                    valueTableEditor->item(r,c)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            }
    }
}

HLargeTextField* HLargeTextDisplay::myHLargeTextField()
{
    return (HLargeTextField*)dLink;
}

HLargeTextDisplay::~HLargeTextDisplay()
{
}

HNumberDisplay::HNumberDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
: HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;
    valueEditor2 = NULL;
    last_correct = true;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
    {
        if(dLink->attribute("gui_simpleedit").isEmpty())
        {
            valueEditor2 = new QSpinBox(this);
            if(dLink->attribute("maximum").isEmpty())
                valueEditor2->setMaximum(999999);
            else
                valueEditor2->setMaximum(dLink->attribute("maximum").toInt());

            if(dLink->attribute("minimum").isEmpty())
                valueEditor2->setMinimum(-999999);
            else
                valueEditor2->setMinimum(dLink->attribute("minimum").toInt());

            if(!dLink->attribute("gui_maxwidth").isEmpty())
                valueEditor2->setMaximumWidth(dLink->attribute("gui_maxwidth").toInt());
            if(!dLink->attribute("gui_minwidth").isEmpty())
                valueEditor2->setMinimumWidth(dLink->attribute("gui_minwidth").toInt());
            connect(valueEditor2,SIGNAL(textChanged(QString)),this,SLOT(valueUpdatedOnGui(QString)));
            layout->addWidget(valueEditor2,stretch);
        }
        else
        {
            valueEditor = new QLineEdit(this);
            valueEditor->setAlignment(Qt::AlignRight);
            if(!dLink->attribute("gui_maxwidth").isEmpty())
                valueEditor->setMaximumWidth(dLink->attribute("gui_maxwidth").toInt());
            if(!dLink->attribute("gui_minwidth").isEmpty())
                valueEditor->setMinimumWidth(dLink->attribute("gui_minwidth").toInt());

            valueSetOnGui_internal();
            connect(valueEditor,SIGNAL(textChanged(QString)),this,SLOT(valueUpdatedOnGui(QString)));
            layout->addWidget(valueEditor,stretch);
        }
    }
    if(data->fieldEditType() == HFieldEdit_ShowReadonly)
    {
        valueShow = new QLabel(this);
        valueSetOnGui_internal();
        layout->addWidget(valueShow,stretch);
    }

    updateValueEditorRoStatus();
    generateGuiElementsAfter();

    if(!dLink->attribute("gui_extrabuttonspacebefore").isEmpty())
        layout->addSpacing(dLink->attribute("gui_extrabuttonspacebefore").toInt());

    if(!dLink->attribute("gui_incrementbutton").isEmpty() && dLink->attribute("gui_incrementbutton") == "yes")
    {
        QString rname = ":/GSAFEPIXMAPS/up45.png";
        if(!dLink->attribute("gui_incrementbuttonicon").isEmpty())
            rname = dLink->attribute("gui_incrementbuttonicon");
        QToolButton *tbUp = new QToolButton(this);
        tbUp->setIcon(QIcon(rname));
        connect(tbUp,SIGNAL(clicked()),this,SLOT(valueIncrement()));
        layout->addWidget(tbUp);
    }

    if(!dLink->attribute("gui_extrabuttonspacemiddle").isEmpty())
        layout->addSpacing(dLink->attribute("gui_extrabuttonspacemiddle").toInt());

    if(!dLink->attribute("gui_decrementbutton").isEmpty() && dLink->attribute("gui_decrementbutton") == "yes")
    {
        QString rname = ":/GSAFEPIXMAPS/down45.png";
        if(!dLink->attribute("gui_decrementbuttonicon").isEmpty())
            rname = dLink->attribute("gui_decrementbuttonicon");
        QToolButton *tbDown = new QToolButton(this);
        tbDown->setIcon(QIcon(rname));
        connect(tbDown,SIGNAL(clicked()),this,SLOT(valueDecrement()));
        layout->addWidget(tbDown);
    }

    if(!dLink->attribute("gui_extrabuttonspaceafter").isEmpty())
        layout->addSpacing(dLink->attribute("gui_extrabuttonspaceafter").toInt());
}

void HNumberDisplay::visualiseValueCorrectness(bool correct)
{
    if(valueEditor == NULL)
        return;
    if(last_correct == correct)
        return;

    QPalette p = valueEditor->palette();
    p.setColor(QPalette::Base,correct ? QColor(255,255,255) : QColor(255,200,200));
    valueEditor->setPalette(p);
    last_correct = correct;
}

int HNumberDisplay::valueUpdatedOnGui(const QString& t)
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    dLink->setStrValue_Gui(t);
    visualiseValueCorrectness(dLink->strValue() == t);
    progressUpdatingData = false;
    return 0;
}

int HNumberDisplay::valueIncrement(void)
{
    dLink->setStrValue_Gui(QString("%1").arg(dLink->strValue().toInt() + 1));
    return 0;
}

int HNumberDisplay::valueDecrement(void)
{
    dLink->setStrValue_Gui(QString("%1").arg(dLink->strValue().toInt() - 1));
    return 0;
}

void HNumberDisplay::valueSetOnGui_internal()
{
    if(valueEditor != NULL)
        valueEditor->setText(dLink->strValue());
    if(valueEditor2 != NULL)
        valueEditor2->setValue(myHNumberField()->value());
    if(valueShow != NULL)
        valueShow->setText(dLink->strValue());
    visualiseValueCorrectness(true);
}

void HNumberDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
        valueEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
}

HNumberField* HNumberDisplay::myHNumberField()
{
    return (HNumberField*)dLink;
}

HNumberDisplay::~HNumberDisplay()
{
}

HFloatingDisplay::HFloatingDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
: HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;
    last_correct = true;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
    {
        valueEditor = new QLineEdit(this);
        valueEditor->setAlignment(Qt::AlignRight);
        if(!dLink->attribute("gui_maxwidth").isEmpty())
            valueEditor->setMaximumWidth(dLink->attribute("gui_maxwidth").toInt());
        if(!dLink->attribute("gui_minwidth").isEmpty())
            valueEditor->setMinimumWidth(dLink->attribute("gui_minwidth").toInt());

        valueSetOnGui_internal();
        connect(valueEditor,SIGNAL(textChanged(QString)),this,SLOT(valueUpdatedOnGui(QString)));
        layout->addWidget(valueEditor,stretch);
    }
    if(data->fieldEditType() == HFieldEdit_ShowReadonly)
    {
        valueShow = new QLabel(this);
        valueSetOnGui_internal();
        layout->addWidget(valueShow,stretch);
    }

    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

void HFloatingDisplay::visualiseValueCorrectness(bool correct)
{
    if(valueEditor == NULL)
        return;
    if(last_correct == correct)
        return;

    QPalette p = valueEditor->palette();
    p.setColor(QPalette::Base,correct ? QColor(255,255,255) : QColor(255,200,200));
    valueEditor->setPalette(p);
    last_correct = correct;
}

int HFloatingDisplay::valueUpdatedOnGui(const QString& t)
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    dLink->setStrValue_Gui(t);
    visualiseValueCorrectness(dLink->strValue() == t);
    progressUpdatingData = false;
    return 0;
}

void HFloatingDisplay::valueSetOnGui_internal()
{
    if(valueEditor != NULL)
        valueEditor->setText(dLink->strValue());
    if(valueShow != NULL)
        valueShow->setText(dLink->displayValue());
    visualiseValueCorrectness(true);
}

void HFloatingDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
        valueEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
}

HFloatingField* HFloatingDisplay::myHFloatingField()
{
    return (HFloatingField*)dLink;
}

HFloatingDisplay::~HFloatingDisplay()
{
}

HDateDisplay::HDateDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;
    unknownEditor = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
    {
        valueEditor = new QDateEdit(this);
        valueEditor->setDate( myHDateField()->value() );
        valueEditor->setCalendarPopup(true);

        if(myHDateField()->unknownAllowed())
        {
            unknownEditor = new QCheckBox(this);
            unknownEditor->setText(QObject::tr("Unknown","HDateDisplay"));
            if(myHDateField()->value().isNull())
            {
                unknownEditor->setChecked(true);
                valueEditor->setEnabled(false);
            }
            else
            {
                unknownEditor->setChecked(false);
                valueEditor->setEnabled(true);
            }

            layout->addWidget(unknownEditor);
            connect(unknownEditor,SIGNAL(clicked()),this,SLOT(valueunkUpdatedOnGui()));
        }

        connect(valueEditor,SIGNAL(dateChanged(const QDate)),this,SLOT(valueUpdatedOnGui(const QDate)));
        layout->addWidget(valueEditor,stretch);
    }
    if(data->fieldEditType() == HFieldEdit_ShowReadonly)
    {
        valueShow = new QLabel(this);
        valueShow->setText(dLink->displayValue());
        layout->addWidget(valueShow,stretch);
    }
    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

int HDateDisplay::valueUpdatedOnGui(const QDate d)
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    if(unknownEditor == NULL || (unknownEditor != NULL && !unknownEditor->isChecked()))
        myHDateField()->setValue_Gui(d);

    progressUpdatingData = false;
    return 0;
}

int HDateDisplay::valueunkUpdatedOnGui()
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    if(unknownEditor != NULL)
    {
        if(unknownEditor->isChecked())
        {
            valueEditor->setEnabled(false);
            valueEditor->setDate(QDate());
            myHDateField()->setValue_Gui(QDate());
        }
        else
        {
            valueEditor->setEnabled(true);
            if(myHDateField()->lastValidValue.isEmpty())
                myHDateField()->lastValidValue = QDate::currentDate().toString(Qt::ISODate);
            dLink->setStrValue_Gui(myHDateField()->lastValidValue);
            valueEditor->setDate(myHDateField()->value());
        }
    }
    progressUpdatingData = false;
    return 0;
}

void HDateDisplay::valueSetOnGui_internal()
{

    if(valueEditor != NULL)
        valueEditor->setDate( myHDateField()->value() );
    if(unknownEditor != NULL)
        unknownEditor->setChecked( myHDateField()->unknownAllowed() && dLink->strValue().isEmpty() );
    if(valueShow != NULL)
        valueShow->setText(dLink->strValue());
    updateValueEditorRoStatus();
}

void HDateDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
    {
        if(dLink->fieldEditType() == HFieldEdit_DefaultEditable)
        {
            if(myHDateField()->unknownAllowed() && myHDateField()->value().isNull() )
                valueEditor->setEnabled(false);
            else
                valueEditor->setEnabled(true);
        }
        else
        {
            valueEditor->setEnabled(false);
        }
    }
    if(unknownEditor != NULL)
        unknownEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
}

HDateField* HDateDisplay::myHDateField()
{
    return (HDateField*)dLink;
}

HDateDisplay::~HDateDisplay()
{
}

HTimestampDisplay::HTimestampDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    valueShow = new QLabel(this);
    valueSetOnGui_internal();
    layout->addWidget(valueShow,stretch);

    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

void HTimestampDisplay::valueSetOnGui_internal()
{
    if(valueShow != NULL)
        valueShow->setText(dLink->displayValue());
}

HTimestampField *HTimestampDisplay::myHTimestampField()
{
    return (HTimestampField *)dLink;
}

HTimestampDisplay::~HTimestampDisplay()
{
}

HCheckDisplay::HCheckDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
    {
        valueEditor = new QCheckBox(this);
        valueSetOnGui_internal();
        connect(valueEditor,SIGNAL(clicked()),this,SLOT(valueUpdatedOnGui()));
        layout->addWidget(valueEditor,stretch);
    }
    if(data->fieldEditType() == HFieldEdit_ShowReadonly)
    {
        valueShow = new QLabel(this);
        valueSetOnGui_internal();
        layout->addWidget(valueShow,stretch);
    }
    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

int HCheckDisplay::valueUpdatedOnGui()
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    myHCheckField()->setValue_Gui( valueEditor->isChecked() );
    progressUpdatingData = false;
    return 0;
}

void HCheckDisplay::valueSetOnGui_internal()
{

    if(valueEditor != NULL)
        valueEditor->setChecked( myHCheckField()->value() );
    if(valueShow != NULL)
        valueShow->setText( myHCheckField()->displayValue() );
}

void HCheckDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
        valueEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
}

HCheckField* HCheckDisplay::myHCheckField()
{
    return (HCheckField*)dLink;
}

HCheckDisplay::~HCheckDisplay()
{
}

HXSelectDisplay::HXSelectDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;
    radioButtons = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
    {
        if(dLink->attribute("radiobuttons") != "yes")
        {
            valueEditor = new QComboBox(this);
            valueEditor->clear();
            valueEditor->addItems( uniField_selectableValues() );
            valueSetOnGui_internal();
            connect(valueEditor,SIGNAL(activated(int)),this,SLOT(valueUpdatedOnGui(int)));
            layout->addWidget(valueEditor,stretch);
        }
        else
        {
            int sp = 0;
            if(!dLink->attribute("radiobutton_space").isEmpty())
                sp = dLink->attribute("radiobutton_space").toInt();

            radioButtons = new QButtonGroup(this);
            QStringList vals = uniField_selectableValues();
            int i,c = vals.count();
            for(i = 0 ; i < c ; ++i)
            {
                QRadioButton *rin = new QRadioButton(vals[i],this);
                radioButtons->addButton(rin,i);
                layout->addWidget(rin,stretch);
                if(sp > 0)
                    layout->addSpacing(sp);
            }
            valueSetOnGui_internal();
            connect(radioButtons,SIGNAL(idClicked(int)),this,SLOT(valueUpdatedOnGui(int)));
        }
    }
    if(data->fieldEditType() == HFieldEdit_ShowReadonly)
    {
        valueShow = new QLabel(this);
        valueSetOnGui_internal();
        layout->addWidget(valueShow,stretch);
    }
    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

const QList<QString> HXSelectDisplay::uniField_selectableValues()
{
    if(dLink->className() == "HNumSelectField")
        return ((HNumSelectField *)dLink)->selectableValues();
    if(dLink->className() == "HTxtSelectField")
        return ((HTxtSelectField *)dLink)->selectableValues();
    return QList<QString>();
}

int HXSelectDisplay::uniField_currentIndexInMemory()
{
    if(dLink->className() == "HNumSelectField")
        return ((HNumSelectField *)dLink)->indexOfSelectableKey( ((HNumSelectField *)dLink)->strValue().toInt() );
    if(dLink->className() == "HTxtSelectField")
        return ((HTxtSelectField *)dLink)->indexOfSelectableKey( ((HTxtSelectField *)dLink)->strValue() );
    return 0;
}

void HXSelectDisplay::uniField_setMemoryValueByIndex(int idx)
{
    if(dLink->className() == "HNumSelectField")
    {
        QList<int> keys = ((HNumSelectField *)dLink)->selectableKeys();
        if(keys.size() >= idx)
            ((HNumSelectField *)dLink)->setStrValue_Gui( QString("%1").arg(keys[idx]) );
    }

    if(dLink->className() == "HTxtSelectField")
    {
        QStringList keys = ((HTxtSelectField *)dLink)->selectableKeys();
        if(keys.size() >= idx)
            ((HTxtSelectField *)dLink)->setStrValue_Gui( keys[idx] );
    }
}

int HXSelectDisplay::guiElementsNeedUpdate()
{
    HFieldDisplay::guiElementsNeedUpdate();

    if(valueEditor != NULL)
    {
        valueEditor->clear();
        valueEditor->addItems( uniField_selectableValues() );
        int idx = uniField_currentIndexInMemory();
        if(idx < 0)
            idx = 0;
        valueEditor->setCurrentIndex(idx);
    }

    return 0;
}

int HXSelectDisplay::valueUpdatedOnGui(int idx)
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    uniField_setMemoryValueByIndex(idx);
    progressUpdatingData = false;
    return 0;
}

void HXSelectDisplay::valueSetOnGui_internal()
{
    if(valueEditor != NULL)
    {
        int idx = uniField_currentIndexInMemory();
        if(idx < 0)
            idx = 0;
        valueEditor->setCurrentIndex(idx);
    }
    if(radioButtons != NULL)
    {
        int idx = uniField_currentIndexInMemory();
        if(idx < 0)
            idx = 0;
        QAbstractButton *ab = radioButtons->button(idx);
        ab->setChecked(true);
    }
    if(valueShow != NULL)
        valueShow->setText( dLink->displayValue() );
}

void HXSelectDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
            valueEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
    if(radioButtons != NULL)
    {
        QList<QAbstractButton *> bts = radioButtons->buttons();
        int i,c = bts.count();
        for(i = 0 ; i < c ; ++i)
            bts[i]->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
    }
}

HXSelectDisplay::~HXSelectDisplay()
{
}

HSqlXChooseDisplay::HSqlXChooseDisplay(QWidget *parent,HField *data,HDispObjectFlags flags)
 : HFieldDisplay(parent,data,flags)
{
    valueShow = NULL;
    valueEditor = NULL;
    valueEditorMod = NULL;
    popup_dlg = NULL;

    int stretch = 0;
    generateGuiElementsBefore();

    if(!dLink->attribute("gui_horiz_mainvalw_stretch").isEmpty())
        stretch = dLink->attribute("gui_horiz_mainvalw_stretch").toInt();

    if(data->fieldEditType() == HFieldEdit_DefaultEditable || data->fieldEditType() == HFieldEdit_Readonly)
    {
        if(myHSqlXChooseField()->attribute("easyselect") == "yes")
        {
            valueEditor = new QComboBox(this);
            valueEditor->clear();
            guiElementsNeedUpdate();

            refreshButton = new QToolButton(this);
            refreshButton->setIcon(QPixmap(":/GSAFEPIXMAPS/refreshbutton.png"));

            connect(valueEditor,SIGNAL(activated(int)),this,SLOT(valueUpdatedOnGui(int)));
            connect(refreshButton,SIGNAL(clicked()),this,SLOT(refreshSelectableValues()));

            layout->addWidget(valueEditor,stretch);
            layout->addWidget(refreshButton);
        }
        else
        {
            valueEditorMod = new QPushButton(this);
            guiElementsNeedUpdate();

            connect(valueEditorMod,SIGNAL(clicked()),this,SLOT(popupSelect()));
            layout->addWidget(valueEditorMod,stretch);
        }
    }
    if(data->fieldEditType() == HFieldEdit_ShowReadonly)
    {
        valueShow = new QLabel(this);
        valueShow->setText(dLink->displayValue());
        layout->addWidget(valueShow,stretch);
    }
    updateValueEditorRoStatus();
    generateGuiElementsAfter();
}

int HSqlXChooseDisplay::refreshSelectableValues()
{
    uniField_refreshKVs();
    guiElementsNeedUpdate();
    return 0;
}

int HSqlXChooseDisplay::guiElementsNeedUpdate()
{
    HFieldDisplay::guiElementsNeedUpdate();

    if(valueEditor != NULL)
    {
        valueEditor->clear();
        valueEditor->addItems( uniField_selectableValues() );
        valueSetOnGui_internal();
    }
    if(valueEditorMod != NULL)
    {
        valueSetOnGui_internal();
    }

    return 0;
}

int HSqlXChooseDisplay::valueUpdatedOnGui(int idx)
{
    if(progressUpdatingData)
        return 0;

    progressUpdatingData = true;
    bool ok;
    QString toSet = uniField_keyStrValOnIndex(idx,&ok);
    myHSqlXChooseField()->setStrValue_Gui(toSet);
    progressUpdatingData = false;
    return 0;
}

int HSqlXChooseDisplay::valueUpdatedOnGuiStr(QString keyname)
{
    if(progressUpdatingData)
        return 0;

    if(keyname.isEmpty())
        return 0;

    if(keyname == "__NULL__" && uniField_unknownAllowed())
        keyname = "";

    progressUpdatingData = true;
    sdebug(QString("Set from popup: %1").arg(keyname));
    myHSqlXChooseField()->setStrValue_Gui(keyname);
    if(popup_dlg != NULL)
        popup_dlg->close();
    progressUpdatingData = false;
    valueSetOnGui_internal();
    return 0;
}

void HSqlXChooseDisplay::valueSetOnGui_internal()
{
    if(valueEditor != NULL)
    {
        int idx = uniField_indexOfSelectableKey( myHSqlXChooseField()->strValue() );
        if(idx < 0)
            idx = 0;
        valueEditor->setCurrentIndex(idx);
    }
    if(valueEditorMod != NULL)
    {
        valueEditorMod->setText( myHSqlXChooseField()->displayValue() );
    }
    if(valueShow != NULL)
        valueShow->setText( myHSqlXChooseField()->displayValue() );
}

void HSqlXChooseDisplay::updateValueEditorRoStatus(void)
{
    if(valueEditor != NULL)
            valueEditor->setEnabled(dLink->fieldEditType() == HFieldEdit_DefaultEditable);
}

HSqlXChooseField* HSqlXChooseDisplay::myHSqlXChooseField()
{
    return (HSqlXChooseField*)dLink;
}

bool HSqlXChooseDisplay::uniField_unknownAllowed()
{
    if(dLink->className() == "HSqlSChooseField")
        return ((HSqlSChooseField *)dLink)->unknownAllowed();
    if(dLink->className() == "HSqlNChooseField")
        return ((HSqlNChooseField *)dLink)->unknownAllowed();
    return false;
}

void HSqlXChooseDisplay::uniField_refreshKVs()
{
    if(dLink->className() == "HSqlSChooseField")
        ((HSqlSChooseField *)dLink)->refreshKVs();
    if(dLink->className() == "HSqlNChooseField")
        ((HSqlNChooseField *)dLink)->refreshKVs();
}

const QList<QString> HSqlXChooseDisplay::uniField_selectableValues()
{
    if(dLink->className() == "HSqlSChooseField")
        return ((HSqlSChooseField *)dLink)->selectableValues();
    if(dLink->className() == "HSqlNChooseField")
        return ((HSqlNChooseField *)dLink)->selectableValues();
    return QList<QString>();
}

const QList<QString> HSqlXChooseDisplay::uniField_selectableKeysAsStr()
{
    if(dLink->className() == "HSqlSChooseField")
        return ((HSqlSChooseField *)dLink)->selectableKeys();
    if(dLink->className() == "HSqlNChooseField")
    {
        QList<QString> toStr;
        QList<int> sk = ((HSqlNChooseField *)dLink)->selectableKeys();
        int i,c = sk.size();
        for(i = 0 ; i < c ; i++)
            toStr.push_back(QString("%1").arg(sk[i]));
        return toStr;
    }
    return QList<QString>();
}

QString HSqlXChooseDisplay::uniField_keyStrValOnIndex(int idx,bool *ok)
{
    *ok = false;
    if(dLink->className() == "HSqlSChooseField")
    {
        QList<QString> keys = ((HSqlSChooseField *)dLink)->selectableKeys();
        if(keys.size() >= idx)
            *ok = true;
        return keys[idx];
    }
    if(dLink->className() == "HSqlNChooseField")
    {
        QList<int> keys = ((HSqlNChooseField *)dLink)->selectableKeys();
        if(keys.size() >= idx)
            *ok = true;
        return QString("%1").arg(keys[idx]);
    }
    return QString();
}

int HSqlXChooseDisplay::uniField_indexOfSelectableKey(QString keyString)
{
    if(dLink->className() == "HSqlSChooseField")
        return ((HSqlSChooseField *)dLink)->indexOfSelectableKey(keyString);
    if(dLink->className() == "HSqlNChooseField")
        return ((HSqlNChooseField *)dLink)->indexOfSelectableKey(keyString.toInt());
    return -1;
}

int HSqlXChooseDisplay::popupSelect()
{
    popup_dlg = new HSqlXChooseDisplayPopup(this);
    connect(popup_dlg,SIGNAL(customUserEvent(QString,QString)),myHSqlXChooseField(),SLOT(customUserEventDispatcher(QString,QString)));
    connect(myHSqlXChooseField(),SIGNAL(featureChanged()),popup_dlg,SLOT(guiElementsNeedUpdateSlot()));
    connect(myHSqlXChooseField(),SIGNAL(dataChanged()),popup_dlg,SLOT(valueUpdatedInMemory()));

    popup_dlg->exec();
    delete popup_dlg;
    popup_dlg = NULL;
    return 0;
}

HSqlXChooseDisplay::~HSqlXChooseDisplay()
{
}

//End of gSAFE gui.cpp
