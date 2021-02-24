/*  gSAFE - LIB
    general Sql dAtabase FrontEnd
    http://hyperprog.com/gsafe/

   (C) 2005-2021 Peter Deak  (hyper80@gmail.com)

    License: LGPLv2.1

    dialib.h
*/

#ifndef GSAFE__DIALIB_HEADER_FILE_X_
#define GSAFE__DIALIB_HEADER_FILE_X_

/** \defgroup dialib dialib */
/*  @{  */

#include <QtCore>
#include <QtGui>

#ifndef COMPILED_WITH_QT4X
#include <QtWidgets>
#endif

#include "datalib.h"
#include "guilib.h"

/** Standard quick paramerized GUI builder class.\n
 *  You can build GUI of dialog box, widget or frame, easily by parameters.
 *  You have to call the makeGui() function to build the necessary GUI. The gui will be resizeable, with stretch.\n
 *  Possible parts of the builded gui:
 *  - Caption text - The HDialogData sets the caption of the dialog/widget to this text.
 *  - Data object 1 - You can pass a HTable/HList/HPlainDataMatrix or a NULL value. A correspond GUI class will placed:
 *      HTable->HDispTable , HList->HDispList , HPlainDataMatrix->HDispPlainDataMatrix , NULL->Nothing.
 *  - Data object 2 - You can pass a HTable/HList/HPlainDataMatrix or a NULL value. A correspond GUI class will placed:
 *      HTable->HDispTable , HList->HDispList , HPlainDataMatrix->HDispPlainDataMatrix , NULL->Nothing.
 *  - Functions - You can define more additional parameter to handle special things. For example the orientation of the GUI.
 *  - Title text - A simple text displayed center of the top of the control group.
 *  - Button 1 text - (optional) If this text is not empty a QPushButton placed the bottom of dialog.
 *  - Button 2 text - (optional) If this text is not empty a QPushButton placed the bottom of dialog.
 *  - Toolbuttons - (Optional) You can define maximum 8 tool button.
 *  - Size - (optional) You can set the initial size of the window.
 *
 *  Some standard composition of builded GUI. In the picture all possible parts is enabled, keep in mind that most of them is optional!
 *  \image html pictures_doc/hdialib.png
 *
 *  The functions parameter is text which can be the combination of the following words ("Ok|EscC|Vert")
 *  - "Ok" - Put a "Ok" button to the bottom of the GUI, which closes the dialog too.
 *  - "OkIsAccept" - Accept the dialog if Ok pressed. (Otherwise it means close())
 *  - "EscC" - Close the window when the user hit Escape.
 *  - "DHoriz" or "DVert" - The orientation of dialog. Horizontal or Vertical. (Default is vertical)
 *  - "THoriz" or "TVert" - The orientation of toolbuttons
 *  - "AutoExec" - Automatically exec the dialog after build the GUI.
 *  - "TB1Close" - Close dialog after hit button 1.
 *  - "TB1AcceptClose" - When the dialog closed by the toolbutton1 it's do an accept() instead of close().
 *  - "TB2Close" - Close dialog after hit button 2.
 *  - "TB2AcceptClose" - When the dialog closed by the toolbutton2 it's do an accept() instead of close().
 *  - "InScroll" - Place the Tables/List into a scrollwindow.
 *  - "DestructiveClose" - Dectructive close the dialog.
 *  - "DeleteData" - Delete the metadata object after close.
 *  - "StrToE" - Put stretch to the end if HDiapTable. (The HDispTable does stretched when the window grow vertically)
 *  - "ToolCenter" - Toolbuttons are aligned to center
 *
 * If you would like to generate the dialog box too, you can use the HDialog class instead this. \n\n
 * A small code part from the tutorial:
 \code
   MyDialog::MyDialog(QWidget *parent) : QDialog(parent)
   {
    HList *plist = ... ;
    ...
    //build the gui with the hdialog lib
        makeGui(this,"My tasks (gSAFE tutorial)",plist,NULL,
                                "EscC|Vert|StrToE|DeleteData|DestructiveClose|NonModal|InScroll",
                "Hit doubleclick to edit!","insert","refresh list",NULL,640,480);

        connect(plist,SIGNAL(actionOnRecord(QString)),this,SLOT(editRecord(QString)));
        connect(toolbutton2,SIGNAL(clicked()),plist,SLOT(readList()));
        connect(toolbutton1,SIGNAL(clicked()),this,SLOT(insertRecord()));
    ...
 \endcode    */
class HDialogData
{
    public:
        /** Constructor. Does nothing */
        HDialogData(void);

        /** Generate the necessary GUI (See the class description for details)
         *  @param basew the base QWidget descendant where the GUI is builded.
         *  @param caption the Caption text
         *  @param dobj1 Data object 1
         *  @param dobj2 Data object 2
         *  @param func The function modifiers (See class description)
         *  @param ttext title text
         *  @param tbutt1 the text of Button1
         *  @param tbutt2 the text of Button2
         *  @param toolbuttons The pixmaps of toolbuttons.
         *  @param xsize the initial horizontal size
         *  @param ysize the initial vertical size         */
        int makeGui(QWidget *basew,QString caption=0,
                    HBase *dobj1=NULL,HBase *dobj2=NULL,
                    QString func="Ok|EscC|Vert",
                    QString ttext="",
                    QString tbutt1="",QString tbutt2="",QList<QPixmap *> *toolbuttons=NULL,
                    int xsize=320,int ysize=240 );

    public:
        bool EscC;
        bool t1C;
        bool t1AC;
        bool t2C;
        bool t2AC;
        bool inscroll;
        bool deletedata;
        bool declose;
        HDispTable *table1,*table2;
        HDispList  *list1 ,*list2;
        HDispPlainDataMatrix *dmatrix1, *dmatrix2;
        QPushButton *toolbutton1,*toolbutton2,*closebutton;
        QScrollArea *sv1,*sv2;
        QToolButton *pixTool[8];
        QLabel *topLabel;

        /** Inticates (after the dialog exec) wheter the toolbutton 1 is pressed */
        bool tb1clicked;
        /** Inticates (after the dialog exec) wheter the toolbutton 2 is pressed */
        bool tb2clicked;
};

/** Standard quick buildable paramerized dialog box based on HDialogData.
 *  \image html pictures_doc/hdialib.png
 *  See HDialogData for parameters
 \code
    ...
    HDialog *newdialog;

    newdialog = new HDialog(parent,"Add a new item",item_table,NULL,
                            "Ok|EscC|Vert|OkIsAccept","New item","","",NULL);
        if(newdialog->exec())
        {
                runCheck(); // check the data
                if(item_table->insertRecord() != 0)
                {
                //error occured
                ...
                }
        }
    delete newdialog;
    ...
 \endcode    */
class HDialog : public QDialog , public HDialogData
{
    Q_OBJECT

    public:
        /** Static function to start an independent generated dialog (alloc a object, exec, and delete it.)
         *  See HDialogData for details of parameters */
        static int run(QWidget *parent=0,QString caption=0,HBase *dobj1=NULL,
                        HBase *dobj2=NULL,QString func="Ok|EscC|Vert",QString ttext="",
                        QString tbutt1="",QString tbutt2="",QList<QPixmap *> *toolbuttons=NULL,int xsize=320,int ysize=240);

        /** Create a special generated QDialog with necessary GUI (See the HDialogData class description for details)
         *  @param basew the base QWidget descendant where the GUI is builded.
         *  @param caption the Caption text
         *  @param dobj1 Data object 1
         *  @param dobj2 Data object 2
         *  @param func The function modifiers (See class description)
         *  @param ttext title text
         *  @param tbutt1 the text of Button1
         *  @param tbutt2 the text of Button2
         *  @param toolbuttons The pixmaps of toolbuttons.
         *  @param xsize the initial horizontal size
         *  @param ysize the initial vertical size         */
        HDialog(QWidget *parent=0,
                QString caption=0,
                HBase *dobj1=NULL,
                HBase *dobj2=NULL,
                QString func="Ok|EscC|Vert",
                QString ttext ="",
                QString tbutt1="",
                QString tbutt2="",
                QList<QPixmap *> *toolbuttons=NULL,
                int xsize=320,
                int ysize=240
               );

        /** Destructor */
        ~HDialog(void);

    protected:
        void keyPressEvent(QKeyEvent *e);

    public:
        /** The parent dialog */
        QWidget *parent_dialog;

        /** Returns the generated toolbutton object by it's index. */
        QToolButton *getToolButtObj(int i) { return pixTool[i]; }

    public:
        /** Returns the first display container HDispTable or HDispList if exists. Otherwise returns null. */
        QFrame *get1DispCont(void) { if(table1 != NULL) return table1;
                                     if(list1 != NULL)  return list1;
                                     if(dmatrix1 != NULL) return dmatrix1;
                                     return NULL;                       }

        /** Returns the second display container HDispTable or HDispList if exists. Otherwise returns null. */
        QFrame *get2DispCont(void) { if(table2 != NULL) return table2;
                                     if(list2 != NULL)  return list2;
                                     if(dmatrix2 != NULL) return dmatrix2;
                                     return NULL;                       }

    private slots:
        int  tbutton1Cl(void);
        int  tbutton2Cl(void);
        int  lic(void);

    signals:
        /** This signal is emitted when the Button1 is pressed */
        void tbutton1Clicked(void);
        /** This signal is emitted when the Button2 is pressed */
        void tbutton2Clicked(void);
        /** This signal is emitted when one object is HList and the listItemChanged signal of HList is emitted */
        void listItemChanged(void);

};

class QWidget;
class QString;
class QLineEdit;

/** Gui helper class to easily get a one line data */
class HGetTextBox : public QDialog
{
    Q_OBJECT

    public:
        /** Generates a dialog which gets a branch of text data. The explain texts of the values are
         *  listed in exptexts list. (Determines the number of the requested strings)
         *  If the largetexts value is true the dialog uses a multi lined text editors */
        HGetTextBox(QWidget *parent,QString cap,QString text,QList<QString> exptexts,bool largetexts=false);

        /** You can reach the edited values through this list */
        QList<QString> results;

    protected:
        bool large;
        QList<QLineEdit *> ledits;
        QList<QTextEdit *> mledits;

    public slots:
        int clickOk(void);

};

/* @} */

#endif
