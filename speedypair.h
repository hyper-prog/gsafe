/*
    HSpeedyPair - Template Lib
    A dual AVL tree based pair storing template code

   (C) 2006 Peter Deak  (hyper80@gmail.com)

    speedypair.h
*/
#ifndef __H_SPEEDYPAIR_CONTAINER_TEMPLATE__
#define __H_SPEEDYPAIR_CONTAINER_TEMPLATE__

#ifndef NULL
#ifdef  __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

#define max(a,b) ( ((a)>(b)) ? (a) : (b) )


/** HPair is the return type of HSpeedyPair::getArray() */
template <class KEY,class VALUE>
class HPair
{
    public:
        KEY key;
        VALUE value;
};

/* NOT DOCUMENTED: HSpeedypairNode: For internal use only! See HSpeedyPair */
template <class MYVAL,class TOVAL>
class HSpeedypairNode
{
    public:

        MYVAL data;
        HSpeedypairNode<MYVAL,TOVAL> *left,*right;
        HSpeedypairNode<TOVAL,MYVAL> *pair;

        HSpeedypairNode(MYVAL d)
        {
            left  = NULL;
            right = NULL;
            pair = NULL;
            data = d;
        }
    
        ~HSpeedypairNode(void)
        {
            if(left != NULL)
                delete left;
            left = NULL;
    
            if(right != NULL)
                delete right;
            right = NULL;

            #ifndef ONE_SIDE_ONLY
                ;
            #else
            if(pair != NULL)
                delete pair;
            #endif
            pair = NULL;
        }

        static void inorder(HSpeedypairNode<MYVAL,TOVAL> *here,int *seq,void *array,int mode = 0);

        static void insertTo(HSpeedypairNode<MYVAL,TOVAL> * &here, HSpeedypairNode<MYVAL,TOVAL> *e);

        //rotations
        static void rleft  (HSpeedypairNode<MYVAL,TOVAL> * &here);
        static void rright (HSpeedypairNode<MYVAL,TOVAL> * &here);
        static void drleft (HSpeedypairNode<MYVAL,TOVAL> * &here);
        static void drright(HSpeedypairNode<MYVAL,TOVAL> * &here);

        static int height(HSpeedypairNode<MYVAL,TOVAL> *t);
        static HSpeedypairNode* find_min(HSpeedypairNode<MYVAL,TOVAL> *t);
    
};


/** HSpeedyPair is a key-value pair template container, which based on dual avl tree.
 *  Uses an AVL to Key->Value search, and an another AVT tree to Value->Key search. */
template <class KEY,class VALUE>
class HSpeedyPair
{
    private:
        int nume;
        HSpeedypairNode<KEY,VALUE> *root_key;

        #ifndef ONE_SIDE_ONLY
        HSpeedypairNode<VALUE,KEY> *root_value;
        #endif

        bool f;
        //do not use:
        KEY   *arrayk;
        VALUE *arrayv;
        HPair<KEY,VALUE> *arrayp;
        int seq;

    public:
        /** The notfound value for key */
        KEY     key_notfound_data;
        /** The notfound value for value */
        VALUE   value_notfound_data;
    public:
        HSpeedyPair(void);
        HSpeedyPair(KEY kn,VALUE vn);
        ~HSpeedyPair(void);

        void clear(void);
        void addPair(KEY k,VALUE v);
        VALUE getValue(KEY   key  );

        #ifndef ONE_SIDE_ONLY
        KEY   getKey  (VALUE value);
        #endif

        int num(void) { return nume; }
        int heightKTree(void) { return HSpeedypairNode<KEY,VALUE>::height(root_key); }
        
        #ifndef ONE_SIDE_ONLY
        int heightVTree(void) { return HSpeedypairNode<VALUE,KEY>::height(root_value); }
        #endif

        bool found(void) { return f; }
        
        KEY   *getKArray(void);
        
        #ifndef ONE_SIDE_ONLY
        VALUE *getVArray(void);
        #endif

        HPair<KEY,VALUE> *getArray(void);
};


/*  Program code */

template <class KEY,class VALUE>
HSpeedyPair<KEY,VALUE>::HSpeedyPair(void)
{
    root_key = NULL;

    #ifndef ONE_SIDE_ONLY
    root_value = NULL;
    #endif

    nume = 0;
}

template <class KEY,class VALUE>
HSpeedyPair<KEY,VALUE>::HSpeedyPair(KEY kn,VALUE vn)
{
    root_key = NULL;

    #ifndef ONE_SIDE_ONLY
    root_value = NULL;
    #endif

    nume = 0;
    key_notfound_data   = kn;
    value_notfound_data = vn;
}

template <class KEY,class VALUE>
HSpeedyPair<KEY,VALUE>::~HSpeedyPair(void)
{
    clear();
}

template <class KEY,class VALUE>
void HSpeedyPair<KEY,VALUE>::clear(void)
{
    if(root_key != NULL)
        delete root_key;
    root_key = NULL;

    #ifndef ONE_SIDE_ONLY
    if(root_value != NULL)
            delete root_value;
    root_value = NULL;
    #endif

    nume = 0;
}

template <class KEY,class VALUE>
void HSpeedyPair<KEY,VALUE>::addPair(KEY k,VALUE v)
{
      ++nume;
      HSpeedypairNode<KEY,VALUE>* nk=new HSpeedypairNode<KEY,VALUE>(k);
      HSpeedypairNode<VALUE,KEY>* nv=new HSpeedypairNode<VALUE,KEY>(v);

      //Set cross reference
      nk->pair = nv;
      nv->pair = nk;

      HSpeedypairNode<KEY,VALUE>::insertTo(root_key  ,nk); //calls to the private insert function

      #ifndef ONE_SIDE_ONLY
      HSpeedypairNode<VALUE,KEY>::insertTo(root_value,nv); //calls to the private insert function
      #endif
}

template <class KEY,class VALUE>
VALUE HSpeedyPair<KEY,VALUE>::getValue(KEY key)
{
    HSpeedypairNode<KEY,VALUE> *t = root_key;
    
    f = true;
    while(1)
    {
        if(t == NULL)
        {   //Not found!
            f = false;
            return value_notfound_data;
        }
        if(t->data < key)
        {
            t = t->right;
            continue;
        }
        if(t->data > key)
        {
            t = t->left;
            continue;
        }
        return t->pair->data;
    }
}

#ifndef ONE_SIDE_ONLY
template <class KEY,class VALUE>
KEY HSpeedyPair<KEY,VALUE>::getKey(VALUE value)
{
    HSpeedypairNode<VALUE,KEY> *t = root_value;

    f = true;
    while(1)
    {
        if(t == NULL)
        {   //Not found!
            f = false;
            return key_notfound_data;
        }
        if(t->data < value)
        {
            t = t->right;
            continue;
        }
        if(t->data > value)
        {
            t = t->left;
            continue;
        }
        return t->pair->data;
    }
}
#endif

template <class MYVAL,class TOVAL>
void HSpeedypairNode<MYVAL,TOVAL>::insertTo(HSpeedypairNode<MYVAL,TOVAL> * &here,
                                           HSpeedypairNode<MYVAL,TOVAL> *  e)
{
    if(here == NULL) //Root node is null, the new element insert as root.
         here = e;

    else
    {
        if(here->data < e->data)
        {
            insertTo(here->right,e);
            if(HSpeedypairNode<MYVAL,TOVAL>::height(here->right) - 
               HSpeedypairNode<MYVAL,TOVAL>::height(here->left)   == 2)
             {
                 if(e->data > here->right->data)
                     HSpeedypairNode<MYVAL,TOVAL>::rleft(here);
                 else
                     HSpeedypairNode<MYVAL,TOVAL>::drright(here);
             }
        }
        else if(here->data > e->data)
        {
            insertTo(here->left,e);
            if(HSpeedypairNode<MYVAL,TOVAL>::height(here->left) - 
               HSpeedypairNode<MYVAL,TOVAL>::height(here->right)  == 2)
             {
                 if(e->data < here->left->data)
                     HSpeedypairNode<MYVAL,TOVAL>::rright(here);
                 else
                     HSpeedypairNode<MYVAL,TOVAL>::drleft(here);
             }
        }
    }
}

template <class MYVAL,class TOVAL>
void HSpeedypairNode<MYVAL,TOVAL>::rleft(HSpeedypairNode<MYVAL,TOVAL> * &here)
{
      HSpeedypairNode<MYVAL,TOVAL> *tmp = here->right;
      here->right = tmp->left;
      tmp->left = here;
      here = tmp;
}

template <class MYVAL,class TOVAL>
void HSpeedypairNode<MYVAL,TOVAL>::rright(HSpeedypairNode<MYVAL,TOVAL> * &here)
{
      HSpeedypairNode<MYVAL,TOVAL> *tmp = here->left;
      here->left = tmp->right;
      tmp->right = here;
      here = tmp;
}

template <class MYVAL,class TOVAL>
void HSpeedypairNode<MYVAL,TOVAL>::drleft(HSpeedypairNode<MYVAL,TOVAL> * &here)
{
      HSpeedypairNode<MYVAL,TOVAL> *tmp2;
      HSpeedypairNode<MYVAL,TOVAL> *tmp1;

      tmp2 = here->left;
      tmp1 = tmp2->right;
      tmp2->right = tmp1->left;
      tmp1->left = tmp2;
      here->left = tmp1;
      HSpeedypairNode<MYVAL,TOVAL> * tmp4 = here->left;
      here->left = tmp4->right;
      tmp4->right = here;
      here=tmp4;
}

template <class MYVAL,class TOVAL>
void HSpeedypairNode<MYVAL,TOVAL>::drright(HSpeedypairNode<MYVAL,TOVAL> * &here)
{
      HSpeedypairNode<MYVAL,TOVAL> *tmp2;
      HSpeedypairNode<MYVAL,TOVAL> *tmp1;

      tmp2 = here->right;
      tmp1 = tmp2->left;
      tmp2->left = tmp1->right;
      tmp1->right = tmp2;
      here->right = tmp1;
      HSpeedypairNode<MYVAL,TOVAL> *tmp4 = here->right;
      here->right = tmp4->left;
      tmp4->left=here;
      here=tmp4;
}


template <class MYVAL,class TOVAL>
int HSpeedypairNode<MYVAL,TOVAL>::height(HSpeedypairNode<MYVAL,TOVAL> *t)
{
      if(t==NULL)
      {
            return -1;
      }
      else
            return 1 + max((HSpeedypairNode<MYVAL,TOVAL>::height(t->left)),
                           (HSpeedypairNode<MYVAL,TOVAL>::height(t->right)));
}

template <class MYVAL,class TOVAL>
HSpeedypairNode<MYVAL,TOVAL> * HSpeedypairNode<MYVAL,TOVAL>::find_min(HSpeedypairNode<MYVAL,TOVAL> *t)         
{
      if(t==NULL)
            return NULL;
      else
            if(t->left==NULL)
                  return t;
            else
                  return(HSpeedypairNode<MYVAL,TOVAL>::find_min(t->left));
}

template <class MYVAL,class TOVAL>
void HSpeedypairNode<MYVAL,TOVAL>::inorder(HSpeedypairNode<MYVAL,TOVAL> *here,int *seq,void *array,int mode)
{
    if(here->left != NULL) 
        HSpeedypairNode<MYVAL,TOVAL>::inorder(here->left ,seq,array,mode);
    
    if(mode)
    {
        ((HPair<MYVAL,TOVAL> *)array)[(*seq)  ].key   = here->data; 
        ((HPair<MYVAL,TOVAL> *)array)[(*seq)++].value = here->pair->data;   
    }
    else
        ((MYVAL *)array)[(*seq)++] = here->data;    
    
    if(here->right != NULL) 
        HSpeedypairNode<MYVAL,TOVAL>::inorder(here->right,seq,array,mode);
}

template <class KEY,class VALUE>
KEY * HSpeedyPair<KEY,VALUE>::getKArray(void)
{
    arrayk = new KEY[num()];
    seq = 0;
    HSpeedypairNode<KEY,VALUE>::inorder(root_key,&seq,arrayk);
    return arrayk;
}

#ifndef ONE_SIDE_ONLY
template <class KEY,class VALUE>
VALUE * HSpeedyPair<KEY,VALUE>::getVArray(void)
{
    arrayv = new VALUE[num()];
    seq = 0;
    HSpeedypairNode<VALUE,KEY>::inorder(root_value,&seq,arrayv);
    return arrayv;
}
#endif

template <class KEY,class VALUE>
HPair<KEY,VALUE> *HSpeedyPair<KEY,VALUE>::getArray(void)
{
    arrayp = new HPair<KEY,VALUE>[num()];
    seq = 0;
    HSpeedypairNode<KEY,VALUE>::inorder(root_key,&seq,arrayp,1);
    return arrayp;
}
#endif
//end code
