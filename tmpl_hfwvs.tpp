/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   tmpl_hfwvs.h
*/

#include "dm.h"

template <typename T>
HFieldWithValueStore<T>::HFieldWithValueStore()
{

}

template <typename T>
HFieldWithValueStore<T>::~HFieldWithValueStore()
{

}

template <typename T>
const QList<T> HFieldWithValueStore<T>::selectableKeys()
{
    return selectables.keys();
}

template <typename T>
const QList<QString> HFieldWithValueStore<T>::selectableValues()
{
    return selectables.values();
}

template <typename T>
T HFieldWithValueStore<T>::selectableKey(int index)
{
    return selectables.keyByIdx(index);
}

template <typename T>
QString HFieldWithValueStore<T>::selactableValue(int index)
{
    return selectables.valueByIdx(index);
}

template <typename T>
int HFieldWithValueStore<T>::indexOfSelectableKey(T key)
{
    return selectables.indexOfKey(key);
}

template <typename T>
int HFieldWithValueStore<T>::indexOfSelectableValue(QString value)
{
    return selectables.indexOfValue(value);
}
