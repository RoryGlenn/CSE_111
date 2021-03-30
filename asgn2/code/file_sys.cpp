// $Id: file_sys.cpp,v 1.9 2020-10-26 21:32:08-07 - - $

#include <cassert>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <sstream>

#include "debug.h"
#include "file_sys.h"
using namespace std;

size_t inode::next_inode_nr{1};

ostream &operator<<(ostream &out, file_type type)
{
   switch (type)
   {
   case file_type::PLAIN_TYPE:
      out << "PLAIN_TYPE";
      break;
   case file_type::DIRECTORY_TYPE:
      out << "DIRECTORY_TYPE";
      break;
   default:
      assert(false);
   };
   return out;
}

///// INODE STATE //////////////

inode_state::inode_state()
{
   DEBUGF('i', "root = " << root << ", cwd = " << cwd
                         << ", prompt = \"" << prompt() << "\"");

   // make root a pointer to a directory type
   // set the current working directory to root
   // set the root directory to the path of "/"

   string dot = ".";
   string dotdot = "..";

   this->root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   this->root->set_path("/");
   this->root->set_name("/");
   this->cwd = root;

   this->root->get_contents()->add_dirents(dot, root);
   this->root->get_contents()->add_dirents(dotdot, root);

   // auto dirents = this->get_root()->get_contents()->get_dirents();
}

const string &inode_state::prompt() const
{
   return prompt_;
}

ostream &operator<<(ostream &out, const inode_state &state)
{
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

bool inode_state::H_directory_exists(inode_state &state, string dirname)
{
   auto it = state.get_cwd()->get_contents()->get_dirents().begin();
   auto end = state.get_cwd()->get_contents()->get_dirents().end();

   // check if directory already exists or
   // if the parent to the directory does not exist!
   for (; it != end; ++it)
   {
      if (it->first == dirname)
      {
         if (it->second->get_type() == file_type::DIRECTORY_TYPE)
         {
            // throw an error because we found
            // the item name and it is a directory!
            return true;
         }
      }
   }

   // there is no directory called "/"
   // in the current directory so lets make it
   return false;
}

bool inode_state::H_does_exist(inode_state &state, string name)
{
   // check if directory already exists or
   // if the parent to the directory does not exist!

   auto it = state.get_cwd()->get_contents()->get_dirents().begin();
   auto end = state.get_cwd()->get_contents()->get_dirents().end();
   for (; it != end; ++it)
   {
      if (it->first == name)
      {
         if (it->second->get_type() == file_type::DIRECTORY_TYPE ||
             it->second->get_type() == file_type::PLAIN_TYPE)
            return true;
      }
   }

   // there is no file or directory called name in the current directory
   return false;
}

base_file_ptr inode_state::get_contents(inode_ptr _content)
{
   return _content->get_contents();
}

inode_ptr inode_state::get_item_ptr(string &name, inode_ptr &_cwd)
{
   auto dirents = this->get_contents(_cwd)->get_dirents();
   auto it = dirents.find(name);

   if (it != dirents.end())
   {
      return it->second;
   }

   return nullptr;
}

bool inode_state::
    H_plain_file_exists(inode_state &state, string filename)
{
   auto it = state.get_cwd()->get_contents()->get_dirents().begin();
   auto end = state.get_cwd()->get_contents()->get_dirents().end();

   // check if directory already exists
   // or if the parent to the directory does not exist!
   for (; it != end; ++it)
   {
      if (it->first == filename)
      {
         if (it->second->get_type() == file_type::PLAIN_TYPE)
         {
            return true;
         }
      }
   }

   return false;
}


// slice the string according to where the "/" is
// and store it in directories vector
wordvec inode_state::string_to_wordvec(string chop_me)
{
   string tmp_str;
   stringstream ss(chop_me);
   wordvec directories;

   while (getline(ss, tmp_str, '/'))
   {
      if (tmp_str == "")
      {
         // return empty wordvec on error
         directories.clear();
         return directories;
      }
      else
      {
         directories.push_back(tmp_str);
      }
   }
   if (directories.empty())
   {
      directories.push_back(chop_me);
   }

   return directories;
}

inode_ptr inode_state::path_to_inode_ptr(wordvec &_pathvec)
{
   inode_ptr ptr = this->get_cwd();
   inode_ptr error_ptr = make_shared<inode>(file_type::DIRECTORY_TYPE);

   if (_pathvec.empty())
   {
      error_ptr->set_error("404");
      return error_ptr;
   }

   // iterate over elements in pathvec
   int size = static_cast<int>(_pathvec.size());

   for (int i = 0; i < size; i++)
   {
      auto dirents = ptr->get_contents()->get_dirents();
      auto iter = dirents.find(_pathvec[i]);

      // if we found it in the map
      if (iter != dirents.end())
      {
         ptr = iter->second;
      }
      // did not find it
      else if (iter == dirents.end())
      {
         // it is only ok to not find the file if we are
         // on the last iteration of the loop.
         // For example, "folder1/folder2/file"
         if (i != size - 1)
         {
            // we didn't find anything, throw error and return
            error_ptr->set_error("404");
            return error_ptr;
         }
      }
   }

   return ptr;
}

// INODE

inode::inode(file_type _type) : inode_nr(next_inode_nr++)
{
   switch (_type)
   {
   case file_type::PLAIN_TYPE:
      contents = make_shared<plain_file>();
      this->type = file_type::PLAIN_TYPE;
      break;
   case file_type::DIRECTORY_TYPE:
      contents = make_shared<directory>();
      this->type = file_type::DIRECTORY_TYPE;
      break;
   default:
      assert(false);
   }

   DEBUGF('i', "inode " << inode_nr << ", type = " << _type);
}

size_t inode::get_inode_nr() const
{
   DEBUGF('i', "inode = " << this->inode_nr);
   return this->inode_nr;
}

size_t inode::get_size()
{
   return this->contents->size();
}

file_error::file_error(const string &what) : runtime_error(what) {}

////////////////
// BASE FILE

const wordvec &base_file::readfile() const
{
   throw file_error("is a " + error_file_type());
}

void base_file::readfile(std::string &)
{
   throw file_error("is a " + error_file_type());
}

wordvec base_file::get_file_data()
{
   throw file_error("is a " + error_file_type());
}

bool base_file::H_directory_exists()
{
   throw file_error("is a " + error_file_type());
}

size_t base_file::size()
{
   throw file_error("is a " + error_file_type());
}

void base_file::writefile(const wordvec &)
{
   throw file_error("is a " + error_file_type());
}

void base_file::remove(const string &)
{
   throw file_error("is a " + error_file_type());
}

inode_ptr base_file::mkdir(const string &)
{
   throw file_error("is a " + error_file_type());
}

void base_file::mkdir(const string &, inode_state &)
{
   throw file_error("is a " + error_file_type());
}

inode_ptr base_file::mkfile(string)
{
   throw file_error("is a " + error_file_type());
}

map<string, inode_ptr> base_file::get_dirents()
{
   throw file_error("is a " + error_file_type());
}

void base_file::add_dirents(string, inode_ptr)
{
   throw file_error("is a " + error_file_type());
}

void base_file::print_dirents()
{
   throw file_error("is a " + error_file_type());
}

void base_file::remove(inode_state &, const string &)
{
   throw file_error("is a " + error_file_type());
}

void base_file::remove_recursion(inode_ptr &)
{
   throw file_error("is a " + error_file_type());
}

string base_file::get_path()
{
   throw file_error("is a " + error_file_type());
}

vector<string> base_file::get_directory_content()
{
   throw file_error("is a " + error_file_type());
}

/////////////////////////
// PLAIN FILE

size_t plain_file::size()
{
   DEBUGF('i', "size = " << get_file_data());

   size_t _size = 0;

   for (auto word : this->data)
   {
      for (auto chr : word)
      {
         if (chr)
         {
         }
         _size++;
      }
   }
   return _size;
}

const wordvec &plain_file::readfile() const
{
   DEBUGF('i', data.size());
   cout << this->data << endl;
   return this->data;
}

void plain_file::writefile(const wordvec &words)
{
   DEBUGF('i', words.size());
   int start = 2;
   for (int i = start; i < static_cast<int>(words.size()); i++)
   {
      this->data.push_back(words[i]);
   }
}

void plain_file::readfile(string &file)
{
   if (file == "")
   {
   }
   cout << this->data << endl;
}

wordvec plain_file::get_file_data()
{
   return this->data;
}

inode_ptr plain_file::mkfile(string)
{
   throw file_error("is a plain file");
}

vector<string> plain_file::get_directory_content()
{
   throw file_error("is a plain file");
}

void plain_file::remove_recursion(inode_ptr &)
{
   throw file_error("is a plain file");
}

///////////////////////////
// DIRECTORY

size_t directory::size()
{
   DEBUGF('i', "size = " << this->dirents.size());
   return this->dirents.size();
}

void directory::remove(const string &filename)
{
   auto iter = this->dirents.find(filename);

   if (iter != this->dirents.end())
   {
      this->dirents.erase(iter);
   }
   else
   {
      remove_recursion(iter->second);
   }
}

void directory::remove_recursion(inode_ptr &ptr)
{
   for (auto iter = dirents.begin(); iter != dirents.end(); ++iter)
   {
      if (iter->first != ".." && iter->first != ".")
      {
         if (iter->second->get_type() == file_type::DIRECTORY_TYPE)
         {
            ptr->get_contents()->remove_recursion(ptr);
         }
      }
   }

   this->dirents.clear();
}

void directory::mkdir(const string &dirname, inode_state &state)
{
   DEBUGF('i', dirname);
   inode_ptr parent;
   string parent_name;
   inode_ptr new_shared_directory_inode_ptr =
       make_shared<inode>(file_type::DIRECTORY_TYPE);

   new_shared_directory_inode_ptr->set_parent(state.get_cwd());
   new_shared_directory_inode_ptr->set_name(dirname);

   parent = new_shared_directory_inode_ptr->get_parent();
   parent_name = parent->get_path();

   new_shared_directory_inode_ptr->get_contents()->
                           add_dirents("..", parent);
   state.set_cwd(new_shared_directory_inode_ptr);

   new_shared_directory_inode_ptr->get_contents()->
                     add_dirents(".", state.get_cwd());
   state.set_cwd(parent);

   if (parent_name != "/")
   {
      new_shared_directory_inode_ptr->set_path(
         new_shared_directory_inode_ptr->get_path() 
                        + parent_name + "/" + dirname);
   }
   else
   {
      new_shared_directory_inode_ptr->set_path(
         new_shared_directory_inode_ptr->get_path()
                                     + "/" + dirname);
   }

   state.get_cwd()->get_contents()->
      add_dirents(dirname, new_shared_directory_inode_ptr);
}

inode_ptr directory::mkfile(string filename)
{
   DEBUGF('i', filename);
   inode_ptr ptr = make_shared<inode>(file_type::PLAIN_TYPE);
   ptr->set_name(filename);
   this->dirents[filename] = ptr;
   return ptr;
}

const wordvec &directory::readfile() const
{
   DEBUGF('i', data);
   return this->data;
}

void directory::readfile(std::string) const
{
}

void directory::print_dirents()
{
   for (auto iter = this->dirents.begin();
        iter != this->dirents.end(); ++iter)
   {
      if (iter->second->get_type() == file_type::DIRECTORY_TYPE)
      {
         cout << "   " << iter->second->get_inode_number();
         cout << "   " << iter->second->get_contents()->size();
         cout << "   " << iter->first + "/" << endl;
      }
      else if (iter->second->get_type() == file_type::PLAIN_TYPE)
      {
         cout << "   " << iter->second->get_inode_number();
         cout << "   " << iter->second->get_contents()->size();
         cout << "   " << iter->first << endl;
      }
   }
}

map<string, inode_ptr> directory::get_dirents()
{
   return this->dirents;
}

void directory::add_dirents(string key, inode_ptr value)
{
   this->dirents[key] = value;
}

bool directory::H_directory_exists(inode_state &state)
{
   // check if directory already exists
   // or if the parent to the directory does not exist!
   auto it = state.get_cwd()->get_contents()->get_dirents().begin();
   auto end = state.get_cwd()->get_contents()->get_dirents().end();

   for (; it != end; ++it)
   {
      if (it->first == "")
      {
         if (it->second->get_type() == file_type::DIRECTORY_TYPE)
         {
            // throw an error because we found
            // the item name and it is a directory!
            return true;
         }
      }
   }

   // there is no directory called "/"
   // in the current directory so lets make it
   return false;
}

wordvec directory::get_directory_content()
{
   inode_ptr ptr;
   string str;
   wordvec directory_cont;

   for (auto iter = this->dirents.begin();
        iter != this->dirents.end(); ++iter)
   {
      str = std::to_string(iter->second->get_inode_nr());
      directory_cont.push_back(str);

      str = std::to_string(iter->second->get_size());
      directory_cont.push_back(str);

      if (iter->first == "." || iter->first == "..")
      {
         directory_cont.push_back(iter->first);
      }
      else
      {
         if (iter->second->get_type() == file_type::PLAIN_TYPE)
         {
            directory_cont.push_back(iter->first);
         }
         else if (iter->second->get_type() == file_type::DIRECTORY_TYPE)
         {
            directory_cont.push_back(iter->first + "/");
         }
      }
   }

   return directory_cont;
}
