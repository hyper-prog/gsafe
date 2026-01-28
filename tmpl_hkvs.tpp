/* gSAFE - LIB
   general Sql dAtabase FrontEnd
   http://hyperprog.com/gsafe/

    (C) 2005-2026 Péter Deák (hyper80@gmail.com)

   License: Apache 2.0

   tmpl_hkvs.h
*/

#include "dm.h"

template <typename T>
HKeyValueStore<T>::HKeyValueStore()
{
    clear();
}

template <typename T>
HKeyValueStore<T>::~HKeyValueStore()
{

}

template <typename T>
void HKeyValueStore<T>::clear()
{
    vKeys.clear();
    vValues.clear();
    uKeys.clear();
    uValues.clear();
}

template <typename T>
void HKeyValueStore<T>::addItem(T key,QString val)
{
    vKeys.push_back(key);
    vValues.push_back(val);
}

template <typename T>
void HKeyValueStore<T>::setItems(QList<T>& keys,QList<QString>& values)
{
    vKeys.clear();
    vValues.clear();
    if(keys.count() == values.count())
    {
        vKeys = keys;
        vValues = values;
    }
}

template <typename T>
void HKeyValueStore<T>::addUnlistedItem(T key,QString val)
{
    uKeys.push_back(key);
    uValues.push_back(val);
}

template <typename T>
void HKeyValueStore<T>::setUnlistedItems(QList<T>& keys,QList<QString>& values)
{
    uKeys.clear();
    uValues.clear();
    if(keys.count() == values.count())
    {
        uKeys = keys;
        uValues = values;
    }
}

template <typename T>
const QList<T>& HKeyValueStore<T>::keys()
{
    return vKeys;
}

template <typename T>
const QList<QString>& HKeyValueStore<T>::values()
{
    return vValues;
}

template <typename T>
const QList<T>& HKeyValueStore<T>::unlistedKeys()
{
    return uKeys;
}

template <typename T>
const QList<QString>& HKeyValueStore<T>::unlistedValues()
{
    return uValues;
}

template <typename T>
bool HKeyValueStore<T>::hasKey(T key,bool unlistedIncluded)
{
    if(vKeys.contains(key))
        return true;
    if(unlistedIncluded && uKeys.contains(key))
        return true;
    return false;
}

template <typename T>
QString HKeyValueStore<T>::value(T key,bool unlistedIncluded)
{
    int idx;
    idx = vKeys.indexOf(key);
    if(idx >= 0)
        return vValues[idx];
    if(!unlistedIncluded)
        return "";
    idx = uKeys.indexOf(key);
    if(idx >= 0)
        return uValues[idx];
    return "";
}

template <typename T>
T HKeyValueStore<T>::keyByIdx(int index)
{
    return vKeys[index];
}

template <typename T>
QString HKeyValueStore<T>::valueByIdx(int index)
{
    return vValues[index];
}

template <typename T>
int HKeyValueStore<T>::indexOfKey(T key)
{
    return vKeys.indexOf(key);
}

template <typename T>
int HKeyValueStore<T>::indexOfValue(QString val)
{
    return vValues.indexOf(val);
}


