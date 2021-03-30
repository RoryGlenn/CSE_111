// $Id: file_sys.h,v 1.8 2020-10-22 14:37:26-07 - - $

#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
#include "util.h"

using namespace std;

// inode_t -
//    An inode is either a directory or a plain file.

enum class file_type
{
   PLAIN_TYPE,
   DIRECTORY_TYPE
};

class inode;
class base_file;
class plain_file;
class directory;
using inode_ptr = shared_ptr<inode>;         
using base_file_ptr = shared_ptr<base_file>; 
ostream &operator<<(ostream &, file_type);   

//
// inode_state -
//    A small convenient class to maintain the state of the simulated
//    process:  the root (/), the current directory (.), and the
//    prompt.

// this is a singleton class, only one can exist
class inode_state
{
   friend class inode;
   friend ostream &operator<<(ostream &out, const inode_state &);

private:
   inode_ptr root{nullptr}; // root = root directory
   inode_ptr cwd{nullptr};  // cwd = current working directory
   string prompt_{"% "};
   string error = "";

public:
   inode_state(const inode_state &) = delete; // copy ctor
   inode_state();
   inode_state &operator=(const inode_state &) = delete; // op=
   const string &prompt() const;
   void prompt(const string &);

   // get functions
   inode_ptr &get_root() { return this->root; }
   inode_ptr &get_cwd() { return this->cwd; }
   string get_prompt() { return this->prompt_; }
   string get_path(inode_ptr _ptr);
   virtual string get_error() { return this->error; }
   virtual base_file_ptr get_contents(inode_ptr _content);
   virtual inode_ptr get_item_ptr(string &name, inode_ptr &_cwd);

   // set functions
   void set_root(inode_ptr _root) { this->root = _root; }
   void set_cwd(inode_ptr _cwd) { this->cwd = _cwd; }
   void set_prompt(string prompt) { this->prompt_ = prompt; }
   void set_error(string _error) { this->error = _error; }

   // helper functions
   virtual bool H_directory_exists(inode_state &state, string dirname);
   virtual bool H_plain_file_exists(inode_state &state, string filename);
   virtual bool H_does_exist(inode_state &state, string name);
   inode_ptr    path_to_inode_ptr(wordvec &_pathvec);
   wordvec      string_to_wordvec(string chop_me);
};

// class inode -
// inode ctor -
//    Create a new inode of the given type.
// get_inode_nr -
//    Retrieves the serial number of the inode.  Inode numbers are
//    allocated in sequence by small integer.
// size -
//    Returns the size of an inode.  For a directory, this is the
//    number of dirents.  For a text file, the number of characters
//    when printed (the sum of the lengths of each word, plus the
//    number of words.
//

class inode
{
   friend class inode_state;

private:
   static size_t next_inode_nr;
   size_t inode_nr;
   base_file_ptr contents;
   file_type type;
   inode_ptr parent;
   string path;
   string name;
   string data; 
   string error;

public:
   inode(file_type);

   // get functions
   size_t        get_inode_nr() const;
   size_t        get_next_inode_nr() { return this->next_inode_nr; }
   size_t        get_inode_number() { return this->inode_nr; }
   inode_ptr     get_parent() { return this->parent; }
   base_file_ptr get_contents() { return this->contents; }
   string       &get_path() { return this->path; }
   string        get_name() { return this->name; }
   string        get_data() { return this->data; }
   file_type     get_type() { return this->type; }
   inode_ptr    &get_cwd(inode_state &state) { return state.get_cwd(); }
   size_t        get_size();
   string        get_error() { return this->error; }

   // set functions
   void set_path(string _path)        { this->path   = _path;   }
   void set_parent(inode_ptr _parent) { this->parent = _parent; }
   void set_name(string _name)        { this->name   = _name;   }
   void set_data(string _data)        { this->data   = _data;   }
   void set_error(string _error)      { this->error  = _error;  }
   
   // printer
   void print_path() { cout << this->path; }
};

//
// class base_file -
// Just a base class at which an inode can point.  No data or
// functions.  Makes the synthesized members useable only from
// the derived classes.

class file_error : public runtime_error
{
public:
   explicit file_error(const string &what);
};

class base_file
{
protected:
   base_file() = default;
   virtual const string &error_file_type() const = 0;

public:
   virtual ~base_file() = default;
   base_file(const base_file &) = delete;
   base_file &operator=(const base_file &) = delete;
   virtual const wordvec &readfile() const;
   virtual void readfile(string &);

   virtual void writefile(const wordvec &);
   virtual void remove(const string &);
   virtual inode_ptr mkdir(const string &);
   virtual inode_ptr mkfile(string);

   virtual void print_dirents();
   virtual map<string, inode_ptr> get_dirents();
   virtual void add_dirents(string, inode_ptr);

   virtual void mkdir(const string &, inode_state &);
   virtual void remove(inode_state &, const string &);
   virtual void remove_recursion(inode_ptr &);
   virtual size_t size();
   virtual wordvec get_file_data();

   // Helper functions
   virtual bool H_directory_exists();
   virtual string get_path();
   virtual wordvec get_directory_content();
};

//
// class plain_file -
// Used to hold data.
// synthesized default ctor -
//    Default vector<string> is a an empty vector.
// readfile -
//    Returns a copy of the contents of the wordvec in the file.
// writefile -
//    Replaces the contents of a file with new contents.

class plain_file : public base_file
{
private:
   virtual const string &error_file_type() const override
   {
      static const string result = "plain file";
      return result;
   }

   wordvec data;

public:
   virtual const wordvec &readfile() const override;
   virtual void writefile(const wordvec &newdata) override;
   virtual inode_ptr mkfile(string) override;

   virtual void readfile(string &file) override;
   virtual size_t size() override;
   virtual wordvec get_file_data() override;
   virtual wordvec get_directory_content() override;
   virtual void remove_recursion(inode_ptr &ptr) override;
};

// class directory -
// Used to map filenames onto inode pointers.
// default ctor -
//    Creates a new map with keys "." and "..".
// remove -
//    Removes the file or subdirectory from the current inode.
//    Throws an file_error if this is not a directory, the file
//    does not exist, or the subdirectory is not empty.
//    Here empty means the only entries are dot (.) and dotdot (..).
// mkdir -
//    Creates a new directory under the current directory and
//    immediately adds the directories dot (.) and dotdot (..) to it.
//    Note that the parent (..) of / is / itself.  It is an error
//    if the entry already exists.
// mkfile -
//    Create a new empty text file with the given name.  Error if
//    a dirent with that name exists.

class directory : public base_file
{
private:
   // Must be a map, not unordered_map, so printing is lexicographic
   map<string, inode_ptr> dirents;

   virtual const string &error_file_type() const override
   {
      static const string result = "directory";
      return result;
   }

   string error = "";

   // this is just a test
   wordvec data;

public:
   virtual size_t size() override;
   virtual void remove(const string &filename) override;
   virtual inode_ptr mkfile(string filename) override;

   virtual void readfile(string file) const;
   virtual const wordvec &readfile() const override;
   virtual map<string, inode_ptr> get_dirents() override;
   virtual void add_dirents(string key, inode_ptr value) override;
   virtual void print_dirents() override;
   virtual void mkdir(const string &dirname, inode_state &state) override;
   virtual void set_error(string _error) { this->error = _error; }

   // Helper functions
   virtual bool H_directory_exists(inode_state &state);

   virtual string get_error() { return this->error; }
   virtual vector<string> get_directory_content() override;
   virtual void remove_recursion(inode_ptr &ptr) override;
};

#endif
