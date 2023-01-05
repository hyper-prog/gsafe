/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

   (C) 2005-2023 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    guilib
*/

#ifndef GSAFE__TEXTS_HEADER_FILE_X_
#define GSAFE__TEXTS_HEADER_FILE_X_

/** \defgroup texts texts */
/*  @{  */

#ifdef GSAFETEXT_LANG_HU
//===========================================================================================//
// begin GSAFETEXT_LANG_HU

/** Unknown text (lowercase) "unknown" */
#define GSAFETEXT_UNKNOWN       "ismeretlen"
/** Unknown text "Unknown" */
#define GSAFETEXT_TEXT_UNKNOWN  "Ismeretlen"

/** The default text of the HCheck checked "Yes" */
#define GSAFETEXT_YES_HAVE      "igen/van"
/** The default text of the HCheck unchecked "No" */
#define GSAFETEXT_NO_HAVENOT    "nem/nincs"

/** The title of HShowPrintHtml external program calling dialog. "Calling external program..." */
#define GSAFETEXT_CALLEXTERNAL  "Külsõ program hívása..."

/** The text of the HShowPrintHtml external program calling dialog 1.
 *  "External program is started!("  */
#define GSAFETEXT_STARTDIATEXT1 "Külsõ program elindítva! ("
/** The text of the HShowPrintHtml external program calling dialog 2..
 *  ")\nDon't close this window, it will disappear automatically, if you exit from the external program!"  */
#define GSAFETEXT_STARTDIATEXT2 ")\nEzt az ablakot ne zárd be, magától el fog tünni, ha a külsõ programból kiléptél!"

/** SqlChooseDialog select button text "Select" */
#define GSAFETEXT_DISPSQLCH_SELECTBUTTON    "Kiválaszt"
/** SqlChooseDialog select dialog title "Select a value" */
#define GSAFETEXT_DISPSQLCH_TITLE           "Érték kiválasztása"

#define GSAFETEXT_XMLO_WINTITLE             "Excl 2003 XML export..."
#define GSAFETEXT_XMLO_GETLABEL             "Az elmentendõ XML export helye és neve:"
#define GSAFETEXT_XMLO_OUTFILETITLE         "Az XML export fájl"
#define GSAFETEXT_SAVE                      "Ment"
#define GSAFETEXT_CLOSE                     "Bezár"
#define GSAFETEXT_CANNOT_CREATE_FILE        "Nem tudom létrehozni a fájlt! (%1)"

// end of GSAFETEXT_LANG_HU
//===========================================================================================//

#else
//===========================================================================================//
// begin GSAFETEXT_LANG_ENG

/** Unknown text (lowercase) "unknown" */
#define GSAFETEXT_UNKNOWN       "unknown"
/** Unknown text "Unknown" */
#define GSAFETEXT_TEXT_UNKNOWN  "Unknown"

/** The default text of the HCheck checked "Yes" */
#define GSAFETEXT_YES_HAVE      "yes/have"
/** The default text of the HCheck unchecked "No" */
#define GSAFETEXT_NO_HAVENOT    "no/haven't"

/** The title of HShowPrintHtml external program calling dialog. "Calling external program..." */
#define GSAFETEXT_CALLEXTERNAL  "Calling external program..."

/** The text of the HShowPrintHtml external program calling dialog 1.
 *  "External program is started!("  */
#define GSAFETEXT_STARTDIATEXT1 "External program is started! ("
/** The text of the HShowPrintHtml external program calling dialog 2.
 *  ")\nDon't close this window, it will disappear automatically, if you exit from the external program!"  */
#define GSAFETEXT_STARTDIATEXT2 ")\nDon't close this window, it will disappear automatically, if you exit from the external program!"

/** SqlChooseDialog select button text "Select" */
#define GSAFETEXT_DISPSQLCH_SELECTBUTTON    "Select"
/** SqlChooseDialog select dialog title "Select a value" */
#define GSAFETEXT_DISPSQLCH_TITLE           "Select a value"

#define GSAFETEXT_XMLO_WINTITLE             "Excl 2003 XML output..."
#define GSAFETEXT_XMLO_GETLABEL             "The name and location of the output XML file:"
#define GSAFETEXT_XMLO_OUTFILETITLE         "The output xml file"
#define GSAFETEXT_SAVE                      "Save"
#define GSAFETEXT_CLOSE                     "Close"
#define GSAFETEXT_CANNOT_CREATE_FILE        "Cannot create file! (%1)"

// end GSAFETEXT_LANG_ENG
//===========================================================================================//
#endif // else branch

/*  @}  */

#endif
//end.
