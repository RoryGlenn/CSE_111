// $Id: main.cpp,v 1.13 2021-02-01 18:58:18-08 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>

////////////////////
#include <cassert>
#include <cerrno>
#include <fstream>
#include <iomanip>
#include <regex>
#include <stdexcept>
#include <typeinfo>
////////////////////

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using namespace std;

void scan_options(int argc, char **argv);
void match_lines();

using str_str_map = listmap<string, string>;
using str_str_pair = str_str_map::value_type;
str_str_map global_map;


void four_way_test(string key, string value)
{
   if ( !key.empty() && !value.empty() )
   {
      // key = value
      
      str_str_pair pair = str_str_pair(key, value);
      str_str_map::iterator iter = global_map.insert(pair);
      cout << iter->first << " " << "= " << iter->second << endl;
   }
   else if ( !key.empty() && value.empty() )
   {
      // key = 
      
      str_str_map::iterator iter = global_map.find(key);
      if (iter != global_map.end())
      {
         // erase the key!
         iter = global_map.erase(iter);
      }
      return;
   }
   else if ( key.empty() && !value.empty() )
   {
      // = value
      // All of the key and value pairs with the given value
      // are printed in lexicographic order sorted by key
      auto iter = global_map.begin();
      for (; iter != global_map.end(); ++iter)
      {
         if ( iter->second == value )
         {
            cout << iter->first << " = " << iter->second << endl;
         }
      }   
   }
   else if ( key.empty() && value.empty() )
   {
      // = 

      // all of the key and value pairs in
      // the map are printed in lexicographic order
      auto iter = global_map.begin();
      for (; iter != global_map.end(); ++iter)
      {
         cout << iter->first << " = " << iter->second << endl;
      }
   } 
   else
   {
      // I dont think this can ever happen
   }

}


void scan_options(int argc, char **argv)
{
   opterr = 0;
   for (;;)
   {
      int option = getopt(argc, argv, "@:");
      
      if (option == EOF)
      {
         break;
      }
         
      switch (option)
      {
         case '@':
               debugflags::setflags(optarg);
               break;
         default:
               complain() << "-" << char(optopt) 
                     << ": invalid option" << endl;
               break;
      }
   }
}




void match_lines()
{
   int linenumber = 1;
   regex comment_regex   {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex   {R"(^\s*([^=]+?)\s*$)"};

   while( !cin.eof() )
   {
      string line;
      getline(cin, line);

      cout << "-: " << linenumber << ": " << line;

      if ( !cin.eof() )
      {
         cout << endl;
      }

      smatch result;

      if (regex_search(line, result, comment_regex))
      {
         // Comment or empty line
         linenumber++;
         continue;
      }
      
      if (regex_search(line, result, key_value_regex))
      {
         four_way_test (result[1],  result[2]);
      }
      else if (regex_search(line, result, trimmed_regex))
      {
         // retrieve the value of the key if they key exists
         auto iter = global_map.find(result[1]);

         if (iter != global_map.end())
         {
            // print the key and value
            cout << iter->first << " = " << iter->second << endl;
         }
         else
         {
            cout << result[1] << ": key not found" << endl;
         }
      }
      else
      {
         assert(false and "This can not happen.");
      }

      linenumber++;

   }

}


int main(int argc, char **argv)
{

   regex comment_regex   {R"(^\s*(#.*)?$)"};
   regex key_value_regex {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex trimmed_regex   {R"(^\s*([^=]+?)\s*$)"};
   bool error = false;

   scan_options(argc, argv);

   if (argc > 1) 
   {
      // loop through command line args
      for (int i = 1; i < argc; i++)
      {
         ifstream infile;
         
         int file_line_count = 1;
         string arg = argv[i];

         // if we want to read from command line, 
         // ./keyvalue file1 - file2
         if (arg == "-")
         {
            int linenumber = 1;

            while( !cin.eof() )
            {
               string line;
               getline(cin, line);

               // hacky fix to print correctly
               if (linenumber == 1)
               {
                  cout << arg << ": " << linenumber << ": " << line;
               }


               if ( !cin.eof() )
               {
                  cout << endl;
               }

               smatch result;

               if (regex_search(line, result, comment_regex))
               {
                  // Comment or empty line
                  linenumber++;
                  continue;
               }
               
               if (regex_search(line, result, key_value_regex))
               {
                  four_way_test (result[1],  result[2]);
               }
               else if (regex_search(line, result, trimmed_regex))
               {
                  // retrieve the value of the key if they key exists
                  auto iter = global_map.find(result[1]);

                  if (iter != global_map.end())
                  {
                     // print the key and value
                     cout << iter->first << " = " 
                          << iter->second << endl;
                  }
                  else
                  {
                     cout << result[1] << ": key not found" << endl;
                  }
               }
               else
               {
                  assert(false and "This can not happen.");
               }

               linenumber++;
            }

            continue;
         }

         infile.open(argv[i]);

         if (infile.good())
         {
            while( !infile.eof() )
            {
               string line;
               getline(infile, line);

               cout << argv[i] << ": " << file_line_count
                    << ": " << line;

               if ( !infile.eof() )
               {
                  cout << endl;
               }

               smatch result;

               if (regex_search(line, result, comment_regex))
               {
                  // Comment or empty line
                  file_line_count++;
                  continue;
               }
               
               if (regex_search(line, result, key_value_regex))
               {
                  four_way_test (result[1],  result[2]);
               }
               else if (regex_search(line, result, trimmed_regex))
               {
                  // retrieve the value of the key if they key exists
                  auto iter = global_map.find(result[1]);

                  if (iter != global_map.end())
                  {
                     // print the key and value
                     cout << iter->first  << " = "
                          << iter->second << endl;
                  }
                  else
                  {
                     cout << result[1] << ": key not found" << endl;
                  }
               }
               else
               {
                  assert(false and "This can not happen.");
               }
            
               file_line_count++;
            }

            infile.close();
         }
         else
         {
            // if we we can't open or find the file.
            cout << argv[0] << ": " << argv[i] 
                 << ": No such file or directory";
            error = true;

            // only print a new line if we are not 
            // at the last iteration of the loop
            if (i != argc-1 )
            {
               cout << endl;
            }
         }
         
         // print a newline between files
         // but not for the last file.
         if ( i != argc-1 && !error)
         {
            cout << endl;
         }

         error = false;

      }

   }
   else 
   {
      // if no files were given on the command line
      match_lines();
   }

}
