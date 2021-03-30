// $Id: listmap.tcc,v 1.15 2019-10-30 12:44:53-07 - - $

#include "listmap.h"
#include "debug.h"

// how to submit
// submit cse111-wm.w21 asg3 *.cpp *.h *.tcc README Makefile


//
/////////////////////////////////////////////////////////////////
// Operations on listmap.
/////////////////////////////////////////////////////////////////
//

//
// listmap::~listmap()
//
template <typename key_t, typename mapped_t, class less_t>
listmap<key_t, mapped_t, less_t>::~listmap()
{
   DEBUGF('l', reinterpret_cast<const void *>(this));
}

//
// iterator listmap::insert (const value_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t, mapped_t, less_t>::iterator
listmap<key_t, mapped_t, less_t>::insert(const value_type &pair)
{
   // DEBUGF('l', &pair << "->" << pair);

   // case 1: the key exists, replace the data
   auto iter = this->find(pair.first);
   if (iter != this->end())
   {
      iter->second = pair.second;
      return iter;
   }

   // case 2a: the key doesn't exist
   // search the map, and use the less than function
   // to find out where to insert it in lexicographic order
   iter = this->begin();
   for (; iter != this->end(); ++iter)
   {
      if ( less(pair.first, iter->first) )
      {  
         // B->next = C, B->prev = A
         // A->next = B
         // C->prev = B
         node *N = new node(iter.where, iter.where->prev, pair);
         iter.where->prev->next = N; 
         iter.where->prev = N;
         return N;
      }
   }

   // case 2b: if we got to the end of the map and
   // did not find any key, insert it at the end of the map
   // B->next = C, B->prev = A
   // A->next = B
   // C->prev = B
   node *N = new node(iter.where, iter.where->prev, pair);
   iter.where->prev->next = N;
   iter.where->prev = N;
   return N;
}


//
// listmap::find(const key_type&)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t, mapped_t, less_t>::iterator
listmap<key_t, mapped_t, less_t>::find(const key_type &that)
{
   // DEBUGF('l', that);

   for (auto iter = this->begin(); iter != this->end(); ++iter)
   {
      // if the strings match
      if (iter->first == that)
      {
         return iter;
      }
   }

   // if we did not find the item we are looking for, return end()
   return this->end();
}

//
// iterator listmap::erase (iterator position)
//
template <typename key_t, typename mapped_t, class less_t>
typename listmap<key_t, mapped_t, less_t>::iterator
listmap<key_t, mapped_t, less_t>::erase(iterator position)
{
   // DEBUGF('l', &*position);
   iterator next_node = position.where->next;
   position.where->prev->next = position.where->next;
   position.where->next->prev = position.where->prev;
   delete position.where;
   return next_node;
}
