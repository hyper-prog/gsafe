﻿/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2022 Péter Deák (hyper80@gmail.com)

    License: LGPLv2.1

    guiext.h
*/

#ifndef GSAFE__GUIEXT_HEADER_FILE_X_
#define GSAFE__GUIEXT_HEADER_FILE_X_

#include <QtCore>
#include <QFrame>
#include <QDialog>

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
class QTableWidget;
class QTableWidgetItem;

class HQuickTable;
class HFieldDisplay;
class HDynTable;
class HSqlXChooseDisplay;

QColor html6HexColor(QString hc);

class HSqlXChooseDisplayPopup : public QDialog
{
    Q_OBJECT

    private:
        QString popupped_key;

    public:
        HSqlXChooseDisplayPopup( HSqlXChooseDisplay* p);
        ~HSqlXChooseDisplayPopup();

        HQuickTable* table;
        QPushButton* selectButton;
        bool disabled_recvkey;
        bool key_received;
        QString received_key;

    protected:
        HSqlXChooseDisplay* dparent;

        QVBoxLayout* SqlChooseDialogLayout;
        QHBoxLayout* layout1;

    public slots:
        int updateList(void);
        int guiElementsNeedUpdateSlot();
        int valueUpdatedInMemory();

        int captureNotify(QString tblname);

    protected slots:
        int nullButtonPressed(void);
        int selButtonPressed(void);
        int listviewitemSelected(const QString& s);

        int toolbarButtonClicked(void);
        int popup_catch_0(void);
        int popup_catch_1(void);
        int popup_catch_2(void);
        int popup_catch_3(void);
        int popup_catch_4(void);
        int popUpMenuActivated(const QString& key);

    signals:
        void customUserEvent(QString what,QString ckey);
        void itemSelected(const QString& s);    //this connected to the parents
};

/** HDataMatrixDisplay is a GUI viewer class for a HDataMatrix object.
 *  Shows the data in a HQuickTable, and put a xml output button and a print preview button above the lister.
 *  It can put an optional html output button too, if the content have html output.
 *  (The html content will be generated by HPlainDataMatrix::getContentAsHtml()
 *   the html content will be generated with the following parameters: "html center" )
 *  @see HDataMatrix
 */
class HDataMatrixDisplay : public QFrame
{
    Q_OBJECT

    protected:
        QString genhtml_parameters;
        HDataMatrix *data;
        HQuickTable *qtable;
        HDispObjectFlags startflags;
        bool dataobject_deleted;

    public:
        /** Creates a HDataMatrixDisplay object
         *  @param parent the parent GUI widget (QWidget descendant)
         *  @param d the HDataMatrix object to show.
         *  @param flags : Can accept HDispFlag_DeleteDataObjectOnDestroy      */
        HDataMatrixDisplay(QWidget *parent,HDataMatrix *d,HDispObjectFlags flags = HDispFlag_Default);
        /** Destructor */
        ~HDataMatrixDisplay(void);

    public:
        /** Sets html generation parameters.
         *  The html content will be generated by HDataMatrix::getContentAsHtml()
         *  with parameters "html center" by default. You can redefine this parameterstring with this function */
        void setGenHtmlParameters(QString p) { genhtml_parameters = p; }

        /** Sets the color of the specified column */
        void setColumnColor(int col,QColor color);

        HQuickTable *quickTable(void) { return qtable; }

        virtual void updateFromDataMatrix_Common(void);
        void updateFromDataMatrixDataCells(void);

    protected slots:
        int listItemChanged(const QString key);

    public slots:

        int updateFromDataMatrix(void);

        /** Activate the key "k" element */
        int slotActivateItem(const QString k);
        /** Alternate activate the key "s" element */
        int itemAlternateActivated(const QString& s);

    signals:
        /** The user activated (clicked) on the following keyed element "k" */
        void activateItem(QString k);
};


class HRecordLinesDisplay : public HDataMatrixDisplay
{
    Q_OBJECT

public:
    HRecordLinesDisplay(QWidget *parent,HRecordLines *rl,HDispObjectFlags flags = HDispFlag_Default);
    ~HRecordLinesDisplay();

    virtual void updateFromDataMatrix_Common(void);
    void updateQTableFeaturesFromRecordLines(void);

protected slots:
    int listItemChanged(const QString key);

protected:
    HRecordLines *rldata;
    QString alternate_action_key;
    QStringList alternate_action_event_keys;

protected slots:
    int altact_catch_0(void);
    int altact_catch_1(void);
    int altact_catch_2(void);
    int altact_catch_3(void);
    int altact_catch_4(void);
    int popUpMenuActivated(const QString& key);

};


/** This class can connect a HDynTable object to a QTableWidget object.
 *  It initializes the QTableWidget object the correct way (See HDynTable describe)
 *  It keeps the two object's connected and handles the data changes until it's deleted.
 *  See the using example in HDynTable describe.
 *  @see HDynTable */
class HDynTableDisplayConnector : public QObject
{
    Q_OBJECT

public:
    /** Initializes the connection between the HDynTable and the QTableWidget */
    HDynTableDisplayConnector(HDynTable *dynt,QTableWidget *qtw);
    ~HDynTableDisplayConnector(void);

private:
    typedef struct
    {
        QString name;
        int r,c;
        QTableWidgetItem *glink;
    } cellData;

    HDynTable *dynt;
    QList<cellData> cells;
    bool ccprogress;
    bool rfprogress;

public slots:
    /** It refresh the gui from the HDynTable */
    int refreshGui(void);
    /** The QTableWidget's cellChanged signal will be connected to this slot. You don't need to use. */
    int slotCellChanged(int row,int column);

signals:
    /** Emitted when the user changed the data in the gui.
     *  When thi signal is emitted the data was already changed in the HDynTable. */
    void dataChangedByGui(void);

};

#endif

//End of gSAFE gui.h
