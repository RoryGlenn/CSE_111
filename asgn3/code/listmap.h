// $Id: listmap.h,v 1.25 2020-11-20 10:39:00-08 - - $

// how to submit
// submit cse111-wm.w21 asg3 *.cpp *.h *.tcc README Makefile

#ifndef __LISTMAP_H__
#define __LISTMAP_H__

#include "debug.h"
#include "xless.h"
#include "xpair.h"


#define SHOW_LINK(FLAG, PTR)                        \
   {                                                \
      DEBUGB(FLAG, #PTR << "=" << PTR               \
                        << ": next=" << PTR->next   \
                        << ", prev=" << PTR->prev); \
   }

// key_t and mapped_t are the elements to be kept in the list.
// less_t is the class used to determine the order-ing of 
//the list and defaults to xless<key_t>.
template <typename key_t, typename mapped_t, 
                  class less_t = xless<key_t> >
class listmap
{
public:
   using key_type = key_t;
   using mapped_type = mapped_t;
   using value_type = xpair<const key_type, mapped_type>;

private:
   less_t less;
   struct node;

   // In a list with 'n' nodes, there are 'n + 1' links, 
   // each node having a link, and the list itself 
   // having a link, but not node values.
   struct link
   {
      node *next{};
      node *prev{};
      link(node *next_, node *prev_) : next(next_), prev(prev_) {}
   };


   struct node : link
   {
      value_type value{};
      node(node *next_, node *prev_, const value_type &value_):
         link(next_, prev_), value(value_) {}
   };

   node *anchor()
   {
      return static_cast<node *>(&anchor_);
   }

   link anchor_{anchor(), anchor()};

public:
   class iterator;
   listmap(){};
   listmap(const listmap &);
   listmap &operator=(const listmap &);
   ~listmap();

   // takes a 'pair' as a single argument, 
   // if the key is already there, the value is replaced
   iterator insert(const value_type &);

   // searches and returns an iterator. 
   // If find does not find anything, return the 'end()' iterator
   iterator find(const key_type &);

   // The item pointed at by the argument iterator 
   // is deleted from the list.
   // The returned iterator points at the position 
   // immediately following that which was erased.
   iterator erase(iterator position);

   // We dont bother with a constant iterator because 
   // 'end()' is just the list itself
   iterator begin() { return anchor()->next; }
   iterator end() { return anchor(); }

   bool empty() const { return anchor_.next == &anchor_; }
   operator bool() const { return not empty(); }

};

// specifies precisely which class the iterator belongs to.
template <typename key_t, typename mapped_t, class less_t>
class listmap<key_t, mapped_t, less_t>::iterator
{

private:
   // only a listmap is permitted to construct a valid iterator
   friend class listmap<key_t, mapped_t, less_t>;

   // the iterator keeps track of both the node and the list as a whole,
   // so that end() an return an iterator "off the end".
   listmap<key_t, mapped_t, less_t>::node *where{nullptr};
   iterator(node *where_) : where(where_){};

public:
   iterator() {}

   // returns a ref to some value type (key, value) in the list.
   // Selections are then by dot '.'
   value_type &operator*()
   {
      SHOW_LINK('b', where);
      return where->value;
   }

   // Returns a pointer to some value type, from which
   // fields can be selected with an arrow '->'
   value_type *operator->()
   {
      return &(where->value);
   }
   

   // ++itor
   // --itor
   // Move backwards and forwards along the list.
   // Moving off the end with ++nd moving from an end iterator
   // to the last element does not requre special coding,
   // since the list is a circular list. We don't bother
   // here with the postfix operators
   iterator &operator++()
   {
      where = where->next;
      return *this;
   }
   iterator &operator--()
   {
      where = where->prev;
      return *this;
   }

   bool operator==(const iterator &that) const
   {
      return this->where == that.where;
   }

   bool operator!=(const iterator &that) const
   {
      return this->where != that.where;
   }
};

#include "listmap.tcc"
#endif
