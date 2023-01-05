/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   gui.h
*/

#ifndef GSAFE__GUI_HEADER_FILE_X_
#define GSAFE__GUI_HEADER_FILE_X_

#include <QtCore>
#include <QFrame>

#include <data.h>
#include <dm.h>
#include <ftypes.h>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QTextEdit;
class QDateEdit;
class QCheckBox;
class QLineEdit;
class QComboBox;
class QPushButton;
class QToolButton;
class QSpinBox;
class QButtonGroup;
class HFieldDisplay;
class QTableWidget;
class QTableWidgetItem;

class HSqlXChooseDisplayPopup;

HFieldDisplay * newDispObjForDataField(QWidget *parent,HField *df,HDispObjectFlags upperClassFlags);

class HRecordDisplay : public QFrame
{
    Q_OBJECT

public:
    HRecordDisplay(QWidget *parent,HRecord *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HRecordDisplay();

protected:

    HRecord *dLink;
    QVBoxLayout *layout;
    HDispObjectFlags startflags;
};

class HFieldDisplay : public QFrame
{
    Q_OBJECT

public:

    HFieldDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HFieldDisplay();

public slots:
    int guiElementsNeedUpdateSlot();
    int valueUpdatedInMemory();
    int timedUpdateSlot(void);

protected:
    virtual void valueSetOnGui_internal();
    virtual int guiElementsNeedUpdate();
    void generateGuiElementsBefore();
    void generateGuiElementsAfter();

    virtual void updateValueEditorRoStatus(void);

    virtual void paintEvent(QPaintEvent *e);

    bool progressUpdatingData;
    HField *dLink;
    int blink;

    HDispObjectFlags startflags;

    QLabel *descriptionLabel,*beforeLabel,*afterLabel;

public:
    QHBoxLayout *layout;

};

class HStaticDisplay : public HFieldDisplay
{
    Q_OBJECT

public:

    HStaticDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HStaticDisplay();

protected:
    virtual void valueSetOnGui_internal();

    HStaticField *myHStaticField();

    QLabel    *valueShow;
};

class HSKeyDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HSKeyDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HSKeyDisplay();

protected:
    virtual void valueSetOnGui_internal();

    HSKeyField *myHSKeyField();

    QLabel    *valueShow;
};

class HNKeyDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HNKeyDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HNKeyDisplay();

protected:
    virtual void valueSetOnGui_internal();

    HNKeyField *myHNKeyField();

    QLabel     *valueShow;
};

class HSmallTextDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HSmallTextDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HSmallTextDisplay();

public slots:
    int valueUpdatedOnGui(const QString& t);

protected:
    virtual void valueSetOnGui_internal();
    virtual void updateValueEditorRoStatus(void);
    HSmallTextField *myHSmallTextField();

    QLineEdit *valueEditor;
    QLabel    *valueShow;
};

class HLargeTextDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HLargeTextDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HLargeTextDisplay();

public slots:
    int valueUpdatedOnGui();
    int valueUpdatedOnGuiTbl(QTableWidgetItem *twi);
    int handleDoubleClick(QTableWidgetItem *item);

protected:
    virtual void valueSetOnGui_internal();
    virtual void updateValueEditorRoStatus(void);
    HLargeTextField *myHLargeTextField();

    QTextEdit *valueEditor;
    QTableWidget *valueTableEditor;
    QTextEdit *valueShow;
};

class HNumberDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HNumberDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HNumberDisplay();

public slots:
    int valueUpdatedOnGui(const QString& t);
    int valueIncrement(void);
    int valueDecrement(void);

protected:
    virtual void valueSetOnGui_internal();
    virtual void updateValueEditorRoStatus(void);
    HNumberField *myHNumberField();

    bool last_correct;
    void visualiseValueCorrectness(bool correct);

    QLineEdit *valueEditor;
    QSpinBox  *valueEditor2;
    QLabel    *valueShow;
};

class HFloatingDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HFloatingDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HFloatingDisplay();

public slots:
    int valueUpdatedOnGui(const QString& t);

protected:
    virtual void valueSetOnGui_internal();
    virtual void updateValueEditorRoStatus(void);
    HFloatingField *myHFloatingField();

    bool last_correct;
    void visualiseValueCorrectness(bool correct);

    QLineEdit *valueEditor;
    QLabel    *valueShow;
};

class HDateDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HDateDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HDateDisplay();

public slots:
    int valueUpdatedOnGui(const QDate d);
    int valueunkUpdatedOnGui();

protected:
    virtual void valueSetOnGui_internal();
    virtual void updateValueEditorRoStatus(void);
    HDateField* myHDateField();

    QDateEdit *valueEditor;
    QCheckBox *unknownEditor;
    QLabel    *valueShow;
};

class HTimestampDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HTimestampDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HTimestampDisplay();

protected:
    virtual void valueSetOnGui_internal();

    HTimestampField *myHTimestampField();

    QLabel     *valueShow;
};

class HCheckDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HCheckDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HCheckDisplay();

public slots:
    int valueUpdatedOnGui();

protected:
    virtual void valueSetOnGui_internal();
    virtual void updateValueEditorRoStatus(void);
    HCheckField* myHCheckField();

    QCheckBox *valueEditor;
    QLabel    *valueShow;
};

class HXSelectDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HXSelectDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HXSelectDisplay();

public slots:
    int valueUpdatedOnGui(int idx);

protected:
    const QList<QString> uniField_selectableValues();
    int uniField_currentIndexInMemory();
    void uniField_setMemoryValueByIndex(int idx);

    virtual void valueSetOnGui_internal();
    virtual int guiElementsNeedUpdate();
    virtual void updateValueEditorRoStatus(void);

    QComboBox *valueEditor;
    QButtonGroup *radioButtons;
    QLabel    *valueShow;
};

class SqlChooseDialog;
class HSqlXChooseDisplay : public HFieldDisplay
{
    Q_OBJECT

public:
    HSqlXChooseDisplay(QWidget *parent,HField *data,HDispObjectFlags flags = HDispFlag_Default);
    ~HSqlXChooseDisplay();

    HSqlXChooseField* myHSqlXChooseField();

public slots:
    int valueUpdatedOnGui(int idx);
    int valueUpdatedOnGuiStr(QString keyname);
    int refreshSelectableValues();
    int popupSelect();

public:
    bool uniField_unknownAllowed();
    void uniField_refreshKVs();
    const QList<QString> uniField_selectableValues();
    const QList<QString> uniField_selectableKeysAsStr();

protected:
    virtual void valueSetOnGui_internal();
    virtual int guiElementsNeedUpdate();
    virtual void updateValueEditorRoStatus(void);

    QString uniField_keyStrValOnIndex(int idx,bool *ok);
    int uniField_indexOfSelectableKey(QString keyString);

    QComboBox *valueEditor;
    QPushButton *valueEditorMod;
    QToolButton *refreshButton;
    QLabel    *valueShow;

    HSqlXChooseDisplayPopup *popup_dlg;

};


#endif

//End of gSAFE gui.h
