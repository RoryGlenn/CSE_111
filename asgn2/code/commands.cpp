// $Id: commands.cpp,v 1.20 2021-01-11 15:52:17-08 - - $

// to turn on debug flags and print flags associated with i
// yshell -@i

// how the graders use files
//    cd into the dot.score folder and run these commands
//       mk.build
//       mk.tests

// submit cse111-wm.w21 asgn file1 file2 file3

// For example:
//         submit cse111-wm.w21 asg2 *.cpp *.h README Makefile

#include "commands.h"
#include "debug.h"

#include <algorithm>
#include <sstream>
#include <cstring>


command_hash cmd_hash
{
   {"cat", fn_cat},
   {"cd", fn_cd},
   {"echo", fn_echo},
   {"exit", fn_exit},
   {"ls", fn_ls},
   {"lsr", fn_lsr},
   {"make", fn_make},
   {"mkdir", fn_mkdir},
   {"prompt", fn_prompt},
   {"pwd", fn_pwd},
   {"rm", fn_rm},
   {"rmr", fn_rmr},
   {"#", fn_hash},
};


void fn_hash (inode_state& , const wordvec&)
{

}


// for fn_exit()
bool is_number(string word)
{
   for (auto c : word)
   {
      if (!isdigit(c))
      {
         return false;
      }
   }
   return true;
}

// for fn_exit()
int word_to_number(string word)
{
   int result = 0;
   for (auto c : word)
   {
      result = result*10 + c - '0';
   }
   return result % 128;
}



// slice the string according to where the "/" is
// and store it in directories vector
wordvec string_to_wordvec(string chop_me)
{
   string tmp_str;
   stringstream ss(chop_me);
   wordvec directories;

   while ( getline(ss, tmp_str, '/') )
   {
      if (tmp_str == "")
      {
         // return an empty vector on error
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


void print_file(string &pathname)
{
   wordvec wVec = string_to_wordvec(pathname);

   if (wVec.empty())
   {
      throw command_error( pathname + 
            ": No such file or directory" );
      return;
   }
   else
   {
      cout << wVec.back() << endl;
   }
}


void print_directory(inode_state &state, inode_ptr &cwd)
{
   wordvec content = state.get_contents(cwd)->get_directory_content();
   int size = static_cast<int>(content.size())-2;
   int step_size = 3;
   wordvec path;
   inode_ptr temp = cwd;
   inode_ptr ptr = nullptr;
   string pathname = "";
   string parent = "..";

   if (cwd != state.get_root())
   {
      while (temp != state.get_root())
      {
         path.push_back(temp->get_name());

         auto dirents = state.get_contents(temp)->get_dirents();
         auto iter = dirents.find(parent);

         if (iter != dirents.end())
         {
            ptr = iter->second;
         }

         temp = ptr;
      }

      for (auto itor = path.crbegin(); 
            itor != path.crend(); ++itor)
      {
         pathname += "/" + *itor;
      }
   }
   else
   {
      pathname = "/";
   }


   if (pathname == "/")
   {
      cout << pathname << ":" << endl;
   }
   else
   {
      cout << pathname << endl;
   }
   

   // print directory contents
   for (int i = 0; i < size; i += step_size)
   {
      // should i put this in a switch case?
      if ( content[i] == "." || content[i] == ".." )
      {
         cout << "   " << content[i] << "/";
      }
      else
      {
         cout << "   " << content[i];
      }

      if (content[i+1] == "." || content[i+1] == ".."  )
      {
         cout << "   " << content[i+1] << "/";
      }
      else
      {
         cout << "   " << content[i+1];
      }

      if (content[i+2] == "." || content[i+2] == ".." )
      {
         cout << "   " << content[i+2] << "/";
      }
      else
      {
         cout << "   " << content[i+2];
      }

      cout << "\n";

   }
}


command_fn find_command_fn(const string &cmd)
{
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)

   if (cmd.size() >= 1)
   {
      if (cmd[0] != '#')
      {
         DEBUGF('c', "[" << cmd << "]");
         const auto result = cmd_hash.find(cmd);

         if (result == cmd_hash.end())
         {
            throw command_error(cmd + ": no such function");
         }

         return result->second;
      }
   }

   const auto result = cmd_hash.find("#");
   return result->second;

}

command_error::command_error(const string &what) : runtime_error(what)
{
}


int exit_status_message()
{
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}



void fn_cat(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   int size = static_cast<int>(words.size());

   if (words.size() == 1)
   {
      throw command_error("cat: missing operand");
   }
   else if (words.size() > 1)
   {
      for (int i = 1; i < size; i++)
      {
         string tmp;
         stringstream ss(words[i]);
         wordvec directories;

         // if we are given a filepath with a "/" in it
         // chop up the string and put it in directories
         directories = string_to_wordvec(words[i]);

         if (directories.empty())
         {
            throw command_error( words[1] + 
                  ": No such file or directory" );
            return;            
         }

         for (string path : directories)
         {
            inode_ptr file_ptr = state.get_cwd();
            auto dirents = state.get_contents(file_ptr)->get_dirents();
            auto iter = dirents.find(path);

            if (iter != dirents.end())
            {
               // gets the pointer to the file
               file_ptr = dirents.find(path)->second;
            }

            if (file_ptr->get_type() == file_type::DIRECTORY_TYPE)
            {
               throw command_error("cat: " + words[1] + 
                              ": No such file or directory");
            }
            else if (file_ptr->get_type() == file_type::PLAIN_TYPE)
            {
               state.get_contents(file_ptr)->readfile();
            }
         }
      }
   }
}


void fn_cd(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   if (words.size() == 1 && words[0] == "cd")
   {
      // if only "cd" was given
      // go back to the root directory
      state.set_cwd(state.get_root());
   }
   else if (words.size() > 1)
   {
      string error;
      string token;
      string delimiter = "/";
      string s = words[1];
      wordvec directories;
      bool is_directory_set = false;
      bool is_error = false;

      directories = string_to_wordvec(words[1]);

      if ( directories.empty())
      {
         throw command_error( words[1] + 
               ": No such file or directory" );         
         return;
      }

      // check to see if the item in directories are in the cwd
      for (int i = 0; i < static_cast<int>(directories.size()); i++)
      {
         auto it = state.get_cwd()->get_contents()->
                     get_dirents().begin(); 
         auto end = state.get_cwd()->get_contents()->
                     get_dirents().end();
         for (; it != end; ++it)
         {
            if (it->first == directories[i])
            {
               if (it->second->get_type() == file_type::DIRECTORY_TYPE)
               {
                  state.set_cwd(it->second);
                  is_directory_set = true;
               }
               else
               {
                  // found the item but it is not a directory
                  is_error = true;
                  is_directory_set = false;
                  error = directories[i];
                  throw command_error( "cd: " + directories[i] + 
                        ": No such file or directory");
                  return;
               }

               break;
            }
         }

         if (is_error)
         {
            break;
         }
      }

      // not sure if I need this
      if (!is_directory_set)
      {
         // folder that was requested is not a directory
         throw command_error("cat: " + words[1] + 
                        ": No such file or directory");
      }
   }
}

void fn_echo(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);
   cout << word_range(words.cbegin() + 1, words.cend()) << endl;
}




void fn_exit(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   if (words.size() == 1)
   {
      exec::status(0);
   }

   else if (words.size() == 2 && is_number(words[1]))
   {
      exec::status (word_to_number(words[1]));
   }
   else
   {
      exec::status(127);
   }
   throw ysh_exit();
}


void fn_ls(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   inode_ptr cwd = state.get_cwd();
   string dot = ".";
   string dotdot = "..";
   wordvec goto_parent;
   wordvec goto_root;
   goto_root.push_back(dot);
   goto_parent.push_back(dotdot);

   if (words.size() == 1)
   {
      // only "ls" was given
      cout << cwd->get_path() + ":\n";
      cwd->get_contents()->print_dirents();
   }
   else if (words.size() > 1)
   {
      if (words[1] == "/")
      {
         // this is really my lazy way of doing things
         // instead of cd'ing into the directory
         // search through the map to get the inode_ptr
         // associated with the directory
         fn_cd(state, goto_root);
         cout << "/:\n";
         state.get_cwd()->get_contents()->print_dirents();
         state.set_cwd(cwd);
      }
      else if (words[1] == dot)
      {
         cout << dot + ":\n";
         cwd->get_contents()->print_dirents();
      }
      else if (words[1] == dotdot)
      {
         fn_cd(state, goto_parent);
         cout << dotdot + ":\n";
         state.get_cwd()->get_contents()->print_dirents();
         state.set_cwd(cwd);
      }
      else
      {
         cwd->print_path();
         cout << ":\n";
         cwd->get_contents()->print_dirents();
      }
   }
}

vector<inode_ptr> lsr_recursion
   (inode_state &state, inode_ptr &ptr, vector<inode_ptr> &directories)
{
   auto dirents = state.get_contents(ptr)->get_dirents();

   for (auto iter = dirents.begin(); iter != dirents.end(); ++iter)
   {
      if ( iter->second->get_type() == file_type::DIRECTORY_TYPE )
      {
         if (iter->first != "." && iter->first != "..")
         {
            auto find_result = 
            find(directories.begin(), directories.end(), iter->second);

            if (find_result == directories.end())
            {
               directories.push_back(iter->second);

               // continue to get the rest of the subdirectories
               lsr_recursion(state, iter->second, directories);
            }
         }
      }
   }

   return directories;
}

void fn_lsr(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   vector<inode_ptr> directories;
   inode_ptr         save_ptr     = state.get_cwd();
   string            cwd_name     = state.get_cwd()->get_name();
   inode_ptr         cwd          = state.get_cwd();

   directories.push_back(cwd);

   // get all of the subdirectories in the cwd
   if (words.size() == 1)
   {
      // if only "lsr" was given
      directories = lsr_recursion(state, state.get_cwd(), directories);
   }
   else if (words.size() >= 2)
   {
      string tmp;
      stringstream ss(words[1]);
      wordvec word_directories;

      // if we are given a filepath with a "/" in it
      // chop up the string and put it in directories
      if (words[1] != "/")
      {
         while (getline(ss, tmp, '/'))
         {
            word_directories.push_back(tmp);
         }

         for (auto name : word_directories)
         {
            auto dirents = state.get_contents(cwd)->get_dirents();
            auto iter = dirents.find(name);

            if (iter != dirents.end())
            {
               // get the ptr associatd with the
               // file/directory name
               cwd = iter->second;
            }
            else
            {
               throw command_error("lsr: " + name 
                  + ": No such file or directory");
               break;
            }

            if (cwd->get_type() == file_type::PLAIN_TYPE)
            {
               print_file(name);
            }
            else if (cwd->get_type() == file_type::DIRECTORY_TYPE)
            {
               // continue to get the rest of the subdirectories
               directories = lsr_recursion(state, cwd, directories);
            }

         }
      }
      else if (words[1] == "/")
      {
         // handle the root
         inode_ptr root = state.get_root();
         directories.clear();
         directories.push_back(root);
         directories = lsr_recursion(state, root, directories);
      }
   }

   for (int i = 0; i < static_cast<int>(directories.size()); i++)
   {
      print_directory(state, directories[i]);
   }

   // restores the cwd in case things got messed up in the process
   state.set_cwd(save_ptr);
}

void fn_make(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   auto cwd = state.get_cwd();

   if (words.size() == 1)
   {
      throw command_error("make: missing operand");
      return;
   }
   else if (words.size() >= 2)
   {
      wordvec path_vec = string_to_wordvec(words[1]);

      if (path_vec.empty())
      {
         throw command_error("make: " + words[1] + 
                              ": invalid file path");
      }

      inode_ptr ptr = state.path_to_inode_ptr(path_vec);
 
      // the file or directory does not exist
      if (ptr->get_error() == "404")
      {
         // invalid file path
         // no such directory exists
         throw command_error("make: " + words[1] +
                     ": No such file or directory" );
         return;
      }

      //  if the ptr already exists && 
      //  is of type directory && 
      //  is the back of path_vec
      if ( ptr->get_error() == "" && 
            ptr->get_type() == file_type::DIRECTORY_TYPE && 
            ptr->get_name() == path_vec.back() )
      {
         throw command_error("make: " + words[1] +
                     ": is already a directory" );
         return;
      }

      // create file or directory
      inode_ptr new_ptr = ptr->get_contents()->mkfile(path_vec.back());
      new_ptr->set_parent(ptr);


      if (words.size() > 2)
      {
         new_ptr->get_contents()->writefile(words);
      }

   }

}


void fn_mkdir(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   string token;
   string filepath;
   wordvec directories;
   auto cwd = state.get_cwd();
   auto tcwd = state.get_cwd();

   if (words.size() == 1)
   {
      throw command_error("mkdir: missing operand");
      return;
   }

   // get the full filepath
   else if (words.size() > 1)
   {
      directories = string_to_wordvec(words[1]);

      if (directories.empty())
      {
         throw command_error( words[1] + 
               ": No such file or directory" );  
         return;       
      }
   }

   // if we are given 1 directory to create
   //    if directory does not exist
   //       create directory
   if (directories.size() == 1)
   {
      if (state.H_directory_exists(state, directories[0]))
      {
         throw command_error("mkdir: cannot create directory '"
            + words[1] + "': File exists");
      }
      else
      {
         state.get_cwd()->get_contents()->mkdir(directories[0], state);
      }

   }

   else if (directories.size() > 1)
   {
      for (int i = 0; i < static_cast<int>(directories.size()); i++)
      {
         // all the directories in the file path 
         // need to exist except the very last one
         if (i != static_cast<int>(directories.size()-1))
         {
            // this one NEEDS to exist
            if (state.H_directory_exists(state, directories[i]))
            {
               // cd into the directory
               wordvec directory;
               directory.push_back("cd");
               directory.push_back(directories[i] + "/");
               fn_cd(state, directory);
            }
            else
            {
               throw command_error("mkdir:  cannot create directory '"
                  + words[1] + "': No such file or directory");
               break;
            }
         }
         else if (i == static_cast<int>(directories.size())-1)
         {
            if (state.H_directory_exists(state, directories[i]))
            {
               // this one needs to NOT exist
               throw command_error("mkdir: cannot create directory '"
                  + words[1] + "': File exists");
               break;
            }
            else
            {
               // last directory in the filepath 
               // doesn't exist so let make it
               // eg. test1/test2/test3, we want to make test3
               state.get_cwd()->get_contents()->
                  mkdir(directories[i], state);
            }
         }
      }
   }

   state.set_cwd(cwd);
}


void fn_prompt(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   string prompt = "\0";

   if (words.size() == 1)
   {
      throw command_error("prompt: missing operand");
   }
   else if (words.size() > 1)
   {
      for (auto word : words)
      {
         if (word != "prompt")
         {
            prompt += word + " ";
         }
      }

      state.set_prompt(prompt);
   }
}


void fn_pwd(inode_state &state, const wordvec &words)
{

   if (!words.empty())
   {
      // silences the warning
   }

   DEBUGF('c', state);
   DEBUGF('c', words);
   auto cwd = state.get_cwd();
   cwd->print_path();
   printf("\n");
}


void fn_rm(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   wordvec directories;
   string tmp;
   auto cwd = state.get_cwd();

   if (words.size() == 1)
   {
      throw command_error("rm: missing operand");
   }
   else if (words.size() > 1)
   {
      directories = string_to_wordvec(words[1]);

      if (directories.empty())
      {
         throw command_error( words[1] + 
               ": No such file or directory" );  
         return;
      }

      inode_ptr ptr = state.path_to_inode_ptr(directories);

      // the file or directory does not exist
      if (ptr->get_error() == "404")
      {
         // invalid file path
         // no such directory exists
         throw command_error("make: " + words[1] +
               ": No such file or directory" );
         return;
      }

      if (ptr->get_type() == file_type::DIRECTORY_TYPE)
      {
         // throw an exception and return if the directory has contents
         if ( ptr->get_contents()->get_dirents().size() > 2 )
         {
            throw command_error("fm: " + words[1] +
                  ": cannot remove directory with contents" );
            return;
         }
      }

      // delete file/directory here
      inode_ptr parent_ptr = ptr->get_parent();
      parent_ptr->get_contents()->remove(ptr->get_name());
   }
}

// jk, theres no recursion
void rmr_recursion(inode_state &state, wordvec words)
{
   wordvec directories = string_to_wordvec(words[1]);
  
   if (words.size() == 2 && words[1] == "/")
   {
      throw command_error( "rmr: cannot remove root" );
      return;
   }

   if (directories.empty())
   {
      throw command_error( words[1] + 
            ": No such file or directory" );
      return;
   }
  
   inode_ptr ptr = state.path_to_inode_ptr(directories);

   // the file or directory does not exist
   if (ptr->get_error() == "404")
   {
      // invalid file path
      // no such directory exists
      throw command_error("rmr: " + words[1] +
                     ": No such file or directory" );
      return;
   }

   // delete file/directory here
   inode_ptr parent_ptr = ptr->get_parent();
   parent_ptr->get_contents()->remove(ptr->get_name());

}


// rmr dir
// deletes every subdirectory of dir, and lastly dir in its parent.
// also all files in dir.
// pseudocode:
// depth(t):
//     loop over all children of t.
//     process t itself
// Just a standard depth first postorder.
void fn_rmr(inode_state &state, const wordvec &words)
{
   DEBUGF('c', state);
   DEBUGF('c', words);

   if (words.size() == 1)
   {
      throw command_error("rmr: No file or directory specified");
   }
   else if (words.size() > 1)
   {
      rmr_recursion(state, words);
   }
}
