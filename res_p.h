/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Peter Deak  (hyper80@gmail.com)

   License: Apache 2.0

    res_p.h
*/

#ifndef GSAFE__RESOURCEPROVIDER_HEADER_FILE_X_
#define GSAFE__RESOURCEPROVIDER_HEADER_FILE_X_

/** Interface class for objects which provides resources for HyperDoc objects. */
class HResourceProvider
{
    public:
        virtual QString     getResource(QString rcname) = 0;
        virtual bool        hasResource(QString rcname) = 0;
        virtual QStringList resources  (void)           = 0;

};

#endif
